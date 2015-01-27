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

#include "scale.hh"
#include "../base/new_operation.hh"


PF::ScalePar::ScalePar():
  OpParBase(),
  rotate_angle("rotate_angle",this,0),
  scale_mode("scale_mode",this, SCALE_MODE_FIT, "SCALE_MODE_FIT", "Fit"),
  scale_unit("scale_unit", this, SCALE_UNIT_PERCENT, "SCALE_UNIT_PERCENT", "percent"),
  scale_width_pixels("scale_width_pixels",this,0),
  scale_height_pixels("scale_height_pixels",this,0),
  scale_width_percent("scale_width_percent",this,100),
  scale_height_percent("scale_height_percent",this,100),
  scale_width_mm("scale_width_mm",this,0),
  scale_height_mm("scale_height_mm",this,0),
  scale_width_cm("scale_width_cm",this,0),
  scale_height_cm("scale_height_cm",this,0),
  scale_width_inches("scale_width_inches",this,0),
  scale_height_inches("scale_height_inches",this,0),
  scale_resolution("scale_resolution",this,300)
{
  //scale_mode.add_enum_value( SCALE_MODE_FILL, "SCALE_MODE_FILL", "Fill" );
  //scale_mode.add_enum_value( SCALE_MODE_RESIZE, "SCALE_MODE_RESIZE", "Resize" );

  scale_unit.add_enum_value( SCALE_UNIT_PX, "SCALE_UNIT_PX", "pixels" );
  scale_unit.add_enum_value( SCALE_UNIT_MM, "SCALE_UNIT_MM", "mm" );
  scale_unit.add_enum_value( SCALE_UNIT_CM, "SCALE_UNIT_CM", "cm" );
  scale_unit.add_enum_value( SCALE_UNIT_INCHES, "SCALE_UNIT_INCHES", "inches" );

  set_type( "scale" );
}



VipsImage* PF::ScalePar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
	if( srcimg == NULL ) return NULL;
	VipsImage* out;

	if( rotate_angle.get() != 0 ) {
	  if( vips_similarity(srcimg, &out, "angle", rotate_angle.get(),
	      //"idx", (double)-1*srcimg->Xsize/4,
	      //"idy", (double)-1*srcimg->Ysize/2,
	      //"odx", (double)srcimg->Xsize/4,
	      //"ody", (double)srcimg->Ysize/2,
	      NULL) ) {
	    return NULL;
	  }
	  set_image_hints( out );
	  srcimg = out;
	} else {
	  PF_REF( srcimg, "ScalePar::build(): srcimg ref for angle=0" );
	}

  int scale_factor = 1;
  for(unsigned int l = 0; l < level; l++ ) {
    scale_factor *= 2;
  }
  int full_width = srcimg->Xsize * scale_factor;
  int full_height = srcimg->Ysize * scale_factor;
  float scale_width = 0;
  float scale_height = 0;

  if( scale_unit.get_enum_value().first == PF::SCALE_UNIT_PX ) {
    scale_width  = ((float)scale_width_pixels.get())/full_width;
    scale_height = ((float)scale_height_pixels.get())/full_height;
  }
  if( scale_unit.get_enum_value().first == PF::SCALE_UNIT_PERCENT ) {
    if( scale_mode.get_enum_value().first == PF::SCALE_MODE_FIT ) {
      scale_width  = scale_width_percent.get()/100.0f;
      scale_height = scale_width_percent.get()/100.0f;
    } else {
      scale_width  = scale_width_percent.get()/100.0f;
      scale_height = scale_height_percent.get()/100.0f;
    }
  }
  if( scale_unit.get_enum_value().first == PF::SCALE_UNIT_MM ) {
    float new_width = ((float)scale_width_mm.get())*scale_resolution.get()/25.4f;
    float new_height = ((float)scale_height_mm.get())*scale_resolution.get()/25.4f;
    scale_width  = new_width/full_width;
    scale_height = new_height/full_height;
  }
  if( scale_unit.get_enum_value().first == PF::SCALE_UNIT_CM ) {
    float new_width = ((float)scale_width_cm.get())*scale_resolution.get()/2.54f;
    float new_height = ((float)scale_height_cm.get())*scale_resolution.get()/2.54f;
    scale_width  = new_width/full_width;
    scale_height = new_height/full_height;
  }
  if( scale_unit.get_enum_value().first == PF::SCALE_UNIT_INCHES ) {
    int new_width = scale_width_inches.get()*scale_resolution.get();
    int new_height = scale_height_inches.get()*scale_resolution.get();
    scale_width  = ((float)new_width)/full_width;
    scale_height = ((float)new_height)/full_height;
  }

  if( scale_mode.get_enum_value().first == PF::SCALE_MODE_FIT ) {
    if( level!=0 || scale_width==0 || scale_height==0 ||
        scale_width==1 || scale_height==1 ) {
      //PF_REF( srcimg, "ScalePar::build(): srcimg ref (editing mode)" );
      return srcimg;
    }
    float scale = MIN( scale_width, scale_height );

    VipsInterpolate* interpolate = vips_interpolate_new( "nohalo" );
    if( !interpolate )
      interpolate = vips_interpolate_new( "bicubic" );
    if( !interpolate )
      interpolate = vips_interpolate_new( "bilinear" );

    if( vips_resize(srcimg, &out, scale, "interpolate", interpolate, NULL) ) {
      std::cout<<"ScalePar::build(): vips_resize() failed."<<std::endl;
      PF_UNREF( interpolate, "ScalePar::build(): interpolate unref" );
      return srcimg;
    }
    PF_UNREF( interpolate, "ScalePar::build(): interpolate unref" );
    PF_UNREF( srcimg, "ScalePar::build(): srcimg unref after vips_resize()" );
  } else {
    //PF_REF( srcimg, "ScalePar::build(): srcimg ref (editing mode)" );
    return srcimg;
  }

  return out;

}


PF::ProcessorBase* PF::new_scale()
{
  return( new PF::Processor<PF::ScalePar,PF::ScaleProc>() );
}
