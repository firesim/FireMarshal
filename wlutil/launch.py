import socket
import logging
import os
import subprocess as sp
from . import wlutil

jobProcs = []
vdeProc = None


# Terminates jobs unless they have stopped running already
def cleanUpSubProcesses():
    log = logging.getLogger()
    for proc in jobProcs:
        if proc.poll() is None:
            log.info(f'cleaning up launched workload process {proc.pid}')
            proc.terminate()
    cleanUpVDE()


# Register clean up function with wlutil.py so it can be called by SIGINT handler
wlutil.registerCleanUp(cleanUpSubProcesses)


# Start vde_plug server
def startVDE():
    log = logging.getLogger()
    global vdeProc
    log.info('Starting VDE')
    vdeProc = sp.Popen(["vde_plug", "vxvde://", "cmd://slirpvde - --host=172.16.0.1/16 --dns=172.16.1.3"], stderr=sp.STDOUT)


# Terminate vdeProc unless it has stopped running already
def cleanUpVDE():
    log = logging.getLogger()
    if vdeProc and vdeProc.poll() is None:
        log.info(f'cleaning up VDE process {vdeProc.pid}')
        vdeProc.terminate()


# Kinda hacky (technically not guaranteed to give a free port, just very likely)
def get_free_tcp_port():
    tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp.bind(('', 0))
    addr, port = tcp.getsockname()
    tcp.close()
    return str(port)


# Returns a command string to launch the given config in spike. Must be called with shell=True.
def getSpikeCmd(config, nodisk=False):
    log = logging.getLogger()

    if 'img' in config and config['img-hardcoded']:
        log.warn("You have hard-coded a disk image in your workload. Spike does not support disk images, your workload may not work correctly. Consider building with the '--nodisk' option (for linux-based workloads).")
    elif 'img' in config and not nodisk:
        raise ValueError("Spike does not support disk-based configurations")

    if 'spike' in config:
        spikeBin = str(config['spike'])
    else:
        spikeBin = 'spike'

    cmd = [spikeBin,
           config.get('spike-args', ''),
           ' -p' + str(config['cpus']),
           ' -m' + str(int(config['mem'] / (1024*1024)))]

    if nodisk:
        cmd.append(str(wlutil.noDiskPath(config['bin'])))
    else:
        cmd.append(str(config['bin']))

    return " ".join(cmd)


# Returns a command string to luanch the given config in qemu. Must be called with shell=True.
def getQemuCmd(config, count=1, nodisk=False):

    if nodisk:
        exe = str(wlutil.noDiskPath(config['bin']))
    else:
        exe = str(config['bin'])

    if 'qemu' in config:
        qemuBin = str(config['qemu'])
    else:
        qemuBin = 'qemu-system-riscv64'

    # We should start static IP addresses at 172.16.0.2 => 4 = 1 (min val of count) + 1
    count += 1
    machigh = '00'
    if count < 16:
        maclow = '0' + hex(count)[2:]
    else:
        maclow = hex(count)[2:]

    cmd = [qemuBin,
           '-nographic',
           '-bios none',
           '-smp', str(config['cpus']),
           '-machine', 'virt',
           '-m', str(int(config['mem'] / (1024*1024))),
           '-kernel', exe,
           '-object', 'rng-random,filename=/dev/urandom,id=rng0',
           '-device', 'virtio-rng-device,rng=rng0',
           '-device', f'virtio-net-device,netdev=vde0,mac=00:12:6d:00:{machigh}:{maclow}',
           '-netdev', 'vde,id=vde0,sock=vxvde://']

    if 'img' in config and not nodisk:
        cmd = cmd + ['-device', 'virtio-blk-device,drive=hd0',
                     '-drive', 'file=' + str(config['img']) + ',format=raw,id=hd0']

    return " ".join(cmd) + " " + config.get('qemu-args', '')


def launchWorkload(baseConfig, jobs=None, spike=False, silent=False, captureOutput=True):
    """Launches the specified workload in functional simulation.

    cfgName: unique name of the workload in the cfgs
    cfgs: initialized configuration (contains all possible workloads)
    jobs: List of job names to launch. If jobs is None we use the parent of
    all the jobs (i.e.  the top-level workload in the config).
    spike: Use spike instead of the default qemu as the functional simulator
    silent: If false, the output from the simulator will be displayed to
        stdout. If true, only the uartlog will be written (it is written live and
        unbuffered so users can still 'tail' the output if they'd like).
    captureOutput: If true, the output for each workload will be captured in a uartlog file stored
        in a separate workload specific directory. If false, no output will be captured. (Useful for building workloads.)

    Returns: Path of output directory
    """
    log = logging.getLogger()

    if spike and baseConfig.get('spike', True) is None:
        raise RuntimeError("This workload does not support spike")

    if not spike and baseConfig.get('qemu', True) is None:
        raise RuntimeError("This workload does not support qemu")

    if not spike:
        startVDE()

    if jobs is None:
        configs = [baseConfig]
    else:
        configs = [baseConfig['jobs'][j] for j in jobs]

    baseResDir = wlutil.getOpt('res-dir') / wlutil.getOpt('run-name')

    screenIdentifiers = {}

    try:
        jobSlot = 0
        for config in configs:
            if config['launch']:

                jobSlot += 1

                if spike:
                    cmd = getSpikeCmd(config, config['nodisk'])
                else:
                    cmd = getQemuCmd(config, jobSlot, config['nodisk'])

                log.info(f"\nLaunching job {config['name']}")
                log.info(f'Running: {cmd}')

                if captureOutput:
                    runResDir = baseResDir / config['name']
                    uartLog = runResDir / "uartlog"
                    os.makedirs(runResDir)
                    scriptCmd = f'script -f -c "{cmd}" {uartLog}'
                else:
                    scriptCmd = cmd

                if silent and captureOutput:
                    log.info("For live output see: " + str(uartLog))

                if not silent and len(configs) == 1:
                    jobProcs.append(sp.Popen(["screen", "-S", config['name'], "-m", "bash", "-c", scriptCmd], stderr=sp.STDOUT))
                else:
                    jobProcs.append(sp.Popen(["screen", "-S", config['name'], "-D", "-m", "bash", "-c", scriptCmd], stderr=sp.STDOUT))

                screenIdentifiers[config['name']] = config['name']
                log.info('Opened screen session for {0} with identifier {1}'.format(config['name'], screenIdentifiers[config['name']]))

        log.info("\nList of screen session identifers:")
        for config in configs:
            if config['launch']:
                log.info(f"{config['name']}: {screenIdentifiers[config['name']]}")
        log.info("\n")

        for proc in jobProcs:
            proc.wait()
        cleanUpVDE()

    except Exception:
        cleanUpSubProcesses()
        raise

    finally:
        cleanUpVDE()

    for config in configs:
        if 'outputs' in config and captureOutput:
            runResDir = baseResDir / config['name']
            outputSpec = [wlutil.FileSpec(src=f, dst=runResDir) for f in config['outputs']]
            wlutil.copyImgFiles(config['img'], outputSpec, direction='out')

    if 'post_run_hook' in baseConfig and captureOutput:
        prhCmd = [baseConfig['post_run_hook'].path] + baseConfig['post_run_hook'].args + [baseResDir]
        log.info("Running post_run_hook script: " + ' '.join([str(x) for x in prhCmd]))
        try:
            wlutil.run(prhCmd, cwd=config['workdir'])
        except sp.CalledProcessError as e:
            log.info("\nRun output available in: " + str(baseResDir))
            raise RuntimeError("Post run hook failed:\n" + str(e.output))

    return baseResDir
