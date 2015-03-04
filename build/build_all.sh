#! /bin/bash

vips_install=$(pwd)/VIPS/build
rebuild_VIPS=1
update_VIPS=1
if [ ${rebuild_VIPS} -eq 1 ]; then
		rm -rf ${vips_install}
		mkdir -p ${vips_install}
		cd VIPS

		if [ ${update_VIPS} -eq 1 ]; then
        if [ -e libvips ]; then
            cd libvips
            git pull
				    ./bootstrap.sh
        else
				    git clone https://github.com/jcupitt/libvips.git
				    cd libvips
				    ./bootstrap.sh
        fi
    else
				cd libvips
		fi

		FLAGS="-O2 -msse4.2 -ffast-math"
		FLAGS="$FLAGS -ftree-vectorize"
		CFLAGS="$FLAGS" CXXFLAGS="$FLAGS -fpermissive" \
				./configure --prefix=${vips_install} --disable-gtk-doc --disable-gtk-doc-html \
				--disable-introspection --enable-debug=no --without-python  --without-magick --without-libwebp\
				--enable-pyvips8=no --enable-shared=no --enable-static=yes
		if [ $? -ne 0 ]; then
				echo "VIPS configure failed"
				exit 1
		fi
    #make clean depclean
		make install
		if [ $? -ne 0 ]; then
				echo "VIPS compilation failed"
				exit 1
		fi

		cd ../..
fi

#rm -rf Release
mkdir -p Release
cd Release

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${vips_install}/lib/pkgconfig"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd) -DINSTALL_PREFIX=$(pwd) ../../ && make install
