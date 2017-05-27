#$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
#exit

$HOME/inst/bin/crossroad install pkg-config libtiff-devel libpng-devel libjpeg8-devel gtkmm2-devel liborc-devel libexiv2-devel liblcms2-devel libxml2-devel libfftw3-3 fftw3-devel libexif-devel 
#wget http://www.vips.ecs.soton.ac.uk/supported/8.4/vips-8.4.5.tar.gz
#tar xzvf vips-8.4.5.tar.gz
#(cd vips-8.4.5 && crossroad configure --disable-gtk-doc --disable-gtk-doc-html --disable-introspection --enable-debug=no --without-python --without-magick --without-libwebp && make && make install)
$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
