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

#include "clone.hh"
#include "../base/new_operation.hh"
#include "../operations/convertformat.hh"


PF::ClonePar::ClonePar(): 
  source_channel("source_channel",this,PF::CLONE_CHANNEL_RGB,"RGB","RGB"),  
  PF::BlenderPar() 
{
  source_channel.add_enum_value(PF::CLONE_CHANNEL_GREY,"Grey","Grey");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_R,"R","R");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_G,"G","G");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_B,"B","B");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_Lab,"Lab","Lab");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_L,"L","L");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_a,"a","a");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_b,"b","b");

  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();

  set_type( "clone" );
}



VipsImage* PF::ClonePar::Lab2grayscale(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );

  if( csin != PF::PF_COLORSPACE_LAB ) {
    convert2lab->get_par()->set_image_hints( srcimg );
    convert2lab->get_par()->set_format( get_format() );
    std::vector<VipsImage*> in2; in2.push_back( srcimg );
    VipsImage* tempimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
    if( !tempimg ) 
      return NULL;
    g_object_unref( srcimg );
    srcimg = tempimg;
  }
  switch( ch ) {
  case PF::CLONE_CHANNEL_L:
    if( vips_extract_band( srcimg, &out, 0, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  case PF::CLONE_CHANNEL_a:
    if( vips_extract_band( srcimg, &out, 1, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  case PF::CLONE_CHANNEL_b:
    if( vips_extract_band( srcimg, &out, 2, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  }

  return out;
}


VipsImage* PF::ClonePar::rgb2grayscale(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );

  if( csin != PF::PF_COLORSPACE_RGB ) {
      return NULL;
  }
  switch( ch ) {
  case PF::CLONE_CHANNEL_R:
    if( vips_extract_band( srcimg, &out, 0, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  case PF::CLONE_CHANNEL_G:
    if( vips_extract_band( srcimg, &out, 1, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  case PF::CLONE_CHANNEL_B:
    if( vips_extract_band( srcimg, &out, 2, NULL ) )
      return NULL;
    g_object_unref( srcimg );
    vips_image_init_fields( out,
			    get_xsize(), get_ysize(), 
			    1, get_format(),
			    get_coding(),
			    get_interpretation(),
			    1.0, 1.0);
    break;
  }

  return out;
}


VipsImage* PF::ClonePar::build(std::vector<VipsImage*>& in, int first, 
			       VipsImage* imap, VipsImage* omap, 
			       unsigned int& level)
{
  if( in.empty() ) return NULL;
  VipsImage* srcimg = NULL;
  if( in.size() > 1 ) srcimg = in[1];
  VipsImage* out = NULL;

  colorspace_t cs = get_colorspace();

  if( cs == PF::PF_COLORSPACE_GRAYSCALE ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      if( ch==PF::CLONE_CHANNEL_Lab ||
	  ch==PF::CLONE_CHANNEL_L ||
	  ch==PF::CLONE_CHANNEL_a ||
	  ch==PF::CLONE_CHANNEL_b ) {
	unsigned int level2 = level;
	out = Lab2grayscale( srcimg, ch, level2 );
      }
      if( ch==PF::CLONE_CHANNEL_RGB ||
	  ch==PF::CLONE_CHANNEL_R ||
	  ch==PF::CLONE_CHANNEL_G ||
	  ch==PF::CLONE_CHANNEL_B ) {
	unsigned int level2 = level;
	out = rgb2grayscale( srcimg, ch, level2 );
      }
    }

    if( !out ) {
      // image cannot be created, we revert to a ablack image of the correct size
      if( vips_black( &out, get_xsize(), get_ysize(), NULL ) ) {
	if( in[0] )
	  g_object_ref( in[0] );
	return in[0];
      }

      if( get_format() != out->BandFmt ) {
	VipsImage* tmpimg = out;
	std::vector<VipsImage*> in2;
	in2.push_back( tmpimg );
	convert_format->get_par()->set_image_hints( tmpimg );
	convert_format->get_par()->set_format( get_format() );
	out = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
	g_object_unref( tmpimg );
      }
    }
  }

  if( cs == PF::PF_COLORSPACE_RGB ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      if( ch==PF::CLONE_CHANNEL_RGB ) {
	out = srcimg;
	g_object_ref( out );
      }
    }
  }

  if( cs == PF::PF_COLORSPACE_LAB ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      if( ch==PF::CLONE_CHANNEL_Lab ) {
	out = srcimg;
	g_object_ref( out );
      }
    }
  }

  std::vector<VipsImage*> in2;
  if( in.size() > 1 ) {
    in2.push_back( in[0] );
    in2.push_back( out );
  } else {
    in2.push_back( NULL );
    in2.push_back( out );
  }
#ifndef NDEBUG
  std::cout<<"PF::ClonePar::build(): source channel="<<source_channel.get_enum_value().second.first
	   <<"    target colorspace="<<cs;
  if( out )
    std::cout<<"    output colorspace="<<PF::convert_colorspace( out->Type );
  std::cout<<std::endl;
#endif
  VipsImage* out2 = PF::BlenderPar::build( in2, 0, NULL, omap, level );
  g_object_unref( out );
  return out2;
}


PF::ProcessorBase* PF::new_clone()
{
  return( new PF::Processor<PF::ClonePar,PF::CloneProc>() );
}
