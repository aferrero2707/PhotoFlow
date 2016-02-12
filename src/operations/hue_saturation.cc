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

//#include <arpa/inet.h>

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../dt/common/colorspaces.h"
//#include "../base/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "gaussblur.hh"
#include "hsl_mask.hh"

#include "hue_saturation.hh"



PF::HueSaturationPar::HueSaturationPar(): 
  OpParBase(),
  hue("hue",this,0),
  hue_eq("hue_eq",this,0),
  saturation("saturation",this,0),
  saturation_eq("saturation_eq",this,0),
  contrast("contrast",this,0),
  contrast_eq("contrast_eq",this,0),
  brightness("brightness",this,0),
  brightness_eq("brightness_eq",this,0),
  brightness_is_gamma("brightness_is_gamma",this,false),
  exposure("exposure",this,1),
  white_level("white_level",this,0),
  black_level("black_level",this,0),
  hue_H_equalizer( "hue_H_equalizer", this ),
  hue_S_equalizer( "hue_S_equalizer", this ),
  hue_L_equalizer( "hue_L_equalizer", this ),
  hue_H_equalizer_enabled( "hue_H_equalizer_enabled", this, false ),
  hue_S_equalizer_enabled( "hue_S_equalizer_enabled", this, false ),
  hue_L_equalizer_enabled( "hue_L_equalizer_enabled", this, false ),
  saturation_H_equalizer( "saturation_H_equalizer", this ),
  saturation_S_equalizer( "saturation_S_equalizer", this ),
  saturation_L_equalizer( "saturation_L_equalizer", this ),
  contrast_H_equalizer( "contrast_H_equalizer", this ),
  contrast_S_equalizer( "contrast_S_equalizer", this ),
  contrast_L_equalizer( "contrast_L_equalizer", this ),
  brightness_H_equalizer( "brightness_H_equalizer", this ),
  brightness_S_equalizer( "brightness_S_equalizer", this ),
  brightness_L_equalizer( "brightness_L_equalizer", this ),
  show_mask("show_mask",this,false),
  invert_mask("invert_mask",this,false),
  feather_mask("feather_mask",this,false),
  feather_radius("feather_radius",this,5.0f)
{
  set_type("hue_saturation" );

  set_default_name( _("basic adjustments") );

  mask = new_hsl_mask();
  blur = new_gaussblur();

  int id = 0;
  eq_vec[id++] = &hue_H_equalizer;
  eq_vec[id++] = &hue_S_equalizer;
  eq_vec[id++] = &hue_L_equalizer;
/*
  eq_vec[id++] = &saturation_H_equalizer;
  eq_vec[id++] = &saturation_S_equalizer;
  eq_vec[id++] = &saturation_L_equalizer;
  eq_vec[id++] = &contrast_H_equalizer;
  eq_vec[id++] = &contrast_S_equalizer;
  eq_vec[id++] = &contrast_L_equalizer;
  eq_vec[id++] = &brightness_H_equalizer;
  eq_vec[id++] = &brightness_S_equalizer;
  eq_vec[id++] = &brightness_L_equalizer;
*/

  hue_H_equalizer.get().set_circular( true );

  float x1 = 0, y1 = 0., x2 = 1, y2 = 0.;
  for( id = 0; id < 3; id++ ) {
    eq_vec[id]->get().set_point( 0, x1, y1 );
    eq_vec[id]->get().set_point( 1, x2, y2 );
    eq_vec[id]->store_default();
  }
  /*
  for( id = 0; id < 3; id+=3 ) {
    float x = 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
  }
*/
  x1 = 0; y1 = 0.5;
  //eq_vec[0]->get().set_point( 0, x1, y1 );

  //lab_profile = dt_colorspaces_create_lab_profile();
  std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/Lab-D50-Identity-elle-V4.icc";
  lab_profile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );
  transform = NULL;
}



void PF::HueSaturationPar::update_curve( PF::Property<PF::SplineCurve>* curve, float* vec )
{
  curve->get().lock();
  //std::cout<<"CurvesPar::update_curve() called. # of points="<<curve.get().get_npoints()<<std::endl;std::cout.flush();
  for(int i = 0; i <= 65535; i++) {
    float x = ((float)i)/65535;
    float y = curve->get().get_value( x );
    if( y>1 ) y=1;
    if( y<0 ) y=0;
    vec[i] = y;
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec8[i]="<<vec8[i]<<std::endl;
  }
  curve->get().unlock();
}



VipsImage* PF::HueSaturationPar::build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level)
{
  for( int id = 0; id < 3; id++ ) {
    if( eq_vec[id]->is_modified() )
      update_curve( eq_vec[id], vec[id] );
    eq_enabled[id] = false;
    //std::cout<<"eq_vec["<<id<<"]->get().get_npoints()="<<eq_vec[id]->get().get_npoints()<<std::endl;
    //for( size_t pi = 0; pi < eq_vec[id]->get().get_npoints(); pi++ ) {
      //std::cout<<"  get_point("<<pi<<").second="<<eq_vec[id]->get().get_point(pi).second<<std::endl;
      //if( fabs(eq_vec[id]->get().get_point(pi).second) > 0.001 ) {
        //eq_enabled[id] = true;
        //break;
      //}
    //}
  }
  eq_enabled[0] = hue_H_equalizer_enabled.get();
  eq_enabled[1] = hue_S_equalizer_enabled.get();
  eq_enabled[2] = hue_L_equalizer_enabled.get();


  icc_data = PF::get_icc_profile_data( in[0] );

  void *prof_data;
  size_t prof_data_length;

  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
                           &prof_data, &prof_data_length ) ) {
    if( transform )
      cmsDeleteTransform( transform );

    //std::cout<<"ConvertColorspacePar::build(): image="<<in[0]<<" data="<<data<<" data_length="<<data_length<<std::endl;
    cmsHPROFILE in_profile = cmsOpenProfileFromMem( prof_data, prof_data_length );

    transform = NULL;
    cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, in_profile );
    cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, lab_profile );

    transform = cmsCreateTransform( in_profile,
        TYPE_RGB_FLT,//infmt,
        lab_profile,
        TYPE_Lab_FLT,//outfmt,
        INTENT_RELATIVE_COLORIMETRIC,
        cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE );

    transform_inv = cmsCreateTransform( lab_profile,
        TYPE_Lab_FLT,//outfmt,
        in_profile,
        TYPE_RGB_FLT,//infmt,
        INTENT_RELATIVE_COLORIMETRIC,
        cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE );
  }


  std::vector<VipsImage*> in2;
  if( in.size() < 1 )
    return NULL;

  in2.push_back( in[0] );
  PF::HSLMaskPar* mask_par = dynamic_cast<PF::HSLMaskPar*>( mask->get_par() );
  PF::GaussBlurPar* blur_par = dynamic_cast<PF::GaussBlurPar*>( blur->get_par() );
  if( mask_par ) {
    std::vector<VipsImage*> in3; in3.push_back( in[0] );
    mask_par->get_H_curve() = hue_H_equalizer;
    mask_par->get_S_curve() = hue_S_equalizer;
    mask_par->get_L_curve() = hue_L_equalizer;
    mask_par->set_H_curve_enabled( hue_H_equalizer_enabled.get() );
    mask_par->set_S_curve_enabled( hue_S_equalizer_enabled.get() );
    mask_par->set_L_curve_enabled( hue_L_equalizer_enabled.get() );
    mask_par->set_invert( get_invert_mask() );
    mask_par->set_image_hints( in[0] );
    mask_par->set_format( get_format() );
    VipsImage* imask = mask_par->build( in3, 0, NULL, NULL, level );

    if( feather_mask.get() == true ) {
      blur_par->set_radius( feather_radius.get() );
      blur_par->set_image_hints( in[0] );
      blur_par->set_format( get_format() );
      in3.clear();
      in3.push_back( imask );
      VipsImage* blurred = blur_par->build( in3, 0, NULL, NULL, level );
      std::cout<<"blurred = "<<blurred<<std::endl;
      PF_UNREF( imask, "HueSaturationPar::build(): imask unref");
      imask = blurred;
    }

    in2.push_back( imask );
  }

  VipsImage* out = PF::OpParBase::build( in2, 0, imap, omap, level );

  if( in2.size()>1 ) PF_UNREF( in2[1], "HueSaturationPar::build(): in[2] unref");

  return out;
}


PF::ProcessorBase* PF::new_hue_saturation()
{
  return new PF::Processor<PF::HueSaturationPar,PF::HueSaturation>();
}
