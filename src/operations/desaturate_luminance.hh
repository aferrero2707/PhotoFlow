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

#ifndef VIPS_DESATURATE_LUMINANCE_H
#define VIPS_DESATURATE_LUMINANCE_H

namespace PF 
{
  class DesaturateLuminancePar: public OpParBase
  {
    ICCProfile* profile;
    ProcessorBase* proc_average;
    ProcessorBase* convert2lab;
    ProcessorBase* convert_cs;

  public:
    DesaturateLuminancePar();

    bool has_intensity() { return false; }
    ICCProfile* get_profile() { return profile; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  
  template < OP_TEMPLATE_DEF > 
  class DesaturateLuminanceProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, DesaturateLuminancePar* par)
    {
    }
  };


  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class DesaturateLuminanceProc< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, DesaturateLuminancePar* par)
    {
      ICCProfile* profile = par->get_profile();
      if( !profile ) return;

      Rect *r = &oreg->valid;
      //int x, y, xomap, y0, dx1=CHMIN, dx2=PF::ColorspaceInfo<colorspace>::NCH-CHMIN, ch, CHMAXplus1=CHMAX+1;
      int x, y, y0;
      int width = r->width;
      int line_size = width * oreg->im->Bands;
      float* pin;
      float* pin2;
      float* pout;
      float* line = NULL;
      float val;
      if( profile->get_trc_type()!=PF_TRC_LINEAR ) line = new float[line_size];
      for( y = 0; y < r->height; y++ ) {
        y0 = r->top + y;
        pin = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, y0 );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, y0 );
        if( line ) {
          for( x = 0; x < line_size; x++ ) {
            line[x] = profile->perceptual2linear( pin[x] );
          }
          pin2 = line;
        } else {
          pin2 = pin;
        }

        for( x = 0; x < line_size; x += 3 ) {
          val = profile->get_luminance( pin2[x], pin2[x+1], pin2[x+2] );
          if( profile->get_trc_type()!=PF_TRC_LINEAR )
            val = profile->linear2perceptual( val );
          pout[x] = val;
          pout[x+1] = val;
          pout[x+2] = val;
        }
      }
    }
  };

  ProcessorBase* new_desaturate_luminance();
}

#endif 


