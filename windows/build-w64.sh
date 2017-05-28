#$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make && make install
ls /usr/lib/gcc
#exit

$HOME/inst/bin/crossroad install pkg-config libtiff-devel libpng-devel libjpeg8-devel gtkmm2-devel liborc-devel libexiv2-devel liblcms2-devel libxml2-devel libxml2-tools libfftw3-3 fftw3-devel libexif-devel 

# PugiXML
(git clone https://github.com/zeux/pugixml.git && cd pugixml && $HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON && make && make install)

# VIPS
wget http://www.vips.ecs.soton.ac.uk/supported/8.4/vips-8.4.5.tar.gz
tar xzvf vips-8.4.5.tar.gz
(cd vips-8.4.5 && $HOME/inst/bin/crossroad configure --disable-gtk-doc --disable-gtk-doc-html --disable-introspection --enable-debug=no --without-python --without-magick --without-libwebp && make -j 3 && make install)

#PhotoFlow
$HOME/inst/bin/crossroad cmake -DCMAKE_BUILD_TYPE=Release -DBUNDLED_LENSFUN=ON .. && make -j 3 && make install
