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

#ifndef PF_RAW_LOADER_H
#define PF_RAW_LOADER_H

#include <string>

#include "raw_image.hh"

#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/imagepyramid.hh"

#include "../operations/convertformat.hh"
#include "fast_demosaic.hh"

namespace PF
{

  class RawLoaderPar: public OpParBase
  {
    Property<std::string> file_name;
    RawImage* raw_image;

    std::string temp_file_name;
    int temp_fd;
    VipsImage* image;
    VipsImage* demo_image;
    PF::ProcessorBase* convert_format;
    PF::ProcessorBase* fast_demosaic;

    std::string current_file;
    VipsBandFormat current_format;

    ImagePyramid pyramid;

  public:
    RawLoaderPar();
    ~RawLoaderPar();

    std::string get_file_name() { return file_name.get_str(); }
    void set_file_name( const std::string& name ) { file_name.set_str( name ); }
    void set_file_name( const char* name ) { set_file_name( std::string( name ) ); }

    void set_format( VipsBandFormat fmt ) { 
      OpParBase::set_format(VIPS_FORMAT_UCHAR); 
    }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class RawLoader
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  ProcessorBase* new_raw_loader();
}

#endif 


