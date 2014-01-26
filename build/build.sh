#! /bin/bash

rm CMakeCache.txt

if [ x"$1" = "xdebug" ]; then
    #export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:PATH_TO_VIPS_INSTALL_WITH_DEBUG/lib/pkgconfig"
    cmake -DCMAKE_BUILD_TYPE=Debug ../
    make VERBOSE=1
else
    #export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:PATH_TO_VIPS_INSTALL/lib/pkgconfig"
    cmake -DCMAKE_BUILD_TYPE=Release ../
    make
fi


