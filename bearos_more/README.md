# more for BearOS

This is a simple text pager for BearOS. It's a simplified version of the
traditional Linux `more` utility. It's not as simple as it might first seem,
however, because it has to cope with a situation where the input source might
be redirected from another application. In that case, the keyboard control has
to be taken from the console device, rather than just stdin.
 
## Usage 

    more [-r rows] [-c columns] [files...]

If no file is specified, `more` reads from standard in.

