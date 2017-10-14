#! /bin/bash

mkdir -p /work/checkout
mkdir -p /work/build
cd /work
WD=$(pwd)
prefix=$WD/inst

export PATH=$prefix/bin:$PATH
export LD_LIBRARY_PATH=$prefix/lib:$LD_LIBRARY_PATH

(rm -rf Python-2.7.13* && wget https://www.python.org/ftp/python/2.7.13/Python-2.7.13.tar.xz && tar xJvf Python-2.7.13.tar.xz && cd Python-2.7.13 && ./configure --prefix=$prefix --enable-shared --enable-unicode=ucs2 && make && make install)

(rm -rf jhbuild && git clone https://github.com/GNOME/jhbuild.git && cd jhbuild && patch -p1 -i /sources/appimage/jhbuild-run-as-root.patch && ./autogen.sh --prefix=$prefix && make && make install)

which -a jhbuild

jhbuild -f "/sources/appimage/jhbuildrc" -m "$(pwd)/jhbuild/modulesets/gnome-suites-core-3.28.modules" build gtkmm-3

exit

export PKG_CONFIG_PATH=/app/lib/pkgconfig:${PKG_CONFIG_PATH}
export LD_LIBRARY_PATH=/app/lib:${LD_LIBRARY_PATH}

wget http://github.com/zeux/pugixml/releases/download/v1.8/pugixml-1.8.tar.gz
tar xzvf pugixml-1.8.tar.gz
cd pugixml-1.8
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON -DBUILD_PKGCONFIG=ON -DCMAKE_INSTALL_PREFIX=/app ..
make install
cd ../..

wget https://github.com/jcupitt/libvips/releases/download/v8.5.6/vips-8.5.6.tar.gz
tar xzvf vips-8.5.6.tar.gz
cd vips-8.5.6
#./autogen.sh
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
