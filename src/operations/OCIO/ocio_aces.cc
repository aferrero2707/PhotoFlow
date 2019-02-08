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
#include "../icc_transform.hh"
#include "ocio_aces.hh"

PF::OCIOACESPar::OCIOACESPar():
  OpParBase(),
  view_name("view_name",this,PF::ACES_VIEW_sRGB,"ACES_VIEW_sRGB","sRGB")
{
  view_name.add_enum_value(PF::ACES_VIEW_sRGB,"ACES_VIEW_sRGB","sRGB");
  view_name.add_enum_value(PF::ACES_VIEW_DCDM,"ACES_VIEW_DCDM","DCDM");
  view_name.add_enum_value(PF::ACES_VIEW_DCDM_P3_gamut_clip,"ACES_VIEW_DCDM_P3_gamut_clip","DCDM P3 gamut clip");
  view_name.add_enum_value(PF::ACES_VIEW_P3_D60,"ACES_VIEW_P3_D60","P3-D60");
  view_name.add_enum_value(PF::ACES_VIEW_P3_D60_ST2084_1000_nits,"ACES_VIEW_P3_D60_ST2084_1000_nits","P3-D60 ST2084 1000 nits");
  view_name.add_enum_value(PF::ACES_VIEW_P3_D60_ST2084_2000_nits,"ACES_VIEW_P3_D60_ST2084_2000_nits","P3-D60 ST2084 2000 nits");
  view_name.add_enum_value(PF::ACES_VIEW_P3_D60_ST2084_4000_nits,"ACES_VIEW_P3_D60_ST2084_4000_nits","P3-D60 ST2084 4000 nits");
  view_name.add_enum_value(PF::ACES_VIEW_P3_DCI,"ACES_VIEW_P3_DCI","P3-DCI");
  view_name.add_enum_value(PF::ACES_VIEW_Rec_2020,"ACES_VIEW_Rec_2020","Rec.2020");
  view_name.add_enum_value(PF::ACES_VIEW_Rec_2020_ST2084_1000_nits,"ACES_VIEW_Rec_2020_ST2084_1000_nits","Rec.2020 ST2084 1000 nits");
  view_name.add_enum_value(PF::ACES_VIEW_Rec_709,"ACES_VIEW_Rec_709","Rec.709");
  view_name.add_enum_value(PF::ACES_VIEW_Rec_709_D60_sim,"ACES_VIEW_Rec_709_D60_sim","Rec.709 D60 sim.");
  view_name.add_enum_value(PF::ACES_VIEW_sRGB_D60_sim,"ACES_VIEW_sRGB_D60_sim","sRGB D60 sim.");
  view_name.add_enum_value(PF::ACES_VIEW_RAW,"ACES_VIEW_RAW","Raw");
  view_name.add_enum_value(PF::ACES_VIEW_LOG,"ACES_VIEW_LOG","Log");

  convert2ACES = new_icc_transform();
  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2ACES->get_par() );
  icc_par->set_out_profile( PF::ICCStore::Instance().get_profile(PF::PROF_TYPE_ACES,PF::PF_TRC_LINEAR) );

  set_type( "ocio_aces" );

  set_default_name( _("ACES (OCIO)") );
}



VipsImage* PF::OCIOACESPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap,
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) {
    std::cout<<"OCIOACESPar::build(): null input image"<<std::endl;
    return NULL;
  }


  try {
  // Step 1: Get the config
  //config = OCIO::GetCurrentConfig();
#ifdef __WIN32__
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "\\ocio-configs\\aces_1.0.3\\config.ocio";
#else
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/aces_1.0.3/config.ocio";
#endif
  config  = OCIO::Config::CreateFromFile(configfile.c_str());
  std::cout<<"OCIOACESPar: config="<<config<<std::endl;

  // Step 2: Lookup the display ColorSpace
  displayName = "ACES"; //config->getDefaultDisplay();
  std::cout<<"OCIOACESPar: displayName="<<displayName<<std::endl;
  viewName = "sRGB";
  //viewName = view_name.get_enum_value().second.second;
  std::cout<<"OCIOACESPar: viewName="<<viewName<<std::endl;
  displayColorSpace = config->getDisplayColorSpaceName(displayName.c_str(), viewName.c_str());
  //displayColorSpace = "out_srgb";
  std::cout<<"OCIOACESPar: displayColorSpace="<<displayColorSpace<<std::endl;

  // Step 3: Create a DisplayTransform, and set the input and display ColorSpaces
  // (This example assumes the input is scene linear. Adapt as needed.)

  transform = OCIO::DisplayTransform::Create();
  //transform->setInputColorSpaceName( OCIO::ROLE_SCENE_LINEAR );
  transform->setInputColorSpaceName( OCIO::ROLE_DEFAULT );
  transform->setDisplay( displayName.c_str() );
  transform->setView( viewName.c_str() );

  displayName = "ACES";
  viewName = view_name.get_enum_value().second.second;
  displayColorSpace = config->getDisplayColorSpaceName(displayName.c_str(), viewName.c_str());
  processor = config->getProcessor(OCIO::ROLE_DEFAULT, displayColorSpace.c_str());
  //processor = config->getProcessor(transform);
  std::cout<<"OCIOACESPar: processor="<<processor<<std::endl;

  /*
  float* buf = new float[200*3];
  float delta = 12.0f/199.0f;
  float Y = -8;
  const double inv_log_base = 1.0 / log(2.0);
  for(int i = 0; i < 200; i++) {
    buf[i*3] = buf[i*3+1] = buf[i*3+2] = pow(2,Y);
    Y += delta;
  }
  OCIO::PackedImageDesc img(buf, 200, 1, 3);
  get_processor()->apply(img);
  Y = -8;
  for(int i = 0; i < 200; i++) {
    std::cout<<Y<<" "<<pow(2,Y)<<" "<<pow(buf[i*3],2.2)<<std::endl;
    Y += delta;
  }
  */
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }


  VipsImage* out = NULL;

  std::vector<VipsImage*> in2;
  convert2ACES->get_par()->set_image_hints( srcimg );
  convert2ACES->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( srcimg );
  VipsImage* srgbimg = convert2ACES->get_par()->build(in2, 0, NULL, NULL, level );


  in2.clear(); in2.push_back( srgbimg );
  out = OpParBase::build( in2, 0, imap, omap, level);
  PF_UNREF( srgbimg, "OCIOACESPar::build() srgbimg unref" );

  if( viewName == "sRGB" ) {
    // tag the output image as standard sRGB
    ICCProfile* sRGBprof = PF::ICCStore::Instance().get_srgb_profile(PF::PF_TRC_STANDARD);
    if( out && sRGBprof )
      PF::set_icc_profile( out, sRGBprof );
  } else if( viewName == "Rec.2020_" ) {
      // tag the output image as standard sRGB
      ICCProfile* prof = PF::ICCStore::Instance().get_profile(PF::PROF_TYPE_REC2020, PF::PF_TRC_LINEAR);
      if( out && prof )
        PF::set_icc_profile( out, prof );
  } else {
    // otherwise, tag the image with a NULL profile so that pixels will be sent directly to display
    // without further color management
    PF::set_icc_profile( out, NULL );
  }
	return out;
}


PF::ProcessorBase* PF::new_ocio_aces()
{
  return( new PF::Processor<PF::OCIOACESPar,PF::OCIOACESProc>() );
}
