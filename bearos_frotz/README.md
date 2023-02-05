## Frotz for BearOS

This is a minimal port of Frotz -- the Z-machine game interpreter -- to BearOS.
In order to reduce memory requirements, it has no screen formatting, no support
for Blorb files, no sound, etc.
 
Frotz is not designed for machines with as little memory as a Pico. It loods
the entire game file into memory at the start, and keeps it there for the
duration. Most of the original Infocom games are, in fact, larger in size that
the whole of the Pico's RAM and, therefore, stand no chance of working. Zork
III, at 83kB, _just_ works.

A better -- albeit slower -- way to play these old games on BearOS is to use
the CP/M versions under the `cpm` emulator. The CP/M versions are designed to
run in low RAM.

