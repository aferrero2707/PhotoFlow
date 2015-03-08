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

    //Glib::Threads::Mutex processing_mutex;

  public:
    Pipeline(): modified(false), image(NULL), output(NULL), format(VIPS_FORMAT_UCHAR), level(0), render_mode(PF_RENDER_PREVIEW) {}
    Pipeline( Image* img, VipsBandFormat fmt, int l, rendermode_t m ): image(img), output(NULL), format(fmt), level(l), render_mode(m) {}

    ~Pipeline();

    bool is_modified() { return modified; }
    void set_modified( bool flag ) { modified = flag; }

    Image* get_image() { return image; }

    void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsBandFormat get_format() { return format; }

    void set_level( unsigned int l ) { level = l; }
    unsigned int get_level() { return level; }

    rendermode_t get_render_mode() { return render_mode; }
    void set_render_mode(rendermode_t m) { render_mode = m; }

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
    virtual void sink( const VipsRect& area ) { }

    virtual void process_area( const VipsRect& area ) {}
    virtual void process_start( const VipsRect& area ) {}
    virtual void process_end( const VipsRect& area ) {}
  };

}


#endif /*VIPS_PARITHMETIC_H*/


