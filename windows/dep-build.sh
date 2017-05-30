rm -rf cmake_test
git clone https://github.com/aferrero2707/cmake_test.git && cd cmake_test && mkdir build && cd build && $HOME/inst/bin/crossroad cmake .. 
ls -l CMakeFiles/main.dir/link.txt
cat CMakeFiles/main.dir/link.txt
make VERBOSE=1

exit 0 

# PugiXML
git clone https://github.com/zeux/pugixml.git && cd pugixml 
if [ $? -ne 0 ]; then exit 1; fi
$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON && make && make install
if [ $? -ne 0 ]; then exit 1; fi

cd ..

# VIPS
wget http://www.vips.ecs.soton.ac.uk/supported/8.4/vips-8.4.5.tar.gz
if [ $? -ne 0 ]; then exit 1; fi

tar xzvf vips-8.4.5.tar.gz
if [ $? -ne 0 ]; then exit 1; fi

cd vips-8.4.5 && $HOME/inst/bin/crossroad configure --disable-gtk-doc --disable-gtk-doc-html --disable-introspection --enable-debug=no --without-python --without-magick --without-libwebp && make -j 3 && make install
if [ $? -ne 0 ]; then exit 1; fi

exit 0
