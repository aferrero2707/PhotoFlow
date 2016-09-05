#! /bin/bash
#  Script:    build.sh
#  Purpose:   Build PhotoFlow, create starter scripts.
#  Call:      build.sh <Build-Type>
#             Build-Type: debug for a Debug Build,
#                         test  for a Test Build,
#                         else for a Release Build.
#  Environment Variables:
#  BUILD_TYPE     : the Build-Type as above. Optional.
#  INSTALL_PREFIX : the root directory for PhotoFlow's release, debug or test
#                   builds.
#                   Optional. If omitted, then this is the current directory.
#  CMAKE_EXTRA_PARAMS:  extra parameters to control cmake, such as -DBUNDLED_GEVIV2=OFF.
#  LD_LIBRARY_PATH: a colon-separated list of directories where libraries
#                   should be searched before the standard directories.
#                   If VIPS_INSTALL_PREFIX is set, then this variable will
#                   be extended accordingly.
#  PKG_CONFIG_PATH: see "man pkg-config".
#                   If VIPS_INSTALL_PREFIX is set, then this variable will
#                   be extended accordingly.
#  VIPS_INSTALL_PREFIX: VIPS' install prefix. Optional.
#                   If omitted, then the system's libvips is used.
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

# evaluate arguments and environment variables
## Evaluate BUILD_TYPE and create uppercase and capitalized variants of it.
## BUILD_TYPE: use environment variable BUILD_TYPE, $1 or Release (in this order)
BUILD_TYPE=${BUILD_TYPE:-${1:-Release}}
build_type_upper=$(echo ${BUILD_TYPE} | tr '[:lower:]' '[:upper:]')
build_type_capital=$(echo ${BUILD_TYPE} | tr '[:upper:]' '[:lower:]' | sed 's/.*/\u&/')
echo "build_type_upper:" $build_type_upper >log.txt
echo "build_type_capital:" $build_type_capital >>log.txt

## evaluate INSTALL_PREFIX to determine the installation directory
export INSTALL_PREFIX=${INSTALL_PREFIX:-$(pwd)}
phf_install_prefix=${INSTALL_PREFIX}/${build_type_capital}

# set LD_LIBRARY_PATH
if [ -n "${VIPS_INSTALL_PREFIX}" ]; then
    export PKG_CONFIG_PATH=${VIPS_INSTALL_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}
    export LD_LIBRARY_PATH=${VIPS_INSTALL_PREFIX}/lib:${LD_LIBRARY_PATH}
fi

# clean up before building
rm CMakeCache.txt

echo "Building PhotoFlow..."
echo "Install PhotoFlow to: ${phf_install_prefix}"
echo "Build type: ${build_type_capital}"
echo "Use VIPS in ${VIPS_INSTALL_PREFIX}"
echo "PKG_CONFIG_PATH: ${PKG_CONFIG_PATH}"
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}"

if [ "${build_type_upper}" = "TEST" ]; then
# valid values for CMAKE_BUILD_TYPE:[DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL]
    cmake -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_INSTALL_PREFIX=${phf_install_prefix} \
    -DINSTALL_PREFIX=${phf_install_prefix} \
    ${CMAKE_EXTRA_PARAMS} \
    .. &&
    make VERBOSE=1 install
else
#  make Release, Debug and everything else except Test
    cmake -DCMAKE_BUILD_TYPE=${build_type_upper} \
    -DCMAKE_INSTALL_PREFIX=${phf_install_prefix} \
    -DINSTALL_PREFIX=${phf_install_prefix} \
    ${CMAKE_EXTRA_PARAMS} \
    .. &&
    make VERBOSE=1 install
fi

# If the build broke, then exit here with a message.
return_code=$?
if [ ${return_code} -ne 0 ]; then
    echo "Building PhotoFlow finished with return code ${return_code}."
    exit ${return_code}
fi

printf "Creating starter scripts to ensure Photoflow finds libvips: "
printf "run_photoflow.sh, "
cat <<EOF >${phf_install_prefix}/run_photoflow.sh
#! /bin/bash
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}
echo LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}
${phf_install_prefix}/bin/photoflow
echo Photoflow exited with code \$?.
EOF
chmod a+x ${phf_install_prefix}/run_photoflow.sh

printf "run_pfbatch.sh.\n"
cat << EOF >${phf_install_prefix}/run_pfbatch.sh
#! /bin/bash
export "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}
${phf_install_prefix}/bin/pfbatch
echo Pfbatch exited with code \$?.
EOF
chmod a+x ${phf_install_prefix}/run_pfbatch.sh

echo "Building PhotoFlow finished with return code $?."
