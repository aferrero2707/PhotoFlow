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

#include "../base/processor.hh"
#include "../base/processor_imp.hh"
#include "maxrgb.hh"

using namespace PF;

PF::MaxRGBPar::MaxRGBPar():
  OpParBase(),
  do_max(true)
{
  set_type( "max_rgb" );

  set_default_name( _("max(R,G,B)") );
}



  template < OP_TEMPLATE_DEF >
  class MaxRGBProc: public IntensityProc<T, has_imap>
  {
  public:
    void render(VipsRegion** in, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* out, OpParBase* par)
    {
    }
  };


  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class MaxRGBProc< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      MaxRGBPar* opar = dynamic_cast<MaxRGBPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;
      int x, y;

      if( false ) std::cout<<"MaxRGB: PF_COLORSPACE_RGB"<<std::endl;

      float* pin;
      float* pout;
      float val;
      for( y = 0; y < height; y++ ) {
        pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        if( opar->get_do_max() ) {
          for( x = 0; x < line_size; x+=3 ) {
            val = MAX3(pin[x],pin[x+1],pin[x+2]);
            pout[x+2] = pout[x+1] = pout[x] = val;
          }
        } else {
          for( x = 0; x < line_size; x+=3 ) {
            val = MIN3(pin[x],pin[x+1],pin[x+2]);
            pout[x+2] = pout[x+1] = pout[x] = val;
          }
        }
      }
    }
  };


  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class MaxRGBProc< float, BLENDER, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      if(ireg[0]->im->Bands != 3) return;
      MaxRGBPar* opar = dynamic_cast<MaxRGBPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * ireg[0]->im->Bands;
      //int width = r->width;
      int height = r->height;
      int x, y, xout;

      //if( true ) std::cout<<"MaxRGB: PF_COLORSPACE_GRAYSCALE"<<std::endl;

      float* pin;
      float* pout;
      float val;
      for( y = 0; y < height; y++ ) {
        pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        if( opar->get_do_max() ) {
          for( x = 0, xout = 0; x < line_size; x+=3, xout+=1 ) {
            val = MAX3(pin[x],pin[x+1],pin[x+2]);
            pout[xout] = val;
            //if( true && r->left==0 && r->top==0 )
            //std::cout<<"pin="<<pin[x]<<","<<pin[x+1]<<","<<pin[x+2]<<"  val["<<y<<","<<xout<<"]="<<val<<std::endl;;
          }
        } else {
          for( x = 0, xout = 0; x < line_size; x+=3, xout+=1 ) {
            val = MIN3(pin[x],pin[x+1],pin[x+2]);
            pout[xout] = val;
          }
        }
      }
    }
  };


  PF::ProcessorBase* PF::new_maxrgb()
  { return( new PF::Processor<PF::MaxRGBPar,MaxRGBProc>() ); }

