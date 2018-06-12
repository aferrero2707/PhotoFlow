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

#ifndef GMIC_SMOOTH_WAVELETS_HAAR_H
#define GMIC_SMOOTH_WAVELETS_HAAR_H


#include "../base/processor.hh"


namespace PF 
{

  class GmicSmoothWaveletsHaarPar: public OpParBase
  {
    //Property<int> iterations;
    Property<float> prop_threshold;
    Property<int> prop_iterations;
    Property<int> prop_scales;
    ProcessorBase* gmic;

  public:
    GmicSmoothWaveletsHaarPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }

      
    int get_gmic_padding(int level);

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
    {
      set_padding( get_gmic_padding(level), id);
    }

    /* Function to derive the output area from the input area
     */
    virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
    {
      int pad = get_padding();
      rout->left = rin->left+pad;
      rout->top = rin->top+pad;
      rout->width = rin->width-pad*2;
      rout->height = rin->height-pad*2;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
    {
      int pad = get_padding();
      rin->left = rout->left-pad;
      rin->top = rout->top-pad;
      rin->width = rout->width+pad*2;
      rin->height = rout->height+pad*2;
    }
      


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicSmoothWaveletsHaarProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_gmic_smooth_wavelets_haar();
}

#endif 


