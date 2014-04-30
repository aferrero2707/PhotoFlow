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
    ProcessorBase* processor;
    VipsImage* image;
    int input_id;

    ViewNode(): processor( NULL ), image( NULL ), input_id( -1 ) {}
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
    unsigned int level;

    //Glib::Threads::Mutex processing_mutex;

  public:
    View(): modified(false), image(NULL), output(NULL), format(VIPS_FORMAT_UCHAR), level(0) {}
    View( Image* img, VipsBandFormat fmt, int l ): image(img), output(NULL), format(fmt), level(l) {}

    ~View();

    bool is_modified() { return modified; }
    void set_modified( bool flag ) { modified = flag; }

    Image* get_image() { return image; }

    void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsBandFormat get_format() { return format; }

    void set_level( unsigned int l ) { level = l; }
    unsigned int get_level() { return level; }

    ViewNode* set_node( Layer* layer, Layer* input_layer );
    void set_image( VipsImage* img, unsigned int id );
    void remove_node( unsigned int id );

    ViewNode* get_node( int id ) 
    {
      if( (id<0) || (id>=(int)nodes.size()) ) return NULL;
      return nodes[id];
    }

    VipsImage* get_output() { return output; }
    void set_output( VipsImage* img ) { output = img; }

    bool processing();

    //Glib::Threads::Mutex& get_processing_mutex() { return processing_mutex; }

    void add_sink( ViewSink* sink ) { sinks.push_back( sink ); }

    void lock_processing();
    void unlock_processing();

    void update();
    void update( const VipsRect& area );
  };


  class ViewSink
  {
    View* view;
    bool processing;
    
    Glib::Threads::Mutex processing_mutex;
    int processing_count;

  public:
    ViewSink( View* v ): view(v), processing( false ), processing_count( 0 )
    {
      view->add_sink( this );
    }

    View* get_view() { return view; }

    bool is_processing() { return processing; }
    void set_processing( bool flag ) { processing = flag; }
    
    void processing_count_increase() { processing_count += 1; }
    void processing_count_decrease() { processing_count -= 1; }
    int get_processing_count() { return processing_count; }
    Glib::Threads::Mutex& get_processing_mutex() { return processing_mutex; }

    virtual void update() = 0;
    virtual void update( const VipsRect& area ) { update(); }
  };

}


#endif /*VIPS_PARITHMETIC_H*/


