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

#ifndef PF_LAYER_H
#define PF_LAYER_H

#include <stdlib.h>

#include <list>
#include <vector>
#include <set>

#include <sigc++/sigc++.h>

#include "pftypes.hh"
#include "processor.hh"

namespace PF
{

  class Image;

  class Layer
  {
    friend class LayerManager;

    int32_t id;
    std::string name;
    std::list<Layer*> sublayers;
    std::list<Layer*> imap_layers;
    std::list<Layer*> omap_layers;
    std::vector<int32_t> extra_inputs;

    // Flag indicating that the layer hs been directly or indirectly
    // modified, and therefore that re-building is needed
    bool dirty;

    bool visible;

    bool normal;

    ProcessorBase* processor;

    Image* image;

    bool insert(std::list<PF::Layer*>& list, Layer* l, int32_t lid);
    bool insert_before(std::list<PF::Layer*>& list, Layer* l, int32_t lid);

  public:
    Layer(int32_t id);
    virtual ~Layer()
    {
      std::cout<<"~Layer(): \""<<name<<"\" destructor called."<<std::endl;
      if( processor ) delete (processor );
    }

    std::string get_name() { return name; }
    void set_name( std::string n ) { name = n; modified(); }

    int32_t get_id() { return id; }

    bool is_dirty() { return dirty; }
    void set_dirty( bool d ) { dirty = d; }
    void clear_dirty( ) { dirty = false; }
    

    bool is_visible() { return visible; }
    void set_visible( bool d ) { visible = d; }
    void clear_visible( ) { visible = false; }
    
    bool is_group() { return( !normal ); }
    bool is_normal() { return normal; }
    void set_normal( bool d ) { normal = d; }
    

    ProcessorBase* get_processor() { return processor; }
    void set_processor(ProcessorBase* p) { processor = p; }

    Image* get_image() { return image; }
    void set_image( Image* img ) { image = img; }

    void set_input(int n, int32_t lid) { 
      for( int i = extra_inputs.size(); i <= n; i++ ) extra_inputs.push_back( -1 );
      extra_inputs[n] = lid; 
    }
    void add_input(int32_t lid) { extra_inputs.push_back(lid); }
    void remove_input(int32_t lid);
    std::vector<int32_t>& get_extra_inputs() { return extra_inputs; }

    std::list<Layer*>& get_sublayers() { return sublayers; }
    std::list<Layer*>& get_imap_layers() { return imap_layers; }
    std::list<Layer*>& get_omap_layers() { return omap_layers; }

    bool sublayers_insert(Layer* l, int32_t lid=-1);
    bool sublayers_insert_before(Layer* l, int32_t lid);

    bool imap_insert(Layer* l, int32_t lid=-1);
    bool imap_insert_before(Layer* l, int32_t lid);

    bool omap_insert(Layer* l, int32_t lid=-1);
    bool omap_insert_before(Layer* l, int32_t lid);

    sigc::signal<void> signal_modified;
    void modified() { signal_modified.emit(); }

    bool save( std::ostream& ostr, int level );
  };

};


#endif
