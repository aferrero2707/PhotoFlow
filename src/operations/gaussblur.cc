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
  OpParBase(),
  radius("radius",this,5),
	blur_mode("blur_mode",this,PF_BLUR_FAST,"FAST","Fast"),
	padding(0)
{
	blur_mode.add_enum_value(PF_BLUR_FAST,"FAST","Fast");
	blur_mode.add_enum_value(PF_BLUR_EXACT,"ACCURATE","Accurate");
	
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  blur_sii = new_gaussblur_sii();

  //set_demand_hint( VIPS_DEMAND_STYLE_SMALLTILE );
  set_type( "gaussblur" );

  set_default_name( _("gaussian blur") );
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
	for( unsigned int l = 1; l <= level; l++ )
		radius2 /= 2;

	if( (get_render_mode() == PF_RENDER_PREVIEW) &&
			radius2 < 0.5 ) {
		PF_REF( blurred, "PF::GaussBlurPar::build(): blurred ref" );
		return blurred;
	}

  std::vector<VipsImage*> in2;
  bool do_caching = false;
  int tw = 128, th = 128, nt = 1000;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  int threaded = 1, persistent = 0;

  bool do_fast_blur = false;
  //if( radius2 > 20 ) do_fast_blur = true;
  //if( (get_render_mode() == PF_RENDER_PREVIEW) && (radius2 > 5) ) do_fast_blur = true;
  if( (blur_mode.get_enum_value().first == PF_BLUR_FAST) && (radius2 > 5) ) do_fast_blur = true;

  //if( (get_render_mode() == PF_RENDER_PREVIEW) &&
  //    (blur_mode.get_enum_value().first == PF_BLUR_FAST) &&
  //    (radius2 > 5) ){
  std::cout<<"GaussBlurPar::build(): do_fast_blur="<<do_fast_blur<<std::endl;
  if( do_fast_blur ) {
    // Fast approximate gaussian blur method
    GaussBlurSiiPar* gpar = dynamic_cast<GaussBlurSiiPar*>( blur_sii->get_par() );
    gpar->set_radius( radius2 );

    // First we extend the border of the image to correctly handle edge pixels in the blurring step
    // The additional padding is given by the blur radius
    VipsImage* extended;
    VipsExtend extend = VIPS_EXTEND_COPY;
    if( vips_embed(srcimg, &extended, gpar->get_padding(), gpar->get_padding(),
        srcimg->Xsize+2*gpar->get_padding(), srcimg->Ysize+2*gpar->get_padding(),
        "extend", extend, NULL) ) {
      std::cout<<"GaussBlurPar::build(): vips_embed() failed."<<std::endl;
      return NULL;
    }

    // Memory caching of the padded image
    VipsImage* cached = extended;
    if( do_caching ) {
      if( vips_tilecache(extended, &cached,
          "tile_width", tw, "tile_height", th, "max_tiles", nt,
          "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
        std::cout<<"GaussBlurPar::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( extended, "GaussBlurPar::build(): extended unref" );
    }

    // Fast blurring
    gpar->set_image_hints( cached ); gpar->set_format( get_format() );
    in2.clear(); in2.push_back( cached );
    VipsImage* blurred = gpar->build( in2, 0, NULL, omap, level );
    PF_UNREF( cached, "GaussBlurPar::build(): cached unref" );

    // Final cropping to remove the padding pixels
    VipsImage* cropped;
    //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  blurred->Xsize="<<blurred->Xsize<<"  padding="<<gpar->get_padding()<<std::endl;
    if( vips_crop(blurred, &cropped, gpar->get_padding(), gpar->get_padding(),
        srcimg->Xsize, srcimg->Ysize, NULL) ) {
      std::cout<<"GaussBlurPar::build(): vips_crop() failed."<<std::endl;
      return NULL;
    }
    PF_UNREF( blurred, "GaussBlurPar::build(): blurred unref" );

    padding = gpar->get_padding();

    //std::vector<VipsImage*> parents; parents.push_back(srcimg);
    //PF::image_hierarchy_fill( cropped, gpar->get_padding(), parents );

		return cropped;
	}
	
  // reset padding value, in case the gaussian blur operation fails;
  padding = 0;
  if( srcimg ) {
    int size = (srcimg->Xsize > srcimg->Ysize) ? srcimg->Xsize : srcimg->Ysize;
  
		float accuracy = 0.05;
		VipsPrecision precision = VIPS_PRECISION_FLOAT;
		if( get_render_mode() == PF_RENDER_PREVIEW &&
				(blur_mode.get_enum_value().first == PF_BLUR_FAST) ) {
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
      //VipsImage* cached;
      VipsImage* tmp;
      /*
      if( vips_tilecache(srcimg, &cached,
          "tile_width", tw, "tile_height", th, "max_tiles", nt,
          "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
        std::cout<<"GaussBlurPar::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      */
      std::cout<<"GaussBlurPar::build(): convsep mask size="<<mask->Xsize<<" "<<mask->Ysize<<std::endl;
      result = vips_convsep( srcimg, &tmp, mask,
			     "precision", precision,
			     NULL );
      //g_object_unref( mask );
      PF_UNREF( mask, "PF::GaussBlurPar::build(): mask unref" );
      //PF_UNREF( cached, "PF::GaussBlurPar::build(): cached unref" );
		
			
      if( !result ) {
				if( vips_cast( tmp, &blurred, get_format(), NULL ) ) {
					PF_UNREF( tmp, "PF::GaussBlurPar::build(): tmp unref" );
					return NULL;
				}
        PF_UNREF( tmp, "PF::GaussBlurPar::build(): tmp unref" );
        padding = mask->Xsize;
			}
			//blurred = tmp;
    }
		
  }

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
  VipsImage* out = PF::OpParBase::build( in2, 0, NULL, omap, level );
  //g_object_unref( converted );
  PF_UNREF( converted, "PF::GaussBlurPar::build(): converted unref" );
  return out;
}


PF::ProcessorBase* PF::new_gaussblur()
{
  return( new PF::Processor<PF::GaussBlurPar,PF::GaussBlurProc>() );
}
