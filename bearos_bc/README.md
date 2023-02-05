# bc for BearOS

This is a crude port of the old GNU bc utility to BearOS. The port
mostly consisted of flattening the original directory structure and
doing the lex/yacc stuff manually, as I could not incorporate that
into my own build process.

I also implemented my own readline (line editing) function. Although
the ARM GCC has its own implementation of GNU readline, it doesn't work
when output is to a real terminal.
 
