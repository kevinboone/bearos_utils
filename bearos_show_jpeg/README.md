# show-jpeg for BearOS

## What is this?

`show-jpeg` is a slow, crude command-line utility for displaying JPEG files
on a BearOS graphics device. It's crude because

- It can't scale or otherwise process images, and
- It only supports non-progressive, RGB JPEGs, and
- It only supports display devices with RBB565 colour model.

The last point would be easy to fix, the others less so. The problem is
that most JPEG decoders need more RAM than the Pico can muster. `libjpeg`
will build, but it will try to allocate enough RAM to store the entire
image. Even with changes to the code to limit this RAM gluttony, it still
wants to load at least 8 rows at a time, and that alone needs more
RAM than the Pico has.

So I'm using `picojpeg`, which works fine; but it's limited in not 
supporting progressive-mode JPEGs. I can't find any existing code that
will handle progressive JPEGs in the Pico's RAM.

What makes the utility slow is all the bit-juggling that is needed to
reorganize the pixels to go to the right place on the display device.
The colour-space conversion is also CPU-intensive.

As a result, it takes about four seconds to display a 480x320 image.

Still, given a suitable JPEG, it _does_ work. 

## Legal

The command-line interface and BearOS integration are by Kevin Boone,
released under the terms of the GPL v3.0. The utility contains public-domain
components originally written by Rich Geldreich and Chris Phoenix.

