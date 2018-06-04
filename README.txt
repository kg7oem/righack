righack - swiss army blender for ham radios

This is a work in progress. Really I'm just publishing my own experiments.
Things may change at any time. Nothing is guranteed. With out
any other communication with me there will be no attempt to avoid
breaking others.


KERNEL PATCHING

Righack can offer a virtual serial port via pseudoterminals but changes to the
hardware flow control lines can not be detected well with out a special kernel.
The problem is that at least an unmodified Linux will not allow the TIOCMGET and
TIOCMSET ioctl on a slave PTY fd. This is expected to be a limitation of all
Unix implementations.

In the kernel/ directory is an experimental patch for Linux 4.15.0-rc7 which
enables the required ioctls on a Unix98 PTY slave fd; previous versions of Linux
may work but are not tested. Patches for other Unix flavors are very welcome but
none exist yet.


LEGAL STUFF

This software is being created by Tyler Riddle KG7OEM <cardboardaardvark@gmail.com>.
The current license is GPL3 but I'm not sure I will stick with it in the long
term. Contributions are welcome but any patch submitted for inclusion in the
project must include a statement that a future license change is ok. See
doc/contributing.txt for information.

This project includes some third party software that is licensed under
different terms. This software may include local modifications. See the
src/external directory for this source code.

  * tomlc99 https://github.com/cktan/tomlc99
    Copyright (c) 2017 CK Tan
    MIT License
