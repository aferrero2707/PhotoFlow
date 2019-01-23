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
#include <float.h>
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
#include "image_hierarchy.hh"
#include "photoflow.hh"



#define OP_TEMPLATE_DEF \
    typename T, class BLENDER, PF::colorspace_t CS,	\
    int CHMIN, int CHMAX,								\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP \
    T, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW

#define OP_TEMPLATE_DEF_BLENDER_SPEC \
    typename T, PF::colorspace_t CS,	int CHMIN, int CHMAX,		\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_BLENDER_SPEC(BLENDER_SPEC) \
    T, BLENDER_SPEC< T, CS, CHMIN, CHMAX, has_omap >, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW


#define OP_TEMPLATE_DEF_TYPE_SPEC \
    class BLENDER, PF::colorspace_t CS, int CHMIN, int CHMAX,		\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_TYPE_SPEC(TYPE_SPEC) \
    TYPE_SPEC, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW


#define OP_TEMPLATE_DEF_CS_SPEC \
    typename T, class BLENDER, int CHMIN, int CHMAX,	\
    bool has_imap, bool has_omap, bool PREVIEW

#define OP_TEMPLATE_IMP_CS_SPEC(CS_SPEC) \
    T, BLENDER, CS_SPEC, CHMIN, CHMAX, has_imap, has_omap, PREVIEW



#define OP_TEMPLATE_DEF_PREVIEW_SPEC \
    typename T, class BLENDER, PF::colorspace_t CS,	int CHMIN, int CHMAX,	\
    bool has_imap, bool has_omap

#define OP_TEMPLATE_IMP_PREVIEW_SPEC(PREVIEW_SPEC) \
    T, BLENDER, CS, CHMIN, CHMAX, has_imap, has_omap, PREVIEW_SPEC


#define PF_OUPUT_CACHE_TS 128



namespace PF 
{

class ProcessorBase;
class Layer;

template<typename T>
class PixelMatrix
{
  T* data;
  T** rows;
  T** ptr;
  int w, h, co, ro;
public:
  PixelMatrix(): data(NULL), w(0), h(0), ptr(NULL), rows(NULL) {}
  PixelMatrix(T* buf, int width, int height, int rowstride, int roffs, int coffs):
    data(NULL), w(width), h(height), ro(roffs), co(coffs)
  {
    ptr = new T*[h];
    rows = ptr - roffs;
    for(int i = 0; i < height; i++) {
      ptr[i] = buf - coffs;
      buf += rowstride;
    }
    //std::cout<<"Initialized pixel matrix from buf="<<buf<<"  "<<w<<"x"<<h<<"+"<<coffs<<","<<roffs<<std::endl;
  }
  PixelMatrix(int width, int height, int roffs=0, int coffs=0):
    data(NULL), w(width), h(height), ro(roffs), co(coffs), ptr(NULL), rows(NULL)
  {
    data = new T[w*h];
    int rowstride = w;
    T* buf = data;
    ptr = new T*[h];
    rows = ptr - roffs;
    for(int i = 0; i < height; i++) {
      ptr[i] = buf - coffs;
      buf += rowstride;
    }
    //std::cout<<"Initialized pixel matrix "<<w<<"x"<<h<<"+"<<coffs<<","<<roffs<<std::endl;
  }
  PixelMatrix(const PixelMatrix& m):
    data(NULL), w(0), h(0)
  {
    w = m.width();
    h = m.height();
    ro = m.roffs();
    co = m.coffs();
    data = new T[w*h];
    int rowstride = w;
    T* buf = data;
    ptr = new T*[h];
    rows = ptr - ro;
    for(int i = 0; i < h; i++) {
      ptr[i] = buf - co;
      memcpy(buf, m[i] + co, sizeof(T)*w);
      buf += rowstride;
    }
    //std::cout<<"Initialized pixel matrix "<<w<<"x"<<h<<"+"<<coffs<<","<<roffs<<std::endl;
  }
  ~PixelMatrix()
  {
    if(data) delete[] data;
    if(ptr) delete[] ptr;
  }
  // use as pointer to T**
  operator T**() { return rows; }
  T** get_rows() { return rows; }
  T* operator[](int id) const { return rows[id]; }
  int width() const { return w; }
  int height() const { return h; }
  int roffs() const { return ro; }
  int coffs() const { return co; }
};




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
  ProcessorBase* to_map;

  OperationConfigUI* config_ui;

  // Requested image fields
  int xsize;
  int ysize;
  int bands;
  VipsBandFormat format;
  VipsCoding coding;
  VipsInterpretation interpretation;

  // Padding required by the operation when processing input images
  std::vector<int> input_paddings;
  // Total padding requested to the output of the operation by all the
  // child operations in the pipeline
  std::vector<int> output_paddings;
  bool output_caching_enabled;

  rendermode_t render_mode;

  bool map_flag;

  bool editing_flag;

  bool modified_flag;

  std::string default_name;

  std::list<PropertyBase*> mapped_properties;
  std::list<PropertyBase*> properties;

  Property<float> intensity;

  PropertyBase grey_target_channel;
  PropertyBase rgb_target_channel;
  PropertyBase lab_target_channel;
  PropertyBase cmyk_target_channel;

  Property<bool> mask_enabled;

  int file_format_version;

  Property<bool> previous_layer_is_input;
  Property<bool> enable_padding;
  int test_padding;

public:
  sigc::signal<void> signal_modified;

  OpParBase();

  virtual ~OpParBase()
  {
    //for(unsigned int i = 0; i < outvec.size(); i++ ) {
    //  PF_UNREF( outvec[i], "~OpParBase(): previous outputs unref" );
    //}
    //#ifndef NDEBUG
    std::cout<<"~OpParBase(): deleting operation "<<(void*)this<<" ("<<get_type()<<")"<<std::endl;
    //#endif
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
  // update properties of sub-operations
  virtual void propagate_settings() {}

  void set_processor(ProcessorBase* p) { processor = p; }
  ProcessorBase* get_processor() { return processor; }

  void set_demand_hint(VipsDemandStyle val) { demand_hint = val; }
  VipsDemandStyle get_demand_hint() { return demand_hint; }

  void set_intensity(float val) { intensity.set(val); }
  float get_intensity() { return intensity.get(); }

  bool is_map() { return map_flag; }
  void set_map_flag( bool flag ) { map_flag = flag; }

  virtual bool is_editing_locked() { return false; }
  bool is_editing() { return( editing_flag || is_editing_locked() ); }
  void set_editing_flag( bool flag ) { editing_flag = flag; }

  bool get_mask_enabled() { return mask_enabled.get(); }
  void set_mask_enabled( bool val ) { mask_enabled.update(val); }

  virtual bool convert_inputs_on_load() { return true; }

  bool is_modified() { return modified_flag; }
  void set_modified() { modified_flag = true; }
  void clear_modified();
  virtual void modified()
  {
    set_modified();
    //std::cout<<"OpParBase::modified(): emitting signal_modified."<<std::endl;
    signal_modified.emit();
    //std::cout<<"OpParBase::modified(): signal_modified emitted."<<std::endl;
  }


  std::string get_default_name() { return default_name; }
  void set_default_name( std::string n ) { default_name = n; }

  void set_file_format_version(int v) { file_format_version = v; }
  int get_file_format_version() { return file_format_version; }


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

  bool get_previous_layer_is_input() { return previous_layer_is_input.get(); }

  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
  {
    int p = enable_padding.get() ? test_padding : 0;
    rout->left = rin->left+p;
    rout->top = rin->top+p;
    rout->width = rin->width-p*2;
    rout->height = rin->height-p*2;
  }

  /* Function to derive the area to be read from input images,
       based on the requested output area
   */
  virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
  {
    int p = enable_padding.get() ? test_padding : 0;
    rin->left = rout->left-p;
    rin->top = rout->top-p;
    rin->width = rout->width+p*2;
    rin->height = rout->height+p*2;
  }


  virtual bool has_intensity() { return true; }
  virtual bool has_opacity() { return true; }
  virtual bool has_target_channel() { return false; }
  virtual bool needs_input() { return true; }
  virtual bool needs_caching() { return false; }
  virtual bool init_hidden() { return false; }

  // whether the operation returns an input image at a given zoom level
  // id is the output image index
  virtual bool is_noop( VipsImage* full_res, unsigned int id, unsigned int level )
  {
    return false;
  }
  virtual void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  void set_padding( int p, unsigned int id )
  {
    for( unsigned int i = input_paddings.size(); i <= id; i++ ) input_paddings.push_back(0);
    input_paddings[id] = p;
  }
  int get_padding( unsigned int id = 0 )
  {
    return( (input_paddings.size()>id) ? input_paddings[id] : 0 );
  }
  int get_output_padding( unsigned int id = 0 )
  {
    return( (output_paddings.size()>id) ? output_paddings[id] : 0 );
  }
  bool set_output_padding( int p, unsigned int id = 0 )
  {
    for( unsigned int i = output_paddings.size(); i <= id; i++ ) output_paddings.push_back(0);
    if( output_paddings[id] < p ) {
      output_paddings[id] = p;
      return true;
    }
    return false;
  }
  void reset_output_padding() { output_paddings.clear(); }
  void set_output_caching(bool flag) { output_caching_enabled = flag; }
  bool get_output_caching() { return output_caching_enabled; }
  virtual int get_test_padding() { return 0; }

  // return the number of output images. Equal to 1 in most cases
  virtual int get_output_num() { return 1; }

  rendermode_t get_render_mode() { return render_mode; }
  void set_render_mode(rendermode_t m) { render_mode = m; }

  // called after all properties have been loaded from .pfi file
  virtual void finalize() {}

  virtual void pre_build( rendermode_t /*mode*/ ) {}

  virtual VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap, unsigned int& level);
  virtual std::vector<VipsImage*> build_many(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap, unsigned int& level);
  std::vector<VipsImage*> build_many_internal(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap, unsigned int& level);
  virtual void fill_image_hierarchy(std::vector<VipsImage*>& in,
      VipsImage* imap, VipsImage* omap, std::vector<VipsImage*>& out);


  PropertyBase* get_property(std::string name);

  template<typename T> bool set_property_value(std::string name, const T& newval)
  {
    PropertyBase* prop = get_property(name);
    if( !prop ) return false;
    prop->update( newval );
    return true;
  }



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

  void multiband_image(int w, int h, int b)
  {
    xsize = w; ysize = h;
    bands = b; interpretation = VIPS_INTERPRETATION_MULTIBAND;
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
#include "blend_add.hh"
#include "blend_subtract.hh"
#include "blend_grain_extract.hh"
#include "blend_grain_merge.hh"
#include "blend_multiply.hh"
#include "blend_divide.hh"
#include "blend_screen.hh"
#include "blend_lighten.hh"
#include "blend_darken.hh"
#include "blend_overlay.hh"
#include "blend_soft_light.hh"
#include "blend_hard_light.hh"
#include "blend_vivid_light.hh"
#include "blend_luminosity.hh"
#include "blend_luminance.hh"
#include "blend_color.hh"
#include "blend_exclusion.hh"
#include "blend_lch.hh"

int vips_copy_metadata( VipsImage* in, VipsImage* out );

void print_embedded_profile( VipsImage* img );
};


//int vips_pflayer( VipsImage **in, VipsImage* imap, VipsImage* omap, VipsImage **out, int n, 
//	      PF::OperationBase* op, ... );


#endif
