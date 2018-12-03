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

#include "../base/processor_imp.hh"
#include "icc_transform.hh"
#include "ocio_view.hh"

PF::OCIOViewPar::OCIOViewPar():
  OpParBase()
{

  try {
  // Step 1: Get the config
  //config = OCIO::GetCurrentConfig();
#ifdef __WIN32__
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "\\ocio-configs\\nuke-default\\config.ocio";
#else
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/filmic-blender-master/config.ocio";
#endif
  config  = OCIO::Config::CreateFromFile(configfile.c_str());
  std::cout<<"OCIOViewPar: config="<<config<<std::endl;

  // Step 2: Lookup the display ColorSpace
  device = "sRGB Native 2.2"; //config->getDefaultDisplay();
  std::cout<<"OCIOViewPar: device="<<device<<std::endl;
  transformName = "Filmic Log Encoding Base"; //config->getDefaultView(device);
  std::cout<<"OCIOViewPar: transformName="<<transformName<<std::endl;
  displayColorSpace = config->getDisplayColorSpaceName(device, transformName);
  std::cout<<"OCIOViewPar: displayColorSpace="<<displayColorSpace<<std::endl;

  // Step 3: Create a DisplayTransform, and set the input and display ColorSpaces
  // (This example assumes the input is scene linear. Adapt as needed.)

  transform = OCIO::DisplayTransform::Create();
  transform->setInputColorSpaceName( OCIO::ROLE_SCENE_LINEAR );
  transform->setDisplay( device );
  transform->setView( transformName );

  lookName = "Medium High Contrast";
  //lookName = config->getDisplayLooks(device, transformName); //"Very High Contrast";
  std::cout<<"OCIOViewPar: lookName="<<lookName<<std::endl;
  transform->setLooksOverrideEnabled(true);
  transform->setLooksOverride(lookName);

  //processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, displayColorSpace);
  processor = config->getProcessor(transform);
  std::cout<<"OCIOViewPar: processor="<<processor<<std::endl;
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }

  convert2sRGB = new_icc_transform();
  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2sRGB->get_par() );
  icc_par->set_out_profile( PF::ICCStore::Instance().get_srgb_profile(PF_TRC_LINEAR) );

  set_type( "ocio_view" );
}



VipsImage* PF::OCIOViewPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap,
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) {
    std::cout<<"OCIOViewPar::build(): null input image"<<std::endl;
    return NULL;
  }


  VipsImage* out = NULL;

  std::vector<VipsImage*> in2;
  convert2sRGB->get_par()->set_image_hints( srcimg );
  convert2sRGB->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( srcimg );
  VipsImage* srgbimg = convert2sRGB->get_par()->build(in2, 0, NULL, NULL, level );


  in2.clear(); in2.push_back( srgbimg );
  out = OpParBase::build( in2, 0, imap, omap, level);
  PF_UNREF( srgbimg, "OCIOViewPar::build() srgbimg unref" );

  ICCProfile* sRGBprof = PF::ICCStore::Instance().get_srgb_profile(PF_TRC_GAMMA_22);
  // tag the output image as standard sRGB
  if( out && sRGBprof )
    set_icc_profile( out, sRGBprof );
	return out;
}


PF::ProcessorBase* PF::new_ocio_view()
{
  return( new PF::Processor<PF::OCIOViewPar,PF::OCIOViewProc>() );
}
