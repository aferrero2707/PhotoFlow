#!/bin/bash

long_version="$(date +%Y%m%d)"
#long_version="0.2.7"
version=${long_version}
year=`date +%Y`
#copyright="© $year Andrea Ferrero”

cp ../icons/photoflow.png Icon1024.png
bash make_icon.sh

# photoflow.bundle writes here
rm -rf PhotoFlow
mkdir -p PhotoFlow
dst=$(pwd)/PhotoFlow/photoflow.app
dst_prefix=$dst/Contents/Resources

# jhbuild installs to here
src=~/gtk/inst

function escape () {
        # escape slashes
	tmp=${1//\//\\\/}

	# escape colon
	tmp=${tmp//\:/\\:}

	# escape tilda
	tmp=${tmp//\~/\\~}

	# escape percent
	tmp=${tmp//\%/\\%}

	echo -n $tmp
}

function new () {
	echo > script.sed
}

function sub () {
        echo -n s/ >> script.sed
	escape "$1" >> script.sed
	echo -n / >> script.sed
	escape "$2" >> script.sed
	echo /g >> script.sed
}

function patch () {
	echo patching "$1"

	sed -f script.sed -i "" "$1"
}

cp Info.plist.in Info.plist
new
sub @LONG_VERSION@ "$long_version"
sub @VERSION@ "$version"
sub @COPYRIGHT@ "$copyright"
patch Info.plist

rm -rf $dst 
rm -rf $(pwd)/PhotoFlow/photoflow-$version.app

PKG_CONFIG_PATH=$src/lib/pkgconfig PATH=$src/bin:$PATH JHBUILD_PREFIX=$src basedir=$(pwd)/PhotoFlow gtk-mac-bundler photoflow.bundle


ln -s /Applications $(pwd)/PhotoFlow

#cp $src/lib/pango/1.8.0/modules.cache $dst_prefix/lib/pango/1.8.0
#new
#sub "$src/lib/pango/1.8.0/modules/" ""
#patch $dst_prefix/lib/pango/1.8.0/modules.cache

#rm $dst_prefix/etc/fonts/conf.d/*.conf
#( cd $dst_prefix/etc/fonts/conf.d ; \
#	ln -s ../../../share/fontconfig/conf.avail/*.conf . )

# we can't copy the IM share with photoflow.bundle because it drops the directory
# name, annoyingly
#cp -r $src/share/ImageMagick-* $dst_prefix/share

#cp ~/PhotoFlow/vips/transform-7.30/resample.plg $dst_prefix/lib

#mv $dst ~/Desktop/PhotoFlow/photoflow-$version.app

echo built $(pwd)/PhotoFlow/photoflow.app

echo building .dmg
rm -f $(pwd)/photoflow-$version.app.dmg
size_MB=$(du -ms $(pwd)/PhotoFlow/photoflow.app | cut -f 1)
size_MB=$((size_MB+5))
echo "hdiutil create -megabytes ${size_MB} -srcfolder $(pwd)/PhotoFlow -o $(pwd)/photoflow-$version.app.dmg"
hdiutil create -megabytes ${size_MB} -verbose -srcfolder $(pwd)/PhotoFlow -o $(pwd)/photoflow-$version.app.dmg
echo built $(pwd)/photoflow-$version.app.dmg
