#! /bin/bash
#  Script:  update_libvips.sh
#  Purpose: Update libvips.
#           If libvips is not here yet, clone it from Github.
#           Otherwise run "git pull".
#  Call:    update_libvips.sh
#  Author, License:
#    Copyright (C) 2014, 2016 Ferrero Andrea, Sven Claußner
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

libvips_dir="../libvips"

export OLD_PWD=$(pwd)

echo Updating VIPS...
if [ -e ${libvips_dir} ]; then
    cd ${libvips_dir}
    git checkout v8.5.9
    #git pull
else
	git clone -b v8.5.9 --depth 1 https://github.com/libvips/libvips.git ${libvips_dir}
    #git clone https://github.com/jcupitt/libvips.git ${libvips_dir}
fi

# return to the directory where we called this script from
cd -
