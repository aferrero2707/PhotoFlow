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

#ifndef VIPS_CLONE_H
#define VIPS_CLONE_H

#include <string>

#include "../base/processor.hh"

#include "blender.hh"

namespace PF 
{

enum clone_channel {
  CLONE_CHANNEL_SOURCE,
  CLONE_CHANNEL_GREY,
  CLONE_CHANNEL_RGB,
  CLONE_CHANNEL_R,
  CLONE_CHANNEL_G,
  CLONE_CHANNEL_B,
  CLONE_CHANNEL_MAX_RGB,
  CLONE_CHANNEL_Lab,
  CLONE_CHANNEL_L,
  CLONE_CHANNEL_a,
  CLONE_CHANNEL_b,
  CLONE_CHANNEL_LCh_C,
  CLONE_CHANNEL_LCh_S,
  CLONE_CHANNEL_CMYK,
  CLONE_CHANNEL_C,
  CLONE_CHANNEL_M,
  CLONE_CHANNEL_Y,
  CLONE_CHANNEL_K
};


class RGB2MaskPar: public OpParBase
{
  ICCProfile* profile;
  clone_channel ch;

public:
  RGB2MaskPar();

  bool has_intensity() { return false; }
  ICCProfile* get_profile() { return profile; }

  void set_clone_channel(clone_channel c) { ch = c; }
  clone_channel get_clone_channel() { return ch; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};


template < OP_TEMPLATE_DEF >
class RGB2MaskProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, RGB2MaskPar* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class RGB2MaskProc< float, BLENDER, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, RGB2MaskPar* par)
  {
    ICCProfile* profile = par->get_profile();
    clone_channel ch = par->get_clone_channel();

    //std::cout<<"RGB2MaskProc::render() called, ch="<<ch<<"  profile="<<profile<<std::endl;
    if( !profile ) return;

    VipsRect *r = &oreg->valid;
    //int x, y, xomap, y0, dx1=CHMIN, dx2=PF::ColorspaceInfo<colorspace>::NCH-CHMIN, ch, CHMAXplus1=CHMAX+1;
    int x, y, y0;
    int width = r->width;
    float* pin;
    float* pout;
    for( y = 0; y < r->height; y++ ) {
      y0 = r->top + y;
      pin = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, y0 );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, y0 );

      switch( ch ) {
      case PF::CLONE_CHANNEL_SOURCE:
      case PF::CLONE_CHANNEL_RGB: {
        for( x = 0; x < r->width; x++, pin+=3, pout+=1 ) {
          float R=pin[0], G=pin[1], B=pin[2];
          //if(r->left==0 && r->top==0)
          //  std::cout<<"RGB2MaskProc::render(): R="<<R<<"  G="<<G<<"  B="<<B<<std::endl;
          if( profile->get_trc_type() != PF_TRC_LINEAR ) {
            R = profile->perceptual2linear( R );
            G = profile->perceptual2linear( G );
            B = profile->perceptual2linear( B );
            //if(r->left==0 && r->top==0)
            //  std::cout<<"RGB2MaskProc::render(): lR="<<R<<" lG="<<G<<" lB="<<B<<std::endl;
          }
          float val = profile->get_luminance( R, G, B );
          //if(r->left==0 && r->top==0)
          //  std::cout<<"RGB2MaskProc::render(): L="<<val<<std::endl;
          val = profile->linear2perceptual( val );
          //if(r->left==0 && r->top==0)
          //  std::cout<<"RGB2MaskProc::render(): after l2p: L="<<val<<std::endl;
          pout[0] = val;
        }
        break;
      }
      case PF::CLONE_CHANNEL_R: {
        for( x = 0; x < r->width; x++, pin+=3, pout+=1 ) {
          float val=pin[0];
          if( profile->get_trc_type() == PF_TRC_LINEAR ) {
            val = profile->linear2perceptual( val );
          }
          pout[0] = val;
        }
        break;
      }
      case PF::CLONE_CHANNEL_G: {
        for( x = 0; x < r->width; x++, pin+=3, pout+=1 ) {
          float val=pin[1];
          if( profile->get_trc_type() == PF_TRC_LINEAR ) {
            val = profile->linear2perceptual( val );
          }
          pout[0] = val;
        }
        break;
      }
      case PF::CLONE_CHANNEL_B: {
        for( x = 0; x < r->width; x++, pin+=3, pout+=1 ) {
          float val=pin[2];
          if( profile->get_trc_type() == PF_TRC_LINEAR ) {
            val = profile->linear2perceptual( val );
          }
          pout[0] = val;
        }
        break;
      }
      case PF::CLONE_CHANNEL_MAX_RGB: {
        for( x = 0; x < r->width; x++, pin+=3, pout+=1 ) {
          float R=pin[0], G=pin[1], B=pin[2];
          float val = MAX3( R, G, B );
          if( profile->get_trc_type() == PF_TRC_LINEAR ) {
            val = profile->linear2perceptual( val );
          }
          pout[0] = val;
        }
        break;
      }
      default:
        break;
      }
    }
  }
};


class ClonePar: public OpParBase
{
  PropertyBase source_channel;

  ProcessorBase* convert_format;
  ProcessorBase* convert2lab;
  ProcessorBase* convert_cs;
  ProcessorBase* desaturate;
  ProcessorBase* maxrgb;
  ProcessorBase* trcconv;
  ProcessorBase* rgb2mask;

  VipsImage* Lab2grayscale(VipsImage* in, clone_channel ch, unsigned int& level);
  VipsImage* rgb2grayscale(VipsImage* in, clone_channel ch, unsigned int& level);
  VipsImage* rgb2rgb(VipsImage* in, clone_channel ch, unsigned int& level);
  VipsImage* rgb2maxrgb(VipsImage* in, clone_channel ch, unsigned int& level);
  VipsImage* Lab2rgb(VipsImage* in, clone_channel ch, unsigned int& level);
  VipsImage* L2rgb(VipsImage* in, unsigned int& level);
  VipsImage* grey2rgb(VipsImage* in, unsigned int& level);
  VipsImage* ab2rgb(VipsImage* in, clone_channel ch, unsigned int& level);

public:
  ClonePar();

  /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
   */
  bool has_intensity() { return false; }
  bool needs_input() { return false; }
  bool has_target_channel() { return true; }
  bool accepts_colorspace(colorspace_t);
  bool convert_inputs_on_map_build() { return false; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class CloneProc: public BlenderProc<OP_TEMPLATE_IMP>
{
};

ProcessorBase* new_clone();
}

#endif 


