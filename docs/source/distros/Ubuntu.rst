.. _ubuntu-distro:

The Ubuntu Distribution
=============================

Ubuntu uses the SiFive HiFive Unmatched server image of Ubuntu 20.04 found `here
<https://cdimage.ubuntu.com/releases/20.04.3/release/>`_.

First time setup notes:
- When running ``./marshal build ubuntu-base.json`` for the first time, the process MAY end in a login prompt for Ubuntu. Login with ``root:firesim``, and upon login, exit Qemu with CTRL-A+X
- Login with ``root:firesim``  when launching with Firemarshal/Firesim.
- Make sure to set the ``rootfs-size`` in ``ubuntu-base.json`` to be large enough to account for the space needed for ``sudo apt update && sudo apt-upgrade``.   
