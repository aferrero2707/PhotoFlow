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
  if(verbose) std::cout<<"SH_HL_mapping: verbose="<<verbose<<std::endl;
  return PF::SH_HL_mapping_log(in, sh_compr, hl_compr, pivot, verbose);
}



void PF::TM_lin_exp_params_t::init( float gain, float slope, float lin_max, float knee_strength,
    float compression, float shoulder_slope)
{
   LE_midgray = pow(0.5,2.45);
   LE_gain = gain;
   //std::cout<<"TM_lin_exp_params_t::init: gain="<<LE_gain<<std::endl;
   LE_Ymidgray = LE_midgray * LE_gain;
   LE_slope = slope;
   LE_slope2 = LE_slope * LE_gain;
   LE_linmax = pow(lin_max,2.45);
  //float LE_linmax2 = SH_HL_mapping( LE_linmax, sh_compr, hl_compr, log_pivot );
   LE_linmax2 = LE_linmax;
   LE_Ylinmax = ( LE_linmax2 - LE_midgray ) * LE_slope2 + LE_Ymidgray;
   LE_Srange = 1.0f - LE_Ylinmax;
   LE_compr = compression;
   LE_compr2 = LE_compr*2.5f;
   LE_Sslope = shoulder_slope;
   LE_Kstrength = knee_strength * ( (LE_slope2-1)*1 + 1 );
   LE_Kmax = ((1.f-LE_slope2)*LE_midgray)/(LE_slope2/(sqrt(2.0f)*LE_Kstrength)-LE_slope2);
   LE_Kymax = (LE_Kmax-LE_midgray)*LE_slope2 + LE_Ymidgray;
   LE_Kexp = LE_Kmax * LE_slope2 / LE_Kymax;
}


void PF::TM_lin_exp(const PF::TM_lin_exp_params_t& par, const float& val, float& result)
{
  if( val > par.LE_linmax2 ) {
    // shoulder
    float X = (val - par.LE_linmax2) * par.LE_slope2 * par.LE_compr / par.LE_Srange;

    result = (par.LE_Srange - par.LE_Srange * exp( -log(X*(par.LE_Sslope)+1)/(par.LE_Sslope) )) / par.LE_compr + par.LE_Ylinmax;
    //if( RGB[k]>1) std::cout<<"RGB[k]="<<RGB[k]<<"  X="<<X<<"  result="<<result<<std::endl;
  } else if( val < par.LE_Kmax ) {
    // knee
    float X = val / par.LE_Kmax;
    result = (X>=0) ? par.LE_Kymax * pow(X,par.LE_Kexp) : -par.LE_Kymax * pow(-X,par.LE_Kexp);
  } else {
    // linear part
    result = (val - par.LE_midgray) * par.LE_slope2 + par.LE_Ymidgray;
  }
}



PF::ToneMappingParV2::ToneMappingParV2():
  OpParBase(),
  preset("preset",this,PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST,"TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST","Medium High Contrast"),
  hue_protection("hue_protection",this,false),
  LE_gain("LE_gain",this,1),
  LE_compression("LE_compression",this,0.95),
  LE_slope("LE_slope",this,1.06),
  LE_lin_max("LE_lin_max",this,0.50),
  LE_knee_strength("LE_knee_strength",this,1.2),
  LE_shoulder_slope("LE_shoulder_slope",this,1.2),
  LE_shoulder_slope2("LE_shoulder_slope2",this,0.5),
  LE_shoulder_max("LE_shoulder_max",this,10),
  lumi_blend_frac("lumi_blend_frac",this,0),
  saturation_scaling("saturation_scaling",this,0.0),
  hl_desaturation("hl_desaturation",this,0.0),
  local_contrast_amount("local_contrast_amount",this,0.0),
  local_contrast_radius("local_contrast_radius",this,0.5),
  local_contrast_threshold("local_contrast_threshold",this,0.075),
  icc_data( NULL )
{
  preset.add_enum_value(PF::TONE_MAPPING_PRESET_CUSTOM,"TONE_MAPPING_PRESET_CUSTOM","Custom");
  preset.add_enum_value(PF::TONE_MAPPING_PRESET_BASE_CONTRAST,"TONE_MAPPING_PRESET_BASE_CONTRAST","Base Contrast");
  preset.add_enum_value(PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST,"TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST","Medium High Contrast");
  preset.add_enum_value(PF::TONE_MAPPING_PRESET_HIGH_CONTRAST,"TONE_MAPPING_PRESET_HIGH_CONTRAST","High Contrast");
  preset.add_enum_value(PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST,"TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST","Very High Contrast");

  LE_gain_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 1.40275;
  LE_slope_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 1.07402;
  LE_lin_max_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 0.286394;
  LE_knee_strength_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 9.84638;
  LE_shoulder_slope_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 1.14896;
  LE_shoulder_max_presets[PF::TONE_MAPPING_PRESET_BASE_CONTRAST] = 14.58;

  LE_gain_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 1.23412;
  LE_slope_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 1.10505;
  LE_lin_max_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 0.447559;
  LE_knee_strength_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 9.50194;
  LE_shoulder_slope_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 0.936285;
  LE_shoulder_max_presets[PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST] = 15.45;

  LE_gain_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 1.20607;
  LE_slope_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 1.17455;
  LE_lin_max_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 0.5061;
  LE_knee_strength_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 6.72992;
  LE_shoulder_slope_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 0.737952;
  LE_shoulder_max_presets[PF::TONE_MAPPING_PRESET_HIGH_CONTRAST] = 15.05;

  LE_gain_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 1.20033;
  LE_slope_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 1.30987;
  LE_lin_max_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 0.546617;
  LE_knee_strength_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 2.30421;
  LE_shoulder_slope_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 0.694874;
  LE_shoulder_max_presets[PF::TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST] = 21;


  guided_blur = new_guided_filter();

  set_type("tone_mapping_v2" );

  set_default_name( _("tone mapping") );
}



void PF::ToneMappingParV2::propagate_settings()
{
  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided_blur->get_par() );
  if( guidedpar ) {
    guidedpar->propagate_settings();
  }
}



void PF::ToneMappingParV2::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided_blur->get_par() );
  if( guidedpar ) {
    float ss = 0.01 * local_contrast_radius.get() * MIN(full_res->Xsize, full_res->Ysize);
    guidedpar->set_radius(ss);
    //guidedpar->set_radius(local_contrast_radius.get());
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


void PF::ToneMappingParV2::pre_build( rendermode_t mode )
{
  if( preset.get_enum_value().first != PF::TONE_MAPPING_PRESET_CUSTOM ) {
    LE_gain.set( LE_gain_presets[preset.get_enum_value().first] );
    LE_slope.set( LE_slope_presets[preset.get_enum_value().first] );
    LE_lin_max.set( LE_lin_max_presets[preset.get_enum_value().first] );
    LE_knee_strength.set( LE_knee_strength_presets[preset.get_enum_value().first] );
    LE_shoulder_slope.set( LE_shoulder_slope_presets[preset.get_enum_value().first] );
    LE_shoulder_max.set( LE_shoulder_max_presets[preset.get_enum_value().first] );
  }

  float delta = 0;
  bool found = false;
  float compression = (LE_compression.get() == 0) ? 0.5 : LE_compression.get();
  float compression_min = 0, compression_max = 1;
  int iter = 0;
  TM_lin_exp_params_t par;
  float max = LE_shoulder_max.get(), maxout;
  while( !found ) {
    par.init(LE_gain.get(), LE_slope.get(), LE_lin_max.get(), LE_knee_strength.get(),
      compression, LE_shoulder_slope.get());
    TM_lin_exp( par, max, maxout );
    float delta = maxout - 1.0f;
    std::cout<<iter<<"  compression="<<compression<<"  maxout="<<maxout<<std::endl;
    if( delta > -1.0e-10 && delta < 1.0e-10 ) {
      found = true;
      break;
    }
    if( iter > 1000 ) break;
    if( delta > 0 ) { compression_min = compression; compression = (compression_max+compression_min) / 2; }
    else { compression_max = compression; compression = (compression_max+compression_min) / 2; }
    iter++;
  }

  //if( found ) {
    LE_compression.set(compression);
    std::cout<<"ToneMappingParV2::build: max="<<max<<"  maxout="<<maxout<<std::endl;
  //}
}



VipsImage* PF::ToneMappingParV2::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  ICCProfile* new_icc_data = PF::get_icc_profile( in[0] );
  std::cout<<"ToneMappingParV2::build: new_icc_data="<<new_icc_data<<std::endl;

  LE_params.init(LE_gain.get(), LE_slope.get(), LE_lin_max.get(), LE_knee_strength.get(),
      LE_compression.get(), LE_shoulder_slope.get());

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
  rgb_image(in[0]->Xsize, in[0]->Ysize);
  VipsImage* out = OpParBase::build( in2, 0, NULL, NULL, level );
  PF_UNREF(smoothed, "ToneMappingParV2::build(): smoothed unref");

  return out;
}
