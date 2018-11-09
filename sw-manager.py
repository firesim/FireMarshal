#!/usr/bin/env python3
import sys
import argparse
import json
import subprocess as sp
import os
import shutil
import br.br as br
import fedora.fedora as fed
import pprint
import doit
import glob
import re
from config import *

jlevel = "-j" + str(os.cpu_count())

# Some warnings might be missed if they happen early on. This string will
# be printed at the very end
delayed_warning = ""

def main():
    parser = argparse.ArgumentParser(
        description="Build and run (in spike or qemu) boot code and disk images for firesim")
    parser.add_argument('-c', '--config',
                        help='Configuration file to use (defaults to br-disk.json)',
                        nargs='?', default='br-disk.json', dest='config_file')
    subparsers = parser.add_subparsers(title='Commands')

    # Build command
    build_parser = subparsers.add_parser(
        'build', help='Build an image from the given configuration.')
    build_parser.set_defaults(func=handleBuild)

    # Launch command
    launch_parser = subparsers.add_parser(
        'launch', help='Launch an image on a software simulator (defaults to qemu)')
    launch_parser.set_defaults(func=handleLaunch)
    launch_parser.add_argument('-s', '--spike', action='store_true')

    args = parser.parse_args()
    args.config_file = os.path.abspath(args.config_file)

    # Load all the configs from the workload directory
    cfgs = ConfigManager([workload_dir])

    args.func(args, cfgs)
    print(delayed_warning)

class doitLoader(doit.cmd_base.TaskLoader):
    workloads = []

    # @staticmethod
    def load_tasks(self, cmd, opt_values, pos_args):
        task_list = [doit.task.dict_to_task(w) for w in self.workloads]
        config = {'verbosity': 2}
        return task_list, config

# Generate a task-graph loader for the doit "Run" command
# Note: this doesn't depend on the config or runtime args at all. In theory, it
# could be cached, but I'm not going to bother unless it becomes a performance
# issue.
def buildDepGraph(cfgs):
    loader = doitLoader()

    # Define the base-distro tasks
    for d in distros:
        dCfg = cfgs[d]
        loader.workloads.append({
                'name' : dCfg['img'],
                'actions' : [(dCfg['builder'].buildBaseImage, [])],
                'targets' : [dCfg['img']],
                'uptodate': [(dCfg['builder'].upToDate, [])]
            })

    # Non-distro configs 
    for cfgPath in (set(cfgs.keys()) - set(distros)):
        config = cfgs[cfgPath]

        # Add a rule for the binary
        file_deps = [config['linux-config']]

        task_deps = []
        if config['rootfs-format'] == 'cpio':
            file_deps.append(config['img'])
            task_deps.append(config['img'])

        loader.workloads.append({
                'name' : config['bin'],
                'actions' : [(makeBin, [config])],
                'targets' : [config['bin']],
                'file_dep': file_deps,
                'task_dep' : task_deps
                })

        # Add a rule for the image
        task_deps = [config['base-img']]
        file_deps = [config['base-img']]
        if 'overlay' in config:
            for root, dirs, files in os.walk(config['overlay']):
                for f in files:
                    file_deps.append(os.path.join(root, f))
        if 'init' in config:
            file_deps.append(config['init'])
            task_deps.append(config['bin'])
        if 'run' in config:
            file_deps.append(config['run'])
        
        loader.workloads.append({
            'name' : config['img'],
            'actions' : [(makeImage, [config])],
            'targets' : [config['img']],
            'file_dep' : file_deps,
            'task_dep' : task_deps
            })

    return loader

def handleBuild(args, cfgs):
    loader = buildDepGraph(cfgs)
    config = cfgs[args.config_file]
    if config['rootfs-format'] == 'img':
        # Be sure to build the bin first, then the image, because if there is
        # an init script, we need to boot the binary in order to apply it
        doit.doit_cmd.DoitMain(loader).run([config['bin'], config['img']])
        # doit.doit_cmd.DoitMain(loader).run(['info', "/data/repos/firesim/sw/firesim-software/images/br-disk.img"])
    elif config['rootfs-format'] == 'cpio':
        # CPIO must build the image first, since the binary links to it.
        # Since CPIO doesn't support init scripts, we don't need the bin first
        doit.doit_cmd.DoitMain(loader).run([config['img'], config['bin']])

def launchSpike(config):
    sp.check_call(['spike', '-p4', '-m4096', config['bin']])

def launchQemu(config):
    cmd = ['qemu-system-riscv64',
           '-nographic',
           '-smp', '4',
           '-machine', 'virt',
           '-m', '4G',
           '-kernel', config['bin'],
           '-object', 'rng-random,filename=/dev/urandom,id=rng0',
           '-device', 'virtio-rng-device,rng=rng0',
           '-device', 'virtio-net-device,netdev=usernet',
           '-netdev', 'user,id=usernet,hostfwd=tcp::10000-:22']

    if config['rootfs-format'] == 'img':
        cmd = cmd + ['-device', 'virtio-blk-device,drive=hd0',
                     '-drive', 'file=' + config['img'] + ',format=raw,id=hd0']
        cmd = cmd + ['-append', 'ro root=/dev/vda']

    sp.check_call(cmd)

def handleLaunch(args, cfgs):
    config = cfgs[args.config_file]
    if args.spike:
        if config['rootfs-format'] == 'img':
            sys.exit("Spike currently does not support disk-based " +
                    "configurations. Please use an initramfs based image.")
        launchSpike(config)
    else:
        launchQemu(config)

# It's pretty easy to forget to update the linux config for initramfs-based
# workloads. We check here to make sure you've set the CONFIG_INITRAMFS_SOURCE
# option correctly. This only issues a warning right now because you might have
# a legitimate reason to point linux somewhere else (e.g. while debugging).
def checkInitramfsConfig(config):
    global delayed_warning
    if config['rootfs-format'] == 'cpio':
       with open(config['linux-config'], 'rt') as f:
           linux_config = f.read()
           match = re.search(r'^CONFIG_INITRAMFS_SOURCE=(.*)$', linux_config, re.MULTILINE)
           if match:
               initramfs_src = os.path.normpath(os.path.join(linux_dir, match.group(1).strip('\"')))
               if initramfs_src != config['img']:
                   delayed_warning += "WARNING: The workload linux config " + \
                   "'CONFIG_INITRAMFS_SOURCE' option doesn't point to this " + \
                   "workload's image:\n" + \
                   "\tCONFIG_INITRAMFS_SOURCE = " + initramfs_src + "\n" +\
                   "\tWorkload Image = " + config['img'] + "\n" + \
                   "You likely want to change this option to:\n" +\
                   "\tCONFIG_INITRAMFS_SOURCE=" + os.path.relpath(config['img'], linux_dir)
           else:
               delayed_warning += "WARNING: The workload linux config doesn't include a " + \
               "CONFIG_INITRAMFS_SOURCE option, but this workload is " + \
               "using cpio for it's image.\n" + \
               "You likely want to change this option to:\n" + \
               "\tCONFIG_INITRAMFS_SOURCE=" + os.path.relpath(config['img'], linux_dir)
 
# Now build linux/bbl
def makeBin(config):
    # I am not without mercy
    checkInitramfsConfig(config)
            
    shutil.copy(config['linux-config'], os.path.join(linux_dir, ".config"))
    sp.check_call(['make', 'ARCH=riscv', 'vmlinux', jlevel], cwd=linux_dir)
    if not os.path.exists('riscv-pk/build'):
        os.mkdir('riscv-pk/build')

    sp.check_call(['../configure', '--host=riscv64-unknown-elf',
                   '--with-payload=../../riscv-linux/vmlinux'], cwd='riscv-pk/build')
    sp.check_call(['make', jlevel], cwd='riscv-pk/build')
    shutil.copy('riscv-pk/build/bbl', config['bin'])

def makeImage(config):
    if config['base-format'] == config['rootfs-format']:
        shutil.copy(config['base-img'], config['img'])
    elif config['base-format'] == 'img' and config['rootfs-format'] == 'cpio':
        toCpio(config, config['base-img'], config['img'])
    elif config['base-format'] == 'cpio' and config['rootfs-format'] == 'img':
        raise NotImplementedError("Converting from CPIO to raw img is not currently supported")
    else:
        raise ValueError("Invalid formats for base and/or new image: Base=" +
                config['base-format'] + ", New=" + config['rootfs-format'])

    if 'host-init' in config:
        sp.check_call([config['host-init']], cwd=config['workdir'])

    if 'overlay' in config:
        applyOverlay(config['img'], config['overlay'], config['rootfs-format'])

    if 'init' in config:
        if config['rootfs-format'] == 'cpio':
            raise ValueError("CPIO-based images do not support init scripts.")

        # Apply and run the init script
        init_overlay = config['builder'].generateBootScriptOverlay(config['init'])
        applyOverlay(config['img'], init_overlay, config['rootfs-format'])
        launchQemu(config)

        # Clear the init script
        run_overlay = config['builder'].generateBootScriptOverlay(None)
        applyOverlay(config['img'], run_overlay, config['rootfs-format'])

    if 'run' in config:
        run_overlay = config['builder'].generateBootScriptOverlay(config['run'])
        applyOverlay(config['img'], run_overlay, config['rootfs-format'])

def toCpio(config, src, dst):
    sp.check_call(['sudo', 'mount', '-o', 'loop', src, mnt])
    try:
        sp.check_call("sudo find -print0 | sudo cpio --owner root:root --null -ov --format=newc > " + dst, shell=True, cwd=mnt)
    finally:
        sp.check_call(['sudo', 'umount', mnt])

# Apply the overlay directory "overlay" to the filesystem image "img" which
# has format "fmt" (either 'cpio' or 'img').
# Note that all paths must be absolute
def applyOverlay(img, overlay, fmt):
    if fmt == 'img':
        sp.check_call(['sudo', 'mount', '-o', 'loop', img, mnt])
        try:
            # Overlays may not be owned by root, but the filesystem must be.
            # Rsync lets us chown while copying.
            sp.check_call('sudo rsync -a --chown=root:root ' + overlay + '/*' + " " + mnt, shell=True)
        finally:
            sp.check_call(['sudo', 'umount', mnt])

    elif fmt == 'cpio':
        # Note: a quirk of cpio is that it doesn't really overwrite files when
        # doing an overlay, it actually just appends a new file with the same
        # name. Linux handles this just fine (it uses the latest version of a
        # file), but be aware.
        sp.check_call(
                'sudo find ./* -print0 | sudo cpio --owner root:root --null -ov -H newc >> ' + img, cwd=overlay, shell=True)

    else:
        raise ValueError(
            "Only 'img' and 'cpio' formats are currently supported")

main()
