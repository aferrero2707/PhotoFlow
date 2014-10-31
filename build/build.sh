#! /bin/bash

rm CMakeCache.txt

if [ x"$1" = "xdebug" ]; then
    #export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:PATH_TO_VIPS_INSTALL_WITH_DEBUG/lib/pkgconfig"
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(pwd)/Debug  -DINSTALL_PREFIX=$(pwd)/Debug../
    make VERBOSE=1 install
else
    #export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:PATH_TO_VIPS_INSTALL/lib/pkgconfig"
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd)/Release -DINSTALL_PREFIX=$(pwd)/Release ../
    make install
fi


