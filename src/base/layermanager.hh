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

#ifndef PF_LAYER_MANAGER_H
#define PF_LAYER_MANAGER_H

#include <list>
#include <vector>

#include <sigc++/sigc++.h>

#include "pipeline.hh"
#include "layer.hh"

namespace PF
{

  class Image;

  class LayerManager
  {
    std::vector<Layer*> layers_pool;
    std::list<Layer*> layers;

    Image* image;

    //VipsImage* output;

    std::list<Layer*>* get_list( PF::Layer* layer, std::list<PF::Layer*>& list);

    void get_input_layers( Layer* layer, std::list<PF::Layer*>& container,
                           std::list<Layer*>& inputs );

    void get_child_layers( Layer* layer, std::list<PF::Layer*>& container,
                           std::list<Layer*>& children );

    bool get_parent_layers(Layer* layer, std::list< std::pair<std::string,Layer*> >& plist, 
			   std::string parent_name, std::list<Layer*>& list);

    Layer* get_container_layer( Layer* layer, std::list<Layer*>& list );

    PF::CacheBuffer* get_cache_buffer( std::list<Layer*>& list );

    // Walk through the given layer chain and set the "visible" flag
    void update_visible( std::list<Layer*>& list, bool visible );

    // Walk through the given layer chain and set the "dirty" flag of all layers starting from "layer_id" to "true"
    void update_dirty( std::list<Layer*>& list, bool& dirty );

    void reset_dirty( std::list<Layer*>& list );
    
    void update_ui( std::list<Layer*>& list );

    VipsImage* rebuild_chain(Pipeline* pipeline, colorspace_t cs, 
														 int width, int height, 
														 std::list<Layer*>& list, 
														 Layer* previous_layer);
    
  public:
    LayerManager(Image* image);
    ~LayerManager();

    Layer* new_layer();
    void delete_layer( Layer* layer );

    Image* get_image() { return image; }
    void set_image( Image* img ) { image = img; }

    std::list<Layer*>& get_layers() { return layers; }

    std::list<Layer*>* get_list(PF::Layer* layer);

    void expand_layer( PF::Layer* layer, std::list<PF::Layer*>& list );

    void get_input_layers( Layer* layer, std::list<Layer*>& inputs );

    void get_flattened_layers_tree( std::list<Layer*>& inputs );

    void get_child_layers( Layer* layer, std::list<Layer*>& children );

    void get_parent_layers(Layer* layer, std::list< std::pair<std::string,Layer*> >& plist);

    Layer* get_container_layer( Layer* layer );
    Layer* get_container_layer( int id );

    Layer* get_layer(int id);

    //VipsImage* get_output() { return output; }

    bool insert_layer( Layer* layer, int32_t lid=-1 );

    bool remove_layer( Layer* layer );

    PF::CacheBuffer* get_cache_buffer();
    void reset_cache_buffers(bool reinit );


    bool rebuild_prepare();
    bool rebuild(Pipeline* pipeline, colorspace_t cs, int width, int height, VipsRect* area );
    bool rebuild_finalize( bool ui_update=true );

    bool rebuild_all(Pipeline* pipeline, colorspace_t cs, int width, int height);

    void update_ui();

    sigc::signal<void> signal_modified;
    void modified() { signal_modified.emit(); }

    bool save( std::ostream& ostr );
  };


  bool insert_layer( std::list<Layer*>& layers, Layer* layer, int32_t lid );
};


#endif
