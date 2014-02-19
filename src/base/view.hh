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


#include "operation.hh"
#include "photoflow.hh"


namespace PF
{


  class View
  {
    std::vector<VipsImage*> vips_images;
    VipsImage* output;
    VipsBandFormat format;
    int level;

  public:
    View(): output(NULL), format(VIPS_FORMAT_UCHAR), level(0) {}
    View( VipsBandFormat fmt, int l ): output(NULL), format(fmt), level(l) {}

    void set_format( VipsBandFormat fmt ) { format = fmt; }
    VipsBandFormat get_format() { return format; }

    void set_level( int l ) { level = l; }

    void set_image( VipsImage* img, unsigned int id )
    {
      while( vips_images.size() <= (id+1) ) vips_images.push_back(NULL);
      vips_images[id] = img;
    }

    VipsImage* get_image(unsigned int id) 
    {
      if( id >= vips_images.size() ) return NULL;
      return vips_images[id];
    }

    VipsImage* get_output() { return output; }
    void set_output( VipsImage* img ) { output = img; }
  };

}


#endif /*VIPS_PARITHMETIC_H*/


