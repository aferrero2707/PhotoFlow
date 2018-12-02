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
  std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/nuke-default/config.ocio";
#endif
  config  = OCIO::Config::CreateFromFile(configfile.c_str());
  std::cout<<"OCIOViewPar: config="<<config<<std::endl;

  // Step 2: Lookup the display ColorSpace
  device = config->getDefaultDisplay();
  std::cout<<"OCIOViewPar: device="<<device<<std::endl;
  transformName = config->getDefaultView(device);
  std::cout<<"OCIOViewPar: transformName="<<transformName<<std::endl;
  displayColorSpace = config->getDisplayColorSpaceName(device, transformName);
  std::cout<<"OCIOViewPar: displayColorSpace="<<displayColorSpace<<std::endl;

  // Step 3: Create a DisplayTransform, and set the input and display ColorSpaces
  // (This example assumes the input is scene linear. Adapt as needed.)

  transform = OCIO::DisplayTransform::Create();
  transform->setInputColorSpaceName( OCIO::ROLE_SCENE_LINEAR );
  transform->setDisplay( displayColorSpace );

  processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, displayColorSpace);
  std::cout<<"OCIOViewPar: processor="<<processor<<std::endl;
  }
  catch(OCIO::Exception & exception)
  {
      std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
  }

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

  out = OpParBase::build( in, first, imap, omap, level);
  ICCProfile* sRGBprof = PF::ICCStore::Instance().get_srgb_profile(PF_TRC_STANDARD);

  // tag the output image as standard sRGB
  if( out && sRGBprof )
    set_icc_profile( out, sRGBprof );
	return out;
}


PF::ProcessorBase* PF::new_ocio_view()
{
  return( new PF::Processor<PF::OCIOViewPar,PF::OCIOViewProc>() );
}
