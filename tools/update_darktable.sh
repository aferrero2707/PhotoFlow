#! /bin/bash

wd=$(pwd)

if [ -e $HOME/Source/dt-git/darktable ]; then
    cd $HOME/Source/dt-git/darktable
    git pull origin master
    git submodule update --init --recursive
else
    mkdir -p $HOME/Source/dt-git
    cd $HOME/Source/dt-git
    git clone --recursive https://github.com/darktable-org/darktable.git
fi
dtdir=$HOME/Source/dt-git/darktable
cd "$wd"

rm -rf src/external/rawspeed

echo "cp $dtdir/cmake/modules/CheckCompilerFlagAndEnableIt.cmake cmake/Modules"
cp "$dtdir/cmake/modules/CheckCompilerFlagAndEnableIt.cmake" cmake/Modules

echo "cp $dtdir/cmake/modules/CheckCCompilerFlagAndEnableIt.cmake cmake/Modules"
cp "$dtdir/cmake/modules/CheckCCompilerFlagAndEnableIt.cmake" cmake/Modules 

echo "cp $dtdir/cmake/modules/CheckCXXCompilerFlagAndEnableIt.cmake cmake/Modules"
cp "$dtdir/cmake/modules/CheckCXXCompilerFlagAndEnableIt.cmake" cmake/Modules 

cp -a "$dtdir/src/external/rawspeed" src/external
cp src/external/rawspeed/cmake/Modules/*.cmake cmake/Modules

cp "$dtdir/src/external/adobe_coeff.c" "$dtdir/src/external/wb_presets.c" src/dt/external
cp "$dtdir/src/common/colormatrices.c" src/dt/common
cp "$dtdir/src/common/srgb_tone_curve_values.h" src/dt/common
#"$dtdir/src/common/colorspaces.h"
#"$dtdir/src/common/colorspaces.c"

# Patches
#sed -i .bak 's/TRUE/(boolean)TRUE/g' src/external/rawspeed/RawSpeed/DngDecoderSlices.cpp
#sed -i .bak 's/return true/return (boolean)TRUE/g' src/external/rawspeed/RawSpeed/X3fDecoder.cpp
#sed -i .bak 's/return false/return (boolean)FALSE/g' src/external/rawspeed/RawSpeed/X3fDecoder.cpp
#sed -i .bak 's|ThreadSafetyAnalysis.h|../external/ThreadSafetyAnalysis.h|g' src/external/rawspeed/src/librawspeed/common/Mutex.h
#rm -f src/external/rawspeed/RawSpeed/DngDecoderSlices.cpp.bak
sed -i .bak 's%darktable%photoflow%g' src/external/rawspeed/data/CMakeLists.txt
#rm -f src/external/rawspeed/CMakeLists.txt.bak

rm -rf /tmp/dt-git

