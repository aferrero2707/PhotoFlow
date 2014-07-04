/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#ifndef RAW_IMAGE_SOURCE_H
#define RAW_IMAGE_SOURCE_H

#include <vips/vips.h>

#include "../base/rawmatrix.hh"

namespace rtengine {
	
	class RawImageSource
	{
		PF::RawMatrix rawData;
		PF::Array2D<float> red, green, blue;
		int tile_top, tile_left;

		void amaze_demosaic_RT(int winx, int winy, int winw, int winh,
													 int tilex, int tiley, int tilew, int tileh);//Emil's code for AMaZE
	public:

		int FC(int r, int c)
		{
			int color = rawData[r+tile_top].color(c+tile_left);
			if( color == 3 ) color = 1;
			return color;
		}

		// Interface layer between Photoflow and RT code
		void amaze_demosaic(VipsRegion* ir, VipsRegion* oreg);
	};
}


#endif
