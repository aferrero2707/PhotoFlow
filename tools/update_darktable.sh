#! /bin/bash

wd=$(pwd)

if [ -e $HOME/Source/dt-git/darktable ]; then
    cd $HOME/Source/dt-git/darktable
    git pull origin master
else
    mkdir -p $HOME/Source/dt-git
    cd $HOME/Source/dt-git
    git clone https://github.com/darktable-org/darktable.git
fi
dtdir=$HOME/Source/dt-git/darktable
cd "$wd"

rm -rf src/external/rawspeed
cp -a "$dtdir/src/external/rawspeed" src/external
cp "$dtdir/src/external/adobe_coeff.c" "$dtdir/src/external/wb_presets.c" src/dt/external
cp "$dtdir/src/common/colormatrices.c" src/dt/common
cp "$dtdir/src/common/srgb_tone_curve_values.h" src/dt/common
#"$dtdir/src/common/colorspaces.h"
#"$dtdir/src/common/colorspaces.c"

# Patches
sed -i .bak 's/TRUE/(boolean)TRUE/g' src/external/rawspeed/RawSpeed/DngDecoderSlices.cpp
sed -i .bak 's/return true/return (boolean)TRUE/g' src/external/rawspeed/RawSpeed/X3fDecoder.cpp
sed -i .bak 's/return false/return (boolean)FALSE/g' src/external/rawspeed/RawSpeed/X3fDecoder.cpp
#rm -f src/external/rawspeed/RawSpeed/DngDecoderSlices.cpp.bak
sed -i .bak 's%darktable%share/photoflow%g' src/external/rawspeed/CMakeLists.txt
#rm -f src/external/rawspeed/CMakeLists.txt.bak

rm -rf /tmp/dt-git

