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

//#include <vips/cimg_funcs.h>

#include "../../base/processor_imp.hh"
//#include "../icc_transform.hh"
#include "../convert_colorspace.hh"
#include "ocio_config.hh"

PF::OCIOConfigPar::OCIOConfigPar():
  OpParBase(),
  configfile("configfile",this,""/*PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/aces_1.0.3/config.ocio"*/),
  in_profile_type("in_profile_mode",this,PF::PROF_TYPE_NONE,"NONE","NONE"),
  in_trc_type("in_trc_type",this,PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard")),
  cs_in_name("cs_in_name",this,"ACES - ACES2065-1"),
  out_profile_type("out_profile_mode",this,PF::PROF_TYPE_NONE,"NONE","NONE"),
  out_trc_type("out_trc_type",this,PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard")),
  cs_out_name("cs_out_name",this,"Output - sRGB"),
  look_name("look_name",this,"")
{
  in_profile_type.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  in_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  in_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  in_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  in_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  in_profile_type.add_enum_value(PF::PROF_TYPE_XYZ,"XYZ","XYZ D50");
  //in_profile_type.add_enum_value(PF::PROF_TYPE_FROM_SETTINGS,"FROM_SETTINGS","from settings");
  //in_profile_type.add_enum_value(PF::PROF_TYPE_FROM_DISK,"FROM_DISK","ICC from disk");

  in_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR",_("linear"));
  in_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL",_("perceptual"));
  in_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard"));
  in_trc_type.add_enum_value(PF::PF_TRC_GAMMA_18,"PF_TRC_GAMMA_18",_("gamma=1.8"));
  in_trc_type.add_enum_value(PF::PF_TRC_GAMMA_22,"PF_TRC_GAMMA_22",_("gamma=2.2"));

  out_profile_type.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  out_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  out_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  out_profile_type.add_enum_value(PF::PROF_TYPE_XYZ,"XYZ","XYZ D50");
  //out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_SETTINGS,"FROM_SETTINGS","from settings");
  //out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_DISK,"FROM_DISK","ICC from disk");

  out_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR",_("linear"));
  out_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL",_("perceptual"));
  out_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard"));
  out_trc_type.add_enum_value(PF::PF_TRC_GAMMA_18,"PF_TRC_GAMMA_18",_("gamma=1.8"));
  out_trc_type.add_enum_value(PF::PF_TRC_GAMMA_22,"PF_TRC_GAMMA_22",_("gamma=2.2"));

  csconv = new_convert_colorspace();

  set_type( "ocio_transform" );

  set_default_name( _("OCIO transform") );
}



VipsImage* PF::OCIOConfigPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap,
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) {
    std::cout<<"OCIOConfigPar::build(): null input image"<<std::endl;
    return NULL;
  }


  try {
  config  = OCIO::Config::CreateFromFile(configfile.get().c_str());
  std::cout<<"OCIOConfigPar: config="<<config<<std::endl;

  std::cout<<"OCIOConfigPar: creating transfrom \""<<cs_in_name.get()<<"\""
      <<" -> \""<<cs_out_name.get()<<"\""<<std::endl;

  OCIO::LookTransformRcPtr transform = OCIO::LookTransform::Create();
  transform->setSrc( cs_in_name.get().c_str() );
  transform->setDst( cs_out_name.get().c_str() );
  if( !look_name.get().empty() )
    transform->setLooks( look_name.get().c_str() );

  //processor = config->getProcessor(cs_in_name.get().c_str(), cs_out_name.get().c_str());
  processor = config->getProcessor(transform);
  std::cout<<"OCIOConfigPar: processor="<<processor<<std::endl;
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }

  PropertyBase* pmode = csconv->get_par()->get_property("profile_mode");
  if( pmode) {
    pmode->set_enum_value(in_profile_type.get_enum_value().first);
  }
  PropertyBase* ptrc = csconv->get_par()->get_property("trc_type");
  if( ptrc) {
    ptrc->set_enum_value(in_trc_type.get_enum_value().first);
  }
  std::cout<<"OCIOConfigPar::build(): pmode="<<pmode<<"  ptrc="<<ptrc<<std::endl;
  PF::ConvertColorspacePar* par = dynamic_cast<PF::ConvertColorspacePar*>(csconv->get_par());
  if(par) {
    par->set_clip_negative(false);
    par->set_clip_overflow(false);
    par->set_bpc(false);
  }

  VipsImage* out = NULL;
  csconv->get_par()->set_image_hints( srcimg );
  csconv->get_par()->set_format( get_format() );
  out = csconv->get_par()->build( in, 0, NULL, NULL, level);

  VipsImage* out2 = NULL;
  std::vector<VipsImage*> in2;
  in2.push_back( out );
  out2 = OpParBase::build( in2, 0, imap, omap, level);
  //PF_UNREF( srgbimg, "OCIOConfigPar::build() srgbimg unref" );

  ICCProfile* outprof = PF::ICCStore::Instance().get_profile(
      (PF::profile_type_t)out_profile_type.get_enum_value().first,
      (PF::TRC_type)out_trc_type.get_enum_value().first);
  if( out2 )
    PF::set_icc_profile( out2, outprof );

  return out2;
}


PF::ProcessorBase* PF::new_ocio_config()
{
  return( new PF::Processor<PF::OCIOConfigPar,PF::OCIOConfigProc>() );
}
