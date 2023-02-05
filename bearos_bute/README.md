# BUTE for BearOS

Bute is a Basically Usable Text Editor, which is about all that can be
said about it. It works, to the extent that it works at all, for editing
small numbers of small files.

## Usage

    bute [options] [files...]

Each file is opened in a new buffer. Buffers can be switched using
ctrl+E. If not filename is specified, BUTE opens an untitled buffer.

## Building BUTE

BUTE can be built for Linux, for testing purposes. However, it needs
the BearOS headers, because these are where the key codes are defined.
To build for Linux:

    BEAROS=/path/to/bearos/api/bearos make -f Makefile.linux

To build for BearOS. just 

    make

As with all BearOS utilities, building Bute requires the `arm-none-eabi-gcc` C
compiler, and specifif settings related to libraries.  These are set out in the
Makefiles.def file at the top of the BearOS utilities source bundle. 

## Basic operation

### Selection

To select text, use the arrow keys, or ctrl+arrow, while holding down the
shift key. ctrl+page keys also work, but these might be mapped onto specific
functions in a terminal emulator. Pressing an arrow key without shift 
cancels the selection. If any other key is pressed whilst a selection is
in effect, the selection is replaced by the key character.  

### Cut and paste

The 'cut' key combination is ctrl+Y. Ctrl+c is more common, but this 
is also used as an interrupt, and I didn't want to duplicate functions.

### Tabs

Bute respects tabs, and retains them as tabs, rather than converting them
to spaces. A tab stop is eight spaces wide; this cannot at present be
changed except by changing the code.

### Auto-indent

If a line begins with whitespace, pressing Enter will start the next line
with the same amount of whitespace. Bute will use the same kind of 
whitespace, that is, tabs or spaces. To disable this feature, use the
`-n` option.

If a block of text is selected, then pressing <tab> indents the whole
block, by two spaces.

### On-screen help

Press ctrl+@. I would rather have used something more mnemonic, like
ctrl+h, but this key combination generates a backspace.

### Undo, redo

Bute supports undo and redo operations, using ctrl+z and ctrl+r. 

## Notes

### -r, -c switches 

These set the size of the screen/terminal, in cases where it cannot be worked
out. Note that Bute requires to know the exact size of the screen -- these
options don't set the size you would like to use, but the size the screen 
actually is.

### Line length

Bute does not wrap lines that are longer than the screen. Instead, it 
scrolls the entire file content to the left, in five-column increments.

## Limitations

Oh, where to start... ?

The screen or terminal must have line wrapping enabled. That is, text written
off the right-hand end of one line must appear at thes start of the next line.
This is the default with Linux terminals, but not with all terminal emulators.
With Minicom, enter "ctrl-A W" to enable wrapping. 

BUTE expects to know the exact size of the screen/terminal. Its logic depends
on knowing this. Using a screen editor over a terminal connection is slow
at best, and I wanted to minimize the amount of control characters that
have to be written. This is easier when the exact size is known in advance.
It should go without saying that BUTE will not respond happily to changes
in the terminal size at runtime.

There are two many occasions where the whole screen is redrawn. This is
very noticeable when using a serial terminal.

## Acknowledgements

The basic logic of BUTE is dervied from an example editor by Michael Ringgaard,
released under a BSD licemce. The original source is here:

http://www.jbox.dk/downloads/edit.c

