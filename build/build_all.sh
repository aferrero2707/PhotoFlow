#! /bin/bash
#  Script:    build_all.sh
#  Purpose:   Update (optionally) and build libvips. Build PhotoFlow.
#  Call:      build_all.sh <Build-Type>
#             Build-Type: debug for a Debug Build,
#                         test  for a Test Build,
#                         else for a Release Build.
#  Environment Variables:
#  BUILD_TYPE     : the Build-Type as above. Optional.
#  CLEAN_VIPS     : 1 if VIPS shall be built from scratch. Optional.
#                   Disabled by default.
#  INSTALL_PREFIX : the root directory for VIPS' and PhotoFlow's builds.
#                   Optional. If omitted, then this is the current directory.
#  UPDATE_VIPS    : 1 if VIPS shall be updated before build. Optional.
#                   Enabled by default.
#  BUILD_VIPS     : 1 if VIPS shall be built. Optional.
#                   Enabled by default.
#  Author, License:
#    Copyright (C) 2014, 2016 Ferrero Andrea, Sven Claussner
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.

# Evaluate arguments and environment variables.
## BUILD_TYPE: use environment variable BUILD_TYPE, $1 or Release (in this order)
BUILD_TYPE=${BUILD_TYPE:-${1:-Release}}
## Capitalize build_type to tolerate misspellings.
export BUILD_TYPE=\
$(echo ${BUILD_TYPE} | tr '[:upper:]' '[:lower:]' | sed 's/.*/\u&/')

if [ "${BUILD_TYPE}" == "Debug" ]; then
    vips_enable_debug="yes"
else
    vips_enable_debug="no"
fi

## INSTALL_PREFIX: use environment variable INSTALL_PREFIX or current directory
## (in this order)
export INSTALL_PREFIX=${INSTALL_PREFIX:-$(pwd)}

## Install VIPS to a separate directory than PhotoFlow.
export VIPS_INSTALL_PREFIX=${INSTALL_PREFIX}/VIPS/${BUILD_TYPE}

## Report detected settings.
echo Building VIPS...
echo Install VIPS to: ${VIPS_INSTALL_PREFIX}
echo Build type: ${BUILD_TYPE}
echo Include debug information: ${vips_enable_debug}

## UPDATE_VIPS: use environment variable or default value 1.
update_VIPS=${UPDATE_VIPS:-1}
if [ ${update_VIPS} -eq 1 ]; then
    echo "Update VIPS: yes"
else
    echo "Update VIPS: no"
fi

## CLEAN_VIPS: use environment variable or default value 0.
clean_VIPS=${CLEAN_VIPS:-0}
if [ ${clean_VIPS} -eq 1 ]; then
    echo "Clean VIPS source directory: yes"
else
    echo "Clean VIPS source directory: no"
fi

## BUILD_VIPS: use environment variable or default value 1.
build_VIPS=${BUILD_VIPS:-1}
if [ ${build_VIPS} -eq 1 ]; then
    echo "Build VIPS: yes"
else
    echo "Build VIPS: no"
fi

# Rebuild and conditionally update.
if [ ${build_VIPS} -eq 1 ]; then
    # store current directory
    export OLD_PWD=$(pwd)

    # clean up
    rm -rf ${VIPS_INSTALL_PREFIX}
    mkdir -p ${VIPS_INSTALL_PREFIX}

    # get update
    if [ ${update_VIPS} -eq 1 ]; then
        ../tools/update_libvips.sh
    fi
    cd ../libvips

    # run libvips bootstrapper
    ./autogen.sh

    export FLAGS="-O2 -msse4.2 -ffast-math"
    export FLAGS="${FLAGS} -ftree-vectorize"
    export CFLAGS="${FLAGS}"
    export CXXFLAGS="${FLAGS} -fpermissive"
    ./configure --prefix=${VIPS_INSTALL_PREFIX} \
    --disable-gtk-doc --disable-gtk-doc-html \
    --disable-introspection --enable-debug==${vips_enable_debug} \
    --without-python --without-magick --without-libwebp \
    --enable-pyvips8=no --enable-shared=yes --enable-static=no
    if [ $? -ne 0 ]; then
        echo "VIPS configure failed"
        exit 1
    fi

    if [ ${clean_vips} -eq 1 ]; then
        make clean depclean
    fi

    make
    if [ $? -ne 0 ]; then
        echo "VIPS compilation failed"
        exit 1
    fi

    make install
    if [ $? -ne 0 ]; then
        echo "VIPS installation failed"
        exit 1
    fi

    # change back to former directory
    cd -
fi

# Exit this script and continue with the PhotoFlow build.
exec ./build.sh
