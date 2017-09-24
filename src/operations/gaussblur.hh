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

#ifndef GAUSS_BLUR_H
#define GAUSS_BLUR_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"

#include "gaussblur_sii.hh"


namespace PF 
{

	enum gaussblur_preview_mode_t {
		PF_BLUR_FAST,
		PF_BLUR_EXACT
	};


  class GaussBlurPar: public OpParBase
  {
    Property<float> radius;
		PropertyBase blur_mode;
    ProcessorBase* convert_format;
    ProcessorBase* blur_sii;

  public:
    GaussBlurPar();

    void set_radius( float r ) { radius.set( r ); }
    float get_radius() { return radius.get(); }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

    bool has_intensity() { return false; }
    bool needs_caching() { return( radius.get() >= 50 ); }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GaussBlurProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
			
    }
  };


  ProcessorBase* new_gaussblur();
}

#endif 


