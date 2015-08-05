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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "unsharp_mask.hh"
#include "volume.hh"


PF::VolumePar::VolumePar():
  OpParBase(), 
  method("method",this,PF::VOLUME_USM,"USM","Unsharp Mask"),
  amount("amount",this,1),
  enable_equalizer("enable_equalizer",this,true),
  blacks_amount("blacks_amount",this,0),
  shadows_amount("shadows_amount",this,0.7),
  midtones_amount("midtones_amount",this,1),
  highlights_amount("highlights_amount",this,0.7),
  whites_amount("whites_amount",this,0),
  usm_radius("usm_radius",this,20)
{
	//method.add_enum_value(PF::SHARPEN_USM,"USM","Unsharp Mask");
	//method.add_enum_value(PF::SHARPEN_MICRO,"MICRO","Micro Contrast");

  usm = new_unsharp_mask();

  // The tone curve is initialized as a bell-like curve with the
  // mid-tones point at 100% and the shadows/highlights points at 0%
  float xpt = 0.0, ypt = blacks_amount.get();
  tone_curve.set_point( 0, xpt, ypt );
  xpt = 1.0; ypt = whites_amount.get();
  tone_curve.set_point( 1, xpt, ypt );

  tone_curve.add_point( 0.25, shadows_amount.get() );
  tone_curve.add_point( 0.5, midtones_amount.get() );
  tone_curve.add_point( 0.75, highlights_amount.get() );

  set_type("volume" );

  set_default_name( _("local contrast") );
}


VipsImage* PF::VolumePar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  tone_curve.lock();
  float xpt, ypt;
  xpt=0.0; ypt=blacks_amount.get(); tone_curve.set_point( 0, xpt, ypt );
  xpt=0.25; ypt=shadows_amount.get(); tone_curve.set_point( 1, xpt, ypt );
  xpt=0.5; ypt=midtones_amount.get(); tone_curve.set_point( 2, xpt, ypt );
  xpt=0.75; ypt=highlights_amount.get(); tone_curve.set_point( 3, xpt, ypt );
  xpt=1.0; ypt=whites_amount.get(); tone_curve.set_point( 4, xpt, ypt );

  //std::cout<<"CurvesPar::update_curve() called. # of points="<<curve.get().get_npoints()<<std::endl;std::cout.flush();
  for(int i = 0; i <= FormatInfo<unsigned char>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned char>::RANGE;
    vec8[i] = tone_curve.get_value( x );
    //vec8[i] = (short int)(y*FormatInfo<unsigned char>::RANGE);
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec8[i]="<<vec8[i]<<std::endl;
  }
  for(int i = 0; i <= FormatInfo<unsigned short int>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned short int>::RANGE;
    vec16[i] = tone_curve.get_value( x );
    //vec16[i] = (int)(y*FormatInfo<unsigned short int>::RANGE);
   //if(i%1000 == 0)
    //if(curve.get().get_points().size()>100)
   //   std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec16[i]="<<vec16[i]<<"  points="<<curve.get().get_points().size()<<std::endl;
  }
  tone_curve.unlock();

  VipsImage* lce = NULL;
  switch( method.get_enum_value().first ) {
  case PF::VOLUME_USM: {
    UnsharpMaskPar* usmpar = dynamic_cast<UnsharpMaskPar*>( usm->get_par() );
    if( usmpar ) {
      usmpar->set_radius( usm_radius.get() );
      usmpar->set_image_hints( in[0] );
      usmpar->set_format( get_format() );
      lce = usmpar->build( in, first, imap, omap, level );
    }
    break;
  }
  default:
    break;
  }
  
  if( !lce ) {
    std::cout<<"VolumePar::build(): NULL local contrast enhanced image"<<std::endl;
    return NULL;
  }

  std::vector<VipsImage*> in2;
  in2.push_back(in[0]);
  in2.push_back(lce);
  VipsImage* out = OpParBase::build( in2, 0, imap, omap, level );

  PF_UNREF( lce, "VolumePar::build(): lce unref" );

  return out;
}


PF::ProcessorBase* PF::new_volume()
{
  return new PF::Processor<PF::VolumePar,PF::VolumeProc>();
}
