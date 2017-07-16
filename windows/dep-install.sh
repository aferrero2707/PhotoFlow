#! /bin/bash
#$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
#ls /usr/lib/gcc
#exit

#for counter in $(seq 1 10); do
echo "try 1"
$HOME/inst/bin/crossroad install libtiff5 libtiff-devel libpng16-16 libpng-devel libjpeg8 libjpeg8-devel libgtkmm-2_4-1 gtkmm2-devel libexpat1 libexpat-devel win_iconv win_iconv-devel liblcms2-2 liblcms2-devel libxml2-2 libxml2-devel libxml2-tools libfftw3-3 fftw3-devel libexif12 libexif-devel  
#$HOME/inst/bin/crossroad install libxml2-devel libxml2-tools
#if [ $? -eq 0 -o $counter -eq 10 ]; then 
#	break
#fi
#done

#ls $HOME/.local/share/crossroad/roads/w64/phf-build/lib/pkgconfig
#cat $HOME/.local/share/crossroad/roads/w64/phf-build/lib/pkgconfig/libxml-2.0.pc

if [ $? -ne 0 ]; then 
	exit 1
fi
exit 0
