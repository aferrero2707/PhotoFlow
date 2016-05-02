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

#include "unsharp_mask.hh"


PF::UnsharpMaskPar::UnsharpMaskPar(): 
	PixelProcessorPar(), 
	radius("radius",this,1), 
	amount("amount",this,100)
{
	blur = new_gaussblur();
	
	set_type( "unsharp_mask" );
}






VipsImage* PF::UnsharpMaskPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( (int)in.size() > first ) srcimg = in[first];

	if( !srcimg )
		return NULL;

	double radius2 = radius.get();
	for( unsigned int l = 1; l < level; l++ )
		radius2 /= 2;

	blur->get_par()->set_image_hints( this );
	PropertyBase* pradius = blur->get_par()->get_property("radius");
	if(!pradius) return NULL;
	pradius->import( &radius );

	VipsImage* blurred = blur->get_par()->build( in, first, NULL, NULL, level );
	if( !blurred )
		return NULL;

	std::vector<VipsImage*> in2;
	in2.push_back(srcimg);
	in2.push_back(blurred);
	VipsImage* out = PF::OpParBase::build( in2, 0, imap, omap, level );
	PF_UNREF( blurred, "PF::GaussBlurPar::build(): blurred unref" );

	return out;
}


PF::ProcessorBase* PF::new_unsharp_mask()
{
  return ( new PF::Processor<PF::UnsharpMaskPar,PF::UnsharpMask>() );
}

