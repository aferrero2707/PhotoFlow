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

#include "gaussblur_sii.hh"


PF::GaussBlurSiiPar::GaussBlurSiiPar():
  OpParBase(),
  radius("radius",this,5)
{
  set_demand_hint( VIPS_DEMAND_STYLE_SMALLTILE );
  set_type( "gaussblur_sii" );
}



VipsImage* PF::GaussBlurSiiPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  VipsImage* blurred = PF::OpParBase::build( in, first, NULL, omap, level );
  return blurred;
}


PF::ProcessorBase* PF::new_gaussblur_sii()
{
  return( new PF::Processor<PF::GaussBlurSiiPar,PF::GaussBlurSiiProc>() );
}
