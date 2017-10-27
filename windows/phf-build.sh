#! /bin/bash
arch=$1

echo "Compiling photoflow"
#$HOME/inst/bin/crossroad $arch phf-build <<EOF
crossroad cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUNDLED_LENSFUN=ON .. && make -j 3 && make install
if [ $? -ne 0 ]; then exit 1; fi
#exit 0
#EOF
