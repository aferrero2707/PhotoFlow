/* 
    File cache_buffer.hh: implementation of the CacheBuffer class.

    The CacheBuffer objects are used to keep a pre-computed copy of the output of the associated operation.
    The computation is performed in the background while the program is idle. The CacheBuffer uses an image pyramid
    for fast zooming.
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


#ifndef CACHE_BUFFER_H
#define CACHE_BUFFER_H

#include <math.h>

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>


#include <vips/vips.h>
//#include <vips/vips>

#include "pftypes.hh"

#include "format_info.hh"

#include "property.hh"

#include "imagepyramid.hh"



#define PF_CACHE_BUFFER_TILE_SIZE 128


namespace PF 
{

  class CacheBuffer
  {
    // Image to be cached
    VipsImage* image;
    // Image associate to the disk buffer
    VipsImage* cached;

    ImagePyramid pyramid;

    std::string filename;
    int fd;

    //Flag indicating if the processing is completed
    bool completed;

    // Coordinates of the tile being processed
    int step_x, step_y;

  public:
    CacheBuffer();

    virtual ~CacheBuffer()
    {
    }

    VipsImage* get_image() { return image; }
    void set_image( VipsImage* img ) { image = img; }

    ImagePyramid& get_pyramid() { return pyramid; }

    void reset();
    bool is_completed() { return completed; }

    void step();
  };


};

#endif
