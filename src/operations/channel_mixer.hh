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

#ifndef VIPS_CHANNEL_MIXER_H
#define VIPS_CHANNEL_MIXER_H

#include "../base/format_info.hh"
#include "../base/pixel_processor.hh"

namespace PF 
{

class ChannelMixerPar: public PixelProcessorPar
{
  Property<float> red_mix, green_mix, blue_mix;
public:
  ChannelMixerPar():
    PixelProcessorPar(),
    red_mix("red_mix",this,1),
    green_mix("green_mix",this,0),
    blue_mix("blue_mix",this,0)
{
    set_type( "channel_mixer" );

    set_default_name( _("channel mixer") );
}
  float get_red_mix() { return red_mix.get(); }
  float get_green_mix() { return green_mix.get(); }
  float get_blue_mix() { return blue_mix.get(); }
};





template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
class ChannelMixerProc
{
  ChannelMixerPar* par;
public:
  ChannelMixerProc(ChannelMixerPar* p): par(p) {}

  void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout)
  {
    for( int ch = CHMIN; ch <= CHMAX; ch++ )
      pout[x+ch] = p[first][x+ch];
  }
};


template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
class ChannelMixerProc<T,PF_COLORSPACE_RGB,CHMIN,CHMAX,PREVIEW,OP_PAR>
{
  ChannelMixerPar* par;
public:
  ChannelMixerProc(ChannelMixerPar* p): par(p) {}

  void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout)
  {
    float sum = par->get_red_mix() + par->get_green_mix() + par->get_blue_mix();
    typename FormatInfo<T>::PROMOTED newval = 0;
    int i = x;
    if( sum < -1.0e-15 || sum > 1.0e-15 ) {
      newval = (typename FormatInfo<T>::PROMOTED)( (par->get_red_mix()*p[first][x] + 
          par->get_green_mix()*p[first][x+1] +
          par->get_blue_mix()*p[first][x+2])/sum );
    }
    clip(newval,pout[x]);
    pout[x+1] = pout[x];
    pout[x+2] = pout[x];
  }
};


template < OP_TEMPLATE_DEF >
class ChannelMixer: public PixelProcessor< OP_TEMPLATE_IMP, ChannelMixerPar, ChannelMixerProc >
{
};


ProcessorBase* new_channel_mixer();

}

#endif 


