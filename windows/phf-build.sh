#! /bin/bash
arch=$1

update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

/usr/bin/x86_64-w64-mingw32-gcc -v

mkdir -p /work/phf-build && cd /work/phf-build
echo "Compiling photoflow"
#$HOME/inst/bin/crossroad $arch phf-build <<EOF
crossroad cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUNDLED_LENSFUN=ON /sources && make -j 2 && make install
if [ $? -ne 0 ]; then exit 1; fi
cd /sources
./windows/package-w64.sh
#exit 0
#EOF
