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

#ifndef PF_MAXRGB_H
#define PF_MAXRGB_H

#include <iostream>

#include "../base/format_info.hh"

namespace PF 
{

  class MaxRGBPar: public OpParBase
  {
    bool do_max;
  public:
    MaxRGBPar();

    bool get_do_max() { return do_max; }
    void set_do_max(bool b) { do_max = b; }

    bool has_intensity() { return false; }
    bool has_target_channel() { return true; }
  };


  ProcessorBase* new_maxrgb();
}

#endif 


