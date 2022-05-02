FireMarshal
==================================

This tool builds base images for several linux-based distros that work with qemu,
spike, and FireSim.

This is just a quick primer. To see full documentation, please see the official
documentation:
https://firemarshal.readthedocs.io/en/latest/index.html

# Setup

The easiest way to use Marshal is to run it via Chipyard
(https://chipyard.readthedocs.io/en/latest/) or FireSim
(https://docs.fires.im/en/latest/). However, this is not required. 

Run the following script to install all of FireMarshal's dependencies (standard packages from apt/yum, python libraries, open-source packages, submodules):

```
./install/marshal-dependencies.sh
```

Note that this script downloads dependencies to `install/dependencies` and installs them at the path pointed to by the `RISCV` environment variable. Thus, the script requires `RISCV` to be set and FireMarshal expects `PATH` to contain the path it points to (Chipyard/FireSim already do this).

Running bare-metal workloads requires a minimal set of dependencies. You can install just these by passing the `-b` flag as follows:

```
./install/marshal-dependencies.sh -b
```

### Note for Ubuntu
The libguestfs-tools package (needed for the guestmount command) does not work
out of the box on Ubuntu. See
https://github.com/firesim/firesim-software/issues/30 for a workaround.

## RISC-V Tools
In addition to the dependencies installed above, you will need a RISC-V compatible toolchain, the RISC-V isa simulator (spike). This is already installed by Chipyard/FireSim.

See the [Chipyard documentation](https://chipyard.readthedocs.io/en/latest/Chipyard-Basics/Initial-Repo-Setup.html#building-a-toolchain)
for help setting up a known-good toolchain and environment.

# Basic Usage

Building workloads:

    ./marshal build br-base.json

To run in qemu:

    ./marshal launch br-base.json

To install into FireSim (assuming you cloned this as a submodule of FireSim or Chipyard):

    ./marshal install br-base.json

# Security Note
Be advised that FireMarshal will run initialization scripts provided by
workloads. These scripts will have all the permissions your user has, be sure
to read all workloads carefully before building them.

# Releases
The master branch of this project contains the latest unstable version of
FireMarshal. It should generally work correctly, but it may contain bugs or
other inconsistencies from time to time. For stable releases, see the release
git tags or github releases page.

# Getting Help / Discussion:
* For general questions, help, and discussion: use the FireSim user forum: https://groups.google.com/forum/#!forum/firesim
* For bugs and feature requests: use the github issue tracker: https://github.com/firesim/FireMarshal/issues
* See CONTRIBUTING.md for information on how to contribute to the project 
