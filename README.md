PhotoFlow
=========

The aim of the project is to provide a fully non-destructive photo retouching program that includes a complete workflow from RAW image development to high-quality printing.

Here is the general feature (whish)list:

- Fully non-destructive, layer-based photo editing workflow with realtime preview of the final image
- Support for 8-bits and 16-bits integer as well as 32-bits and 64-bits floating point precision, selectable at runtime and on a per-image basis
- Plugin-based architecture: new tools can be implemented as separate modules that are loaded at runtime
- Allows to load and edit images of arbitrary size, thanks to the underlying rendering engine based on the VIPS library
- Fully color managed workflow: user-defined input, work and output profiles, soft-proofing, etc...
- Support for layer grouping and layer masks
- Support for common photo editing tools: levels, curves, brightness-contrast control, blurring, sharpening, cropping, resizing, colorspace conversions, etc..., all implemented in the form of image layers
- User-friendly interface to develop new tools and image filters as external plugins

You can follow the development of photoflow and learn about its usage in the dedicated blog: http://photoflowblog.blogspot.fr/

# Downloading and compiling

Photoflow can be compiled under both gtk 2.x and gtk 3.x.

The pixel rendering engine of PhotoFlow is based on VIPS (http://www.vips.ecs.soton.ac.uk). You have to install a recent version of VIPS in order to compile PhotoFlow.

Once VIPS is set up and running, follow these steps to download and compile PhotoFlow:

1. get master branch from GitHub:

        $ git clone https://github.com/aferrero2707/PhotoFlow.git

2. The build system is based on CMake, so you have to have it installed in your system.
   PhotoFlow provides an helper script to simplify the building process;
   in order to use it you have to go into the "build" subdirectory and run the "build.sh" script:

        $ cd PhotoFlow/build
        $ bash ./build.sh
   
   If you have installed VIPS in an unusual place, you can edit the build script and set the PKG_CONFIG_PATH variable accordingly

   To compile the code in debug mode, use
   
        $ bash ./build.sh debug
   
   instead.
   
   If you do not already have a very recent version of VIPS installed in your system, a second helper script is provided 
   which will download, configure and compile libvips for you, and link it against photoflow:
   
        $ cd PhotoFlow/build
        $ bash ./build_all.sh
   

3. If all goes well, you can now open an image file with PhotoFlow:

        $ ./Release/photoflow image_file_name

   There are some test images available:

        $ ./Release/photoflow ../testimages/Lab_curves.pfi
   or

        $ ./Release/photoflow ../testimages/orton.pfi
        

# Current status

PhotoFlow is in a early development stage. The present version allows to open an image file and apply basic editing filters via non-destructive adjustment layers. The individual layers can be activated and de-activated using the radio buttons on the right of the image. Moreover, the layers structure can be saved to disk and re-opened again via the command line.
Loading and processing of RAW images (demosaicing, white balance, exposure adjustment, etc.) is now available, at least at a basic level.

# Roadmap and development plans

The following list shows what features are currently planned or being implemented, in a kind of priority order:

- Implement colorpsace conversions based on ICC profiles (partly implemented already)

- Complete the list of supported blending modes

- Pencil tool for simple hand drawing (almost completely implemented)


