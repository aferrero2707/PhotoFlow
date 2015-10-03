/* 
    File raster_image.hh: implementation of the RasterImage class.

    The RasterImage objects
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

#ifndef PF_RASTER_IMAGE_H
#define PF_RASTER_IMAGE_H

#include <string>

#include <vips/vips.h>

#include <glibmm.h>

#include <gexiv2/gexiv2-metadata.h>

#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/imagepyramid.hh"
#include "../base/photoflow.hh"
#include "../base/exif_data.hh"


namespace PF 
{

  class RasterImage
  {
    int nref;
		std::string file_name;
    std::string file_name_real;

		// VipsImage storing the raster data 
    VipsImage* image;

    exif_data_t exif_data;

    GExiv2Orientation orientation;

    ImagePyramid pyramid;

  public:
    RasterImage( const std::string name );
    ~RasterImage();

    void ref() { nref += 1; }
    void unref() { nref -= 1; }
    int get_nref() { return nref; }

    std::string get_file_name() { return file_name_real; }
    VipsImage* get_image(unsigned int& level);

    GExiv2Orientation get_orientation() { return orientation; }

    void print_exif( PF::exif_data_t* data );
    void print_exif();
    void print_icc();
    void print_icc( VipsImage* img );
  };

	extern std::map<Glib::ustring, PF::RasterImage*> raster_images;

}

#endif
