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
    std::vector<std::string> preview_cache_file_names;
    std::vector<std::string> render_cache_file_names;
    unsigned int cache_files_num;

  protected:

    std::vector<RasterImage*> raster_image_vec;

  public:
    UntiledOperationPar();
    ~UntiledOperationPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }
    bool init_hidden() { return false; }

    void set_cache_files_num( unsigned int n )
    {
      for( unsigned int i = cache_files_num; i < n; i++ ) {
        preview_cache_file_names.push_back( std::string() );
        render_cache_file_names.push_back( std::string() );
        raster_image_vec.push_back( NULL );
      }
      cache_files_num = n;
    }
    unsigned int get_cache_files_num() { return cache_files_num; }
    std::string get_preview_cache_file_name(unsigned int n)
    {
      if( (n>=0) && (n<preview_cache_file_names.size()) )
        return preview_cache_file_names[n];
      else
        return std::string();
    }
    std::string get_render_cache_file_name(unsigned int n)
    {
      if( (n>=0) && (n<render_cache_file_names.size()) )
        return render_cache_file_names[n];
      else
        return std::string();
    }

    void refresh() { do_update = true; }

    int get_padding( int level );      

    bool import_settings( OpParBase* pin );

    std::string get_cache_file_name( unsigned int n );
    void update_raster_images();
    RasterImage* get_raster_image( unsigned int n );
    void raster_image_detach( unsigned int n );
    void raster_image_attach( unsigned int n );
    void raster_images_attach()
    {
      for( unsigned int i = 0; i < get_cache_files_num(); i++ )
        raster_image_attach(i);
    }

    void pre_build( rendermode_t mode );

    std::string save_image( VipsImage* image, VipsBandFmt format );

    std::vector<VipsImage*> get_output( unsigned int& level );
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


