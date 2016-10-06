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

#ifndef GMIC_SHARPEN_TEXTURE_H
#define GMIC_SHARPEN_TEXTURE_H


#include "../base/processor.hh"


namespace PF 
{

  class GmicSharpenTexturePar: public OpParBase
  {
    Property<float> prop_radius;
    Property<float> prop_strength;
    ProcessorBase* gmic;

    float padding;

  public:
    GmicSharpenTexturePar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching();

    void set_radius( float s ) { prop_radius.set( s ); }
    void set_strength( float s ) { prop_strength.set( s ); }


    int get_padding( int level );      


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicSharpenTextureProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_gmic_sharpen_texture();
}

#endif 


