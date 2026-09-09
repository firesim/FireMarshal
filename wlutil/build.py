import doit
import shutil
import logging
import os
import subprocess as sp
import pathlib
import contextlib
from . import wlutil
from . import launch as wllaunch

taskLoader = None


# Print task target or file dep changes
# Taken from: https://github.com/pydoit/doit/issues/329
def print_deps(task, changed):
    log = logging.getLogger()
    for t in task.targets:
        if not os.path.exists(t):
            log.debug(f"Running task {task.name} because one of its targets does not exist anymore: {t}")
            return

    if changed:
        log.debug(f"Running task {task.name} because the following changed: {changed}")


class doitLoader(doit.cmd_base.TaskLoader2):
    workloads = []

    # Idempotent add (no duplicates)
    def addTask(self, tsk):
        if not any(t['name'] == tsk['name'] for t in self.workloads):
            # add dep. tracker to each task
            tsk['actions'] = [print_deps] + tsk['actions']
            self.workloads.append(tsk)

    def load_doit_config(self):
        return {**wlutil.getOpt('doitOpts'), **{'check_file_uptodate': wlutil.WithMetadataChecker}}

    def load_tasks(self, cmd, pos_args):
        task_list = [doit.task.dict_to_task(w) for w in self.workloads]
        return task_list


def buildBusybox(config):
    """Builds the local copy of busybox (needed by linux initramfs).

    This is called as a doit task (added to the graph in buildDepGraph())
    """

    try:
        wlutil.checkSubmodule(wlutil.getOpt('busybox-dir'))
    except wlutil.SubmoduleError as e:
        return doit.exceptions.TaskFailed(e)

    busyboxDir = wlutil.getOpt('busybox-dir')
    # taken from Buildroot 2026.08-rc3, which uses the same patch to fix https://lists.busybox.net/pipermail/busybox/2026-August/092422.html
    # https://gitlab.com/buildroot.org/buildroot/-/blob/2026.08-rc3/package/busybox/0006-tc-Fix-compilation-with-Linux-v6.8-rc1.patch
    busyboxPatch = wlutil.getOpt('wlutil-dir') / 'busybox-patches' / '0001-tc-fix-linux-6.8.patch'
    patchApplied = False

    try:
        patchCheck = wlutil.run(['git', 'apply', '--check', str(busyboxPatch)], cwd=busyboxDir, check=False)
        if patchCheck.returncode == 0:
            wlutil.run(['git', 'apply', str(busyboxPatch)], cwd=busyboxDir)
            patchApplied = True
        else:
            # Accept a patch left applied by an interrupted prior build, but fail
            # if the BusyBox sources are incompatible with the vendored patch.
            reverseCheck = wlutil.run(['git', 'apply', '--reverse', '--check', str(busyboxPatch)],
                                      cwd=busyboxDir, check=False)
            if reverseCheck.returncode != 0:
                wlutil.run(['git', 'apply', '--check', str(busyboxPatch)], cwd=busyboxDir)

        busyboxConfig = busyboxDir / '.config'
        shutil.copy(wlutil.getOpt('wlutil-dir') / 'busybox-config', busyboxConfig)

        # Use lp64 mabi to avoid rvv instructions due to autovectorization of glibc functions, see https://github.com/firesim/FireMarshal/pull/327
        # Vector-capable workloads/cores must rebuild BusyBox with their
        # workload-specific vector ISA and ABI settings.
        busyboxLines = busyboxConfig.read_text().splitlines()
        busyboxFlags = '-march=rv64gc -mabi=lp64'
        busyboxLdFlags = '-mabi=lp64'
        for index, line in enumerate(busyboxLines):
            if line == 'CONFIG_EXTRA_CFLAGS=""':
                busyboxLines[index] = 'CONFIG_EXTRA_CFLAGS="' + busyboxFlags + '"'
            elif line == 'CONFIG_EXTRA_LDFLAGS=""':
                busyboxLines[index] = 'CONFIG_EXTRA_LDFLAGS="' + busyboxLdFlags + '"'
        busyboxConfig.write_text('\n'.join(busyboxLines) + '\n')
        wlutil.run(['make', '-j' + str(wlutil.getOpt('jlevel'))], cwd=busyboxDir)
    finally:
        if patchApplied:
            wlutil.run(['git', 'apply', '--reverse', str(busyboxPatch)], cwd=busyboxDir)

    shutil.copy(wlutil.getOpt('busybox-dir') / 'busybox', wlutil.getOpt('initramfs-dir') / 'disk' / 'bin/')
    shutil.copy(wlutil.getOpt('busybox-dir') / 'busybox', wlutil.getOpt('initramfs-dir') / 'nodisk' / 'bin/')
    return True


def handleHostInit(config):
    log = logging.getLogger()
    if 'host-init' in config:
        log.debug("Applying host-init: " + str(config['host-init']))
        if not config['host-init'].path.exists():
            raise ValueError("host-init script " + str(config['host-init']) + " not found.")

        wlutil.run([config['host-init'].path] + config['host-init'].args, cwd=config['workdir'])


def handlePostBin(config, linuxBin):
    log = logging.getLogger()
    if 'post-bin' in config:
        log.debug("Applying post-bin: " + str(config['post-bin']))
        if not config['post-bin'].path.exists():
            raise ValueError("post-bin script " + str(config['post-bin']) + " not found.")

        # add linux src and bin path to the environment
        postbinEnv = os.environ.copy()
        if 'linux' in config:
            postbinEnv.update({'FIREMARSHAL_LINUX_SRC': config['linux']['source'].as_posix()})
            postbinEnv.update({'FIREMARSHAL_LINUX_BIN': linuxBin})

        wlutil.run([config['post-bin'].path] + config['post-bin'].args, env=postbinEnv, cwd=config['workdir'])


def submoduleDepsTask(submodules, name=""):
    """Returns a calc_dep task for doit to check if submodule is up to date.
    Packaging this in a calc_dep task avoids unnecessary checking that can be
    slow."""
    def submoduleDeps(submodules):
        return {'uptodate': [wlutil.config_changed(wlutil.checkGitStatus(sub)) for sub in submodules]}

    return {'name': name,
            'actions': [(submoduleDeps, [submodules])]}


def kmodDepsTask(cfg, taskDeps=None, name=""):
    """Check if the kernel modules in cfg are uptodate (suitable for doit's calc_dep function)"""

    def checkMods(cfg):
        log = logging.getLogger()

        if 'modules' not in cfg['linux']:
            return

        for driverDir in cfg['linux']['modules'].values():
            if not driverDir.exists():
                log.warn("WARNING: Required module " + str(driverDir) + " does not exist: Assuming the workload is not uptodate.")
                return False
            try:
                p = wlutil.run(["make", "-q", "LINUXSRC=" + str(cfg['linux']['source'])], cwd=driverDir, check=False)

                if p.returncode != 0:
                    return False
            except Exception as e:
                log.warn("WARNING: Error when checking if module " + str(driverDir) + " is up to date: Assuming workload is not up to date. Error: " + str(e))
                return False

        return True

    def calcModsAction(cfg):
        return {'uptodate': [checkMods(cfg)]}

    task = {'name': name,
            'actions': [(calcModsAction, [cfg])]}

    if taskDeps is not None:
        task['task_dep'] = taskDeps

    return task


def fileDepsTask(name, taskDeps=None, overlay=None, files=None):
    """Returns a task dict for a calc_dep task that calculates the file
    dependencies represented by an overlay and/or a list of FileSpec objects.
    Either can be None.

    taskDeps should be a list of names of tasks that must run before
    calculating dependencies (e.g. host-init)"""

    def fileDeps(overlay, files):
        """The python-action for the filedeps task, returns a dictionary of dependencies"""
        deps = []
        if overlay is not None:
            deps.append(overlay)

        if files is not None:
            deps += [f.src for f in files if not f.src.is_symlink()]

        for dep in deps.copy():
            if dep.is_dir():
                deps += [child for child in dep.glob('**/*') if not child.is_symlink()]

        return {'file_dep': [str(f) for f in deps if not f.is_dir()]}

    task = {'name': 'calc_' + name + '_dep',
            'actions': [(fileDeps, [overlay, files])]}

    if taskDeps is not None:
        task['task_dep'] = taskDeps

    return task


def addDep(loader, config):
    """Adds 'config' to the doit dependency graph ('loader')"""

    # Linux-based workloads depend on this task
    loader.addTask({
        'name': 'build_busybox',
        'actions': [(buildBusybox, [config])],
        'targets': [wlutil.getOpt('initramfs-dir') / 'disk' / 'bin' / 'busybox',
                    wlutil.getOpt('initramfs-dir') / 'nodisk' / 'bin' / 'busybox'],
        'file_dep': [wlutil.getOpt('wlutil-dir') / 'busybox-config',
                     wlutil.getOpt('wlutil-dir') / 'busybox-patches' / '0001-tc-fix-linux-6.8.patch'],
        'uptodate': [wlutil.config_changed(wlutil.checkGitStatus(wlutil.getOpt('busybox-dir'))),
                     wlutil.config_changed(wlutil.getToolVersions())]
        })

    hostInit = []
    # Host-init task always runs because we can't tell if its uptodate and we
    # don't know its inputs/outputs.
    if 'host-init' in config:
        loader.addTask({
            'name': str(config['host-init']),
            'actions': [(handleHostInit, [config])],
        })
        hostInit = [str(config['host-init'])]

    # Add a rule for the binary
    # The bootbinary's initramfs and kernel build depend on the base image
    # inputs. Keep those as file dependencies as well as task dependencies;
    # otherwise a rebuilt Buildroot image can leave a stale disk bootbinary.
    bin_file_deps = [] + config['base-deps']
    bin_task_deps = [] + hostInit + config['base-deps']
    bin_targets = []
    if 'linux' in config:
        bin_file_deps += config['linux']['config']
        bin_task_deps.append('build_busybox')
        bin_targets.append(config['dwarf'])

    if config['use-parent-bin']:
        bin_task_deps.append(str(config['base-bin']))

    diskBin = []
    if 'bin' in config:
        targets = [str(config['bin'])]
        if 'dwarf' in config:
            targets.append(str(config['dwarf']))
        # Add driver dwarf files as targets
        if 'driver-dwarfs' in config:
            targets.extend([str(dwarf_path) 
                           for dwarf_path in config['driver-dwarfs'].values()])

        moddeps = []
        if 'firmware' in config:
            moddeps.append(config['firmware']['source'])

        bin_calc_dep_tsks = [
                submoduleDepsTask(moddeps, name="_submodule_deps_"+config['name']),
            ]

        if 'linux' in config:
            moddeps.append(config['linux']['source'])
            bin_calc_dep_tsks.append(kmodDepsTask(config, name="_kmod_deps_"+config['name']))

        for tsk in bin_calc_dep_tsks:
            loader.addTask(tsk)

        loader.addTask({
                'name': str(config['bin']),
                'actions': [(makeBin, [config])],
                'targets': targets,
                'file_dep': bin_file_deps,
                'task_dep': bin_task_deps,
                'calc_dep': [tsk['name'] for tsk in bin_calc_dep_tsks]
                })
        diskBin = [str(config['bin'])]

    # Add a rule for the nodisk version if requested
    nodiskBin = []
    if config['nodisk'] and 'bin' in config:
        nodisk_file_deps = bin_file_deps.copy()
        nodisk_task_deps = bin_task_deps.copy()
        if 'img' in config:
            nodisk_file_deps.append(config['img'])
            nodisk_task_deps.append(str(config['img']))

        targets = [str(wlutil.noDiskPath(config['bin']))]
        if 'dwarf' in config:
            targets.append(str(wlutil.noDiskPath(config['dwarf'])))
        # Add driver dwarf files as targets for nodisk build
        if 'driver-dwarfs' in config:
            targets.extend([str(wlutil.noDiskPath(dwarf_path)) 
                           for dwarf_path in config['driver-dwarfs'].values()])

        uptodate = []
        if 'firmware' in config:
            uptodate.append(wlutil.config_changed(wlutil.checkGitStatus(config['firmware']['source'])))
        if 'linux' in config:
            uptodate.append(wlutil.config_changed(wlutil.checkGitStatus(config['linux']['source'])))

        loader.addTask({
                'name': str(wlutil.noDiskPath(config['bin'])),
                'actions': [(makeBin, [config], {'nodisk': True})],
                'targets': targets,
                'file_dep': nodisk_file_deps,
                'task_dep': nodisk_task_deps,
                'uptodate': uptodate
                })
        nodiskBin = [str(wlutil.noDiskPath(config['bin']))]

    # Add a rule for running script after binary is created (i.e. for ext. modules)
    # Similar to 'host-init' always runs if exists
    postBin = []
    post_bin_task_deps = diskBin + nodiskBin  # also used to get the bin path
    if 'post-bin' in config:
        loader.addTask({
            'name': str(config['post-bin']),
            'actions': [(handlePostBin, [config, post_bin_task_deps[0]])],
            'task_dep': post_bin_task_deps,
        })
        postBin = [str(config['post-bin'])]

    # Add a rule for the image (if any)
    img_file_deps = []
    img_task_deps = [] + hostInit + postBin + config['base-deps']
    img_calc_deps = []
    img_uptodate = []
    if 'img' in config:
        if 'base-img' in config:
            img_file_deps.append(config['base-img'])

        if 'files' in config or 'overlay' in config:
            # We delay calculation of files and overlay dependencies to runtime
            # in order to catch any generated inputs
            fdepsTask = fileDepsTask(config['name'], taskDeps=img_task_deps,
                                     overlay=config.get('overlay'),
                                     files=config.get('files'))
            img_calc_deps.append(fdepsTask['name'])
            loader.addTask(fdepsTask)
        if 'guest-init' in config:
            img_file_deps.append(config['guest-init'].path)
            img_task_deps.append(str(config['bin']))
        if 'runSpec' in config and config['runSpec'].path is not None:
            img_file_deps.append(config['runSpec'].path)
        if 'cfg-file' in config:
            img_file_deps.append(config['cfg-file'])
        if 'distro' in config:
            img_uptodate += config['builder'].upToDate()

        loader.addTask({
            'name': str(config['img']),
            'actions': [(makeImage, [config])],
            'targets': [config['img']],
            'file_dep': img_file_deps,
            'task_dep': img_task_deps,
            'calc_dep': img_calc_deps,
            'uptodate': img_uptodate
            })


# Generate a task-graph loader for the doit "Run" command
# Note: this doesn't depend on the config or runtime args at all. In theory, it
# could be cached, but I'm not going to bother unless it becomes a performance
# issue.
def buildDepGraph(cfgs):
    loader = doitLoader()

    for cfgPath in cfgs.keys():
        config = cfgs[cfgPath]

        if config['isDistro'] and 'img' in config:
            loader.addTask({
                    'name': str(config['img']),
                    'actions': [(config['builder'].buildBaseImage)],
                    'targets': [config['img']],
                    'file_dep': config['builder'].fileDeps(),
                    'uptodate': (config['builder'].upToDate() +
                                 [wlutil.config_changed(wlutil.getToolVersions())])
            })
        else:
            addDep(loader, config)

            if 'jobs' in config.keys():
                for jCfg in config['jobs'].values():
                    addDep(loader, jCfg)

    return loader


def buildWorkload(cfgName, cfgs, buildBin=True, buildImg=True, lspOnly=False):
    # This should only be built once (multiple builds will mess up doit)
    global taskLoader
    if taskLoader is None:
        taskLoader = buildDepGraph(cfgs)

    config = cfgs[cfgName]

    if lspOnly:
        # For LSP mode, we only need to generate compile_commands.json
        if 'linux' not in config:
            print("Warning: No Linux configuration found for LSP generation")
            return 0

        # Create a simple task for LSP generation using the existing build system
        lspTask = {
            'name': f"lsp-{cfgName}",
            'actions': [(makeBin, [config], {'lspOnly': True})],
            'targets': [str(config['out-dir'] / 'compile_commands.json')],
            'file_dep': config['linux']['config'] if isinstance(config['linux']['config'], list) else [config['linux']['config']],
            'task_dep': []
        }

        # Add the LSP task to the loader
        taskLoader.addTask(lspTask)

        doitHandle = doit.doit_cmd.DoitMain(taskLoader)
        return doitHandle.run([str(config['out-dir'] / 'compile_commands.json')])

    imgList = []
    binList = []

    if buildBin and 'bin' in config:
        if config['nodisk']:
            binList.append(wlutil.noDiskPath(config['bin']))
        else:
            binList.append(config['bin'])

    if 'img' in config and buildImg and not config['img-hardcoded']:
        imgList.append(config['img'])

    if 'jobs' in config.keys():
        for jCfg in config['jobs'].values():
            if buildBin:
                binList.append(jCfg['bin'])
                if jCfg['nodisk']:
                    binList.append(wlutil.noDiskPath(jCfg['bin']))

            if 'img' in jCfg and buildImg and not jCfg['img-hardcoded']:
                imgList.append(jCfg['img'])

    doitHandle = doit.doit_cmd.DoitMain(taskLoader)

    # The order isn't critical here, we should have defined the dependencies correctly in loader
    return doitHandle.run([str(p) for p in binList + imgList])


def makeInitramfs(srcs, cpioDir, includeDevNodes=False):
    """Generate a cpio archive containing each of the sources and store it in cpioDir.
    Return a path to the generated archive.
    srcs: are a list of paths to directories to include, sources will be
          applied in-order (potentially overwriting duplicate files).
    cpioDir: Scratch directory to produce outputs in
    includeDevNodes: If true, will include '/dev/console' and '/dev/tty' special files."""

    # Generate individual cpios for each source
    cpios = []
    for src in srcs:
        dst = cpioDir / (src.name + '.cpio')
        with contextlib.suppress(FileNotFoundError):
            os.remove(dst)
        wlutil.toCpio(src, dst)
        cpios.append(dst)

    if includeDevNodes:
        cpios.append(wlutil.getOpt('initramfs-dir') / 'devNodes.cpio')

    # Generate final cpio
    finalPath = cpioDir / 'initramfs.cpio'
    with contextlib.suppress(FileNotFoundError):
        os.remove(finalPath)
    with open(finalPath, 'wb') as finalF:
        for cpio in cpios:
            with open(cpio, 'rb') as srcF:
                shutil.copyfileobj(srcF, finalF)

    return finalPath


def generateKConfig(kfrags, linuxSrc):
    """Generate the final .config in linuxSrc from the provided list of kernel
    configuration fragments. Fragments will be applied on top of defconfig."""
    linuxCfg = linuxSrc / '.config'
    defCfg = wlutil.getOpt('gen-dir') / 'defconfig'

    # Create a defconfig to use as reference
    wlutil.run(['make'] + wlutil.getOpt('linux-make-args') + ['defconfig'], cwd=linuxSrc)
    shutil.copy(linuxCfg, defCfg)

    # Create a config from the user fragments
    kconfigEnv = os.environ.copy()
    kconfigEnv['ARCH'] = 'riscv'
    kconfigEnv['CROSS_COMPILE'] = 'riscv64-unknown-linux-gnu-'
    wlutil.run([linuxSrc / 'scripts/kconfig/merge_config.sh', str(defCfg)] +
               list(map(str, kfrags)), env=kconfigEnv, cwd=linuxSrc)


def makeInitramfsKfrag(src, dst):
    with open(dst, 'w') as f:
        f.write("CONFIG_BLK_DEV_INITRD=y\n")
        f.write('CONFIG_INITRAMFS_SOURCE="' + str(src) + '"\n')


def makeModules(cfg):
    """Build all the kernel modules for this config. The compiled kmods will be
    put in the appropriate location in the initramfs staging area."""

    linCfg = cfg['linux']
    drivers = []
    driver_dwarfs = []

    # Prepare the linux source with the proper config
    generateKConfig(linCfg['config'], linCfg['source'])
    cfg['out-dir'].mkdir(parents=True, exist_ok=True)
    shutil.copy(linCfg['source'] / '.config', cfg['out-dir'] / 'linux_module_config')

    # Build modules (if they exist)
    if ('modules' in linCfg) and (len(linCfg['modules']) != 0):
        # Prepare the linux source for building external modules
        wlutil.run(["make"] + wlutil.getOpt('linux-make-args') +
                   ["modules_prepare", '-j' + str(wlutil.getOpt('jlevel'))],
                   cwd=linCfg['source'])

        # MODPOST errors are warnings, since we built the extmods without building the kernel first
        makeCmd = "make KBUILD_MODPOST_WARN=1 LINUXSRC=" + str(linCfg['source'])

        for driverName, driverDir in linCfg['modules'].items():
            wlutil.checkSubmodule(driverDir)

            # Drivers don't seem to detect changes in the kernel
            wlutil.run(makeCmd + " clean", cwd=driverDir, shell=True)
            wlutil.run(makeCmd, cwd=driverDir, shell=True)
            
            # Collect compiled kernel modules
            ko_files = list(driverDir.glob("*.ko"))
            drivers.extend(ko_files)
            
            # Generate dwarf files for each driver
            for ko_file in ko_files:
                # Find the corresponding .o file (unstripped object with debug info)
                o_file = driverDir / (ko_file.stem + ".o")
                if (o_file.exists() and 'driver-dwarfs' in cfg and 
                        driverName in cfg['driver-dwarfs']):
                    # Use the configured dwarf file path
                    dwarf_file = cfg['driver-dwarfs'][driverName]
                    shutil.copy(o_file, dwarf_file)
                    driver_dwarfs.append(dwarf_file)

    kernelVersion = sp.run(["make", "-s", "ARCH=riscv", "kernelrelease"], cwd=linCfg['source'], stdout=sp.PIPE, universal_newlines=True).stdout.strip()
    driverDir = wlutil.getOpt('initramfs-dir') / "drivers" / "lib" / "modules" / kernelVersion

    # Always start from a clean slate
    try:
        shutil.rmtree(driverDir.parent)
    except FileNotFoundError:
        pass
    driverDir.mkdir(parents=True)

    # Copy in our new drivers
    for driverPath in drivers:
        shutil.copy(driverPath, driverDir)

    # Setup the dependency file needed by modprobe to load the drivers
    wlutil.run(['depmod', '-b', str(wlutil.getOpt('initramfs-dir') / "drivers"), kernelVersion])
    
    # Return the list of generated driver dwarf files
    return driver_dwarfs


def makeOpenSBI(config, nodisk=False):
    payload = config['linux']['source'] / 'arch' / 'riscv' / 'boot' / 'Image'
    size = payload.stat().st_size

    # Sometimes static variables can exceed the size of the flat image
    # Look in the vmlinux ELF for the max address, and use that if its greater
    vmlinux = config['linux']['source'] / 'vmlinux'
    # don't use wlutil.run, since we need to process the stdout
    proc = sp.Popen(['readelf', '--segments', '--wide', vmlinux], stdout=sp.PIPE, universal_newlines=True)
    proc.wait()
    for line in iter(proc.stdout.readline, ''):
        line = line.strip()
        if "LOAD" in line:
            cols = line.split()
            base = int(cols[3], 16)
            memsize = int(cols[5], 16)
            if base + memsize > size:
                size = base + memsize

    # Align to next MiB
    payloadSize = ((size + 0xfffff) // 0x100000) * 0x100000
    # Default memory layout:
    #
    # 0x80000000  +--------------------------------------+
    #             | OpenSBI                              | runtime base
    #             | Remaining alignment space            |
    # 0x80200000  +--------------------------------------+
    #             | Linux kernel                         | runtime base
    #             | Includes initramfs for nodisk builds | + FW_PAYLOAD_OFFSET
    #             | Reserved through payloadSize         |
    #             +--------------------------------------+
    #             | Copied device tree from bootrom      | FW_PAYLOAD_FDT_ADDR
    #             +--------------------------------------+
    #
    payloadFdtAddr = 0x80000000 + 0x200000 + payloadSize
    makeArgsOpts = ['PLATFORM=generic',
                    'FW_PAYLOAD_OFFSET=0x200000',
                    'FW_PAYLOAD_PATH=' + str(payload),
                    'FW_PAYLOAD_FDT_ADDR=0x{:X}'.format(payloadFdtAddr)]

    args = wlutil.getOpt('linux-make-args') + makeArgsOpts

    if 'opensbi-build-args' in config['firmware']:
        args += config['firmware']['opensbi-build-args']

    wlutil.run(['make'] + wlutil.getOpt('linux-make-args') + args,
               cwd=config['firmware']['source'])

    return config['firmware']['source'] / 'build' / 'platform' / 'generic' / 'firmware' / 'fw_payload.elf'


def makeBin(config, nodisk=False, lspOnly=False):
    """Build the binary specified in 'config'.

    This is called as a doit task (see buildDepGraph() and addDep())
    """
    if config['use-parent-bin'] and not nodisk:
        config['bin'].parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(config['base-bin'], config['bin'])
        if 'dwarf' in config:
            config['dwarf'].parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(config['base-dwarf'], config['dwarf'])
        # Copy driver dwarf files from the parent's out-dir. The parent may
        # predate driver-dwarf emission; warn instead of failing so old base
        # images keep working (rebuild the parent to get the dwarfs).
        if 'driver-dwarfs' in config:
            log = logging.getLogger()
            for driverName, dwarf_path in config['driver-dwarfs'].items():
                base_dwarf = config['base-bin'].parent / (driverName + "-dwarf")
                if base_dwarf.exists():
                    dwarf_path.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy(base_dwarf, dwarf_path)
                else:
                    log.warn("WARNING: parent image has no dwarf for driver '" +
                             driverName + "' (" + str(base_dwarf) +
                             "); rebuild the parent workload to generate it")
        return True

    # We assume that if you're not building linux, then the image is pre-built (e.g. during host-init)
    if 'linux' in config:
        initramfsIncludes = []

        # makeModules copies driver dwarfs into out-dir; it must exist first
        config['out-dir'].mkdir(parents=True, exist_ok=True)

        # Some submodules are only needed if building Linux
        try:
            wlutil.checkSubmodule(config['linux']['source'])
            wlutil.checkSubmodule(config['firmware']['source'])

            makeModules(config)
        except wlutil.SubmoduleError as err:
            return doit.exceptions.TaskFailed(err)

        initramfsIncludes.append(wlutil.getOpt('initramfs-dir') / 'drivers')
        cpioDir = config['out-dir']
        cpioDir = pathlib.Path(cpioDir)
        initramfsPath = ""
        if nodisk:
            initramfsIncludes += [wlutil.getOpt('initramfs-dir') / "nodisk"]
            with wlutil.mountImg(config['img'], wlutil.getOpt('mount-dir')):
                initramfsIncludes = [wlutil.getOpt('mount-dir')] + initramfsIncludes
                # This must be done while in the mountImg context
                initramfsPath = makeInitramfs(initramfsIncludes, cpioDir, includeDevNodes=True)
        else:
            initramfsIncludes += [wlutil.getOpt('initramfs-dir') / "disk"]
            initramfsPath = makeInitramfs(initramfsIncludes, cpioDir, includeDevNodes=True)

        if lspOnly:
            # For LSP mode, just generate compile_commands.json
            generateKConfig(config['linux']['config'], config['linux']['source'])
            wlutil.run(['make'] + wlutil.getOpt('linux-make-args') + ['compile_commands.json'],
                       cwd=config['linux']['source'])

            # Copy the generated compile_commands.json to the output directory
            shutil.copy(config['linux']['source'] / 'compile_commands.json',
                        config['out-dir'] / 'compile_commands.json')
            shutil.copy(config['linux']['source'] / '.config',
                        config['out-dir'] / 'linux_config')
        else:
            # Normal build process
            makeInitramfsKfrag(initramfsPath, cpioDir / "initramfs.kfrag")
            generateKConfig(config['linux']['config'] + [cpioDir / "initramfs.kfrag"], config['linux']['source'])
            wlutil.run(['make'] + wlutil.getOpt('linux-make-args') + ['vmlinux', 'Image', '-j' + str(wlutil.getOpt('jlevel'))], cwd=config['linux']['source'])
            # copy files needed to build linux (busybox copying is put here so that it is shown per linux build)
            shutil.copy(config['linux']['source'] / '.config', config['out-dir'] / 'linux_config')
            shutil.copy(wlutil.getOpt('busybox-dir') / '.config', config['out-dir'] / 'busybox_config')

            fw = makeOpenSBI(config, nodisk)

            config['bin'].parent.mkdir(parents=True, exist_ok=True)
            config['dwarf'].parent.mkdir(parents=True, exist_ok=True)
            
            # Ensure driver dwarf directories exist
            if 'driver-dwarfs' in config:
                for dwarf_path in config['driver-dwarfs'].values():
                    if nodisk:
                        wlutil.noDiskPath(dwarf_path).parent.mkdir(
                            parents=True, exist_ok=True)
                    else:
                        dwarf_path.parent.mkdir(parents=True, exist_ok=True)
            
            if nodisk:
                shutil.copy(fw, wlutil.noDiskPath(config['bin']))
                shutil.copy(config['linux']['source'] / 'vmlinux', 
                           wlutil.noDiskPath(config['dwarf']))
                # Copy driver dwarf files for nodisk build
                if 'driver-dwarfs' in config:
                    for dwarf_path in config['driver-dwarfs'].values():
                        if dwarf_path.exists():
                            shutil.copy(dwarf_path, 
                                       wlutil.noDiskPath(dwarf_path))
            else:
                shutil.copy(fw, config['bin'])
                shutil.copy(config['linux']['source'] / 'vmlinux', config['dwarf'])
                # Driver dwarf files are already generated and copied 
                # during makeModules

    return True


def makeImage(config):
    log = logging.getLogger()

    # Remove old image so that you re-apply the overlay/files/etc
    with contextlib.suppress(FileNotFoundError):
        os.remove(config['img'])

    # Create new image from a copy of the base
    if 'base-img' in config:
        config['img'].parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(config['base-img'], config['img'])

    # Resize if needed
    if config['img-sz'] != 0:
        wlutil.resizeFS(config['img'], config['img-sz'])

    if 'overlay' in config:
        log.debug("Applying overlay: " + str(config['overlay']))
        wlutil.applyOverlay(config['img'], config['overlay'])

    if 'files' in config:
        log.debug("Applying file list: " + str(config['files']))
        wlutil.copyImgFiles(config['img'], config['files'], 'in')

    if 'guest-init' in config:
        log.debug("Applying init script: " + str(config['guest-init'].path))
        if not config['guest-init'].path.exists():
            raise ValueError("Init script " + str(config['guest-init'].path) + " not found.")

        # Apply and run the init script
        init_overlay = config['builder'].generateBootScriptOverlay(
            str(config['guest-init'].path), config['guest-init'].args)
        wlutil.applyOverlay(config['img'], init_overlay)
        wlutil.run(wllaunch.getQemuCmd(config), shell=True, level=logging.DEBUG)

        # Clear the init script
        run_overlay = config['builder'].generateBootScriptOverlay(None, None)
        wlutil.applyOverlay(config['img'], run_overlay)

    if 'runSpec' in config:
        spec = config['runSpec']
        if spec.command is not None:
            log.debug("Applying run command: " + str(spec.command))
            scriptPath = wlutil.genRunScript(spec.command)
        else:
            log.debug("Applying run script: " + str(spec.path))
            scriptPath = spec.path

        if not scriptPath.exists():
            raise ValueError("Run script " + str(scriptPath) + " not found.")

        run_overlay = config['builder'].generateBootScriptOverlay(scriptPath, spec.args)
        wlutil.applyOverlay(config['img'], run_overlay)

    # Tighten the image size if requested
    if config['img-sz'] == 0:
        wlutil.resizeFS(config['img'], 0)
