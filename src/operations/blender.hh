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

#ifndef VIPS_BLENDER_H
#define VIPS_BLENDER_H

#include <unistd.h>
#include <string>

#include "../base/operation.hh"
#include "../base/blender.hh"

namespace PF 
{

  class BlenderPar: public OpParBase
  {
    Property<blendmode_t> blend_mode;
    Property<float> opacity;
    Property<int> shift_x;
    Property<int> shift_y;

    ProcessorBase* white;

    cmsHPROFILE profile_bottom, profile_top;
    cmsHTRANSFORM transform;
    ICCProfile* icc_data_bottom;
    ICCProfile* icc_data_top;

    bool adjust_geom( VipsImage* in, VipsImage** out,
                      int width, int height, unsigned int level );

  public:
    BlenderPar();

    ICCProfile* get_icc_data_top() { return icc_data_top; }
    ICCProfile* get_icc_data_bottom() { return icc_data_bottom; }
    ICCProfile* get_icc_data() { return icc_data_bottom; }
    cmsHTRANSFORM get_transform() { return transform; }

    blendmode_t get_blend_mode() { 
      return( (blendmode_t)blend_mode.get_enum_value().first ); 
    }
    void set_blend_mode(blendmode_t mode) { 
      blend_mode.set_enum_value( (int)mode ); 
    }

    void set_opacity(float val) { opacity.set(val); }
    float get_opacity() { return opacity.get(); }

    /* Set processing hints:
       1. the intensity parameter makes no sense for a blending operation, 
       creation of an intensity map is not allowed
       2. the operation can work without an input image;
       the blending will be set in this case to "passthrough" and the image
       data will be simply linked to the output
    */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  
  template < OP_TEMPLATE_DEF > 
  class BlenderProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int ,
                VipsRegion* , VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      if( (n != 2) || (ireg[0] == NULL) || (ireg[1] == NULL) ) {
        std::cerr<<"BlenderProc::render(): wrong number of input images"<<std::endl;
        return;
      }

      BlenderPar* bpar = dynamic_cast<BlenderPar*>(par);
      if( !bpar ) return;
      float opacity = bpar->get_opacity();
      Blender<T,CS,CHMIN,CHMAX,has_omap> blender( bpar->get_blend_mode(), opacity );
      //std::cout<<"BlenderProc::render(): bpar->get_icc_data()="<<bpar->get_icc_data()<<std::endl;
      blender.set_transform( bpar->get_transform() );
      blender.set_icc_data( bpar->get_icc_data() );
#ifndef NDEBUG
      //usleep(1000);
      std::cout<<"BlenderProc::render(): opacity="<<opacity<<std::endl;
      std::cout<<"BlenderProc::render(): CS="<<CS<<"  CHMIN="<<CHMIN<<"  CHMAX="<<CHMAX<<std::endl;
#endif
      blender.blend( ireg[0], ireg[1], oreg, omap );
    }
  };


  ProcessorBase* new_blender();
}


#endif
