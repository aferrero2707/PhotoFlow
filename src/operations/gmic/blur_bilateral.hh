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

#ifndef GMIC_BLUR_BILATERAL_H
#define GMIC_BLUR_BILATERAL_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"


namespace PF 
{

  class GmicBlurBilateralPar: public OpParBase
  {
    Property<int> iterations;
    Property<float> sigma_s;
    Property<float> sigma_r; 
    Property<int> bgrid_s;
    Property<int> bgrid_r; 

    ProcessorBase* gmic;

  public:
    GmicBlurBilateralPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return true; }

    int get_padding( int level );

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicBlurBilateralProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
			
    }
  };




  ProcessorBase* new_gmic_blur_bilateral();
}

#endif 


