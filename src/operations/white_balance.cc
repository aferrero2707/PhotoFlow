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

#include "../base/exif_data.hh"
#include "white_balance.hh"

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../external/darktable/src/common/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "../external/darktable/src/external/wb_presets.c"

int PF::wb_sample_x = 0;
int PF::wb_sample_y = 0;

PF::WhiteBalancePar::WhiteBalancePar():
      OpParBase(), image_data( NULL ),
      wb_mode("wb_mode",this,PF::WB_CAMERA,"CAMERA","CAMERA"),
      //wb_red("wb_red",this,1),
      //wb_green("wb_green",this,1),
      //wb_blue("wb_blue",this,1),
      camwb_corr_red("camwb_corr_red",this,1),
      camwb_corr_green("camwb_corr_green",this,1),
      camwb_corr_blue("camwb_corr_blue",this,1),
      wb_target_L("wb_target_L",this,-100),
      wb_target_a("wb_target_a",this,10),
      wb_target_b("wb_target_b",this,12),
      saturation_level_correction( "saturation_level_correction", this, 1),
      black_level_correction( "black_level_correction", this, 1)
{
  wb_mode.add_enum_value(PF::WB_CAMERA,"CAMERA","CAMERA");
  wb_mode.add_enum_value(PF::WB_SPOT,"SPOT","Spot");
  wb_mode.add_enum_value(PF::WB_COLOR_SPOT,"COLOR_SPOT","Color spot");
  wb_mode.add_enum_value(PF::WB_UNIWB,"UNIWB","Uni WB");
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

  char tstr[100];
  for( unsigned int i = 0; i < PF::WB_LAST; i++ ) {
    snprintf(tstr,99,"wb_red_%d", i);
    wb_red[i] = new Property<float>(tstr, this, -1);
    //add_property( wb_red[i ]);
    snprintf(tstr,99,"wb_green_%d", i);
    wb_green[i] = new Property<float>(tstr, this, -1);
    //add_property( wb_green[i ]);
    snprintf(tstr,99,"wb_blue_%d", i);
    wb_blue[i] = new Property<float>(tstr, this, -1);
    //add_property( wb_blue[i ]);
  }

  set_type("white_balance" );
  set_default_name( _("white balance") );
}


void PF::WhiteBalancePar::init_wb_coefficients( dcraw_data_t* idata, std::string camera_maker, std::string camera_model )
{
  std::cout<<"WhiteBalancePar::init_wb_coefficients() called"<<std::endl;
  std::map< int, std::pair<std::string,std::string> > wb_modes = wb_mode.get_enum_values();
  std::map< int, std::pair<std::string,std::string> >::iterator wbi;
  for( wbi = wb_modes.begin(); wbi != wb_modes.end(); wbi++ ) {
    float def_red = 1, def_green = 1, def_blue = 1;
    int wb_id = wbi->first;
    switch( wb_id ) {
    case PF::WB_CAMERA:
      def_red = (idata != NULL) ? idata->color.cam_mul[0] : 1;
      def_green = (idata != NULL) ? idata->color.cam_mul[1] : 1;
      def_blue = (idata != NULL) ? idata->color.cam_mul[2] : 1;
      break;
    default: {
      for(int i = 0; i < wb_preset_count; i++) {
        if( camera_maker == wb_preset[i].make &&
            camera_model == wb_preset[i].model ) {
          if( wbi->second.second == wb_preset[i].name &&
              wb_preset[i].tuning == 0 ) {
            def_red = wb_preset[i].channel[0];
            def_green = wb_preset[i].channel[1];
            def_blue = wb_preset[i].channel[2];
          }
        }
      }
      break;
    }
    case PF::WB_SPOT:
    case PF::WB_COLOR_SPOT:
      break;
    }
    float def_min = MIN3(def_red, def_green, def_blue);
    wb_red[wb_id]->store_default(def_red/def_min);
    wb_green[wb_id]->store_default(def_green/def_min);
    wb_blue[wb_id]->store_default(def_blue/def_min);

    wb_red[wb_id]->reset();
    wb_green[wb_id]->reset();
    wb_blue[wb_id]->reset();
  }
}



VipsImage* PF::WhiteBalancePar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  image_data = get_raw_data( in[0] );
  //if( !image_data ) return NULL;

  size_t blobsz;
  PF::exif_data_t* exif_data;
  if( vips_image_get_blob( in[0], PF_META_EXIF_NAME,
      (void**)&exif_data,
      &blobsz ) ) {
    std::cout<<"WhiteBalancePar::build() could not extract exif_custom_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(PF::exif_data_t) ) {
    std::cout<<"WhiteBalancePar::build() wrong exif_custom_data size."<<std::endl;
    return NULL;
  }
  if( wb_red[PF::WB_CAMERA]->get() < 0 )
    init_wb_coefficients( get_image_data(), exif_data->camera_maker, exif_data->camera_model );
/*
  float def_red = 1, def_green = 1, def_blue = 1;
  switch( wb_mode.get_enum_value().first ) {
  case PF::WB_CAMERA:
    //wb_red_current = (image_data != NULL) ? image_data->color.cam_mul[0] : 1;
    //wb_green_current = (image_data != NULL) ? image_data->color.cam_mul[1] : 1;
    //wb_blue_current = (image_data != NULL) ? image_data->color.cam_mul[2] : 1;
    def_red = (image_data != NULL) ? image_data->color.cam_mul[0] : 1;
    def_green = (image_data != NULL) ? image_data->color.cam_mul[1] : 1;
    def_blue = (image_data != NULL) ? image_data->color.cam_mul[2] : 1;
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
  case PF::WB_UNIWB:
    wb_red_current = 1;
    wb_green_current = 1;
    wb_blue_current = 1;
    break;
  default: {
    size_t blobsz;
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
*/

  float min_mul_in = (image_data != NULL) ? image_data->color.wb_mul[0] : 1;
  float max_mul_in = (image_data != NULL) ? image_data->color.wb_mul[0] : 1;
  if( image_data ) {
    for(int i = 1; i < 4; i++) {
      if( image_data->color.wb_mul[i] < min_mul_in )
        min_mul_in = image_data->color.wb_mul[i];
      if( image_data->color.wb_mul[i] > max_mul_in )
        max_mul_in = image_data->color.wb_mul[i];
    }
  }

  int wb_id = wb_mode.get_enum_value().first;
  wb_red_current = wb_red[wb_id]->get();
  wb_green_current = wb_green[wb_id]->get();
  wb_blue_current = wb_blue[wb_id]->get();

  float mul[4];
  mul[0] = wb_red_current;
  mul[1] = wb_green_current;
  mul[2] = wb_blue_current;
  mul[3] = wb_green_current;
  float min_mul = wb_red_current;
  float max_mul = wb_red_current;
  for(int i = 1; i < 4; i++) {
    if( mul[i] < min_mul )
      min_mul = mul[i];
    if( mul[i] > max_mul )
      max_mul = mul[i];
  }

  for(int i = 0; i < 4; i++) {
    mul[i] = mul[i] * min_mul_in / min_mul;
    //mul[i] = mul[i] / min_mul;
  }

  wb_red_current = mul[0] / ((image_data != NULL) ? image_data->color.wb_mul[0] : 1);
  wb_green_current = mul[1] / ((image_data != NULL) ? image_data->color.wb_mul[1] : 1);
  wb_blue_current = mul[2] / ((image_data != NULL) ? image_data->color.wb_mul[2] : 1);

  if( image_data ) {
    std::cout<<"WhiteBalancePar::build(): CAM WB="<<image_data->color.cam_mul[0]<<","<<image_data->color.cam_mul[1]<<","<<image_data->color.cam_mul[2]<<std::endl;
    std::cout<<"WhiteBalancePar::build(): USR WB="<<image_data->color.wb_mul[0]<<","<<image_data->color.wb_mul[1]<<","<<image_data->color.wb_mul[2]<<std::endl;
  }
  std::cout<<"WhiteBalancePar::build(): WB mul="<<mul[0]<<","<<mul[1]<<","<<mul[2]<<std::endl;
  std::cout<<"WhiteBalancePar::build(): WB cur="<<wb_red_current<<","<<wb_green_current<<","<<wb_blue_current<<std::endl;

  VipsImage* image = OpParBase::build( in, first, NULL, NULL, level );
  if( !image )
    return NULL;

  return image;
}
