#! /bin/bash

wd=$(pwd)

for f in $(ls src/external/exiv2/src/*.cpp src/external/exiv2/src/*.h* src/external/exiv2/include/exiv2/*.h*); do
	sed -i .bak "s|\"config.h\"|\"config_exiv2.h\"|g" "$f"
done
