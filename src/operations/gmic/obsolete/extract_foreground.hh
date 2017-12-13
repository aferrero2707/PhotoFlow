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

#ifndef GMIC_EXTRACT_FOREGROUND_H
#define GMIC_EXTRACT_FOREGROUND_H


#include "../base/processor.hh"

#include "../vips/gmic/gmic/src/gmic.h"

#include "../operations/raster_image.hh"


namespace PF 
{

  enum extract_fg_preview_mode_t {
    EXTRACT_FG_PREVIEW_POINTS,
    EXTRACT_FG_PREVIEW_MASK
    //EXTRACT_FG_PREVIEW_BLEND
  };

  class GmicExtractForegroundPar: public OpParBase
  {
    Property< std::list< std::pair<int,int> > > fg_points;
    Property< std::list< std::pair<int,int> > > bg_points;

    PF::ProcessorBase* convert_format;
    PF::ProcessorBase* convert_format2;
    PF::ProcessorBase* blender;

    char* custom_gmic_commands;
    gmic* gmic_instance;

    bool do_update;
    std::string preview_cache_file_name;
    std::string render_cache_file_name;

    extract_fg_preview_mode_t preview_mode;

    RasterImage* raster_image;

    ProcessorBase* mask_proc;

    gmic* new_gmic();

  public:
    GmicExtractForegroundPar();
    ~GmicExtractForegroundPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }
    bool init_hidden() { return false; }

    std::string get_preview_cache_file_name() { return preview_cache_file_name; }
    std::string get_render_cache_file_name() { return render_cache_file_name; }

    Property< std::list< std::pair<int,int> > >& get_fg_points() { return fg_points; }
    Property< std::list< std::pair<int,int> > >& get_bg_points() { return bg_points; }

    extract_fg_preview_mode_t get_preview_mode() { return preview_mode; }
    void set_preview_mode( extract_fg_preview_mode_t mode ) { preview_mode = mode; }

    void refresh() { do_update = true; }

    int get_padding( int level );      

    bool import_settings( OpParBase* pin );

    void pre_build( rendermode_t mode );

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicExtractForegroundProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_gmic_extract_foreground();
}

#endif 


