#! /bin/bash


gmicdir="$1"
wd=$(pwd)
cd "$gmicdir"
cp -a COPYING README "$wd"/src/vips/gmic/gmic
cd src
cp CImg.h gmic.cpp gmic.h "$wd"/src/vips/gmic/gmic/src
cp gmic_stdlib.gmic "$wd"/src/vips/gmic/gmic/src/gmic_def.gmic
