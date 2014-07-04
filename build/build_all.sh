#! /bin/bash

vips_install=$(pwd)/VIPS/build
rm -rf ${vips_install}
mkdir -p ${vips_install}
cd VIPS

#rm -rf vips-7.40.3
#wget http://www.vips.ecs.soton.ac.uk/supported/current/vips-7.40.3.tar.gz
#tar xzvf vips-7.40.3.tar.gz
#cd vips-7.40.3

rm -rf libvips
git clone https://github.com/jcupitt/libvips.git
cd libvips
./bootstrap.sh

FLAGS="-O2 -msse4.2 -ffast-math"
FLAGS="$FLAGS -ftree-vectorize -fdump-tree-vect-details"
CFLAGS="$FLAGS" CXXFLAGS="$FLAGS -fpermissive" \
./configure --prefix=${vips_install} --disable-gtk-doc --disable-gtk-doc-html \
  --disable-introspection --enable-debug=no --enable-cxx=no --without-python
if [ $? -ne 0 ]; then
		echo "VIPS configure failed"
		exit 1
fi
make install
if [ $? -ne 0 ]; then
		echo "VIPS compilation failed"
		exit 1
fi

cd ../..

rm -f CMakeCache.txt

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${vips_install}/lib/pkgconfig"
cmake -DCMAKE_BUILD_TYPE=Release ../ && make

