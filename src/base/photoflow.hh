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

    static PhotoFlow* instance;
  public:
    PhotoFlow(): render_mode(PF_RENDER_PREVIEW) {}

    static PhotoFlow& Instance();

    rendermode_t get_render_mode() { return render_mode; }
    void set_render_mode(rendermode_t m) { render_mode = m; }

    Image* get_image() { return active_image; }
    void set_image(Image* i) { active_image = i; }

    void set_new_op_func( new_op_func_t f ) { new_op_func = f; }

    ProcessorBase* new_operation(std::string opname, Layer* current_layer)
    {
      if( new_op_func ) return new_op_func( opname, current_layer );
      else return NULL;
    }
  };

}


#endif 


