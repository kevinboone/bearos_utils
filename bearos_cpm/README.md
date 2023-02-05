# CP/M for BearOS

`cpm` is a CP/M emulator for BearOS, substantially based on the work of Joseph
Allen and Parag Patel. 

Under emulation on a Pico, `cpm` runs CP/M programs at about the same speed
as an original 4MHz Z80 machine would, which provides an added measure
of realism.

## Usage

    cpm [program.cpm]
    cpm

When used without arguments, the emulator will show the `A>` prompt.  A program
name can be entered here, as with real CP/M.

## Notes

CP/M takes the current working directory to be drive A. CP/M itself has no
support for directories.

`cpm` emulates a machine with 64kB RAM.

CP/M applications were traditionally fussy about terminal set-up, and this
remains the case -- but worse -- under an emulator.

