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

#ifndef GMIC_UNTILED_OP__H
#define GMIC_UNTILED_OP__H


#include "../../base/processor.hh"

#include "../../vips/gmic/gmic/src/gmic.h"

#include "../convertformat.hh"
#include "../raster_image.hh"
#include "../untiled_op.hh"


namespace PF 
{

  class GmicUntiledOperationPar: public UntiledOperationPar
  {
    char* custom_gmic_commands;
    gmic* gmic_instance;

  protected:

    gmic* new_gmic();

  public:
    GmicUntiledOperationPar();
    ~GmicUntiledOperationPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }
    bool init_hidden() { return false; }

    virtual void set_metadata( VipsImage* in, VipsImage* out )
    {
      UntiledOperationPar::set_metadata( in, out );
      vips_image_init_fields( out,
          in->Xsize, in->Ysize, in->Bands,
          IM_BANDFMT_FLOAT,
          in->Coding, in->Type, in->Xres, in->Yres
          );
    }

    bool run_gmic( VipsImage* in, std::string command );
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicUntiledOperationProc
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


