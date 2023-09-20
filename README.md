
# True Type Font to Bitmap Image (TTF2BMP)
## Convert true type fonts to monospace images
Usage
 - `ttf2bmp -h` `ttf2bmp --help` - Help message
 - `ttf2bmp -io input.ttf output.png` - Conversion
## Dependencies
 - Freetype2
 - libpng
## Building
 - Recommended GCC compiler version 13
 - Include freetype and system libraries
   - Currently only tested on linux
 - Provided Makefile should work fine given the libaries exist and directories are changed to represent your environment
## Notes
 - Currently under vigorous testing and bug fixing
 - Not all additional arguments (seen in -h/--help) work as expected
 - Must install to a $PATH directory
   - I don't recommend adding to /bin, project is still unstable
### See TODO.md
