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
#include "ocio_filmic.hh"

PF::OCIOFilmicPar::OCIOFilmicPar():
  OpParBase(),
  look_name("look_name",this,PF::OCIO_FILMIC_MEDIUM_HIGH,"OCIO_FILMIC_MEDIUM_HIGH","Medium High Contrast")
{
  look_name.add_enum_value(PF::OCIO_FILMIC_MEDIUM_HIGH,"OCIO_FILMIC_MEDIUM_HIGH","Medium High Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_BASE,"OCIO_FILMIC_BASE","Base Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_VERY_LOW,"OCIO_FILMIC_VERY_LOW","Very Low Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_LOW,"OCIO_FILMIC_LOW","Low Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_MEDIUM_LOW,"OCIO_FILMIC_MEDIUM_LOW","Medium Low Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_MEDIUM_HIGH,"OCIO_FILMIC_MEDIUM_HIGH","Medium High Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_HIGH,"OCIO_FILMIC_HIGH","High Contrast");
  look_name.add_enum_value(PF::OCIO_FILMIC_VERY_HIGH,"OCIO_FILMIC_VERY_HIGH","Very High Contrast");

  try {
  // Step 1: Get the config
  //config = OCIO::GetCurrentConfig();
#ifdef __WIN32__
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "\\ocio-configs\\filmic-blender-master\\config.ocio";
#else
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/filmic-blender/config.ocio";
#endif
  config  = OCIO::Config::CreateFromFile(configfile.c_str());
  std::cout<<"OCIOFilmicPar: config="<<config<<std::endl;

  // Step 2: Lookup the display ColorSpace
  device = "sRGB"; //config->getDefaultDisplay();
  std::cout<<"OCIOFilmicPar: device="<<device<<std::endl;
  transformName = "Filmic Log Encoding Base"; //config->getDefaultView(device);
  std::cout<<"OCIOFilmicPar: transformName="<<transformName<<std::endl;
  displayColorSpace = config->getDisplayColorSpaceName(device.c_str(), transformName.c_str());
  std::cout<<"OCIOFilmicPar: displayColorSpace="<<displayColorSpace<<std::endl;

  // Step 3: Create a DisplayTransform, and set the input and display ColorSpaces
  // (This example assumes the input is scene linear. Adapt as needed.)

  transform = OCIO::DisplayTransform::Create();
  transform->setInputColorSpaceName( OCIO::ROLE_SCENE_LINEAR );
  transform->setDisplay( device.c_str() );
  transform->setView( transformName.c_str() );

  //lookName = "Very High Contrast";
  //lookName = "Very Low Contrast";
  lookName = "Medium High Contrast";
  //lookName = "Base Contrast";
  //lookName = config->getDisplayLooks(device, transformName); //"Very High Contrast";
  std::cout<<"OCIOFilmicPar: lookName="<<lookName<<std::endl;
  transform->setLooksOverrideEnabled(true);
  transform->setLooksOverride(lookName.c_str());

  //processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, displayColorSpace);
  processor = config->getProcessor(transform);
  std::cout<<"OCIOFilmicPar: processor="<<processor<<std::endl;

  float* buf = new float[400*3];
  float delta = 12.0f/199.0f;
  float Y = -8;
  const double inv_log_base = 1.0 / log(2.0);
  for(int i = 0; i < 400; i++) {
    buf[i*3] = buf[i*3+1] = buf[i*3+2] = pow(2,Y);
    Y += delta;
  }
  OCIO::PackedImageDesc img(buf, 400, 1, 3);
  get_processor()->apply(img);
  Y = -8;
  for(int i = 0; i < 400; i++) {
    std::cout<<Y<<" "<<pow(2,Y)<<" "<<pow(buf[i*3],2.2)<<std::endl;
    Y += delta;
  }
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }

  convert2sRGB = new_icc_transform();
  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2sRGB->get_par() );
  icc_par->set_out_profile( PF::ICCStore::Instance().get_srgb_profile(PF_TRC_LINEAR) );

  set_type( "ocio_filmic" );

  set_default_name( _("Filmic (OCIO)") );
}



VipsImage* PF::OCIOFilmicPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap,
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) {
    std::cout<<"OCIOFilmicPar::build(): null input image"<<std::endl;
    return NULL;
  }


#ifdef __WIN32__
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "\\ocio-configs\\nuke-default\\config.ocio";
#else
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/filmic-blender/config.ocio";
#endif

  try {
  config  = OCIO::Config::CreateFromFile(configfile.c_str());
  std::cout<<"OCIOFilmicPar: config="<<config<<std::endl;

  // Step 2: Lookup the display ColorSpace
  device = "sRGB";
  std::cout<<"OCIOFilmicPar: device="<<device<<std::endl;
  transformName = "Filmic Log Encoding Base";
  std::cout<<"OCIOFilmicPar: transformName="<<transformName<<std::endl;
  displayColorSpace = config->getDisplayColorSpaceName(device.c_str(), transformName.c_str());
  std::cout<<"OCIOFilmicPar: displayColorSpace="<<displayColorSpace<<std::endl;

  // Step 3: Create a DisplayTransform, and set the input and display ColorSpaces
  // (This example assumes the input is scene linear. Adapt as needed.)

  transform = OCIO::DisplayTransform::Create();
  transform->setInputColorSpaceName( OCIO::ROLE_SCENE_LINEAR );
  transform->setDisplay( device.c_str() );
  transform->setView( transformName.c_str() );

  lookName = look_name.get_enum_value().second.second;
  //lookName = "Medium High Contrast";
  std::cout<<"OCIOFilmicPar: lookName="<<lookName<<std::endl;
  transform->setLooksOverrideEnabled(true);
  transform->setLooksOverride(lookName.c_str());

  //processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, displayColorSpace);
  processor = config->getProcessor(transform);
  std::cout<<"OCIOFilmicPar: processor="<<processor<<std::endl;
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }

  VipsImage* out = NULL;

  std::vector<VipsImage*> in2;
  convert2sRGB->get_par()->set_image_hints( srcimg );
  convert2sRGB->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( srcimg );
  VipsImage* srgbimg = convert2sRGB->get_par()->build(in2, 0, NULL, NULL, level );


  in2.clear(); in2.push_back( srgbimg );
  out = OpParBase::build( in2, 0, imap, omap, level);
  PF_UNREF( srgbimg, "OCIOFilmicPar::build() srgbimg unref" );

  ICCProfile* sRGBprof = PF::ICCStore::Instance().get_srgb_profile(PF_TRC_GAMMA_22);
  // tag the output image as standard sRGB
  if( out && sRGBprof )
    set_icc_profile( out, sRGBprof );
	return out;
}


PF::ProcessorBase* PF::new_ocio_view()
{
  return( new PF::Processor<PF::OCIOFilmicPar,PF::OCIOFilmicProc>() );
}
