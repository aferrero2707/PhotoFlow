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

#include <glibmm.h>

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../dt/common/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "../dt/external/wb_presets.c"

#include "../base/exif_data.hh"
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
  saturation_level_correction( "raw_white_level_correction", this, 0),
  black_level_correction( "raw_black_level_correction", this, 0)
{
  wb_mode.add_enum_value(PF::WB_CAMERA,"CAMERA","CAMERA");
  wb_mode.add_enum_value(PF::WB_SPOT,"SPOT","Spot");
  wb_mode.add_enum_value(PF::WB_COLOR_SPOT,"COLOR_SPOT","Color spot");
  wb_mode.add_enum_value(PF::WB_DAYLIGHT,"DAYLIGHT",Daylight);
  wb_mode.add_enum_value(PF::WB_DIRECT_SUNLIGHT,"DIRECT_SUNLIGHT",DirectSunlight);
  wb_mode.add_enum_value(PF::WB_CLOUDY,"CLOUDY",Cloudy);
  wb_mode.add_enum_value( PF::WB_SHADE,"SHADE",Shade);
  wb_mode.add_enum_value(PF::WB_INCANDESCENT,"INCANDESCENT",Incandescent);
  wb_mode.add_enum_value(PF::WB_INCANDESCENT_WARM,"INCANDESCENT_WARM",IncandescentWarm);
  wb_mode.add_enum_value(PF::WB_TUNGSTEN,"TUNGSTEN",Tungsten);
  wb_mode.add_enum_value(PF::WB_FLUORESCENT,"FLUORESCENT",Fluorescent);
  wb_mode.add_enum_value(PF::WB_FLUORESCENT_HIGH,"FLUORESCENT_HIGH",FluorescentHigh);
  wb_mode.add_enum_value(PF::WB_COOL_WHITE_FLUORESCENT,"COOL_WHITE_FLUORESCENT",CoolWhiteFluorescent);
  wb_mode.add_enum_value(PF::WB_WARM_WHITE_FLUORESCENT,"WARM_WHITE_FLUORESCENT",WarmWhiteFluorescent);
  wb_mode.add_enum_value(PF::WB_DAYLIGHT_FLUORESCENT,"DAYLIGHT_FLUORESCENT",DaylightFluorescent);
  wb_mode.add_enum_value(PF::WB_NEUTRAL_FLUORESCENT,"NEUTRAL_FLUORESCENT",NeutralFluorescent);
  wb_mode.add_enum_value(PF::WB_WHITE_FLUORESCENT,"WHITE_FLUORESCENT",WhiteFluorescent);
  wb_mode.add_enum_value(PF::WB_SODIUM_VAPOR_FLUORESCENT,"SODIUM_VAPOR_FLUORESCENT",SodiumVaporFluorescent);
  wb_mode.add_enum_value(PF::WB_DAY_WHITE_FLUORESCENT,"DAY_WHITE_FLUORESCENT",DayWhiteFluorescent);
  wb_mode.add_enum_value(PF::WB_HIGH_TEMP_MERCURY_VAPOR_FLUORESCENT,"HIGH_TEMP_MERCURY_VAPOR_FLUORESCENT",HighTempMercuryVaporFluorescent);
  wb_mode.add_enum_value(PF::WB_FLASH,"FLASH",Flash);
  wb_mode.add_enum_value(PF::WB_FLASH_AUTO,"FLASH_AUTO",FlashAuto);
  wb_mode.add_enum_value(PF::WB_EVENING_SUN,"EVENING_SUN",EveningSun);
  wb_mode.add_enum_value(PF::WB_UNDERWATER,"UNDERWATER",Underwater);
  wb_mode.add_enum_value(PF::WB_BACK_AND_WHITE,"BACK_AND_WHITE",BlackNWhite);

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
			   &blobsz ) ) {
    std::cout<<"RawOutputPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"RawOutputPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }

  switch( wb_mode.get_enum_value().first ) {
  case PF::WB_CAMERA:
    wb_red_current = image_data->color.cam_mul[0];
    wb_green_current = image_data->color.cam_mul[1];
    wb_blue_current = image_data->color.cam_mul[2];
    //wb_red_current = 1.513467;
    //wb_green_current = 1.343420;
    //wb_blue_current = 4.000183;
    break;
  case PF::WB_SPOT:
  case PF::WB_COLOR_SPOT:
    wb_red_current = wb_red.get();
    wb_green_current = wb_green.get();
    wb_blue_current = wb_blue.get();
    break;
  default: {
    PF::exif_data_t* exif_data;
    if( vips_image_get_blob( in[0], PF_META_EXIF_NAME,
        (void**)&exif_data,
        &blobsz ) ) {
      std::cout<<"RawOutputPar::build() could not extract exif_custom_data."<<std::endl;
      return NULL;
    }
    if( blobsz != sizeof(PF::exif_data_t) ) {
      std::cout<<"RawOutputPar::build() wrong exif_custom_data size."<<std::endl;
      return NULL;
    }
    //char makermodel[1024];
    //char *model = makermodel;
    //dt_colorspaces_get_makermodel_split(makermodel, sizeof(makermodel), &model,
    //    exif_data->exif_maker, exif_data->exif_model );
    for(int i = 0; i < wb_preset_count; i++) {
      if( !strcmp(wb_preset[i].make, exif_data->camera_maker) &&
          !strcmp(wb_preset[i].model, exif_data->camera_model) ) {
        if( wb_mode.get_enum_value().second.second == wb_preset[i].name &&
            wb_preset[i].tuning == 0 ) {
          wb_red_current = wb_preset[i].channel[0];
          wb_green_current = wb_preset[i].channel[1];
          wb_blue_current = wb_preset[i].channel[2];
        }
      }
    }
    break;
  }
  }

  VipsImage* image = OpParBase::build( in, first, NULL, NULL, level );
  if( !image )
    return NULL;

  return image;
}


PF::ProcessorBase* PF::new_raw_preprocessor()
{
  return new PF::Processor<PF::RawPreprocessorPar,PF::RawPreprocessor>();
}
