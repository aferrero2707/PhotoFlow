#!/bin/bash

long_version="-${TRAVIS_BRANCH}-$(date +%Y%m%d)-${TRAVIS_COMMIT}"
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
#src=/usr/local

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

# transfer.sh
function transfer() {
  if [ $# -eq 0 ]; then echo "No arguments specified. Usage:\necho transfer /tmp/test.md\ncat /tmp/test.md | transfer test.md"; return 1; fi 
  tmpfile=$( mktemp -t transferXXX ); 
#  if tty -s; then 
    basefile=$(basename "$1" | sed -e 's/[^a-zA-Z0-9._-]/-/g'); 
    echo "curl --progress-bar --upload-file \"$1\" \"https://transfer.sh/$basefile\""
    curl --progress-bar --upload-file "$1" "https://transfer.sh/$basefile" >> $tmpfile;   
#  else 
#    echo "curl --progress-bar --upload-file \"-\" \"https://transfer.sh/$1\""
#    curl --verbose --progress-bar --upload-file "-" "https://transfer.sh/$1" >> $tmpfile ; 
#  fi; 
  cat $tmpfile; 
  rm -f $tmpfile; 
}

pwd
#transfer ../icons/photoflow.png
#exit


cp Info.plist.in Info.plist
new
sub @LONG_VERSION@ "$long_version"
sub @VERSION@ "$version"
sub @COPYRIGHT@ "$copyright"
patch Info.plist

git clone git://git.gnome.org/gtk-mac-bundler
cd gtk-mac-bundler
make install
cd ..

export PATH=$HOME/.local/bin:$PATH

rm -rf $dst 
rm -rf $(pwd)/PhotoFlow/photoflow-$version.app

ls $(pwd)/PhotoFlow

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

########################################################################
# Upload the AppDir
########################################################################

transfer $(pwd)/photoflow-$version.app.dmg
echo "DMG has been uploaded to the URL above; use something like GitHub Releases for permanent storage"
