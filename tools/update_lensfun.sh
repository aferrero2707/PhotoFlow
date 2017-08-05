#! /bin/bash

wd=$(pwd)

if [ -e $HOME/Source/lensfun ]; then
    cd $HOME/Source/lensfun
    git pull origin master
else
    mkdir -p $HOME/Source/
    cd $HOME/Source/
    git clone https://github.com/lensfun/lensfun.git
fi
lfdir=$HOME/Source/lensfun
cd "$wd"

rm -rf src/external/lensfun.bak
mv src/external/lensfun src/external/lensfun.bak
mkdir -p src/external/lensfun

cp -a "$lfdir" src/external/lensfun.tmp
mv src/external/lensfun.tmp/CMakeLists.txt src/external/lensfun	
mv src/external/lensfun.tmp/ChangeLog src/external/lensfun	
mv src/external/lensfun.tmp/README.md src/external/lensfun	
mv src/external/lensfun.tmp/apps src/external/lensfun		
mv src/external/lensfun.tmp/cmake src/external/lensfun		
mv src/external/lensfun.tmp/data src/external/lensfun		
mv src/external/lensfun.tmp/include src/external/lensfun		
mv src/external/lensfun.tmp/libs src/external/lensfun
mv src/external/lensfun.tmp/tools src/external/lensfun	
rm -rf src/external/lensfun.tmp

cp -a src/external/lensfun.bak/CMakeLists.txt src/external/lensfun
cp -a src/external/lensfun.bak/libs/lensfun/CMakeLists.txt src/external/lensfun/libs/lensfun
rm -rf src/external/lensfun.bak

# Patches
sed -i .bak 's|${CMAKE_INSTALL_FULL_DATAROOTDIR}/lensfun|${CMAKE_INSTALL_FULL_DATAROOTDIR}/photoflow/lensfun|g' src/external/lensfun/include/lensfun/config.h.in.cmake

for f in $(ls src/external/lensfun/libs/lensfun/*.cpp); do
	sed -i .bak 's|"config.h"|"lensfun/config.h"|g' "$f"
	sed -i .bak 's|"lensfun.h"|"lensfun/lensfun.h"|g' "$f"
done
