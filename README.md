PhotoFlow
=========

The aim of the project is to provide a fully non-destructive photo retouching program with a complete workflow including RAW image development.

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

# Windows and OSX installers

Up-to-date Windows and OSX installers are available from the [releases](https://github.com/aferrero2707/PhotoFlow/releases) web page.

# Installing from PPA (Ubuntu Linux and derivate distributions)

There are up-to-date packages available for Ubuntu 14.04/14.10 and Mint 17/17.1 in Dariusz Duma's PPA.
To add the PPA and install photoflow, do the following:

        sudo add-apt-repository ppa:dhor/myway
        sudo apt-get update
        sudo apt-get install photoflow

# Fedora packages

User Oleastre is providing up-to-date VIPS and PhotoFlow packages for Fedora.

To install PhotoFlow, just execute the following commands as root:

        dnf copr enable oleastre/PhotoFlow
        dnf install photoflow

Additional details can be found here: https://copr.fedoraproject.org/coprs/oleastre/PhotoFlow/

# Downloading and compiling

Photoflow can be compiled under both gtk 2.x and gtk 3.x.

The pixel rendering engine of PhotoFlow is based on VIPS (http://www.vips.ecs.soton.ac.uk). You have to install a recent version of VIPS in order to compile PhotoFlow.

Once VIPS is set up and running, follow these steps to download and compile PhotoFlow:

1. get master branch from GitHub:

        $ git clone https://github.com/aferrero2707/PhotoFlow.git

2. The build system is based on CMake, so you have to have it installed in your system.
   You need the following dependencies to build Photoflow: gettext, glib-2.x, libxml-2.0, pkg-config, swig, gtk-doc-tools, automake, gobject-introspection, gnu make, cmake, libtiff, libjpeg, libfftw3, exiv2, lensfun and gtkmm-2.x or gtkmm-3.x.
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

        $ ./Release/bin/photoflow image_file_name
        

# Current status

PhotoFlow is in a early development stage. The present version allows to open an image file and apply basic editing filters via non-destructive adjustment layers. The individual layers can be activated and de-activated using the radio buttons on the right of the image. Moreover, the layers structure can be saved to disk and re-opened again via the command line.
Loading and processing of RAW images (demosaicing, white balance, exposure adjustment, etc.) is now available, at least at a basic level.

# Roadmap and development plans

The following list shows what features are currently planned or being implemented, in a kind of priority order:

- Implement colorpsace conversions based on ICC profiles (partly implemented already)

- Complete the list of supported blending modes

- Pencil tool for simple hand drawing (almost completely implemented)


