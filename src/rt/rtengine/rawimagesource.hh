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

		int FC_roffset, FC_coffset;

		void amaze_demosaic_RT(int winx, int winy, int winw, int winh,
													 int tilex, int tiley, int tilew, int tileh);//Emil's code for AMaZE
		void igv_demosaic_RT(int winx, int winy, int winw, int winh,
													 int tilex, int tiley, int tilew, int tileh);
	public:

		RawImageSource(): FC_roffset(0), FC_coffset(0) {}

		int FC(int r, int c)
		{
      r -= FC_roffset;
      c -= FC_coffset;

      int rr = r+tile_top;
      int cc = c+tile_left;
      if(rr<0) rr = -rr;
      if(cc<0) cc = -cc;
      if( rr >= (tile_top+rawData.GetHeight()) ) {
        int dr = rr -  (tile_top+rawData.GetHeight()) + 1;
        rr = tile_top + rawData.GetHeight() - dr - 1;
      }
      if( cc >= (tile_left+rawData.GetWidth()) ) {
        int dr = cc -  (tile_left+rawData.GetWidth()) + 1;
        rr = tile_left + rawData.GetWidth() - dr - 1;
      }

			int color = rawData[rr].color(cc);
			if( color<0 || color>3)
				return 0;
			if( color == 3 ) color = 1;
			return color;
		}

		// Interface layer between Photoflow and RT code
		void amaze_demosaic(VipsRegion* ir, VipsRegion* oreg);
		void igv_demosaic(VipsRegion* ir, VipsRegion* oreg);
		void false_color_correction(VipsRegion* ir, VipsRegion* oreg);
	};
}


#endif
