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

#include "convert_colorspace.hh"
#include "icc_transform.hh"
#include "gaussblur.hh"
#include "guided_filter.hh"
#include "shadows_highlights.hh"


PF::ShadowsHighlightsPar::ShadowsHighlightsPar():
OpParBase(),
method("method",this,PF::SHAHI_GAUSSIAN,"SHAHI_GAUSSIAN","gaussian"),
shadows("shadows",this,50),
highlights("highlights",this,-50),
wp_adjustment("wp_adjustment",this,0),
radius("radius",this,100),
threshold("threshold",this,0.075),
compress("compress",this,50),
sh_color_adjustment("sh_color_adjustment",this,100),
hi_color_adjustment("hi_color_adjustment",this,50),
in_profile( NULL )
{
  method.add_enum_value(PF::SHAHI_GUIDED,"SHAHI_GUIDED","guided");

  gauss = new_gaussblur();
  guided = new_guided_filter();
  convert2lab = PF::new_convert_colorspace();
  PF::ConvertColorspacePar* csconvpar = dynamic_cast<PF::ConvertColorspacePar*>(convert2lab->get_par());
  if(csconvpar) {
    csconvpar->set_out_profile_mode( PF::PROF_MODE_DEFAULT );
    csconvpar->set_out_profile_type( PF::PROF_TYPE_LAB );
  }
  convert2input = new_icc_transform();

  set_type("shadows_highlights" );

  set_default_name( _("shadows/highlights") );
}


bool PF::ShadowsHighlightsPar::needs_caching()
{
  return true;
}



void PF::ShadowsHighlightsPar::propagate_settings()
{
  GaussBlurPar* gausspar = dynamic_cast<GaussBlurPar*>( gauss->get_par() );
  if( gausspar ) {
    gausspar->set_radius( radius.get() );
    gausspar->propagate_settings();
  }

  PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided->get_par() );
  if( guidedpar ) {
    //float ss = 0.01 * guided_radius.get() * MIN(full_res->Xsize, full_res->Ysize);
    //guidedpar->set_radius(ss);
    //guidedpar->set_threshold(guided_threshold.get());
    guidedpar->propagate_settings();
  }
}


void PF::ShadowsHighlightsPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  std::cout<<"ShadowsHighlightsPar::compute_padding(): method.get_enum_value().first="<<method.get_enum_value().first<<std::endl;
  switch( method.get_enum_value().first ) {
  case PF::SHAHI_GAUSSIAN: {
    GaussBlurPar* gausspar = dynamic_cast<GaussBlurPar*>( gauss->get_par() );
    if( gausspar ) {
      gausspar->set_radius( radius.get() );
      gausspar->propagate_settings();
      gausspar->compute_padding(full_res, id, level);
      set_padding( gausspar->get_padding(id), id );
    }
    break;
  }
  case PF::SHAHI_GUIDED: {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided->get_par() );
    if( guidedpar ) {
      guidedpar->set_radius( radius.get() );
      //guidedpar->set_threshold(threshold.get());
      guidedpar->propagate_settings();
      guidedpar->compute_padding(full_res, id, level);
      set_padding( guidedpar->get_padding(id), id );
    }

  }
  default: break;
  }
}




VipsImage* PF::ShadowsHighlightsPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  std::vector<VipsImage*> in2;

  in_profile = PF::get_icc_profile( in[0] );

  convert2lab->get_par()->set_image_hints( in[0] );
  convert2lab->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( in[0] );
  VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
  if( !labimg ) {
    std::cout<<"ShadowsHighlightsPar::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "ShadowsHighlightsPar::build(): null Lab image" );
    return in[0];
  }
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  extended->Xsize="<<extended->Xsize<<std::endl;


  VipsImage* out = NULL;
  VipsImage* blurred = NULL;
  switch( method.get_enum_value().first ) {
  case PF::SHAHI_GAUSSIAN: {
    GaussBlurPar* gausspar = dynamic_cast<GaussBlurPar*>( gauss->get_par() );
    if( gausspar ) {
      gausspar->set_radius( radius.get() );
      gausspar->set_image_hints( labimg );
      gausspar->set_format( get_format() );
      in2.clear(); in2.push_back( labimg );
      blurred = gausspar->build( in2, 0, NULL, NULL, level );
      PF_UNREF( labimg, "ShadowsHighlightsPar::build(): extended unref after convert2lab" );
    }
    break;
  }
  case PF::SHAHI_GUIDED: {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided->get_par() );
    if( guidedpar ) {
      guidedpar->set_radius( radius.get() );
      //guidedpar->set_threshold(threshold.get());
      guidedpar->set_image_hints( labimg );
      guidedpar->set_format( get_format() );
      in2.clear(); in2.push_back( labimg );
      blurred = guidedpar->build( in2, 0, NULL, NULL, level );
      PF_UNREF( labimg, "ShadowsHighlightsPar::build(): extended unref after convert2lab" );
    }
    break;
  }
  default:
    break;
  }

  if( !blurred ) {
    std::cout<<"ShadowsHighlightsPar::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "ShadowsHighlightsPar::build(): null Lab image" );
    return in[0];
  }

  in2.clear();
  in2.push_back(blurred);
  in2.push_back(labimg);
  VipsImage* shahi = OpParBase::build( in2, 0, imap, omap, level );
  //PF_UNREF( labimg, "ShadowsHighlightsPar::update() labimg unref" );
  PF_UNREF( blurred, "ShadowsHighlightsPar::build() blurred unref" );

  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2input->get_par() );
  //std::cout<<"ImageArea::update(): icc_par="<<icc_par<<std::endl;
  if( icc_par ) {
    //std::cout<<"ImageArea::update(): setting display profile: "<<current_display_profile<<std::endl;
    icc_par->set_out_profile( in_profile );
  }
  convert2input->get_par()->set_image_hints( in[0] );
  convert2input->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( shahi );
#ifndef NDEBUG
  std::cout<<"ShadowsHighlightsPar::build(): calling convert2input->get_par()->build()"<<std::endl;
#endif
  out = convert2input->get_par()->build(in2, 0, NULL, NULL, level );
  PF_UNREF( shahi, "ShadowsHighlightsPar::build() cropped unref" );


  set_image_hints( in[0] );

  return out;
}
