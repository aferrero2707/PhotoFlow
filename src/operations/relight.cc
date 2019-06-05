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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/processor_imp.hh"
#include "tone_mapping_v2.hh"
#include "relight.hh"
#include "operations.hh"



PF::RelightPar::RelightPar():
OpParBase(),
strength("strength",this,pow(10, 0.2)),
range("range",this,0.75),
contrast("contrast",this,0),
LE_compression("LE_compression",this,0.95)
{
  shahl = new_shadows_highlights_v2();
  if( shahl && shahl->get_par() ) {
    PF::OpParBase* par = shahl->get_par();
    par->set_property( "amount", pow(10, 0.5) );
    par->set_property( "shadows", strength.get() );
    par->set_property( "shadows_range", 5.0f );
    par->set_property( "highlights", pow(10, 0.0) );
    par->set_property( "highlights_range", 0.5 );
    par->set_property( "constrast", 0.0f );
    par->set_property( "constrast_threshold", 1 );
    par->set_property( "anchor", range.get() );
    par->set_property( "sh_radius", 128 );
    par->set_property( "sh_threshold", 0.1 );
  }

  tm = new_tone_mapping_v2();
  if( tm && tm->get_par() ) {
    PF::OpParBase* par = tm->get_par();
    PF::PropertyBase* p = par->get_property("preset");
    if( p ) p->set_enum_value(PF::TONE_MAPPING_PRESET_CUSTOM);
    par->set_property( "hue_protection", false );
    par->set_property( "LE_gain", 1 );
    par->set_property( "LE_compression", 0.639112f );
    par->set_property( "LE_slope", contrast.get()+1.0f );
    par->set_property( "LE_lin_max", range.get() );
    par->set_property( "LE_knee_strength", 1.0f );
    par->set_property( "LE_shoulder_slope", 0.0f );
    par->set_property( "LE_shoulder_max", 1.0f );
    par->set_property( "lumi_blend_frac", 0 );
    par->set_property( "saturation_scaling", 0.0 );
    par->set_property( "sh_desaturation", 0.5 );
    par->set_property( "hl_desaturation", 1.0 );
    par->set_property( "local_contrast_amount", 0.0 );
    par->set_property( "local_contrast_radius", 0.5 );
    par->set_property( "local_contrast_threshold", 0.075 );
  }

  set_type("relight" );

  set_default_name( _("relight") );
}


bool PF::RelightPar::needs_caching()
{
  return true;
}


void PF::RelightPar::propagate_settings()
{
  if( shahl && shahl->get_par() ) shahl->get_par()->propagate_settings();
  if( tm && tm->get_par() ) {
    PF::OpParBase* par = tm->get_par();
    PF::PropertyBase* p = par->get_property("preset");
    if( p ) p->set_enum_value(PF::TONE_MAPPING_PRESET_CUSTOM);
    par->set_property( "hue_protection", false );
    par->set_property( "LE_gain", 1 );
    par->set_property( "LE_compression", LE_compression.get() );
    par->set_property( "LE_slope", contrast.get()+1.0f );
    par->set_property( "LE_lin_max", range.get() );
    par->set_property( "LE_knee_strength", 1.0f );
    par->set_property( "LE_shoulder_slope", 0.0f );
    par->set_property( "LE_shoulder_max", 1.0f );
    par->set_property( "lumi_blend_frac", 0 );
    par->set_property( "saturation_scaling", 0.0 );
    par->set_property( "sh_desaturation", 0.5 );
    par->set_property( "hl_desaturation", 1.0 );
    par->set_property( "local_contrast_amount", 0.0 );
    par->set_property( "local_contrast_radius", 0.5 );
    par->set_property( "local_contrast_threshold", 0.075 );
    par->propagate_settings();
  }
}


void PF::RelightPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  int tot_padding = 0;
  if( shahl && shahl->get_par() ) {
    PF::OpParBase* par = shahl->get_par();
    par->set_property( "shadows", strength.get() );
    par->set_property( "shadows_range", range.get() );
    par->propagate_settings();
    par->compute_padding(full_res, id, level);
    tot_padding += par->get_padding(id);
  }

  set_padding( tot_padding, id );
}



void PF::RelightPar::pre_build( rendermode_t mode )
{
  if( tm && tm->get_par() ) {
    PF::OpParBase* par = tm->get_par();
    PF::PropertyBase* p = par->get_property("preset");
    if( p ) p->set_enum_value(PF::TONE_MAPPING_PRESET_CUSTOM);
    par->set_property( "hue_protection", false );
    par->set_property( "LE_gain", 1 );
    par->set_property( "LE_compression", 0.639112f );
    par->set_property( "LE_slope", contrast.get()+1.0f );
    par->set_property( "LE_lin_max", range.get() );
    par->set_property( "LE_knee_strength", 1.0f );
    par->set_property( "LE_shoulder_slope", 0.0f );
    par->set_property( "LE_shoulder_max", 1.0f );
    par->set_property( "lumi_blend_frac", 0 );
    par->set_property( "saturation_scaling", 0.0 );
    par->set_property( "sh_desaturation", 0.5 );
    par->set_property( "hl_desaturation", 1.0 );
    par->set_property( "local_contrast_amount", 0.0 );
    par->set_property( "local_contrast_radius", 0.5 );
    par->set_property( "local_contrast_threshold", 0.075 );

    tm->get_par()->pre_build( mode );

    p = par->get_property("LE_compression");
    if( p ) LE_compression.import(p);
    par->set_property( "LE_compression", LE_compression.get() );

  }
}

VipsImage* PF::RelightPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  std::vector<VipsImage*> in2;

  VipsImage* srcimg = in[0];
  VipsImage* outimg = NULL;
  if( true && strength.get() > 1 && shahl && shahl->get_par() ) {
    PF::OpParBase* par = shahl->get_par();
    par->set_property( "amount", pow(10, 0.5) );
    par->set_property( "shadows", strength.get() );
    par->set_property( "shadows_range", 5.0f );
    par->set_property( "highlights", pow(10, 0.0) );
    par->set_property( "highlights_range", 0.5f );
    par->set_property( "constrast", 0.0f );
    par->set_property( "constrast_threshold", 1 );
    par->set_property( "anchor", range.get() );
    par->set_property( "sh_radius", 128 );
    par->set_property( "sh_threshold", 0.1 );

    par->set_image_hints( srcimg );
    par->set_format( get_format() );
    par->propagate_settings();
    par->compute_padding(srcimg, 0, level);
    in2.clear(); in2.push_back( srcimg );
    outimg = par->build( in2, 0, NULL, NULL, level );
  } else {
    outimg = srcimg;
    PF_REF(srcimg, "RelightPar::build: srcimg unref after tm");
  }

  srcimg = outimg;
  if( contrast.get() > 0 && tm && tm->get_par() ) {
    std::cout<<"RelightPar::build: building the tone mapping module"<<std::endl;
    PF::OpParBase* par = tm->get_par();
    PF::PropertyBase* p = par->get_property("preset");
    if( p ) p->set_enum_value(PF::TONE_MAPPING_PRESET_CUSTOM);
    par->set_property( "hue_protection", false );
    par->set_property( "LE_gain", 1 );
    par->set_property( "LE_compression", LE_compression.get() );
    par->set_property( "LE_slope", contrast.get()+1.0f );
    par->set_property( "LE_lin_max", range.get() );
    par->set_property( "LE_knee_strength", 1.0f );
    par->set_property( "LE_shoulder_slope", 0.0f );
    par->set_property( "LE_shoulder_max", 1.0f );
    par->set_property( "lumi_blend_frac", 0 );
    par->set_property( "saturation_scaling", 0.0 );
    par->set_property( "sh_desaturation", 0.5 );
    par->set_property( "hl_desaturation", 1.0 );
    par->set_property( "local_contrast_amount", 0.0 );
    par->set_property( "local_contrast_radius", 0.5 );
    par->set_property( "local_contrast_threshold", 0.075 );

    par->set_image_hints( srcimg );
    par->set_format( get_format() );
    in2.clear(); in2.push_back( srcimg );
    outimg = par->build( in2, 0, NULL, NULL, level );
    PF_UNREF(srcimg, "RelightPar::build: srcimg unref after tm");
  }

  return outimg;
}



PF::ProcessorBase* PF::new_relight()
{ return new PF::Processor<PF::RelightPar,PF::RelightProc>(); }
