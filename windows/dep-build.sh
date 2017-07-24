#! /bin/bash
arch=$1

echo "Compiling dependencies"
$HOME/inst/bin/crossroad $arch phf-build <<EOF
# PugiXML
git clone https://github.com/zeux/pugixml.git && cd pugixml 
if [ $? -ne 0 ]; then exit 1; fi
$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON && make && make install
if [ $? -ne 0 ]; then exit 1; fi

cd ..

# VIPS
wget https://github.com/jcupitt/libvips/releases/download/v8.5.6/vips-8.5.6.tar.gz
if [ $? -ne 0 ]; then exit 1; fi

tar xzvf vips-8.5.6.tar.gz
if [ $? -ne 0 ]; then exit 1; fi

cd vips-8.5.6 && $HOME/inst/bin/crossroad configure --disable-gtk-doc --disable-gtk-doc-html --disable-introspection --enable-debug=no --without-python --without-magick --without-libwebp  --without-orc && make -j 3 && make install
if [ $? -ne 0 ]; then exit 1; fi

exit 0
EOF
