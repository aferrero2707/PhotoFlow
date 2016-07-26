#! /bin/bash


gmicdir="$1"
wd=$(pwd)
cd "$gmicdir"
cp -a COPYING README "$wd"/src/vips/gmic/gmic
cd src
cp -a CImg.h gmic.cpp gmic.h gmic_stdlib.gmic "$wd"/src/vips/gmic/gmic/src
