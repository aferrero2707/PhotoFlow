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

#include "raw_preprocessor.hh"

int PF::raw_preproc_sample_x = 0;
int PF::raw_preproc_sample_y = 0;

PF::RawPreprocessorPar::RawPreprocessorPar(): 
  OpParBase(), image_data( NULL ),
  wb_mode("wb_mode",this,PF::WB_CAMERA,"CAMERA","CAMERA"),
  wb_red("wb_red",this,1), 
  wb_green("wb_green",this,1), 
  wb_blue("wb_blue",this,1), 
  camwb_corr_red("camwb_corr_red",this,1), 
  camwb_corr_green("camwb_corr_green",this,1), 
  camwb_corr_blue("camwb_corr_blue",this,1), 
  wb_target_L("wb_target_L",this,-100), 
  wb_target_a("wb_target_a",this,10), 
  wb_target_b("wb_target_b",this,12), 
  exposure("exposure",this,1)
{
  wb_mode.add_enum_value(PF::WB_CAMERA,"CAMERA","CAMERA");
  wb_mode.add_enum_value(PF::WB_SPOT,"SPOT","Spot");
  wb_mode.add_enum_value(PF::WB_COLOR_SPOT,"COLOR_SPOT","Color spot");
  set_type("raw_preprocessor" );
}


VipsImage* PF::RawPreprocessorPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;
  
  size_t blobsz;
  if( vips_image_get_blob( in[0], "raw_image_data",
			   (void**)&image_data, 
			   &blobsz ) )
    return NULL;
  if( blobsz != sizeof(dcraw_data_t) )
    return NULL;

  VipsImage* image = OpParBase::build( in, first, NULL, NULL, level );
  if( !image )
    return NULL;

  return image;
}


PF::ProcessorBase* PF::new_raw_preprocessor()
{
  return new PF::Processor<PF::RawPreprocessorPar,PF::RawPreprocessor>();
}
