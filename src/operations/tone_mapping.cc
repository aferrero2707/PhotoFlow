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
  method("method",this,PF::TONE_MAPPING_EXP_GAMMA,"TONE_MAPPING_EXP_GAMMA","exposure/gamma"),
  exposure("exposure",this,1),
  gamma("gamma",this,2.2),
  filmic_A("filmic_A",this,0.22),
  filmic_B("filmic_B",this,0.30),
  filmic_C("filmic_C",this,0.10),
  filmic_D("filmic_D",this,0.20),
  filmic_E("filmic_E",this,0.01),
  filmic_F("filmic_F",this,0.30),
  filmic_W("filmic_W",this,11.2),
  lumi_blend_frac("lumi_blend_frac",this,0),
  icc_data( NULL )
{
  method.add_enum_value(PF::TONE_MAPPING_REINHARD,"TONE_MAPPING_REINHARD","Reinhard");
  method.add_enum_value(PF::TONE_MAPPING_HEJL,"TONE_MAPPING_HEJL","Hejl-Dawson");
  method.add_enum_value(PF::TONE_MAPPING_FILMIC,"TONE_MAPPING_FILMIC","filmic");
  set_type("tone_mapping" );

  set_default_name( _("tone mapping") );
}



VipsImage* PF::ToneMappingPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  std::cout<<"ToneMappingPar::build(): in.size()="<<in.size()<<std::endl;

  exponent = 1.f/gamma.get();

  icc_data = PF::get_icc_profile( in[0] );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  return out;
}



PF::ProcessorBase* PF::new_tone_mapping()
{
  return new PF::Processor<PF::ToneMappingPar,PF::ToneMapping>();
}
