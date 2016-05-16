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

#include "hotpixels.hh"

PF::HotPixelsPar::HotPixelsPar(): 
  OpParBase(), 
  hotp_enable( "hotp_enable", this, false ),
  hotp_threshold("hotp_threshold",this,0.05), 
  hotp_strength("hotp_strength",this,0.25), 
  hotp_permissive( "hotp_permissive", this, false ),
  hotp_markfixed( "hotp_markfixed", this, false )
{
  set_type("hotpixels" );
}

VipsImage* PF::HotPixelsPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  if( !hotp_enable.get() ) {
    PF_REF( in[0], "HotPixelsPar::build(): in[0] ref when disabled" );
    return in[0];
  }

  std::cout<<"PF::HotPixelsPar::build "<<std::endl;
  
  set_pixels_fixed(0);
  
  VipsImage* image = OpParBase::build( in, first, NULL, NULL, level );
  if( !image )
    return NULL;

  return image;
}


PF::ProcessorBase* PF::new_hotpixels()
{
  return new PF::Processor<PF::HotPixelsPar,PF::HotPixels>();
//  return new PF::Processor<PF::HotPixelsPar,PF::HotPixelsProc>();
}
