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

#include "raw_developer.hh"


PF::RawDeveloperPar::RawDeveloperPar(): 
  OpParBase(), output_format( VIPS_FORMAT_NOTSET )
{
  set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
  amaze_demosaic = new_amaze_demosaic();
  fast_demosaic = new_fast_demosaic();
  raw_preprocessor = new_raw_preprocessor();
  raw_output = new_raw_output();
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();

  map_properties( raw_preprocessor->get_par()->get_properties() );
  map_properties( raw_output->get_par()->get_properties() );

  set_type("raw_developer" );
}


VipsImage* PF::RawDeveloperPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;
  
  VipsImage* out_demo;
  std::vector<VipsImage*> in2;

  size_t blobsz;
  if( vips_image_get_blob( in[0], "raw_image_data",
			   (void**)&image_data, 
			   &blobsz ) )
    return NULL;
	std::cout<<"RawDeveloperPar::build(): blobsz="<<blobsz<<std::endl;
  if( blobsz != sizeof(dcraw_data_t) )
    return NULL;
  
  
  VipsImage* input_img = in[0];
	std::cout<<"RawDeveloperPar::build(): input_img->Bands="<<input_img->Bands<<std::endl;
  if( input_img->Bands != 3 ) {
    raw_preprocessor->get_par()->set_image_hints( in[0] );
    raw_preprocessor->get_par()->set_format( VIPS_FORMAT_UCHAR );
    VipsImage* image = raw_preprocessor->get_par()->build( in, 0, NULL, NULL, level );
    if( !image )
      return NULL;
  
    in2.push_back( image );
		//PF::ProcessorBase* demo = amaze_demosaic;
		PF::ProcessorBase* demo = fast_demosaic;
    demo->get_par()->set_image_hints( image );
    demo->get_par()->set_format( VIPS_FORMAT_FLOAT );
    out_demo = demo->get_par()->build( in2, 0, NULL, NULL, level );
    g_object_unref( image );
  } else {
    raw_preprocessor->get_par()->set_image_hints( in[0] );
    raw_preprocessor->get_par()->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* image = raw_preprocessor->get_par()->build( in, 0, NULL, NULL, level );
    if( !image )
      return NULL;
  
    out_demo = image;
  }

  /**/
  raw_output->get_par()->set_image_hints( out_demo );
  raw_output->get_par()->set_format( VIPS_FORMAT_FLOAT );
  in2.clear(); in2.push_back( out_demo );
  VipsImage* out = raw_output->get_par()->build( in2, 0, NULL, NULL, level );
  g_object_unref( out_demo );
  /**/

  //VipsImage* gamma_in = out_demo;
  VipsImage* gamma_in = out;
  VipsImage* gamma = gamma_in;
  /*
  if( vips_gamma(gamma_in, &gamma, "exponent", (float)(2.2), NULL) ) {
    g_object_unref(gamma_in);
    return NULL;
  }
  */

  VipsImage* out2;
  std::vector<VipsImage*> in3;
  in3.push_back( gamma );
  convert_format->get_par()->set_image_hints( gamma );
  convert_format->get_par()->set_format( get_format() );
  out2 = convert_format->get_par()->build( in3, 0, NULL, NULL, level );
  g_object_unref( gamma );

  return out2;
}


PF::ProcessorBase* PF::new_raw_developer()
{
  return new PF::Processor<PF::RawDeveloperPar,PF::RawDeveloper>();
}
