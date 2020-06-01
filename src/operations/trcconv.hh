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

#ifndef PF_TRC_CONV_H
#define PF_TRC_CONV_H

#include <iostream>

#include "../base/format_info.hh"

namespace PF 
{

  class TRCConvPar: public OpParBase
  {
    Property<bool> perceptual;
    PF::ICCProfile* profile;
    bool linear_trc;
  public:
    TRCConvPar();

    bool has_intensity() { return false; }
    bool has_target_channel() { return false; }

    void set_to_perceptual(bool flag) { perceptual.update(flag); }
    bool to_perceptual() { return perceptual.get(); }
    PF::ICCProfile* get_profile() { return profile; }
    bool is_linear_trc() { return linear_trc; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap, unsigned int& level);
  };


  ProcessorBase* new_trcconv();
}

#endif 


