.. _ubuntu-distro:

The Ubuntu Distribution
=============================

Ubuntu uses the SiFive HiFive Unmatched server image of Ubuntu 20.04 found `here
<https://cdimage.ubuntu.com/releases/20.04.3/release/>`_.

First time setup notes:

- When running ``./marshal build ubuntu-base.json`` for the first time, the process MAY end in a login prompt for Ubuntu. Login with ``root:firesim``, and upon login, exit Qemu with CTRL-A+X.

- Login with ``root:firesim``  when launching with Firemarshal/Firesim.

- If you need to install precompiled packages you can boot the image in QEMU and first run ``sudo apt update && sudo apt upgrade``.
  Afterwards you can ``apt`` install anything you need. Make sure the rootfs size is large enough to run all the commands (default is 8GB).
