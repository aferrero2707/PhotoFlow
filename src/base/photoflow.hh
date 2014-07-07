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

#ifndef VIPS_PHOTOFLOW_H
#define VIPS_PHOTOFLOW_H

#include <unistd.h>

#include "pftypes.hh"
//#include "processor.hh"

//#include "image.hh"


namespace PF
{

  class ProcessorBase;
  class Layer;
  class Image;

  class PhotoFlow
  {
    typedef ProcessorBase* (*new_op_func_t)(std::string opname, Layer* current_layer);

    rendermode_t render_mode;

    Image* active_image;

    new_op_func_t new_op_func;
    new_op_func_t new_op_func_nogui;

    std::string cache_dir;
    std::string base_dir;

    bool batch;

    static PhotoFlow* instance;
  public:
    PhotoFlow();

    static PhotoFlow& Instance();

    rendermode_t get_render_mode() { return render_mode; }
    void set_render_mode(rendermode_t m) { render_mode = m; }

    Image* get_image() { return active_image; }
    void set_image(Image* i) { active_image = i; }

    void set_new_op_func( new_op_func_t f ) { new_op_func = f; }
    void set_new_op_func_nogui( new_op_func_t f ) { new_op_func_nogui = f; }

    void set_batch( bool val ) { batch = val; }
    bool is_batch() { return batch; }

    ProcessorBase* new_operation(std::string opname, Layer* current_layer)
    {
      if( new_op_func ) return new_op_func( opname, current_layer );
      else return NULL;
    }
    ProcessorBase* new_operation_nogui(std::string opname, Layer* current_layer)
    {
      if( new_op_func_nogui ) return new_op_func_nogui( opname, current_layer );
      else return NULL;
    }

    void set_base_dir(std::string dir) { base_dir = dir; }
    std::string get_base_dir() { return base_dir; }

    std::string get_cache_dir() { return cache_dir; }
  };


  void pf_object_ref(GObject* object, const char* msg);
#define PF_REF( object, msg ) pf_object_ref( G_OBJECT(object), msg );
  void pf_object_unref(GObject* object, const char* msg);
#define PF_UNREF( object, msg ) pf_object_unref( G_OBJECT(object), msg );
#define PF_PRINT_REF( object, msg ) std::cout<<msg<<" ref_count: "<<G_OBJECT(object)<<"->"<<G_OBJECT(object)->ref_count<<std::endl;
}


#endif 


