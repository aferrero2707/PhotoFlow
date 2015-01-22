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

#ifndef PF_IMAGE_HH
#define PF_IMAGE_HH

#include <stdlib.h>
#include <sigc++/sigc++.h>

#include "layermanager.hh"
#include "pipeline.hh"

#define PREVIEW_PIPELINE_ID 1



namespace PF
{


  class Image: public sigc::trackable
  {
    LayerManager layer_manager;
    std::vector<Pipeline*> pipelines;

    // Flag indicating whether the update should be preformed asynchronously
    bool async;

    // Flag indicating whether the pipelines have to be re-built
    bool modified;

    // Flag indicating whether there is a re-building ongoing
    bool rebuilding;

    bool loaded;

		// Current "pfi" file name associated to this image
		std::string file_name;

    bool disable_update;

    GMutex* rebuild_mutex;
    GCond* rebuild_done;

    GMutex* export_mutex;
    GCond* export_done;

    GMutex* sample_mutex;
    GCond* sample_done;

    GMutex* remove_layer_mutex;
    GCond* remove_layer_done;

    ProcessorBase* convert2srgb;
    ProcessorBase* convert_format;

    void remove_from_inputs( PF::Layer* layer );
    void remove_from_inputs( PF::Layer* layer, std::list<Layer*>& list );
    void remove_layer( PF::Layer* layer, std::list<Layer*>& list );

		VipsImage* sampler_image;
		std::vector<float> sampler_values;

    void update_async();

  public:
    Image();

    ~Image();

    //sigc::signal<void> signal_modified;

    LayerManager& get_layer_manager() { return layer_manager; }

    void do_remove_layer( PF::Layer* layer );
    void remove_layer( PF::Layer* layer );

    Pipeline* add_pipeline( VipsBandFormat fmt, int level, rendermode_t mode=PF_RENDER_PREVIEW )
    {
      Pipeline* pipeline = new Pipeline( this, fmt, level, mode );
      pipelines.push_back( pipeline);
      return pipeline;
    }

    void remove_pipeline( Pipeline* pipeline )
    {
      std::vector<Pipeline*>::iterator i;
      for( i = pipelines.begin(); i != pipelines.end(); i++) {
        if( *i == pipeline) {
          pipelines.erase( i );
          break;
        }
      }
    }

    unsigned int get_npipelines() { return pipelines.size(); }

    Pipeline* get_pipeline(unsigned int n) 
    {
      if( n >= pipelines.size() ) return NULL;
      return(pipelines[n]);
    }

    bool is_async() { return async; }
    void set_async( bool flag ) { async = flag; }

    bool is_modified() { return modified; }
    void set_modified() { modified = true; }
    void clear_modified() { modified = false; }

    bool is_rebuilding() { return rebuilding; }
    void set_rebuilding( bool flag ) { rebuilding = flag; }

    bool is_loaded() { return loaded; }
    void set_loaded( bool flag ) { loaded = flag; }

    //Glib::Threads::Mutex& get_rebuild_mutex() { return rebuild_mutex; }

    void lock() { g_mutex_lock( rebuild_mutex); }
    void unlock() { g_mutex_unlock( rebuild_mutex); }
    void sample_lock() { g_mutex_lock( sample_mutex); }
    void sample_unlock() { g_mutex_unlock( sample_mutex); }
    void remove_layer_lock() { g_mutex_lock( remove_layer_mutex); }
    void remove_layer_unlock() { g_mutex_unlock( remove_layer_mutex); }
    void rebuild_done_signal() { g_cond_signal( rebuild_done ); }
    void export_done_signal() { g_cond_signal( export_done ); }
    void sample_done_signal() { g_cond_signal( sample_done ); }
    void remove_layer_done_signal() { g_cond_signal( remove_layer_done ); }

    void update( PF::Pipeline* pipeline=NULL, bool sync=false );
    void update_all() { update( NULL ); }
    void do_update( PF::Pipeline* pipeline=NULL );


		void sample( int layer_id, int x, int y, int size, 
								 VipsImage** image, std::vector<float>& values );
		void do_sample( int layer_id, VipsRect& area); 

    bool open( std::string filename );

		std::string get_filename() { return file_name; }
    bool save( std::string filename );
    void export_merged( std::string filename );
    void do_export_merged( std::string filename );
  };

  gint image_rebuild_callback( gpointer data );

}


#endif /*VIPS_PARITHMETIC_H*/


