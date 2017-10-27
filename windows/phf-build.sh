#! /bin/bash
arch=$1

mkdir -p /work/phf-build && cd /work/phf-build
echo "Compiling photoflow"
#$HOME/inst/bin/crossroad $arch phf-build <<EOF
crossroad cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUNDLED_LENSFUN=ON /sources && make -j 2 && make install
if [ $? -ne 0 ]; then exit 1; fi
#exit 0
#EOF
