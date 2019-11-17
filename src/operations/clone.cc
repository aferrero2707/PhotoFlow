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

#include <string.h>

#include "../base/photoflow.hh"
#include "../base/processor_imp.hh"
#include "../base/new_operation.hh"
#include "convertformat.hh"
#include "convert_colorspace.hh"
//#include "../operations/convert2srgb.hh"
//#include "desaturate.hh"
#include "desaturate_luminance.hh"
#include "maxrgb.hh"
#include "trcconv.hh"
#include "clone.hh"



PF::RGB2MaskPar::RGB2MaskPar(): OpParBase(), ch(PF::CLONE_CHANNEL_RGB)
{
  set_type( "rgb2mask" );
}



VipsImage* PF::RGB2MaskPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;
  void *profile_data;
  size_t profile_length;
  VipsImage* srcimg = in[0];
  if( !srcimg ) return NULL;
  profile = PF::get_icc_profile( srcimg );

  // No Lab conversion possible if the input image has no ICC profile
  if( !profile ) {
    std::cout<<"RGB2MaskPar::build(): no profile data"<<std::endl;
    PF_REF(srcimg, "RGB2MaskPar::build(): srcimg ref when no profile data");
    return srcimg;
  }

  set_image_hints( srcimg );
  grayscale_image( get_xsize(), get_ysize() );
  out = PF::OpParBase::build( in, first, imap, omap, level );

  // This is a mask image, so it has no associated color profile
  if( out )
    PF::set_icc_profile( out, NULL );
return out;
}



PF::ClonePar::ClonePar(): 
  PF::OpParBase(),
  //source_channel("source_channel",this,PF::CLONE_CHANNEL_RGB,"RGB","RGB")
  source_channel("source_channel",this,PF::CLONE_CHANNEL_SOURCE,"SOURCE",_("all"))
{
  //source_channel.add_enum_value(PF::CLONE_CHANNEL_GREY,"Grey","Grey");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_R,"R","R");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_G,"G","G");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_B,"B","B");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_MAX_RGB,"MAX_RGB","max(R,G,B)");
  //source_channel.add_enum_value(PF::CLONE_CHANNEL_Lab,"Lab","Lab");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_L,"L","L");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_a,"a","a");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_b,"b","b");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_LCh_C,"LCh_C","C (LCh)");
  source_channel.add_enum_value(PF::CLONE_CHANNEL_LCh_S,"LCh_S","S (LCh)");

  source_channel.set_enum_value(PF::CLONE_CHANNEL_SOURCE);

  //convert2lab = PF::new_operation( "convert2lab", NULL );
  convert_cs = PF::new_convert_colorspace();
  convert_format = new_convert_format();
  desaturate = PF::new_desaturate_luminance();
  maxrgb = PF::new_maxrgb();
  trcconv = PF::new_trcconv();
  rgb2mask = new PF::Processor<PF::RGB2MaskPar,PF::RGB2MaskProc>();

  convert2lab = PF::new_convert_colorspace();
  PF::ConvertColorspacePar* csconvpar = dynamic_cast<PF::ConvertColorspacePar*>(convert2lab->get_par());
  if(csconvpar) {
    csconvpar->set_out_profile_mode( PF::PROF_MODE_DEFAULT );
    csconvpar->set_out_profile_type( PF::PROF_TYPE_LAB );
  }

  set_type( "clone" );

  set_default_name( _("channel selector") );
}


bool PF::ClonePar::accepts_colorspace(PF::colorspace_t cs)
{
  if( cs == PF::PF_COLORSPACE_RGB ) return true;
  if( cs == PF::PF_COLORSPACE_LAB ) return true;
  if( cs == PF::PF_COLORSPACE_CMYK ) return true;
  return false;
}



VipsImage* PF::ClonePar::Lab2grayscale(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  //std::cout<<"ClonePar::Lab2grayscale() called"<<std::endl;
  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );

  if( csin != PF::PF_COLORSPACE_LAB ) {
    PF::ConvertColorspacePar* cpar = dynamic_cast<PF::ConvertColorspacePar*>(convert2lab->get_par());
    if( cpar ) {
      if( ch == PF::CLONE_CHANNEL_LCh_C )
        cpar->set_LCh_format();
      else if( ch == PF::CLONE_CHANNEL_LCh_S )
        cpar->set_LSh_format();
      else
        cpar->set_Lab_format();
    }
    std::cout<<"ClonePar::Lab2grayscale(): ch="<<ch<<"  cpar->get_LSh_format()="<<cpar->get_LSh_format()<<std::endl;
    convert2lab->get_par()->set_image_hints( srcimg );
    convert2lab->get_par()->set_format( get_format() );
    convert2lab->get_par()->lab_image( get_xsize(), get_ysize() );
    std::vector<VipsImage*> in2; in2.push_back( srcimg );
    VipsImage* tempimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
    if( !tempimg ) 
      return NULL;
    //std::cout<<"ClonePar::Lab2grayscale(): convert2lab OK"<<std::endl;
    //g_object_unref( srcimg );
    //PF_UNREF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg unref (csin != PF::PF_COLORSPACE_LAB)" );
    srcimg = tempimg;
  } else {
    PF_REF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg ref (csin != PF::PF_COLORSPACE_LAB)" );
  }
  switch( ch ) {
  case PF::CLONE_CHANNEL_SOURCE:
  case PF::CLONE_CHANNEL_L:
    if( vips_extract_band( srcimg, &out, 0, NULL ) )
      return NULL;
    //std::cout<<"ClonePar::Lab2grayscale(): extract_band OK"<<std::endl;

    //g_object_unref( srcimg );
    PF_UNREF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg unref (PF::CLONE_CHANNEL_L)" );
    vips_image_init_fields( out,
                            get_xsize(), get_ysize(), 
                            1, get_format(),
                            get_coding(),
                            get_interpretation(),
                            1.0, 1.0);
    break;
  case PF::CLONE_CHANNEL_a:
  case PF::CLONE_CHANNEL_LCh_C:
  case PF::CLONE_CHANNEL_LCh_S:
    if( vips_extract_band( srcimg, &out, 1, NULL ) )
      return NULL;
    //g_object_unref( srcimg );
    PF_UNREF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg unref (PF::CLONE_CHANNEL_a)" );
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
    //g_object_unref( srcimg );
    PF_UNREF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg unref (PF::CLONE_CHANNEL_b)" );
    vips_image_init_fields( out,
                            get_xsize(), get_ysize(), 
                            1, get_format(),
                            get_coding(),
                            get_interpretation(),
                            1.0, 1.0);
    break;
  default:
    break;
  }

  // This is a mask image, so it has no associated color profile
  if( out )
    PF::set_icc_profile( out, NULL );
  return out;
}


VipsImage* PF::ClonePar::rgb2grayscale(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );

  std::cout<<"ClonePar::rgb2grayscale(): csin="<<csin<<"  PF_COLORSPACE_RGB="<<PF::PF_COLORSPACE_RGB<<std::endl;
  if( csin != PF::PF_COLORSPACE_RGB ) {
    return NULL;
  }

  PF::RGB2MaskPar* par = dynamic_cast<PF::RGB2MaskPar*>(rgb2mask->get_par());
  if( par ) {
    rgb2mask->get_par()->set_image_hints( srcimg );
    rgb2mask->get_par()->set_format( get_format() );
    par->set_clone_channel( ch );
    std::vector<VipsImage*> in2; in2.push_back( srcimg );
    out = par->build( in2, 0, NULL, NULL, level );
  }
  std::cout<<"ClonePar::rgb2grayscale(): out="<<out<<std::endl;
  return out;

  switch( ch ) {
  case PF::CLONE_CHANNEL_RGB: 
  case PF::CLONE_CHANNEL_SOURCE:
    {
      convert2lab->get_par()->set_image_hints( srcimg );
      convert2lab->get_par()->set_format( get_format() );
      convert2lab->get_par()->lab_image( get_xsize(), get_ysize() );
      std::vector<VipsImage*> in2; in2.push_back( srcimg );
      VipsImage* luma = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );

      if( !luma ) return NULL;
      if( vips_extract_band( luma, &out, 0, NULL ) )
        return NULL;
      vips_image_init_fields( out,
                              get_xsize(), get_ysize(), 
                              1, get_format(),
                              get_coding(),
                              get_interpretation(),
                              1.0, 1.0);
      PF_UNREF( luma, "PF::ClonePar::rgb2grayscale(): luma unref (PF::CLONE_CHANNEL_RGB)" );
      break;
    }
  case PF::CLONE_CHANNEL_R:
    if( vips_extract_band( srcimg, &out, 0, NULL ) )
      return NULL;
    //g_object_unref( srcimg );
    //PF_UNREF( srcimg, "PF::ClonePar::rgb2grayscale(): srcimg unref (PF::CLONE_CHANNEL_R)" );
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
    //g_object_unref( srcimg );
    //PF_UNREF( srcimg, "PF::ClonePar::rgb2grayscale(): srcimg unref (PF::CLONE_CHANNEL_G)" );
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
    //g_object_unref( srcimg );
    //PF_UNREF( srcimg, "PF::ClonePar::rgb2grayscale(): srcimg unref (PF::CLONE_CHANNEL_B)" );
    vips_image_init_fields( out,
                            get_xsize(), get_ysize(), 
                            1, get_format(),
                            get_coding(),
                            get_interpretation(),
                            1.0, 1.0);
    break;
  default:
    break;
  }

  // This is a mask image, so it has no associated color profile
  if( out )
    PF::set_icc_profile( out, NULL );
  return out;
}


VipsImage* PF::ClonePar::rgb2rgb(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  VipsImage* out = NULL;
  VipsImage* band = NULL;
  int bandid = -1;
  VipsImage* bandv[3] = {NULL, NULL, NULL};
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );

  if( csin != PF::PF_COLORSPACE_RGB ) {
    return NULL;
  }
  switch( ch ) {
  case PF::CLONE_CHANNEL_SOURCE:
  case PF::CLONE_CHANNEL_RGB:
    out = srcimg;
    g_object_ref( out );
    //std::cout<<"ClonePar::rgb2rgb(): cloning RGB channels"<<std::endl;
    break;
  case PF::CLONE_CHANNEL_R:
    bandid = 0;
    //std::cout<<"ClonePar::rgb2rgb(): cloning R channel"<<std::endl;
    break;
  case PF::CLONE_CHANNEL_G:
    bandid = 1;
    //std::cout<<"ClonePar::rgb2rgb(): cloning G channel"<<std::endl;
    break;
  case PF::CLONE_CHANNEL_B:
    bandid = 2;
    //std::cout<<"ClonePar::rgb2rgb(): cloning B channel"<<std::endl;
    break;
  default: break;
  }

  if( bandid < 0 ) return out;

  if( vips_extract_band( srcimg, &band, bandid, NULL ) )
    return NULL;
  //g_object_unref( srcimg );
  //PF_UNREF( srcimg, "PF::ClonePar::rgb2grayscale(): srcimg unref (PF::CLONE_CHANNEL_R)" );
  vips_image_init_fields( band,
                          get_xsize(), get_ysize(), 
                          1, get_format(),
                          get_coding(),
                          get_interpretation(),
                          1.0, 1.0);
  bandv[0] = band;
  bandv[1] = band;
  bandv[2] = band;
  if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
    PF_UNREF( band, "ClonePar::rgb2rgb(): band unref after bandjoin failure" );
    return NULL;
  }
  PF_UNREF( band, "ClonePar::rgb2rgb(): band unref after bandjoin" );

  void *profile_data;
  size_t profile_length;
  if( PF_VIPS_IMAGE_GET_BLOB( srcimg, VIPS_META_ICC_NAME, &profile_data, &profile_length ) )
    profile_data = NULL;
  
  if( profile_data ) {
    void* profile_data2 = malloc( profile_length );
    if( profile_data2 ) {
      memcpy( profile_data2, profile_data, profile_length );
      vips_image_set_blob( out, VIPS_META_ICC_NAME, 
                           (VipsCallbackFn) g_free, 
                           profile_data2, profile_length );
    }
  }

  return out;
}


VipsImage* PF::ClonePar::rgb2maxrgb(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );

  if( csin != PF::PF_COLORSPACE_RGB ) {
    return NULL;
  }

  colorspace_t cs = get_colorspace();

  maxrgb->get_par()->set_image_hints( srcimg );
  maxrgb->get_par()->set_format( get_format() );
  if( cs == PF::PF_COLORSPACE_GRAYSCALE ) {
    maxrgb->get_par()->grayscale_image(srcimg->Xsize, srcimg->Ysize);
  }
  if( cs == PF::PF_COLORSPACE_RGB ) {
    maxrgb->get_par()->rgb_image(srcimg->Xsize, srcimg->Ysize);
  }
  std::vector<VipsImage*> in2; in2.push_back( srcimg );
  VipsImage* tempimg = maxrgb->get_par()->build( in2, 0, NULL, NULL, level );

  trcconv->get_par()->set_image_hints( tempimg );
  trcconv->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( tempimg );
  VipsImage* out = trcconv->get_par()->build( in2, 0, NULL, NULL, level );
  if( out ) PF_UNREF(tempimg, "ClonePar::rgb2maxrgb(): tempimg unref");

  return out;
}


VipsImage* PF::ClonePar::Lab2rgb(VipsImage* srcimg, clone_channel ch, unsigned int& level)
{
  if( !srcimg ) return NULL;

  if( ch == PF::CLONE_CHANNEL_L )
    return L2rgb( srcimg, level );


  VipsImage* out = NULL;
  VipsImage* band = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );


  void *profile_data;
  size_t profile_length;
  if( PF_VIPS_IMAGE_GET_BLOB( srcimg, VIPS_META_ICC_NAME, &profile_data, &profile_length ) )
    profile_data = NULL;


  if( csin != PF::PF_COLORSPACE_LAB ) {
    convert2lab->get_par()->set_image_hints( srcimg );
    convert2lab->get_par()->set_format( get_format() );
    convert2lab->get_par()->lab_image( get_xsize(), get_ysize() );
    std::vector<VipsImage*> in2; in2.push_back( srcimg );
    VipsImage* tempimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
    if( !tempimg ) 
      return NULL;
    //g_object_unref( srcimg );
    //PF_UNREF( srcimg, "PF::ClonePar::Lab2grayscale(): srcimg unref (csin != PF::PF_COLORSPACE_LAB)" );
    srcimg = tempimg;
  } else {
    PF_REF( srcimg, "PF::ClonePar::Lab2rgb(): srcimg ref (csin != PF::PF_COLORSPACE_LAB)" );
  }
  switch( ch ) {
  case PF::CLONE_CHANNEL_L:
    out = L2rgb( srcimg, level );
    break;
  case PF::CLONE_CHANNEL_a:
    if( vips_extract_band( srcimg, &band, 1, NULL ) )
      return NULL;
    //g_object_unref( srcimg );
    PF_UNREF( srcimg, "PF::ClonePar::Lab2rgb(): srcimg unref (PF::CLONE_CHANNEL_a)" );
    break;
  case PF::CLONE_CHANNEL_b:
    if( vips_extract_band( srcimg, &band, 2, NULL ) )
      return NULL;
    //g_object_unref( srcimg );
    PF_UNREF( srcimg, "PF::ClonePar::Lab2rgb(): srcimg unref (PF::CLONE_CHANNEL_b)" );
    break;
  default: return srcimg;
  }

  vips_image_init_fields( band,
                          get_xsize(), get_ysize(), 
                          1, get_format(),
                          get_coding(),
                          get_interpretation(),
                          1.0, 1.0);

  VipsImage* bandv[3];
  bandv[0] = band;
  bandv[1] = band;
  bandv[2] = band;
  if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
    PF_UNREF( band, "ClonePar::rgb2rgb(): band unref after bandjoin failure" );
    return NULL;
  }
  PF_UNREF( band, "ClonePar::rgb2rgb(): band unref" );
  rgb_image( out->Xsize, out->Ysize );

  if( profile_data ) {
    void* buf = malloc( profile_length );
    if( buf ) {
      memcpy( buf, profile_data, profile_length );
      vips_image_set_blob( out, VIPS_META_ICC_NAME, 
                           (VipsCallbackFn) g_free, buf, profile_length );
    }
  }
  return out;
}


VipsImage* PF::ClonePar::L2rgb(VipsImage* srcimg, unsigned int& level)
{
  if( !srcimg ) return NULL;

  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg ) 
    csin = PF::convert_colorspace( srcimg->Type );


  void *profile_data;
  size_t profile_length;
  if( PF_VIPS_IMAGE_GET_BLOB( srcimg, VIPS_META_ICC_NAME, &profile_data, &profile_length ) )
    profile_data = NULL;

  // No Lab conversion possible if the input image has no ICC profile
  if( !profile_data ) return srcimg;
  
  std::vector<VipsImage*> in; 

  /*
  convert2lab->get_par()->set_image_hints( srcimg );
  convert2lab->get_par()->set_format( get_format() );
  convert2lab->get_par()->lab_image( get_xsize(), get_ysize() );
  in.clear(); in.push_back( srcimg );
  VipsImage* labimg = convert2lab->get_par()->build( in, 0, NULL, NULL, level );
  if( !labimg ) return NULL;
  //PF_UNREF( srcimg, "ClonePar::L2rgb(): srcimg unref" );
  */
  VipsImage* labimg = srcimg;
  PF_REF( srcimg, "ClonePar::L2rgb(): srcimg ref" );

  in.clear(); in.push_back( labimg );
  desaturate->get_par()->set_image_hints( labimg );
  desaturate->get_par()->set_format( get_format() );
  VipsImage* greyimg = desaturate->get_par()->build( in, 0, NULL, NULL, level );
  PF_UNREF( labimg, "ClonePar::L2rgb(): labimg unref" );
  //VipsImage* greyimg = labimg;

  in.clear(); in.push_back( greyimg );
  convert_cs->get_par()->set_image_hints( greyimg );
  convert_cs->get_par()->set_format( get_format() );
  
  PF::ConvertColorspacePar* csconvpar = dynamic_cast<PF::ConvertColorspacePar*>(convert_cs->get_par());
  if(csconvpar) {
    csconvpar->set_out_profile_mode( PF::PROF_MODE_ICC );
    csconvpar->set_out_profile_data( profile_data, profile_length );
  }
  out = convert_cs->get_par()->build( in, 0, NULL, NULL, level );
  PF_UNREF( greyimg, "ClonePar::L2rgb(): greyimg unref" );

  return out;
}


VipsImage* PF::ClonePar::grey2rgb(VipsImage* srcimg, unsigned int& level)
{
  if( !srcimg ) return NULL;

  VipsImage* out = NULL;
  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );

  VipsImage* greyimg[3] = {srcimg, srcimg, srcimg};
  if( vips_bandjoin( greyimg, &out, 3, NULL) ) {
    std::cout<<"ClonePar::grey2rgb(): vips_bandjoin() failed."<<std::endl;
    return NULL;
  }

  return out;
}


VipsImage* PF::ClonePar::build(std::vector<VipsImage*>& in, int first, 
                               VipsImage* imap, VipsImage* omap, 
                               unsigned int& level)
{
  if( in.empty() ) return NULL;
  VipsImage* srcimg = in[0];
  VipsImage* out = NULL;

  colorspace_t cs = get_colorspace();

  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );

#ifndef NDEBUG
  std::cout<<"ClonePar::build(): in.size()="<<in.size()<<"  srcimg="<<srcimg<<std::endl;
  std::cout<<"ClonePar::build(): colorspace="<<cs<<"  input_colorspace="<<csin<<std::endl;
#endif

  if( !srcimg ) {
    if( in.size() > 0 ) {
      PF_REF( in[0], "ClonePar::bluild(): in[0] ref" );
      return in[0];
    } else {
      return NULL;
    }
  }


  if( cs == PF::PF_COLORSPACE_GRAYSCALE ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      std::cout<<"ClonePar::build(): clone_channel="<<ch<<std::endl;
      if( ch==PF::CLONE_CHANNEL_GREY ||
          (ch==PF::CLONE_CHANNEL_SOURCE && csin == PF::PF_COLORSPACE_GRAYSCALE) ) {
        out = srcimg;
        PF_REF( out, "ClonePar::build(): srcimg ref" );
      }
      if( ch==PF::CLONE_CHANNEL_Lab ||
          ch==PF::CLONE_CHANNEL_L ||
          ch==PF::CLONE_CHANNEL_a ||
          ch==PF::CLONE_CHANNEL_b ||
          ch==PF::CLONE_CHANNEL_LCh_C ||
          ch==PF::CLONE_CHANNEL_LCh_S ||
          (ch==PF::CLONE_CHANNEL_SOURCE && csin == PF::PF_COLORSPACE_LAB) ) {
        unsigned int level2 = level;
        out = Lab2grayscale( srcimg, ch, level2 );
      }
      if( ch==PF::CLONE_CHANNEL_RGB ||
          ch==PF::CLONE_CHANNEL_R ||
          ch==PF::CLONE_CHANNEL_G ||
          ch==PF::CLONE_CHANNEL_B ||
          (ch==PF::CLONE_CHANNEL_SOURCE && csin == PF::PF_COLORSPACE_RGB) ) {
        unsigned int level2 = level;
        std::cout<<"ClonePar::build(): calling rgb2grayscale()"<<std::endl;
        out = rgb2grayscale( srcimg, ch, level2 );
        std::cout<<"ClonePar::build(): rgb2grayscale() out="<<out<<std::endl;
      }
      if( ch==PF::CLONE_CHANNEL_MAX_RGB ) {
        unsigned int level2 = level;
        std::cout<<"ClonePar::build(): calling rgb2maxrgb()"<<std::endl;
        out = rgb2maxrgb( srcimg, ch, level2 );
      }
    }

    if( !out ) {
      // image cannot be created, we revert to a black image of the correct size
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
        //g_object_unref( tmpimg );
        PF_UNREF( tmpimg, "PF::ClonePar::build(): tmpimg unref (get_format() != out->BandFmt)" );
      }
    }
  }

  if( cs == PF::PF_COLORSPACE_RGB ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      //std::cout<<"ClonePar::build(): source_channel="<<ch<<std::endl;
      if( ch==PF::CLONE_CHANNEL_GREY ) {
        unsigned int level2 = level;
        //std::cout<<"ClonePar::build(): calling grey2rgb()"<<std::endl;
        out = grey2rgb( srcimg, level2 );
      }
      if( ch==PF::CLONE_CHANNEL_RGB ||
          ch==PF::CLONE_CHANNEL_R ||
          ch==PF::CLONE_CHANNEL_G ||
          ch==PF::CLONE_CHANNEL_B ||
          (ch==PF::CLONE_CHANNEL_SOURCE && csin == PF::PF_COLORSPACE_RGB) ) {
        unsigned int level2 = level;
        //std::cout<<"ClonePar::build(): calling rgb2rgb()"<<std::endl;
        out = rgb2rgb( srcimg, ch, level2 );
      }
      if( ch==PF::CLONE_CHANNEL_MAX_RGB ) {
        unsigned int level2 = level;
        std::cout<<"ClonePar::build(): calling rgb2maxrgb()"<<std::endl;
        out = rgb2maxrgb( srcimg, ch, level2 );
      }
      if( ch==PF::CLONE_CHANNEL_L ||
          ch==PF::CLONE_CHANNEL_a ||
          ch==PF::CLONE_CHANNEL_b ) {
        unsigned int level2 = level;
        out = Lab2rgb( srcimg, ch, level2 );
      }
    }
  }

  if( cs == PF::PF_COLORSPACE_LAB ) {
    if( srcimg ) {
      // The target colorspace is greyscale, therefore we either pick one channel from the source image
      // or we apply the appropriate conversion to grayscale
      clone_channel ch = (clone_channel)source_channel.get_enum_value().first;
      if( ch==PF::CLONE_CHANNEL_Lab ||
          (ch==PF::CLONE_CHANNEL_SOURCE && csin == PF::PF_COLORSPACE_LAB) ) {
        out = srcimg;
        g_object_ref( out );
      }
    }
  }

  return out;
}
