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

#ifndef GMIC_DREAM_SMOOTH_H
#define GMIC_DREAM_SMOOTH_H


#include "../base/processor.hh"

#include "../vips/gmic/gmic/src/gmic.h"

#include "../operations/convertformat.hh"
#include "../operations/raster_image.hh"


namespace PF 
{

  class GmicDreamSmoothPar: public OpParBase
  {
    Property<int> iterations;
    Property<int> prop_interations;
    Property<int> prop_equalize;
    PropertyBase prop_merging_option;
    Property<float> prop_opacity;
    Property<int> prop_reverse;
    Property<float> prop_smoothness;
    Property<int> padding;
    //ProcessorBase* gmic;

    PF::ProcessorBase* convert_format;
    PF::ProcessorBase* convert_format2;
    char* custom_gmic_commands;
    gmic* gmic_instance;

    bool do_update;
    std::string preview_cache_file_name;
    std::string render_cache_file_name;

    RasterImage* raster_image;

  public:
    GmicDreamSmoothPar();
    ~GmicDreamSmoothPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }
    bool init_hidden() { return true; }

    std::string get_preview_cache_file_name() { return preview_cache_file_name; }
    std::string get_render_cache_file_name() { return render_cache_file_name; }

    void refresh() { do_update = true; }

    int get_padding( int level );      

    bool import_settings( OpParBase* pin );

    void pre_build( rendermode_t mode );

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicDreamSmoothProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_gmic_dream_smooth();
}

#endif 


