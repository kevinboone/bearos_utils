# show-jpeg

A simple utility for displaying certain JPEG files on a BearOS graphics
device.

## Usage

    show-jpeg [options] {file}
       -b:   blank display first
       -d:   graphics device (default p:/gfx)
       -i:   show file information
       -v:   show version

## Notes

`show-jpeg` can display non-progressive, RGB JPEG files. Progressive 
format is more widely used, but many tools (e.g., Gimp) are able to 
convert them to non-progressive. Greyscale JPEG files are not yet supported.

`show-jpeg` does not scale. If the file contains an image larger than the
display device, only the top-left corner will be displayed.  

