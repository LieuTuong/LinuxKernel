This project is a really simple example of how to hook system call functions through the sys\_call\_table.

If you're looking to legitimately hook system calls, I recommend first checking out something like the Linux Security Modules (LSM) framework or the Linux Kernel Tracepoints subsystem. These require either a complete kernel rebuild or user-enabled kernel debugging, respectively, to work.

If you're building a rootkit or want to place hooks from a loadable kernel module regardless of whether or not debugging has been enabled and don't want to recompile the kernel, you've come to the right place. This shit is mad janky though and can totally fuck your system up so use at your own risk.

This module was built and tested on Ubuntu 14.04 LTS.
