# hexdump for BearOS

This is a simple hexdump utility for BearOS, which works somewhat
like this Linux version. As well as dumping file contents, however, 
this version can also dump memory. 

## Usage

    hexdump [-C] [-n] [-m] [-s start] [-l length] [file]

`-C` makes `hexdump` display ASCII characters as well as hex. 

If `-m` is given, the memory is displayed. In this case, at least `-s`
must be given, to set the starting address. `-l` is the length in bytes
to dump.

If neither `-m` not a filename are given, `hexdump` reads from standard in.

Normally, successive rows of 16 bytes that have the same contents are
suppressed, and display as "\*\*\*". The `-n` switch turns off this
suppression. 

