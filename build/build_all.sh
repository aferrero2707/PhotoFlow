#! /bin/bash

rm -rf VIPS
vips_install=$(pwd)/VIPS/build
rm -rf ${vips_install}
mkdir -p ${vips_install}
cd VIPS

vips_version=7.40.8
rm -rf vips-*
wget http://www.vips.ecs.soton.ac.uk/supported/7.40/vips-${vips_version}.tar.gz
tar xzvf vips-${vips_version}.tar.gz
cd vips-${vips_version}

#rm -rf libvips
#git clone https://github.com/jcupitt/libvips.git
#cd libvips
#./bootstrap.sh

FLAGS="-O2 -msse4.2 -ffast-math"
FLAGS="$FLAGS -ftree-vectorize -fdump-tree-vect-details"
CFLAGS="$FLAGS" CXXFLAGS="$FLAGS -fpermissive" \
./configure --prefix=${vips_install} --disable-gtk-doc --disable-gtk-doc-html \
    --enable-shared=no --enable-static=yes \
    --disable-introspection --enable-debug=no --without-python
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
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd)/Release ../ && make install

