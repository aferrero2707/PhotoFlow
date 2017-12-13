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

#ifndef IMAGE_PYRAMID_H
#define IMAGE_PYRAMID_H


#include <vector>
#include <string>
#include <vips/vips.h>


namespace PF
{

  extern VipsImage* pyramid_test_image;
  extern GObject* pyramid_test_obj;

  struct PyramidLevel
  {
    std::string raw_file_name;
    int fd;
    VipsImage* image;
  };


  class ImagePyramid
  {
    std::vector<PyramidLevel> levels;

  public:
    ImagePyramid() {}

    ~ImagePyramid();

    void init( VipsImage* image, int fd = -1 );

    void reset();

    void update( const VipsRect& area );

    PyramidLevel* get_level( unsigned int& level );
  };

}


#endif /*VIPS_PARITHMETIC_H*/


