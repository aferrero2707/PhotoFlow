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

#ifndef PF_LOCAL_CONTRAST_V2_H
#define PF_LOCAL_CONTRAST_V2_H

#include "../rt/rtengine/sleef.c"
#include "../base/processor.hh"

namespace PF 
{

  class LocalContrastV2Par: public OpParBase
  {
    Property<float> amount;
    Property<float> radius, threshold, white_level;

    ProcessorBase* loglumi;
    ProcessorBase* guided[10];

    PF::ICCProfile* in_profile;
    float threshold_scale[10];
  public:
    LocalContrastV2Par();

    bool has_intensity() { return false; }
    bool needs_caching() {
      return true;
    }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    PF::ICCProfile* get_profile() { return in_profile; }
    float get_amount() { return amount.get(); }
    float get_white_level() { return white_level.get(); }
      
    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };


  ProcessorBase* new_local_contrast_v2();

}

#endif 


