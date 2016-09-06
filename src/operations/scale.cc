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

/*
 * Post-rotation auto-cropping
 *
 * Source code adapted from Darktable v1.6.8
 */

/** region of interest */
typedef struct dt_iop_roi_t
{
  int x, y, width, height;
  float scale;
} dt_iop_roi_t;


typedef struct dt_iop_clipping_data_t
{
  float angle;              // rotation angle
  float aspect;             // forced aspect ratio
  float m[4];               // rot matrix
  float ki_h, k_h;          // keystone correction, ki and corrected k
  float ki_v, k_v;          // keystone correction, ki and corrected k
  float tx, ty;             // rotation center
  float cx, cy, cw, ch;     // crop window
  float cix, ciy, ciw, cih; // crop window on roi_out 1.0 scale
  uint32_t all_off;         // 1: v and h off, else one of them is used
  uint32_t flags;           // flipping flags
  uint32_t flip;            // flipped output buffer so more area would fit.

  float k_space[4]; // space for the "destination" rectangle of the keystone quadrilatere
  float kxa, kya, kxb, kyb, kxc, kyc, kxd,
  kyd; // point of the "source" quadrilatere (modified if keystone is not "full")
  float a, b, d, e, g, h; // value of the transformation matrix (c=f=0 && i=1)
  int k_apply;
  int crop_auto;
  float enlarge_x, enlarge_y;
} dt_iop_clipping_data_t;



// helper to count corners in for loops:
static void get_corner(const float *aabb, const int i, float *p)
{
  for(int k = 0; k < 2; k++) p[k] = aabb[2 * ((i >> k) & 1) + k];
}



static void mul_mat_vec_2(const float *m, const float *p, float *o)
{
  o[0] = p[0] * m[0] + p[1] * m[1];
  o[1] = p[0] * m[2] + p[1] * m[3];
}


static void transform(float *x, float *o, const float *m, const float t_h, const float t_v)
{
  float rt[] = { m[0], -m[1], -m[2], m[3] };
  mul_mat_vec_2(rt, x, o);
  o[1] *= (1.0f + o[0] * t_h);
  o[0] *= (1.0f + o[1] * t_v);
}



// 1st pass: how large would the output be, given this input roi?
// this is always called with the full buffer before processing.
static void modify_roi_out(dt_iop_clipping_data_t* d, dt_iop_roi_t* roi_in_orig, dt_iop_roi_t* roi_out)
{
  dt_iop_roi_t roi_in_d = *roi_in_orig;
  dt_iop_roi_t *roi_in = &roi_in_d;

  // use whole-buffer roi information to create matrix and inverse.
  float rt[] = { cosf(d->angle), sinf(d->angle), -sinf(d->angle), cosf(d->angle) };
  if(d->angle == 0.0f)
  {
    rt[0] = rt[3] = 1.0;
    rt[1] = rt[2] = 0.0f;
  }

  for(int k = 0; k < 4; k++) d->m[k] = rt[k];

  d->ki_h = d->ki_v = d->k_h = d->k_v = 0.0f;

  *roi_out = *roi_in;

  // correct keystone correction factors by resolution of this buffer
  const float kc = 1.0f / fminf(roi_in->width, roi_in->height);
  d->k_h = d->ki_h * kc;
  d->k_v = d->ki_v * kc;

  d->cx = 0.0f;
  d->cy = 0.0f;
  d->cw = 1.0f;
  d->ch = 1.0f;

  float cropscale = -1.0f;
  // check portrait/landscape orientation, whichever fits more area:
  const float oaabb[4]
                    = { -.5f * roi_in->width, -.5f * roi_in->height, .5f * roi_in->width, .5f * roi_in->height };
  for(int flip = 0; flip < 2; flip++)
  {
    const float roi_in_width = flip ? roi_in->height : roi_in->width;
    const float roi_in_height = flip ? roi_in->width : roi_in->height;
    float newcropscale = 1.0f;
    // fwd transform rotated points on corners and scale back inside roi_in bounds.
    float p[2], o[2],
    aabb[4] = { -.5f * roi_in_width, -.5f * roi_in_height, .5f * roi_in_width, .5f * roi_in_height };
    for(int c = 0; c < 4; c++)
    {
      get_corner(oaabb, c, p);
      transform(p, o, rt, d->k_h, d->k_v);
      for(int k = 0; k < 2; k++)
        if(fabsf(o[k]) > 0.001f) newcropscale = fminf(newcropscale, aabb[(o[k] > 0 ? 2 : 0) + k] / o[k]);
    }
    if(newcropscale >= cropscale)
    {
      cropscale = newcropscale;
      // remember rotation center in whole-buffer coordinates:
      d->tx = roi_in->width * .5f;
      d->ty = roi_in->height * .5f;
      d->flip = flip;

      float ach = d->ch - d->cy, acw = d->cw - d->cx;
      // rotate and clip to max extent
      if(flip)
      {
        roi_out->y = d->tx - (.5f - d->cy) * cropscale * roi_in->width;
        roi_out->x = d->ty - (.5f - d->cx) * cropscale * roi_in->height;
        roi_out->height = ach * cropscale * roi_in->width;
        roi_out->width = acw * cropscale * roi_in->height;
      }
      else
      {
        roi_out->x = d->tx - (.5f - d->cx) * cropscale * roi_in->width;
        roi_out->y = d->ty - (.5f - d->cy) * cropscale * roi_in->height;
        roi_out->width = acw * cropscale * roi_in->width;
        roi_out->height = ach * cropscale * roi_in->height;
      }
      //std::cout<<"acw="<<acw<<" ach="<<ach<<"  cropscale="<<cropscale<<std::endl;
      //std::cout<<"roi_in: "<<roi_in->width<<"x"<<roi_in->height<<"+"<<roi_in->x<<","<<roi_in->y<<std::endl;
      //std::cout<<"roi_out: "<<roi_out->width<<"x"<<roi_out->height<<"+"<<roi_out->x<<","<<roi_out->y<<std::endl;
    }
  }

  // sanity check.
  if(roi_out->x < 0) roi_out->x = 0;
  if(roi_out->y < 0) roi_out->y = 0;
  if(roi_out->width < 1) roi_out->width = 1;
  if(roi_out->height < 1) roi_out->height = 1;

  // save rotation crop on output buffer in world scale:
  d->cix = roi_out->x;
  d->ciy = roi_out->y;
  d->ciw = roi_out->width;
  d->cih = roi_out->height;
}




PF::ScalePar::ScalePar():
      OpParBase(),
      vflip("vflip",this,false),
      hflip("hflip",this,false),
      rotate_angle("rotate_angle",this,0),
      autocrop("autocrop",this,true),
      scale_mode("scale_mode",this, SCALE_MODE_FIT, "SCALE_MODE_FIT", _("Fit")),
      scale_unit("scale_unit", this, SCALE_UNIT_PERCENT, "SCALE_UNIT_PERCENT", _("percent")),
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
      scale_resolution("scale_resolution",this,300),
      rotation_points("rotation_points",this)
{
  //scale_mode.add_enum_value( SCALE_MODE_FILL, "SCALE_MODE_FILL", "Fill" );
  //scale_mode.add_enum_value( SCALE_MODE_RESIZE, "SCALE_MODE_RESIZE", "Resize" );

  scale_unit.add_enum_value( SCALE_UNIT_PX, "SCALE_UNIT_PX", _("pixels") );
  scale_unit.add_enum_value( SCALE_UNIT_MM, "SCALE_UNIT_MM", _("mm") );
  scale_unit.add_enum_value( SCALE_UNIT_CM, "SCALE_UNIT_CM", _("cm") );
  scale_unit.add_enum_value( SCALE_UNIT_INCHES, "SCALE_UNIT_INCHES", _("inches") );

  set_type( "scale" );

  set_default_name( _("scale and rotate") );
}



VipsImage* PF::ScalePar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  if( srcimg == NULL ) return NULL;
  VipsImage* out, *rotated;

  PF_REF( srcimg, "ScalePar::build(): initial srcimg ref" );
  bool do_autocrop = autocrop.get();

  if( is_editing() ) {
    //std::cout<<"ScalePar::build(): editing, returning source image"<<std::endl;
    //PF_REF( srcimg, "ScalePar::build(): srcimg ref (editing mode)" );
    //return srcimg;
    do_autocrop = false;
  }


  in_width = srcimg->Xsize;
  in_height = srcimg->Ysize;
  out_width = in_width;
  out_height = in_height;
  sin_angle = 0;
  cos_angle = 1;

  crop.left = crop.top = 0;
  crop.width = out_width;
  crop.height = out_height;

  if( vflip.get() ) {
    VipsImage* flipped;
    if( vips_flip( srcimg, &flipped, VIPS_DIRECTION_VERTICAL, NULL ) ) {
      PF_UNREF( srcimg, "ScalePar::build(): image unref after vips_flip() failed." );
      return NULL;
    }
    PF_UNREF( srcimg, "ScalePar::build(): image unref after vips_flip()." );
    srcimg = flipped;
  }

  if( hflip.get() ) {
    VipsImage* flipped;
    if( vips_flip( srcimg, &flipped, VIPS_DIRECTION_HORIZONTAL, NULL ) ) {
      PF_UNREF( srcimg, "ScalePar::build(): image unref after vips_flip() failed." );
      return NULL;
    }
    PF_UNREF( srcimg, "ScalePar::build(): image unref after vips_flip()." );
    srcimg = flipped;
  }


  if( rotate_angle.get() != 0 ) {
    sin_angle = sin( rotate_angle.get() * 3.141592653589793 / 180.0 );
    cos_angle = cos( rotate_angle.get() * 3.141592653589793 / 180.0 );
    if( vips_similarity(srcimg, &rotated, "angle", rotate_angle.get(),
        //"idx", (double)-1*srcimg->Xsize/4,
        //"idy", (double)-1*srcimg->Ysize/2,
        //"odx", (double)srcimg->Xsize/4,
        //"ody", (double)srcimg->Ysize/2,
        NULL) ) {
      return NULL;
    }
    PF_UNREF( srcimg, "srcimg unref after rotate" );

    out_width = rotated->Xsize;
    out_height = rotated->Ysize;
    crop.left = crop.top = 0;
    crop.width = out_width;
    crop.height = out_height;

    if( do_autocrop == true ) {
      dt_iop_clipping_data_t data;
      data.angle = rotate_angle.get() * 3.141592653589793 / 180.0;
      dt_iop_roi_t roi_in, roi_out;
      roi_in.x = roi_in.y = 0;
      roi_in.width = srcimg->Xsize;
      roi_in.height = srcimg->Ysize;
      modify_roi_out(&data, &roi_in, &roi_out);
      /*
      int dw1 = static_cast<int>(fabs(sin_angle)*out_height);
      int dw2 = static_cast<int>(fabs(cos_angle)*out_width);
      int dh1 = static_cast<int>(fabs(sin_angle)*out_width);
      int dh2 = static_cast<int>(fabs(cos_angle)*out_height);
      int dw = MIN( dw1, dw2 );
      int dh = MIN( dh1, dh2 );
      std::cout<<"rotation: sin_angle="<<sin_angle<<" cos_angle="<<cos_angle<<std::endl;
      std::cout<<"rotation: out_width="<<out_width<<" dw1="<<dw1<<" dw2="<<dw2<<" dw="<<dw<<std::endl;
      //std::cout<<"rotation: width="<<srcimg->Xsize<<"->"<<cropped_width<<"  height="<<srcimg->Ysize<<"->"<<cropped_height<<std::endl;
      if( vips_crop( srcimg, &out, dw, dh, out_width-2*dw, out_height-2*dh, NULL ) ) {
        std::cout<<"WARNIG: ScalePar::build(): vips_crop() failed."<<std::endl;
        //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  srcimg->Ysize="<<srcimg->Ysize<<std::endl;
        //std::cout<<"vips_crop( srcimg, &out, "<<crop_left.get()/scale_factor<<", "<<crop_top.get()/scale_factor<<", "
        //    <<crop_width.get()/scale_factor<<", "<<crop_height.get()/scale_factor<<", NULL )"<<std::endl;
        return NULL;
      }
      */
      roi_out.width -= 2;
      roi_out.height -= 2;
      int dw = rotated->Xsize - roi_out.width;
      int dh = rotated->Ysize - roi_out.height;
      if( vips_crop( rotated, &out, dw/2, dh/2, roi_out.width, roi_out.height, NULL ) ) {
        std::cout<<"WARNIG: ScalePar::build(): vips_crop() failed."<<std::endl;
        //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  srcimg->Ysize="<<srcimg->Ysize<<std::endl;
        //std::cout<<"vips_crop( srcimg, &out, "<<crop_left.get()/scale_factor<<", "<<crop_top.get()/scale_factor<<", "
        //    <<crop_width.get()/scale_factor<<", "<<crop_height.get()/scale_factor<<", NULL )"<<std::endl;
        return NULL;
      }
      PF_UNREF( rotated, "" );
      crop.left = dw/2;
      crop.top = dh/2;
      crop.width = roi_out.width;
      crop.height = roi_out.height;
    } else {
      out = rotated;
    }
    set_image_hints( out );
    srcimg = out;
  } /*else {
    PF_REF( srcimg, "ScalePar::build(): srcimg ref for angle=0" );
  }*/

  scale_mult = 1;
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
    if( /*level!=0 ||*/ scale_width==0 || scale_height==0 ||
        scale_width==1 || scale_height==1 ) {
      //PF_REF( srcimg, "ScalePar::build(): srcimg ref (editing mode)" );
      return srcimg;
    }
    float scale = MIN( scale_width, scale_height );
    scale_mult = scale;

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
    //return srcimg;

    out = srcimg;
  }

  set_image_hints( out );
  return out;
}


PF::ProcessorBase* PF::new_scale()
{
  return( new PF::Processor<PF::ScalePar,PF::ScaleProc>() );
}
