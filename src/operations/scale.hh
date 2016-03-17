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

#ifndef PF_SCALE_H
#define PF_SCALE_H

#include <string>

#include "../base/property.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"

namespace PF 
{

enum scale_mode_t
{
  SCALE_MODE_FIT,
  SCALE_MODE_FILL,
  SCALE_MODE_RESIZE
};


enum scale_unit_t
{
  SCALE_UNIT_PX,
  SCALE_UNIT_PERCENT,
  SCALE_UNIT_MM,
  SCALE_UNIT_CM,
  SCALE_UNIT_INCHES
};


  class ScalePar: public OpParBase
  {
  Property<bool> vflip, hflip;
    Property<float> rotate_angle;
    Property<bool> autocrop;
    PropertyBase scale_mode;
    PropertyBase scale_unit;
    Property<int> scale_width_pixels, scale_height_pixels;
    Property<float> scale_width_percent, scale_height_percent;
    Property<float> scale_width_mm, scale_height_mm;
    Property<float> scale_width_cm, scale_height_cm;
    Property<float> scale_width_inches, scale_height_inches;
    Property<float> scale_resolution;

    Property< std::vector< std::pair<int,int> > > rotation_points;

    int in_width, in_height;
    int out_width, out_height;
    float sin_angle, cos_angle;
    float scale_mult;
    VipsRect crop;

  public:
    ScalePar();

    bool has_opacity() { return false; }
    bool has_intensity() { return false; }

    std::vector< std::pair<int,int> >& get_rotation_points() { return rotation_points.get(); }

    /* Function to derive the output area from the input area
    */
    virtual void transform(const Rect* rin, Rect* rout)
    {
      int in_w2 = in_width/2;
      int in_h2 = in_height/2;
      int out_w2 = out_width/2;
      int out_h2 = out_height/2;
      float xout = cos_angle*(rin->left-in_w2) - sin_angle*(rin->top-in_h2) + out_w2 - crop.left;
      float yout = sin_angle*(rin->left-in_w2) + cos_angle*(rin->top-in_h2) + out_h2 - crop.top;
      //std::cout<<"xout="<<xout<<" = "<<cos_angle<<"*("<<rin->left<<"-"<<in_w2<<") - "<<sin_angle<<"*("<<rin->top<<"-"<<in_h2<<") + "<<out_w2<<std::endl;
      //std::cout<<"sin_angle="<<sin_angle<<" cos_angle="<<cos_angle<<"  xout="<<xout<<" yout="<<yout<<std::endl;
      rout->left = xout*scale_mult;
      rout->top = yout*scale_mult;
      rout->width = rin->width*scale_mult;
      rout->height = rin->height*scale_mult;
    }

    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin)
    {
      float minus = -1.0;
      int in_w2 = in_width/2;
      int in_h2 = in_height/2;
      int out_w2 = out_width/2;
      int out_h2 = out_height/2;
      float xin = cos_angle*(rout->left-out_w2) + sin_angle*(rout->top-out_h2) + in_w2 + crop.left;
      float yin = minus*sin_angle*(rout->left-out_w2) + cos_angle*(rout->top-out_h2) + in_h2 + crop.top;
      //std::cout<<"xin="<<xin<<" = "<<cos_angle<<"*("<<rout->left<<"-"<<out_w2<<") - "<<sin_angle<<"*("<<rout->top<<"-"<<out_h2<<") + "<<in_w2<<std::endl;
      //std::cout<<"sin_angle="<<sin_angle<<" cos_angle="<<cos_angle<<"  xin="<<xin<<" yin="<<yin<<std::endl;
      rin->left = xin/scale_mult;
      rin->top = yin/scale_mult;
      rin->width = rout->width/scale_mult;
      rin->height = rout->height/scale_mult;
    }


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ScaleProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_scale();
}

#endif 


