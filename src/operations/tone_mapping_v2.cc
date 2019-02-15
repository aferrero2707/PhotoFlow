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

#include "guided_filter.hh"
#include "tone_mapping_v2.hh"


//const int PF::GamutMapNYPoints = 1000;


#define Float_t float
#define Double_t float


float PF::SH_HL_mapping( const float& in, const float& sh_compr, const float& hl_compr, const float& pivot, bool verbose )
{
  return PF::SH_HL_mapping_log(in, sh_compr, hl_compr, pivot, verbose);
}





PF::ToneMappingParV2::ToneMappingParV2():
  OpParBase(),
  method("method",this,PF::TONE_MAPPING_LIN_EXP,"TONE_MAPPING_LIN_EXP","linear + exp"),
  exposure("exposure",this,1),
  sh_compr("sh_compr",this,0.0),
  hl_compr("hl_compr",this,0.0),
  log_pivot("log_pivot",this,0.5),
  gamut_compression("gamut_compression",this,false),
  hue_protection("hue_protection",this,false),
  filmic_A("filmic_A",this,0.22),
  filmic_B("filmic_B",this,0.30),
  filmic_C("filmic_C",this,0.10),
  filmic_D("filmic_D",this,0.20),
  filmic_E("filmic_E",this,0.01),
  filmic_F("filmic_F",this,0.30),
  filmic_W("filmic_W",this,11.2),
  filmic2_preserve_midgray("filmic2_preserve_midgray",this,false),
  filmic2_gamma("filmic2_gamma",this,0),
  filmic2_TS("filmic2_TS",this,0.5),
  filmic2_TL("filmic2_TL",this,0.5),
  filmic2_SS("filmic2_SS",this,2.0),
  filmic2_SL("filmic2_SL",this,0.5),
  filmic2_SA("filmic2_SA",this,1.0),
  AL_Lmax("AL_Lmax",this,2),
  AL_b("AL_b",this,0.85),
  AL_Tsize("AL_Tsize",this,0.5),
  AL_Tlength("AL_Tlength",this,10),
  AL_Tstrength("AL_Tstrength",this,0.5),
  LP_compression("LP_compression",this,2.7),
  LP_slope("LP_slope",this,1.09),
  LP_lin_max("LP_lin_max",this,0.5),
  LP_knee_strength("LP_knee_strength",this,1.2),
  LP_shoulder_smoothness("LP_shoulder_smoothness",this,0),
  LE_gain("LE_gain",this,1),
  LE_compression("LE_compression",this,0.95),
  LE_slope("LE_slope",this,1.06),
  LE_lin_max("LE_lin_max",this,0.50),
  LE_knee_strength("LE_knee_strength",this,1.2),
  LE_shoulder_slope("LE_shoulder_slope",this,1.2),
  LE_shoulder_slope2("LE_shoulder_slope2",this,0.5),
  HD_slope("HD_slope",this,1.1),
  HD_toe_range("HD_toe_range",this,0.00001),
  HD_shoulder_range("HD_shoulder_range",this,0.2),
  gamut_compression_amount("gamut_compression_amount",this,0),
  gamut_compression_exponent("gamut_compression_exponent",this,1),
  lumi_blend_frac("lumi_blend_frac",this,100),
  local_contrast_amount("local_contrast_amount",this,0.0),
  local_contrast_radius("local_contrast_radius",this,10),
  local_contrast_threshold("local_contrast_threshold",this,0.075),
  icc_data( NULL )
{
  method.add_enum_value(PF::TONE_MAPPING_LIN_EXP,"TONE_MAPPING_LIN_EXP","linear + exp.");

  guided_blur = new_guided_filter();

  set_type("tone_mapping_v2" );

  set_default_name( _("tone mapping") );
}



void PF::ToneMappingParV2::propagate_settings()
{
  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided_blur->get_par() );
  if( guidedpar ) {
    guidedpar->set_radius(local_contrast_radius.get());
    guidedpar->set_threshold(local_contrast_threshold.get());
    guidedpar->propagate_settings();
  }
}



void PF::ToneMappingParV2::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided_blur->get_par() );
  if( guidedpar ) {
    float ss = 10; //0.01 * guided_radius.get() * MIN(full_res->Xsize, full_res->Ysize);
    guidedpar->set_radius(local_contrast_radius.get());
    guidedpar->set_threshold(local_contrast_threshold.get());
    guidedpar->propagate_settings();
    guidedpar->compute_padding(full_res, id, level);
    set_padding( guidedpar->get_padding(id), id );
    //if(true)
    //  std::cout<<"ToneMappingParV2()::compute_padding(): guided_r="<<guided_radius.get()
    //  <<"  image size="<<MIN(full_res->Xsize, full_res->Ysize)
    //  <<"  ss="<<ss<<"  level="<<level<<"  padding="<<get_padding(id)<<std::endl;
  }
}



VipsImage* PF::ToneMappingParV2::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  exponent = 1.f;// /gamma.get();
  filmic2_exponent = (filmic2_gamma.get() >= 0) ? 1.f/(filmic2_gamma.get()+1) : (1.f-filmic2_gamma.get());

  ICCProfile* new_icc_data = PF::get_icc_profile( in[0] );
  if( new_icc_data && gamut_compression.get() ) new_icc_data->init_gamut_mapping();

  icc_data = new_icc_data;

  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided_blur->get_par() );

  std::vector<VipsImage*> in2;
  in2.clear();
  in2.push_back(in[0]);
  VipsImage* smoothed = NULL;
  guidedpar->set_convert_to_perceptual(true);
  guidedpar->set_image_hints( in2[0] );
  guidedpar->set_format( get_format() );
  smoothed = guidedpar->build( in2, 0, NULL, NULL, level );
  if( !smoothed ) {
    std::cout<<"ToneMappingParV2::build(): NULL local contrast enhanced image"<<std::endl;
    return NULL;
  }
  //PF_UNREF(cached, "ToneMappingParV2::build(): cached unref");

  in2.clear();
  in2.push_back(smoothed);
  in2.push_back(in[0]);
  VipsImage* out = OpParBase::build( in2, 0, NULL, NULL, level );
  PF_UNREF(smoothed, "ToneMappingParV2::build(): smoothed unref");

  return out;
}
