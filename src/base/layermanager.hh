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

#include "layer.hh"

namespace PF
{

  class LayerManager
  {
    std::vector<Layer*> layers_pool;
    std::list<Layer*> layers;

    VipsImage* output;

    // Walk through the given layer chain and set the "dirty" flag of all layers starting from "layer_id" to "true"
    void update_dirty( std::list<Layer*>& list, bool& dirty );

    void reset_dirty( std::list<Layer*>& list );

    VipsImage* rebuild_chain(colorspace_t cs, VipsBandFormat fmt, 
			     int width, int height, 
			     std::list<Layer*>& list, VipsImage* previous);
  public:
    LayerManager();
    ~LayerManager();

    Layer* new_layer();

    std::list<Layer*>& get_layers() { return layers; }

    Layer* get_layer(int id);

    VipsImage* get_output() { return output; }

    bool rebuild(colorspace_t cs, VipsBandFormat fmt, int width, int height);
    bool rebuild_all(colorspace_t cs, VipsBandFormat fmt, int width, int height);

  };

};


#endif
