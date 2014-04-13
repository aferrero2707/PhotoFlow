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

#include "gaussblur.hh"
#include "../base/new_operation.hh"
#include "../operations/convertformat.hh"
#include "../operations/blender.hh"


PF::GaussBlurPar::GaussBlurPar(): 
  BlenderPar(),
  radius("radius",this,1)
{
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();

  set_type( "gaussblur" );
}



VipsImage* PF::GaussBlurPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* blurred = srcimg;

  if( srcimg ) {
    int size = (srcimg->Xsize > srcimg->Ysize) ? srcimg->Xsize : srcimg->Ysize;
    float pxradius = radius.get()*size/1000;
  
    int result = vips_gaussmat( &mask, pxradius / 2, 0.1, 
				"separable", TRUE,
				"integer", FALSE,
				NULL );

    if( !result ) {
      result = vips_convsep( srcimg, &blurred, mask, 
			     "precision", VIPS_PRECISION_INTEGER,
			     NULL );
      g_object_unref( mask );
      if( !result ) {
	g_object_unref( srcimg );
      }
    }
  }

  std::vector<VipsImage*> in2;
  VipsImage* converted = blurred;
  if( blurred ) {
    if( get_format() != blurred->BandFmt ) {
      in2.clear(); in2.push_back( blurred );
      convert_format->get_par()->set_image_hints( blurred );
      convert_format->get_par()->set_format( get_format() );
      converted = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
      g_object_unref( blurred );
    }
  }

  in2.clear();
  in2.push_back( srcimg );
  in2.push_back( converted );
#ifndef NDEBUG
  // std::cout<<"PF::GaussBlurPar::build(): source channel="<<source_channel.get_enum_value().second.first
  // 	   <<"    target colorspace="<<cs
  // 	   <<"    output colorspace="<<PF::convert_colorspace( out->Type )
  // 	   <<std::endl;
#endif
  VipsImage* out = PF::BlenderPar::build( in2, 0, NULL, omap, level );
  g_object_unref( converted );
  return out;
}


PF::ProcessorBase* PF::new_gaussblur()
{
  return( new PF::Processor<PF::GaussBlurPar,PF::GaussBlurProc>() );
}
