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

#ifndef PF_UNTILED_OP__H
#define PF_UNTILED_OP__H


#include "../base/processor.hh"

#include "convertformat.hh"
#include "raster_image.hh"


namespace PF 
{

  class UntiledOperationPar: public OpParBase
  {
    PF::ProcessorBase* convert_format_in;
    PF::ProcessorBase* convert_format_out;

    bool do_update;
    std::string preview_cache_file_name;
    std::string render_cache_file_name;

  protected:

    RasterImage* raster_image;

  public:
    UntiledOperationPar();
    ~UntiledOperationPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }
    bool init_hidden() { return false; }

    std::string get_preview_cache_file_name() { return preview_cache_file_name; }
    std::string get_render_cache_file_name() { return render_cache_file_name; }

    void refresh() { do_update = true; }

    int get_padding( int level );      

    bool import_settings( OpParBase* pin );

    std::string get_cache_file_name();
    void update_raster_image();
    RasterImage* get_raster_image();
    void raster_image_detach();
    void raster_image_attach();

    void pre_build( rendermode_t mode );

    std::string save_image( VipsImage* image, VipsBandFmt format );

    VipsImage* get_output( unsigned int& level );
  };

  

  template < OP_TEMPLATE_DEF > 
  class UntiledOperationProc
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


