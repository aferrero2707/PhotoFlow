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
#include "options.hh"

//#include "image.hh"

#ifdef LOCALEDIR
#include <libintl.h>
#define _(String) gettext(String)
#else
#define _(String) (String)
#endif

#define PF_FILE_VERSION 4

namespace PF
{

  class ProcessorBase;
  class Layer;
  class Image;

  class PhotoFlow
  {
    typedef ProcessorBase* (*new_op_func_t)(std::string opname, Layer* current_layer);

    Image* active_image;

    new_op_func_t new_op_func;
    new_op_func_t new_op_func_nogui;

    std::string config_dir;
    std::string cache_dir;
    std::string base_dir;
    std::string data_dir;
    std::string locale_dir;

    int preview_pipeline_id;

    bool batch;
    bool single_win_mode;

    Options options;

    static PhotoFlow* instance;
  public:
    PhotoFlow();

    static PhotoFlow& Instance();

    Options& get_options() { return options; }

    Image* get_active_image() { return active_image; }
    void set_active_image(Image* i) { active_image = i; std::cout<<"Active image: "<<i<<std::endl; }

    void set_new_op_func( new_op_func_t f ) { new_op_func = f; }
    void set_new_op_func_nogui( new_op_func_t f ) { new_op_func_nogui = f; }

    void set_preview_pipeline_id( int id ) { preview_pipeline_id = id; }
    int get_preview_pipeline_id() { return preview_pipeline_id; }

    void set_batch( bool val ) { batch = val; }
    bool is_batch() { return batch; }

    bool is_single_win_mode() { return single_win_mode; }

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

    void set_data_dir(std::string dir) { data_dir = dir; }
    std::string get_data_dir() { return data_dir; }

    void set_locale_dir(std::string dir) { locale_dir = dir; }
    std::string get_locale_dir() { return locale_dir; }

    std::string get_cache_dir() { return cache_dir; }
    std::string get_config_dir() { return config_dir; }

    void close();

		void obj_unref( GObject* obj, char* msg=NULL );
		void obj_unref( VipsImage* obj, char* msg=NULL ) { obj_unref( (GObject*)obj, msg ); }
		void obj_unref( VipsRegion* obj, char* msg=NULL ) { obj_unref( (GObject*)obj, msg ); }
  };


  void pf_object_ref(GObject* object, const char* msg);
#define PF_REF( object, msg ) PF::pf_object_ref( G_OBJECT(object), msg );
  void pf_object_unref(GObject* object, const char* msg);
#define PF_UNREF( object, msg ) PF::pf_object_unref( G_OBJECT(object), msg );
#define PF_PRINT_REF( object, msg ) std::cout<<msg<<" ref_count: "<<G_OBJECT(object)<<"->"<<G_OBJECT(object)->ref_count<<std::endl;
}


#endif 


