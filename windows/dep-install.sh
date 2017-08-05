#! /bin/bash
#$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
#ls /usr/lib/gcc
#exit

#for counter in $(seq 1 10); do
arch=$1

echo "Installing packages"
$HOME/inst/bin/crossroad $arch phf-build <<EOF
$HOME/inst/bin/crossroad install libtiff5 libtiff-devel libpng16-16 libpng-devel libjpeg8 libjpeg8-devel libgtkmm-2_4-1 gtkmm2-devel libexpat1 libexpat-devel win_iconv win_iconv-devel liblcms2-2 liblcms2-devel libxml2-2 libxml2-devel libxml2-tools libfftw3-3 fftw3-devel libexif12 libexif-devel  
exit 0
EOF