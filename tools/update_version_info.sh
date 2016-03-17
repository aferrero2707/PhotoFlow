#! /bin/bash

release="$1"
version_file="$2"
branch="$3"
date="$4"
hash="$5"
bindir="$6"

echo "Updating PhotoFlow version info"
echo "bindir: $bindir"
update=0

if [ x"$release" = "x1" ]; then

	version=$(cat "${version_file}" | head -n 1)
	echo "#include <version.hh>" > "$bindir/version.cc.temp"
	echo -n "char* PF::version_string = \"\nPhotoFlow release version " >> "$bindir/version.cc.temp"
	echo -n "$version" >> "$bindir/version.cc.temp"
	echo "\";" >> "$bindir/version.cc.temp"

else

	echo "#include <version.hh>" > "$bindir/version.cc.temp"
	echo -n "char* PF::version_string = \"\nPhotoFlow development version\n\n" >> "$bindir/version.cc.temp"
	echo -n "Git branch: $branch\n\nGit commit date: $date\n\n" >> "$bindir/version.cc.temp"
	echo "Git commit hash: $hash\n\";" >> "$bindir/version.cc.temp"
	
fi

if [ ! -e "$bindir/version.cc" ]; then 
	echo "$bindir/version.cc not found"
	update=1
else
	test=$(diff --brief "$bindir/version.cc.temp" "$bindir/version.cc")
	if [ -n "$test" ]; then
		echo "$bindir/version.cc needs to be updated"
		update=1
	fi
fi

if [ x"$update" = "x1" ]; then
	cp "$bindir/version.cc.temp" "$bindir/version.cc"
	rm -f "$bindir/version.cc.temp"
	echo "PhotoFlow version info updated"
else
	echo "PhotoFlow version info already up-to-date"
fi

