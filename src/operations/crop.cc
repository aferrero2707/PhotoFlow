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

#include "crop.hh"
#include "../base/new_operation.hh"


PF::CropPar::CropPar(): 
  OpParBase(),
  crop_left("crop_left",this,0),
  crop_top("crop_top",this,0),
  crop_width("crop_width",this,0),
  crop_height("crop_height",this,0),
  keep_ar("keep_ar",this,0),
  ar_mode("ar_mode", this, PF::CROP_AR_INPUT,"CROP_AR_INPUT","orignal"),
  ar_width("ar_width",this,100),
  ar_height("ar_height",this,100)
{
  ar_mode.add_enum_value(PF::CROP_AR_NONE,"CROP_AR_NONE","none");
  ar_mode.add_enum_value(PF::CROP_AR_INPUT,"CROP_AR_INPUT","orignal");
  ar_mode.add_enum_value(PF::CROP_AR_CUSTOM,"CROP_AR_CUSTOM","custom");
  ar_mode.add_enum_value(PF::CROP_AR_1_1,"CROP_AR_1_1","1:1");
  ar_mode.add_enum_value(PF::CROP_AR_3_2,"CROP_AR_3_2","3:2");
  ar_mode.add_enum_value(PF::CROP_AR_2_3,"CROP_AR_2_3","2:3");
  ar_mode.add_enum_value(PF::CROP_AR_4_3,"CROP_AR_4_3","4:3");
  ar_mode.add_enum_value(PF::CROP_AR_3_4,"CROP_AR_3_4","3:4");
  ar_mode.add_enum_value(PF::CROP_AR_A4,"CROP_AR_A4","A4");
  ar_mode.add_enum_value(PF::CROP_AR_A4R,"CROP_AR_A4R","A4R");
  ar_mode.add_enum_value(PF::CROP_AR_A3,"CROP_AR_A3","A3");
  ar_mode.add_enum_value(PF::CROP_AR_A3R,"CROP_AR_A3R","A3R");
  ar_mode.add_enum_value(PF::CROP_AR_16_9,"CROP_AR_16_9","16:9");
  ar_mode.add_enum_value(PF::CROP_AR_16_10,"CROP_AR_16_10","16:10");

  set_type( "crop" );

  set_default_name( _("crop") );
}



VipsImage* PF::CropPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
	if( srcimg == NULL ) return NULL;
	VipsImage* out;

  if( (get_render_mode() == PF_RENDER_PREVIEW) && is_editing() ) {
#ifndef NDEBUG
    std::cout<<"CropPar::build(): editing, returning source image"<<std::endl;
#endif
    PF_REF( srcimg, "CropPar::build(): srcimg ref (editing mode)" );
    return srcimg;
  }

  int scale_factor = 1;
  for(unsigned int l = 0; l < level; l++ ) {
    scale_factor *= 2;
  }

  if( ((crop_width.get()/scale_factor) < 1) ||
      ((crop_height.get()/scale_factor) < 1) ) {
    PF_REF( srcimg, "CropPar::build(): srcimg ref (editing mode)" );
    return srcimg;
  }

  int cleft = crop_left.get()/scale_factor;
  int ctop = crop_top.get()/scale_factor;
  int cw = crop_width.get()/scale_factor;
  int ch = crop_height.get()/scale_factor;
  if( (cleft+cw) > srcimg->Xsize ) cw = srcimg->Xsize - cleft;
  if( (ctop+ch) > srcimg->Ysize ) ch = srcimg->Ysize - ctop;

  if( vips_crop( srcimg, &out, cleft, ctop, cw, ch, NULL ) ) {
    std::cout<<"WARNING: CropPar::build(): vips_crop() failed."<<std::endl;
    std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  srcimg->Ysize="<<srcimg->Ysize<<std::endl;
    std::cout<<"vips_crop( srcimg, &out, "<<crop_left.get()/scale_factor<<", "<<crop_top.get()/scale_factor<<", "
        <<crop_width.get()/scale_factor<<", "<<crop_height.get()/scale_factor<<", NULL )"<<std::endl;
    return NULL;
  }
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  srcimg->Ysize="<<srcimg->Ysize<<std::endl;
  //std::cout<<"outimg="<<out<<std::endl;
  //std::cout<<"outimg->Xsize="<<out->Xsize<<"  outimg->Ysize="<<out->Ysize<<std::endl;
  return out;
}
