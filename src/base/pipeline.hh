/* The PF::Pipeline class represents a particular "realization" of a given layer structure,
   for a specific pixel format and zoom level.
   It provides the connection between the layers and their associated VipsImage objects
   for this particular realization.
   
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

#ifndef PIPELINE_H
#define PIPELINE_H

#ifdef GTKMM_2
#include <glibmm/thread.h>
#endif
#ifdef GTKMM_3
#include <glibmm/threads.h>
#endif

#include "operation.hh"
#include "photoflow.hh"


namespace PF
{


  struct PipelineNode
  {
    ProcessorBase* processor;
    ProcessorBase* blender;
    VipsImage* image;
    std::vector<VipsImage*> images;
    VipsImage* blended;
    int input_id;

    PipelineNode(): processor( NULL ), blender( NULL ), image( NULL ), blended( NULL ), input_id( -1 ) {}
    ~PipelineNode();
  };


  class PipelineSink;
  class Image;

  class Pipeline
  {
    bool modified;
    Image* image;
    std::vector<PipelineNode*> nodes;
    VipsImage* output;
    std::vector<PipelineSink*> sinks;
    VipsBandFormat format;
    unsigned int level;
    rendermode_t render_mode;
    int output_layer_id;

    bool auto_zoom;
    int auto_zoom_width, auto_zoom_height;

    bool force_rebuild, op_caching_enabled;

    //Glib::Threads::Mutex mutex;

  public:
    Pipeline():
      modified(false), image(NULL), output(NULL), format(VIPS_FORMAT_UCHAR),
      level(0), render_mode(PF_RENDER_PREVIEW), output_layer_id(-1),
      auto_zoom(false), auto_zoom_width(0), auto_zoom_height(0),
      force_rebuild(false), op_caching_enabled(false) {}
    Pipeline( Image* img, VipsBandFormat fmt, int l, rendermode_t m ):
      image(img), output(NULL), format(fmt), level(l), render_mode(m), output_layer_id(-1),
      auto_zoom(false), auto_zoom_width(0), auto_zoom_height(0),
      force_rebuild(false), op_caching_enabled(false) {}

    ~Pipeline();

    //Glib::Threads::Mutex& get_mutex() { return mutex; }

    bool is_modified() { return modified; }
    void set_modified( bool flag ) { modified = flag; }

    Image* get_image() { return image; }

    void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsBandFormat get_format() { return format; }

    void set_level( unsigned int l )
    {
      if( level != l ) {
        //Glib::Threads::Mutex::Lock lock(mutex);
        set_force_rebuild();
      }
      level = l;
    }
    unsigned int get_level() { return level; }

    rendermode_t get_render_mode() { return render_mode; }
    void set_render_mode(rendermode_t m) { render_mode = m; }

    int get_output_layer_id() { return output_layer_id; }
    void set_output_layer_id( int id ) { output_layer_id = id; }

    bool get_auto_zoom() { return auto_zoom; }
    void set_auto_zoom( bool az, int w=0, int h=0 )
    {
      auto_zoom = az;
      auto_zoom_width = w;
      auto_zoom_height = h;
    }
    int get_auto_zoom_width() { return auto_zoom_width; }
    int get_auto_zoom_height() { return auto_zoom_height; }

    bool get_force_rebuild() { return force_rebuild; }
    void set_force_rebuild() { force_rebuild = true; }
    void clear_force_rebuild() { force_rebuild = false; }

    bool get_op_caching_enabled() { return op_caching_enabled; }
    void set_op_caching_enabled(bool flag) { op_caching_enabled = flag; }

    PipelineNode* set_node( Layer* layer, Layer* input_layer );
    void set_image( VipsImage* img, unsigned int id );
    void set_images( std::vector<VipsImage*> img, unsigned int id );
    void set_blended( VipsImage* img, unsigned int id );
    void remove_node( unsigned int id );

    std::vector<PipelineNode*>& get_nodes(){ return nodes; } 
    PipelineNode* get_node( int id ) 
    {
      if( (id<0) || (id>=(int)nodes.size()) ) return NULL;
      return nodes[id];
    }
    PipelineNode* get_node( Layer* l );

    VipsImage* get_output() { return output; }
    void set_output( VipsImage* img ) { output = img; }

    bool processing();

    //Glib::Threads::Mutex& get_processing_mutex() { return processing_mutex; }

    void add_sink( PipelineSink* sink ) { sinks.push_back( sink ); }
		bool has_sinks() { return( !sinks.empty() ); }

    void lock_processing();
    void unlock_processing();

    void update( VipsRect* area );
    void sink( const VipsRect& area );
  };


  class PipelineSink
  {
    Pipeline* pipeline;
    bool processing;
    
    //Glib::Threads::Mutex processing_mutex;
    int processing_count;

  public:
    PipelineSink( Pipeline* v ): pipeline(v), processing( false ), processing_count( 0 )
    {
      if(pipeline) pipeline->add_sink( this );
    }
		virtual ~PipelineSink() {}

    Pipeline* get_pipeline() { return pipeline; }

    bool is_processing() { return processing; }
    void set_processing( bool flag ) { processing = flag; }
    
    void processing_count_increase() { processing_count += 1; }
    void processing_count_decrease() { processing_count -= 1; }
    int get_processing_count() { return processing_count; }
    //Glib::Threads::Mutex& get_processing_mutex() { return processing_mutex; }

    virtual void update( VipsRect* area ) = 0;
    virtual void sink( const VipsRect& /*area*/ ) { }
    virtual void dispose() { }

    virtual void process_area( const VipsRect& /*area*/ ) {}
    virtual void process_start( const VipsRect& /*area*/ ) {}
    virtual void process_end( const VipsRect& /*area*/ ) {}
  };

}


#endif /*VIPS_PARITHMETIC_H*/


