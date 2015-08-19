#!/bin/bash

#long_version=`date +%Y%m%d`
long_version="0.1.6"
version=${long_version}
year=`date +%Y`
#copyright="© $year Andrea Ferrero”

cp ../../Icon/photoflow.png Icon1024.png
bash make_icon.sh

# photoflow.bundle writes here
rm -rf ~/Desktop/PhotoFlow
mkdir -p ~/Desktop/PhotoFlow
dst=~/Desktop/PhotoFlow/photoflow.app
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
rm -rf ~/Desktop/PhotoFlow/photoflow-$version.app

jhbuild run gtk-mac-bundler photoflow.bundle

ln -s /Applications ~/Desktop/PhotoFlow

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

echo built ~/Desktop/PhotoFlow/photoflow.app

echo building .dmg
rm -f ~/Desktop/photoflow-$version.app.dmg
size_MB=$(du -ms ~/Desktop/PhotoFlow/photoflow.app | cut -f 1)
size_MB=$((size_MB+5))
echo "hdiutil create -megabytes ${size_MB} -srcfolder ~/Desktop/PhotoFlow -o ~/Desktop/photoflow-$version.app.dmg"
hdiutil create -megabytes ${size_MB} -verbose -srcfolder ~/Desktop/PhotoFlow -o ~/Desktop/photoflow-$version.app.dmg
echo built ~/Desktop/photoflow-$version.app.dmg
