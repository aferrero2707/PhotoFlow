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
  radius("radius",this,5),
	preview_mode("preview_mode",this,PF_BLUR_FAST,"FAST","Fast")
{
	preview_mode.add_enum_value(PF_BLUR_FAST,"FAST","Fast");
	preview_mode.add_enum_value(PF_BLUR_EXACT,"ACCURATE","Accurate");
	
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

	double radius2 = radius.get();
	for( int l = 1; l <= level; l++ )
		radius2 /= 2;

	if( (get_render_mode() == PF_RENDER_PREVIEW) &&
			radius2 < 0.5 ) {
		PF_REF( blurred, "PF::GaussBlurPar::build(): blurred ref" );
		return blurred;
	}

	sii_precomp( &coeffs, radius2, 3 );

	
	if( (get_render_mode() == PF_RENDER_PREVIEW) &&
			(preview_mode.get_enum_value().first == PF_BLUR_FAST) &&
			(radius2 > 5) ){
		VipsImage* outnew = PF::OpParBase::build( in, first, NULL, omap, level );
		return outnew;
	}
	

  if( srcimg ) {
    int size = (srcimg->Xsize > srcimg->Ysize) ? srcimg->Xsize : srcimg->Ysize;
  
		float accuracy = 0.05;
		VipsPrecision precision = VIPS_PRECISION_FLOAT;
		if( get_render_mode() == PF_RENDER_PREVIEW &&
				(preview_mode.get_enum_value().first == PF_BLUR_FAST) ) {
			accuracy = 0.2;
			//if( radius2 > 2 )
			//	precision = VIPS_PRECISION_APPROXIMATE;
		}

    /*
		VipsImage* tmp;
		if( vips_gaussblur(srcimg, &tmp, radius2*2, "precision", precision, NULL) )
			return NULL;
		if( vips_cast( tmp, &blurred, get_format(), NULL ) ) {
			PF_UNREF( tmp, "PF::GaussBlurPar::build(): tmp unref" );
			return NULL;
		}
    */
		
    int result = vips_gaussmat( &mask, radius2, accuracy, 
				"separable", TRUE,
				"integer", FALSE,
				NULL );

    if( !result ) {
			VipsImage* tmp;
      result = vips_convsep( srcimg, &tmp, mask, 
			     "precision", precision,
			     NULL );
      //g_object_unref( mask );
      PF_UNREF( mask, "PF::GaussBlurPar::build(): mask unref" );
		
			
      if( !result ) {
				if( vips_cast( tmp, &blurred, get_format(), NULL ) ) {
					PF_UNREF( tmp, "PF::GaussBlurPar::build(): tmp unref" );
					return NULL;
				}
        PF_UNREF( tmp, "PF::GaussBlurPar::build(): tmp unref" );
			}
			//blurred = tmp;
    }
		
  }

  std::vector<VipsImage*> in2;
  VipsImage* converted = blurred;
  if( false && blurred ) {
    if( get_format() != blurred->BandFmt ) {
      in2.clear(); in2.push_back( blurred );
      convert_format->get_par()->set_image_hints( blurred );
      convert_format->get_par()->set_format( get_format() );
      converted = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
      //g_object_unref( blurred );
      PF_UNREF( blurred, "PF::GaussBlurPar::build(): blurred unref" );
    }
  }

	return converted;

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
  //g_object_unref( converted );
  PF_UNREF( converted, "PF::GaussBlurPar::build(): converted unref" );
  return out;
}


PF::ProcessorBase* PF::new_gaussblur()
{
  return( new PF::Processor<PF::GaussBlurPar,PF::GaussBlurProc>() );
}
