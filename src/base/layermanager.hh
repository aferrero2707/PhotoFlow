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

#include "layer.hh"
#include "view.hh"

namespace PF
{

  class Image;

  class LayerManager
  {
    std::vector<Layer*> layers_pool;
    std::list<Layer*> layers;

    Image* image;

    //VipsImage* output;

    // Walk through the given layer chain and set the "dirty" flag of all layers starting from "layer_id" to "true"
    void update_dirty( std::list<Layer*>& list, bool& dirty );

    void reset_dirty( std::list<Layer*>& list );

    VipsImage* rebuild_chain(View& view, colorspace_t cs, 
			     int width, int height, 
			     std::list<Layer*>& list, VipsImage* previous);
  public:
    LayerManager(Image* image);
    ~LayerManager();

    Layer* new_layer();

    Image* get_image() { return image; }
    void set_image( Image* img ) { image = img; }

    std::list<Layer*>& get_layers() { return layers; }

    Layer* get_layer(int id);

    //VipsImage* get_output() { return output; }

    bool rebuild(View& view, colorspace_t cs, int width, int height);
    bool rebuild_all(View& view, colorspace_t cs, int width, int height);

    sigc::signal<void> signal_modified;
    void modified() { signal_modified.emit(); }
  };

};


#endif
