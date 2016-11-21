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

#ifndef DENOISE_H
#define DENOISE_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"


namespace PF 
{

	enum denoise_mode_t {
		PF_NR_ANIBLUR
	};


  class DenoisePar: public OpParBase
  {
    Property<bool> impulse_nr_enable;
    Property<float> impulse_nr_threshold;
    Property<bool> nlmeans_enable;
    Property<float> nlmeans_radius;
    Property<float> nlmeans_strength;
    Property<float> nlmeans_luma_frac;
    Property<float> nlmeans_chroma_frac;
    Property<int> iterations;
    Property<float> amplitude;
    Property<float> sharpness; 
    Property<float> anisotropy;
    Property<float> alpha; 
    Property<float> sigma;
		PropertyBase nr_mode;

	  ProcessorBase* convert2lab;
	  ProcessorBase* convert2input;
    ProcessorBase* impulse_nr;
    ProcessorBase* nlmeans;

    cmsHPROFILE in_profile;

  public:
    DenoisePar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool has_target_channel() { return true; }
    bool needs_caching() { return( nlmeans_enable.get() ); }

      

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class DenoiseProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
			
    }
  };




  ProcessorBase* new_denoise();
}

#endif 


