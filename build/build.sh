#! /bin/bash

rm CMakeCache.txt
#export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:PATH_TO_VIPS_INSTALL/lib/pkgconfig"

#cmake -DCMAKE_BUILD_TYPE=Debug ../
cmake -DCMAKE_BUILD_TYPE=Release ../

make
#make VERBOSE=1


