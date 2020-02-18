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

#ifndef RAW_HL_RECO_H
#define RAW_HL_RECO_H


#include "../base/processor.hh"
#include "raw_image.hh"

#ifndef PF_CLIP
#define PF_CLIP(a,MAX)  ((a)<(MAX)?(a):(MAX))
#endif

namespace PF 
{


  class RawHLRecoPar: public OpParBase
  {
    hlreco_mode_t hlreco_mode;

    float wb_red_current, wb_green_current, wb_blue_current, exposure_current;

  public:

    RawHLRecoPar();

    float get_wb_red() { return wb_red_current; /*wb_red.get();*/ }
    float get_wb_green() { return wb_green_current; /*wb_green.get();*/ }
    float get_wb_blue() { return wb_blue_current; /*wb_blue.get();*/ }

    void set_wb(float r, float g, float b) {
      wb_red_current = r;
      wb_green_current = g;
      wb_blue_current = b;
      //std::cout<<"RawHLRecoPar: setting WB coefficients to "<<r<<","<<g<<","<<b<<std::endl;
    }

    void set_hlreco_mode(hlreco_mode_t m) { hlreco_mode = m; }
    hlreco_mode_t get_hlreco_mode() { return hlreco_mode; }

    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      rgb_image( get_xsize(), get_ysize() );
    }

    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };


  ProcessorBase* new_raw_hl_reco();
}

#endif 


