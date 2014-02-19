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

#include "../operations/blender.hh"

namespace PF 
{

  class ImageReaderPar: public OpParBase
  {
    Property<std::string> file_name;
    VipsImage* image;
    PF::Processor<PF::BlenderPar,PF::BlenderProc>* blender;

  public:
    ImageReaderPar(): 
      OpParBase(), 
      file_name("file_name", this),
      image(NULL) 
    {
      set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
      blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
      set_type("imageread" );
    }

    std::string get_file_name() { return file_name.get_str(); }
    void set_file_name( const std::string& name ) { file_name.set_str( name ); }
    void set_file_name( const char* name ) { set_file_name( std::string( name ) ); }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, VipsImage* imap, VipsImage* omap);
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

}

#endif 


