#! /bin/bash

cd /sources/build
export PKG_CONFIG_PATH=/app/lib/pkgconfig:${PKG_CONFIG_PATH}
export LD_LIBRARY_PATH=/app/lib:${LD_LIBRARY_PATH}
wget https://github.com/jcupitt/libvips/releases/download/v8.5.6/vips-8.5.6.tar.gz
tar xzvf vips-8.5.6.tar.gz
cd vips-8.5.6
./autogen.sh
FLAGS="-g -O2 -march=nocona -mno-sse3 -mtune=generic -ftree-vectorize" CFLAGS="${FLAGS}" CXXFLAGS="${FLAGS} -fpermissive" ./configure --prefix=/app --disable-gtk-doc --disable-gtk-doc-html --disable-introspection --enable-debug=no --without-python --without-magick --without-libwebp --enable-pyvips8=no --enable-shared=yes --enable-static=no
make -j2
sudo make install
cd ..
rm -rf libvips
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/app -DINSTALL_PREFIX=/app -DBUNDLED_LENSFUN=ON ..
if [ x"${TRAVIS_BRANCH}" != "xmaster" ]; then
    echo "#include <version.hh>" > ../src/version.cc || travis_terminate 1;
    echo "const char* PF::version_string = \"PhotoFlow git-${TRAVIS_BRANCH}-${TRAVIS_COMMIT}\";" >> ../src/version.cc || travis_terminate 1;
fi
make -j2
sudo make install
cd ..
