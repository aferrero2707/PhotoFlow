#! /bin/bash
#  UPDATE_VIPS    : 1 if VIPS shall be updated before build. Optional.
#                   Enabled by default.

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
        else
				    git clone https://github.com/jcupitt/libvips.git
				    cd libvips
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
## UPDATE_VIPS: use environment variable or default value 1.
update_VIPS=${UPDATE_VIPS:-1}
if [ ${update_VIPS} -eq 1 ]; then
    echo "Update VIPS: yes"
else
    echo "Update VIPS: no"
fi

#rm -rf Release
mkdir -p Release
cd Release
    # get update
    if [ ${update_VIPS} -eq 1 ]; then
        ../tools/update_libvips.sh
    fi
    cd ../libvips

    # run libvips bootstrapper
    ./autogen.sh
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${vips_install}/lib/pkgconfig"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd) -DINSTALL_PREFIX=$(pwd) ../../ && make install
