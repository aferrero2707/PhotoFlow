/* base class for all PhotoFlow layer operations
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


#ifndef VIPS_PFLAYER_H
#define VIPS_PFLAYER_H

#include <string>
#include <vector>
#include <iostream>

#include <vips/vips.h>
#include <vips/vips>

#include "pftypes.hh"

#include "format_info.hh"

#include "property.hh"



#define OP_TEMPLATE_DEF typename T, class BLENDER, colorspace_t CS, bool has_imap, bool has_omap, bool PREVIEW
#define OP_TEMPLATE_IMP T, BLENDER, CS, has_imap, has_omap, PREVIEW




namespace PF 
{

  class ProcessorBase;

  /* Base class for all operation parameter implementations
   */
  class OpParBase
  {
    VipsDemandStyle demand_hint;
    blendmode_t blend_mode;
    float intensity;
    float opacity;

    VipsImage* out;
    ProcessorBase* processor;

    // Requested image fields
    int xsize;
    int ysize;
    int bands;
    VipsBandFormat format;
    VipsCoding coding;
    VipsInterpretation interpretation;
    
  public:
    OpParBase();

    void set_processor(ProcessorBase* p) { processor = p; }
    ProcessorBase* get_processor() { return processor; }

    void set_demand_hint(VipsDemandStyle val) { demand_hint = val; }
    VipsDemandStyle get_demand_hint() { return demand_hint; }

    blendmode_t get_blend_mode() { return blend_mode; }
    void set_blend_mode(blendmode_t mode) { blend_mode = mode; }

    void set_intensity(float val) { intensity = val; }
    float get_intensity() { return intensity; }

    void set_opacity(float val) { opacity = val; }
    float get_opacity() { return opacity; }

    /* Function to derive the output area from the input area
    */
    virtual void transform(const Rect* rin, Rect* rout)
    {
      rout->left = rin->left;
      rout->top = rin->top;
      rout->width = rin->width;
      rout->height = rin->height;
    }

    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin)
    {
      rin->left = rout->left;
      rin->top = rout->top;
      rin->width = rout->width;
      rin->height = rout->height;
    }


    virtual bool has_intensity() { return true; }
    virtual bool has_opacity() { return true; }

    virtual void build(std::vector<VipsImage*>& in, int first, VipsImage* imap, VipsImage* omap);

    VipsImage* get_image() { return out; }

    int get_xsize() { return xsize; }
    int get_ysize() { return ysize; }
    int get_nbands() { return bands; }
    VipsInterpretation get_interpretation() { return interpretation; }
    VipsBandFormat get_format() { return format; }
    VipsCoding get_coding() { return coding; }
    

    void grayscale_image(int w, int h, VipsBandFormat fmt)
    {
      xsize = w; ysize = h;
      bands = 1; interpretation = VIPS_INTERPRETATION_B_W;
      format = fmt, coding = VIPS_CODING_NONE;
    }

    void rgb_image(int w, int h, VipsBandFormat fmt)
    {
      xsize = w; ysize = h;
      bands = 3; interpretation = VIPS_INTERPRETATION_RGB;
      format = fmt, coding = VIPS_CODING_NONE;
    }

    void lab_image(int w, int h, VipsBandFormat fmt)
    {
      xsize = w; ysize = h;
      bands = 3; interpretation = VIPS_INTERPRETATION_LAB;
      format = fmt, coding = VIPS_CODING_NONE;
    }

    void cmyk_image(int w, int h, VipsBandFormat fmt)
    {
      xsize = w; ysize = h;
      bands = 4; interpretation = VIPS_INTERPRETATION_CMYK;
      format = fmt, coding = VIPS_CODING_NONE;
    }
  };


  /* Base parameters for all transform operations
   *
   * A transform operation can only replace the input image,
   * (ex. icc transform or image rescaling), so intensity or opacity maps are useless
   */
  class OpParTransform: public OpParBase
  {
  public: 
    virtual bool has_intensity() { return false; }
    virtual bool has_opacity() { return false; }
  };



  template<class T, bool has_imap>
  class IntensityProc
  {
  public:
    float get_intensity(float& intensity, T*& p, int& x)
    {
      //std::cout<<"IntensityProc<T,true>::get_intensity(): "<<(intensity*p[x]/(FormatInfo<T>::MAX-FormatInfo<T>::MIN))<<std::endl;
      return(intensity*p[x++]/(FormatInfo<T>::MAX-FormatInfo<T>::MIN));
    }
  };



  template<class T>
  class IntensityProc<T,false>
  {
  public:
    float get_intensity(float& intensity, T*& p, int& x)
    {
      //std::cout<<"IntensityProc<T,false>::get_intensity()"<<std::endl;
      return(intensity);
    }
  };



};


//int vips_pflayer( VipsImage **in, VipsImage* imap, VipsImage* omap, VipsImage **out, int n, 
//	      PF::OperationBase* op, ... );


#endif
