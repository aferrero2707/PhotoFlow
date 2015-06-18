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


#ifndef OPERATION_H
#define OPERATION_H

#include <math.h>
#include <unistd.h>

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>

#include <vips/vips.h>
//#include <vips/vips>

#include "pftypes.hh"

#include "format_info.hh"

#include "property.hh"

#include "color.hh"

#include "photoflow.hh"



#define OP_TEMPLATE_DEF \
  typename T, class BLENDER, colorspace_t CS,	\
    int CHMIN, int CHMAX,								\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP \
  T, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW

#define OP_TEMPLATE_DEF_BLENDER_SPEC \
  typename T, colorspace_t CS,	int CHMIN, int CHMAX,		\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_BLENDER_SPEC(BLENDER_SPEC) \
  T, BLENDER_SPEC< T, CS, CHMIN, CHMAX, has_omap >, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW


#define OP_TEMPLATE_DEF_TYPE_SPEC \
  class BLENDER, colorspace_t CS, int CHMIN, int CHMAX,		\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_TYPE_SPEC(TYPE_SPEC) \
  TYPE_SPEC, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW


#define OP_TEMPLATE_DEF_CS_SPEC \
  typename T, class BLENDER, int CHMIN, int CHMAX,	\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_CS_SPEC(CS_SPEC) \
  T, BLENDER, CS_SPEC, CHMIN, CHMAX, has_imap, has_omap, PREVIEW



#define OP_TEMPLATE_DEF_PREVIEW_SPEC \
  typename T, class BLENDER, colorspace_t CS,	int CHMIN, int CHMAX,	\
    bool has_imap, bool has_omap

#define OP_TEMPLATE_IMP_PREVIEW_SPEC(PREVIEW_SPEC) \
  T, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW_SPEC



namespace PF 
{

  class ProcessorBase;
  class Layer;

  class OperationConfigUI
  {
    std::list<std::string> initial_params;

    Layer* layer;

  public:

    OperationConfigUI( Layer* l ): layer( l ) {}
    virtual ~OperationConfigUI() {}

    Layer* get_layer() { return layer; }
    //void set_layer( Layer* l ) { layer = l; }
    
    virtual void open() = 0;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void do_update() { update(); }
    virtual void update_properties() = 0;
  };

  /* Base class for all operation parameter implementations
   */
  class OpParBase: public sigc::trackable
  {
    VipsDemandStyle demand_hint;

    std::string type;

    ProcessorBase* processor;

    OperationConfigUI* config_ui;

    // Requested image fields
    int xsize;
    int ysize;
    int bands;
    VipsBandFormat format;
    VipsCoding coding;
    VipsInterpretation interpretation;

    rendermode_t render_mode;

    bool map_flag;

    bool editing_flag;

    bool modified_flag;

    std::list<PropertyBase*> mapped_properties;
    std::list<PropertyBase*> properties;

    Property<float> intensity;

    PropertyBase grey_target_channel;
    PropertyBase rgb_target_channel;
    PropertyBase lab_target_channel;
    PropertyBase cmyk_target_channel;

    Property<bool> mask_enabled;

  public:
    sigc::signal<void> signal_modified;

    OpParBase();

    virtual ~OpParBase()
    {
      std::cout<<"~OpParBase(): deleting operation "<<(void*)this<<std::endl;
    }

    std::string get_type() { return type; }
    void set_type( std::string str ) { type = str; }

    std::list<PropertyBase*>& get_properties() { return properties; }
    std::list<PropertyBase*>& get_mapped_properties() { return mapped_properties; }
    void add_property( PropertyBase* p ) 
    { 
      properties.push_back(p); 
      p->signal_modified.connect(sigc::mem_fun(this, &OpParBase::modified) );
    }
    void map_property( PropertyBase* p ) { 
      if( p->is_internal() ) 
        return;
      mapped_properties.push_back(p); 
      p->signal_modified.connect(sigc::mem_fun(this, &OpParBase::modified) );
    }
    void map_properties( std::list<PropertyBase*> pl ) 
    { 
      mapped_properties.insert( mapped_properties.end(),
				pl.begin(), pl.end() ); 
      for( std::list<PropertyBase*>::iterator i = pl.begin();
           i != pl.end(); i++ )
        (*i)->signal_modified.connect(sigc::mem_fun(this, &OpParBase::modified) );
    }
    void save_properties(std::list<std::string>& plist);
    void restore_properties(const std::list<std::string>& plist);

    virtual bool import_settings( OpParBase* pin );

    void set_processor(ProcessorBase* p) { processor = p; }
    ProcessorBase* get_processor() { return processor; }

    void set_demand_hint(VipsDemandStyle val) { demand_hint = val; }
    VipsDemandStyle get_demand_hint() { return demand_hint; }

    void set_intensity(float val) { intensity.set(val); }
    float get_intensity() { return intensity.get(); }

    bool is_map() { return map_flag; }
    void set_map_flag( bool flag ) { map_flag = flag; }

    bool is_editing() { return editing_flag; }
    void set_editing_flag( bool flag ) { editing_flag = flag; }

    bool get_mask_enabled() { return mask_enabled.get(); }

    bool is_modified() { return modified_flag; }
    void set_modified() { modified_flag = true; }
    void clear_modified();
    virtual void modified() { set_modified(); signal_modified.emit(); }


    int get_rgb_target_channel() 
    {
      if( !(rgb_target_channel.get_enum_value().second.first.empty()) )
				return( rgb_target_channel.get_enum_value().first );
      else 
				return -1;
    } 

    int get_lab_target_channel() 
    {
      if( !(lab_target_channel.get_enum_value().second.first.empty()) )
	return( lab_target_channel.get_enum_value().first );
      else 
	return -1;
    } 

    int get_cmyk_target_channel() 
    {
      if( !(cmyk_target_channel.get_enum_value().second.first.empty()) )
				return( cmyk_target_channel.get_enum_value().first );
      else 
				return -1;
    } 

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
    virtual bool needs_input() { return true; }
    virtual bool needs_caching() { return false; }
    virtual bool init_hidden() { return false; }

    rendermode_t get_render_mode() { return render_mode; }
    void set_render_mode(rendermode_t m) { render_mode = m; }

    virtual void pre_build( rendermode_t /*mode*/ ) {}

    virtual VipsImage* build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level);
    virtual std::vector<VipsImage*> build_many(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level);

    PropertyBase* get_property(std::string name);

    OperationConfigUI* get_config_ui() { return config_ui; }
    void set_config_ui( OperationConfigUI* ui ) { config_ui = ui; }

    int get_xsize() { return xsize; }
    int get_ysize() { return ysize; }
    int get_nbands() { return bands; }
    void set_nbands( int n ) { bands = n; }
    VipsInterpretation get_interpretation() { return interpretation; }
    colorspace_t get_colorspace() { return( PF::convert_colorspace( get_interpretation() ) ); }
    VipsBandFormat get_format() { return format; }
    virtual void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsCoding get_coding() { return coding; }
    void set_coding( VipsCoding c ) { coding = c; }
    
		virtual void set_image_hints( OpParBase* op )
		{
			if( !op ) return;
			set_image_hints( op->get_xsize(), op->get_ysize(),
											 op->get_interpretation() );
			bands = op->get_nbands();
			rgb_target_channel.set_enum_value(op->get_rgb_target_channel());
			lab_target_channel.set_enum_value(op->get_lab_target_channel());
			cmyk_target_channel.set_enum_value(op->get_cmyk_target_channel());
			set_demand_hint( op->get_demand_hint() );
			set_map_flag( op->is_map() );
			set_coding( op->get_coding() );
			set_format( op->get_format() );
			set_render_mode( op->get_render_mode() );
		}

    virtual void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      set_image_hints( img->Xsize, img->Ysize,
		       img->Type );
      bands = img->Bands;
    }

    void set_image_hints(int w, int h, VipsInterpretation interpr);
    void set_image_hints(int w, int h, colorspace_t cs);

    void grayscale_image(int w, int h)
    {
      xsize = w; ysize = h;
      bands = 1; interpretation = VIPS_INTERPRETATION_B_W;
      coding = VIPS_CODING_NONE;
    }

    void rgb_image(int w, int h)
    {
      xsize = w; ysize = h;
      bands = 3; interpretation = VIPS_INTERPRETATION_RGB;
      coding = VIPS_CODING_NONE;
    }

    void lab_image(int w, int h)
    {
      xsize = w; ysize = h;
      bands = 3; interpretation = VIPS_INTERPRETATION_LAB;
      coding = VIPS_CODING_NONE;
    }

    void cmyk_image(int w, int h)
    {
      xsize = w; ysize = h;
      bands = 4; interpretation = VIPS_INTERPRETATION_CMYK;
      coding = VIPS_CODING_NONE;
    }


    bool save( std::ostream& ostr, int level );
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
      return(intensity*(p[x++]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE));
    }
  };



  template<class T>
  class IntensityProc<T,false>
  {
  public:
    float get_intensity(float& intensity, T*& /*p*/, int& /*x*/)
    {
      //std::cout<<"IntensityProc<T,false>::get_intensity()"<<std::endl;
      return(intensity);
    }
  };



  template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX, bool has_omap>
  class BlendBase
  {
  public:
    T* pmap;
    void init_line(VipsRegion* omap, int left, int top) { pmap = (T*)VIPS_REGION_ADDR( omap, left, top ); }
  };


  template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX >
  class BlendBase<T, colorspace, CHMIN, CHMAX, false>
  {
  public:
    T* pmap;
    void init_line(VipsRegion* /*omap*/, int /*left*/, int /*top*/) { }
  };


  #include "blend_passthrough.hh"
  #include "blend_normal.hh"
  #include "blend_grain_extract.hh"
  #include "blend_grain_merge.hh"
  #include "blend_multiply.hh"
  #include "blend_screen.hh"
  #include "blend_lighten.hh"
  #include "blend_darken.hh"
  #include "blend_overlay.hh"
  #include "blend_soft_light.hh"
  #include "blend_hard_light.hh"
  #include "blend_vivid_light.hh"
  #include "blend_luminosity.hh"
  #include "blend_color.hh"

  int vips_copy_metadata( VipsImage* in, VipsImage* out );
};


//int vips_pflayer( VipsImage **in, VipsImage* imap, VipsImage* omap, VipsImage **out, int n, 
//	      PF::OperationBase* op, ... );


#endif
