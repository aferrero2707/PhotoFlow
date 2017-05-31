    - cd build
    - wget https://pypi.python.org/packages/f4/ea/4faf47f49928ba276fc555b4ebd8432a77494a63fd7ee25b6bccb5820c67/crossroad-0.6.tar.gz
    - tar xzvf crossroad-0.6.tar.gz
    - cd crossroad-0.6 && ./setup.py install --prefix=$HOME/inst && cd ..
    #- sudo pip3 install crossroad
    - cat $HOME/inst/share/crossroad/scripts/cmake/toolchain-w64.cmake
    - sudo echo "" >> $HOME/inst/share/crossroad/scripts/cmake/toolchain-w64.cmake
    - sudo echo "SET(PKG_CONFIG_EXECUTABLE x86_64-w64-mingw32-pkg-config)" >> $HOME/inst/share/crossroad/scripts/cmake/toolchain-w64.cmake

rm -rf cmake_test
git clone https://github.com/aferrero2707/cmake_test.git && cd cmake_test && mkdir build && cd build && $HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release .. 
echo "================="
ls -l CMakeFiles/main.dir/link.txt
cat CMakeFiles/main.dir/link.txt
echo "================="
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
