# BearOS utilities

For more information about 'BearOS', please see my separate repository:

https://github.com/kevinboone/bearos

This is a collection of simple utilities that demonstrate how to cross-compile
(under Linux) programs for BearOS for the Raspberry Pi Pico. Each module has a
`Makefile` for BearOS, and most have a `Makefile.linux` for building the same
code on Linux. Some of these utilities were written specifically for BearOS,
but others are ports of simple(-ish) Linux programs.

To build these programs, you'll need to edit `Makefile.defs` to indicate the
locations of the BearOS `api/` directory, and the various compiler libraries.

To build the entire bundle, just run `make`. `make stage` copies all the
generated binaries to a directory `rootfs`, which can be copied to top level of
an SD card, for use with BearOS. 

