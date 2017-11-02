#! /bin/bash
arch=$1

mkdir -p /work/w64-build && cd /work/w64-build
echo "Compiling photoflow"
#$HOME/inst/bin/crossroad $arch phf-build <<EOF
crossroad cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUNDLED_LENSFUN=ON /sources && make -j 2 && make install
if [ $? -ne 0 ]; then exit 1; fi
#cd /sources
#./windows/package-w64.sh
exit 0
#EOF
