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

#ifndef PF_IMAGE_TO_MAP_H
#define PF_IMAGE_TO_MAP_H

namespace PF 
{
  class ImageToMapPar: public OpParBase
  {
    ICCProfile* profile;
    ProcessorBase* proc_average;
    ProcessorBase* convert2lab;
    ProcessorBase* convert_cs;

  public:
    ImageToMapPar();

    bool has_intensity() { return false; }
    ICCProfile* get_profile() { return profile; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  
  template < OP_TEMPLATE_DEF > 
  class ImageToMapProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, ImageToMapPar* par)
    {
      std::cout<<"ImageToMapProc::render(): unsupported colorspace "<<CS<<std::endl;
    }
  };


  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class ImageToMapProc< float, BLENDER, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, ImageToMapPar* par)
    {
      ICCProfile* profile = par->get_profile();
      if( !profile ) return;

      VipsRect *r = &oreg->valid;
      //int x, y, xomap, y0, dx1=CHMIN, dx2=PF::ColorspaceInfo<colorspace>::NCH-CHMIN, ch, CHMAXplus1=CHMAX+1;
      int x, xin, y, y0;
      int width = r->width;
      int line_size = width * oreg->im->Bands;
      float* pin;
      float* pin2;
      float* pout;
      float* line = NULL;
      float val;
      //if( profile->get_trc_type()!=PF_TRC_LINEAR ) line = new float[line_size];
      if( false && r->left==0 && r->top==0 ) {
        std::cout<<"ImageToMapProc::render(): profile="<<profile<<std::endl;
        //std::cout<<"ImageToMapProc::render(): profile->has_colorants="<<profile->has_colorants<<std::endl;
        std::cout<<"ImageToMapProc::render(): colorspace="<<convert_colorspace(ireg[in_first]->im->Type)<<std::endl;
      }
      for( y = 0; y < r->height; y++ ) {
        y0 = r->top + y;
        pin = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, y0 );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, y0 );
        if(convert_colorspace(ireg[in_first]->im->Type) == PF_COLORSPACE_RGB) {
          for( x = 0; x < line_size; x++ ) {
            val = profile->get_lightness( pin[0], pin[1], pin[2] );
            if( profile->is_linear() )
              val = profile->linear2perceptual( val );
            *pout = val;
            pout += 1; pin += 3;
          }
        } else if(convert_colorspace(ireg[in_first]->im->Type) == PF_COLORSPACE_LAB) {
          for( x = 0; x < line_size; x++ ) {
            *pout = pin[0];
            pout += 1; pin += 3;
          }
        }
      }
      //if( line ) delete line;
    }
  };


  ProcessorBase* new_image_to_map();
}

#endif 


