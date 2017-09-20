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

#include "../../base/rawmatrix.hh"

#include "LUT.h"
#include "progresslistener.h"
#include "../../operations/raw_image.hh"



namespace rtengine {


class Color
{
public:
  static LUTf igammatab_24_17;
  static LUTf gammatab_24_17a;

  static void Init();
};


	class RawImageSource
	{
		PF::RawMatrix rawData;
		float** rawDataBuf;
		PF::Array2D<float> red, green, blue;
		int tile_top, tile_left;

		int FC_roffset, FC_coffset;

    double fitparams[3][2][16];
    dcraw_data_t* image_data;

    ProgressListener* plistener;

    //VipsImage* add_cfa_border( VipsImage* in, int border );

    void CA_correct_RT(int winx, int winy, int winw, int winh,
                           int tilex, int tiley, int tilew, int tileh,
                           bool autoCA, float cared, float cablue);
    void amaze_demosaic_RT(int winx, int winy, int winw, int winh,
                           int tilex, int tiley, int tilew, int tileh);//Emil's code for AMaZE
		void igv_demosaic_RT(int winx, int winy, int winw, int winh,
													 int tilex, int tiley, int tilew, int tileh);
		void lmmse_demosaic_RT(int winx, int winy, int winw, int winh,
		                       int tilex, int tiley, int tilew, int tileh,
                           int iterations);
    void xtrans_demosaic_RT(int winx, int winy, int winw, int winh,
                           int tilex, int tiley, int tilew, int tileh);

    void refinement(int PassCount, int W, int H);
    void refinement_lassus(int PassCount, int W, int H);

	public:

		RawImageSource();

		void set_image_data( dcraw_data_t* d )
		{
		  image_data = d;
		  if(d) memcpy( fitparams, d->color.ca_fitparams, sizeof(fitparams) );
		}

		int FC(int r, int c)
		{
		  //std::cout<<"RawImageSource::FC(): "<<(void*)image_data<<", "<<FC_roffset<<","<<FC_coffset<<","<<tile_top<<","<<tile_left<<std::endl;
		  //int rr = r, cc = c;

      r -= FC_roffset;
      c -= FC_coffset;

      int rr = r+tile_top;
      int cc = c+tile_left;

      if(rr<0) rr = -rr;
      if(cc<0) cc = -cc;
      if( rr >= (tile_top+(int)(rawData.GetHeight())) ) {
        int dr = rr -  (tile_top+rawData.GetHeight()) + 1;
        rr = tile_top + (int)(rawData.GetHeight()) - dr - 1;
      }
      if( cc >= (tile_left+(int)(rawData.GetWidth())) ) {
        int dr = cc -  (tile_left+rawData.GetWidth()) + 1;
        rr = tile_left + (int)(rawData.GetWidth()) - dr - 1;
      }

      if( false && image_data ) {
        int color2 = ( image_data->idata.filters >> ((((rr+image_data->sizes.top_margin) << 1 & 14) +
            ((cc+image_data->sizes.left_margin) & 1)) << 1) & 3 );
        if( color2 == 3 ) color2 = 1;

        if(rr<8 && cc<8) std::cout<<"rr="<<rr<<" cc="<<cc<<" c="<<color2<<" filters="<<image_data->idata.filters<<std::endl;
        return color2;
      }

      int color = rawData[rr].color(cc);
			if( color<0 || color>3)
				return 0;
			if( color == 3 ) color = 1;

			return color;
		}

		// Interface layer between Photoflow and RT code
    void ca_correct(VipsRegion* ir, VipsRegion* oreg, bool autoCA, float cared, float cablue);
    void amaze_demosaic(VipsRegion* ir, VipsRegion* oreg);
    void igv_demosaic(VipsRegion* ir, VipsRegion* oreg);
    void lmmse_demosaic(VipsRegion* ir, VipsRegion* oreg);
    void xtrans_demosaic(VipsRegion* ir, VipsRegion* oreg);
		void false_color_correction(VipsRegion* ir, VipsRegion* oreg);
	};
}


#endif
