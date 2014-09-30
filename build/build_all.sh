#! /bin/bash

vips_install=$(pwd)/VIPS/build
rebuild_VIPS=0
update_VIPS=1
if [ ${rebuild_VIPS} -eq 1 ]; then
		rm -rf ${vips_install}
		mkdir -p ${vips_install}
		cd VIPS

		if [ ${update_VIPS} -eq 1 ]; then
        rm -rf vips-*
        wget http://www.vips.ecs.soton.ac.uk/supported/current/vips-7.40.8.tar.gz
        tar xzvf vips-7.40.8.tar.gz
        cd vips-7.40.8
				#rm -rf libvips
				#git clone https://github.com/jcupitt/libvips.git
				#cd libvips
				#./bootstrap.sh
		fi

		FLAGS="-O2 -msse4.2 -ffast-math"
		FLAGS="$FLAGS -ftree-vectorize"
		CFLAGS="$FLAGS" CXXFLAGS="$FLAGS -fpermissive" \
				./configure --prefix=${vips_install} --disable-gtk-doc --disable-gtk-doc-html \
				--disable-introspection --enable-debug=no --without-python  --without-magick\
				--enable-shared=no --enable-static=yes
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
fi

rm -rf Release
mkdir -p Release
cd Release

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${vips_install}/lib/pkgconfig"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd) ../../ && make install
