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

#ifndef VIPS_IMAGE_READER_H
#define VIPS_IMAGE_READER_H

#include <string>

#include "../base/processor.hh"
#include "../base/imagepyramid.hh"

#include "../operations/convertformat.hh"
#include "../operations/blender.hh"
#include "../operations/raster_image.hh"

namespace PF 
{

  class ImageReaderPar: public OpParBase
  {
    Property<std::string> file_name;
    // output color profile
    PropertyBase in_profile_mode;
    PropertyBase in_profile_type;
    PropertyBase in_trc_type;
    Property<std::string> in_profile_name;
    PropertyBase out_profile_mode;
    PropertyBase out_profile_type;
    PropertyBase out_trc_type;
    Property<std::string> out_profile_name;

    VipsImage* image;
    PF::ProcessorBase* convert_format;
    PF::ProcessorBase* blender;

    std::string current_file;
    VipsBandFormat current_format;

    ProcessorBase* cs_transform;

    RasterImage* raster_image;

    ImagePyramid pyramid;

  public:
    ImageReaderPar();
    ~ImageReaderPar();

    std::string get_file_name() { return file_name.get_str(); }
    void set_file_name( const std::string& name ) { file_name.set_str( name ); }
    void set_file_name( const char* name ) { set_file_name( std::string( name ) ); }

    profile_mode_t get_in_profile_mode() { return (profile_mode_t)in_profile_mode.get_enum_value().first; }
    profile_mode_t get_out_profile_mode() { return (profile_mode_t)out_profile_mode.get_enum_value().first; }
    profile_type_t get_in_profile_type() { return (profile_type_t)in_profile_type.get_enum_value().first; }
    profile_type_t get_out_profile_type() { return (profile_type_t)out_profile_type.get_enum_value().first; }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ImageReader
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  ProcessorBase* new_image_reader();
}

#endif 


