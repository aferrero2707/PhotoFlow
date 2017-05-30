#! /bin/bash
#$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
ls /usr/lib/gcc
#exit

$HOME/inst/bin/crossroad install libtiff-devel libpng-devel libjpeg8-devel gtkmm2-devel liborc-devel libexiv2-devel liblcms2-devel libxml2-devel libxml2-tools libfftw3-3 fftw3-devel libexif-devel 
#$HOME/inst/bin/crossroad install libxml2-devel libxml2-tools

ls $HOME/.local/share/crossroad/roads/w64/phf-build/lib/pkgconfig
cat $HOME/.local/share/crossroad/roads/w64/phf-build/lib/pkgconfig/glib-2.0.pc

if [ $? -ne 0 ]; then 
	exit 1
fi
exit 0
