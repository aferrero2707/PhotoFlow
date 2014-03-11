/* The PF::View class represents a particular "realization" of a given layer structure,
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

#ifndef VIEW_H
#define VIEW_H

#include <glibmm/threads.h>


#include "operation.hh"
//#include "image.hh"
#include "photoflow.hh"


namespace PF
{


  struct ViewNode
  {
    VipsImage* image;
    int input_id;
  };


  class ViewSink;
  class Image;

  class View
  {
    bool modified;
    Image* image;
    std::vector<ViewNode*> nodes;
    VipsImage* output;
    std::vector<ViewSink*> sinks;
    VipsBandFormat format;
    int level;

    Glib::Threads::Mutex processing_mutex;

  public:
    View(): modified(false), image(NULL), output(NULL), format(VIPS_FORMAT_UCHAR), level(0) {}
    View( Image* img, VipsBandFormat fmt, int l ): image(img), output(NULL), format(fmt), level(l) {}

    ~View();

    bool is_modified() { return modified; }
    void set_modified( bool flag ) { modified = flag; }

    Image* get_image() { return image; }

    void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsBandFormat get_format() { return format; }

    void set_level( int l ) { level = l; }

    void set_node( VipsImage* img, unsigned int id, int input_id );

    ViewNode* get_node( int id ) 
    {
      if( (id<0) || (id>=nodes.size()) ) return NULL;
      return nodes[id];
    }

    VipsImage* get_output() { return output; }
    void set_output( VipsImage* img ) { output = img; }

    bool processing();

    Glib::Threads::Mutex& get_processing_mutex() { return processing_mutex; }

    void add_sink( ViewSink* sink ) { sinks.push_back( sink ); }

    void update();
  };


  class ViewSink
  {
    View* view;
    bool processing;
    
  public:
    ViewSink( View* v ): view(v), processing( false )
    {
      view->add_sink( this );
    }

    View* get_view() { return view; }

    bool is_processing() { return processing; }
    void set_processing( bool flag ) { processing = flag; }
    
    virtual void update() = 0;
  };

}


#endif /*VIPS_PARITHMETIC_H*/


