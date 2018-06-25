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

#include "tone_mapping.hh"



PF::ToneMappingPar::ToneMappingPar():
  OpParBase(),
  method("method",this,PF::TONE_MAPPING_FILMIC2,"TONE_MAPPING_FILMIC2","filmic new"),
  exposure("exposure",this,1),
  gamma("gamma",this,2.2),
  filmic_A("filmic_A",this,0.22),
  filmic_B("filmic_B",this,0.30),
  filmic_C("filmic_C",this,0.10),
  filmic_D("filmic_D",this,0.20),
  filmic_E("filmic_E",this,0.01),
  filmic_F("filmic_F",this,0.30),
  filmic_W("filmic_W",this,11.2),
  filmic2_gamma("filmic2_gamma",this,0),
  filmic2_TS("filmic2_TS",this,0.5),
  filmic2_TL("filmic2_TL",this,0.5),
  filmic2_SS("filmic2_SS",this,2.0),
  filmic2_SL("filmic2_SL",this,0.5),
  filmic2_SA("filmic2_SA",this,1.0),
  AL_Lmax("AL_Lmax",this,2),
  AL_b("AL_b",this,0.85),
  lumi_blend_frac("lumi_blend_frac",this,0),
  icc_data( NULL )
{
  method.add_enum_value(PF::TONE_MAPPING_FILMIC2,"TONE_MAPPING_FILMIC2","filmic new");
  method.add_enum_value(PF::TONE_MAPPING_FILMIC,"TONE_MAPPING_FILMIC","filmic");
  method.add_enum_value(PF::TONE_MAPPING_EXP_GAMMA,"TONE_MAPPING_EXP_GAMMA","gamma");
  method.add_enum_value(PF::TONE_MAPPING_REINHARD,"TONE_MAPPING_REINHARD","Reinhard");
  method.add_enum_value(PF::TONE_MAPPING_HEJL,"TONE_MAPPING_HEJL","Hejl-Dawson");
  method.add_enum_value(PF::TONE_MAPPING_ADAPTIVE_LOG,"TONE_MAPPING_ADAPTIVE_LOG","adaptive log");
  set_type("tone_mapping" );

  set_default_name( _("tone mapping") );
}



VipsImage* PF::ToneMappingPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  exponent = 1.f/gamma.get();
  filmic2_exponent = (filmic2_gamma.get() >= 0) ? 1.f/(filmic2_gamma.get()+1) : (1.f-filmic2_gamma.get());

  icc_data = PF::get_icc_profile( in[0] );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  return out;
}
