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

#ifndef CA_CORRECT_H
#define CA_CORRECT_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../rt/rtengine/rawimagesource.hh"
#include "demosaic_common.hh"



namespace PF
{

  class CACorrectPar: public DemosaicBasePar
  {
    Property<bool> enable_ca;
    Property<bool> auto_ca;

    // Manual CA correction parameters
    Property<float> ca_red, ca_blue;
  public:
    CACorrectPar();

    void set_enable_ca(bool flag) { enable_ca.update(flag); }
    bool get_auto_ca() { return auto_ca.get(); }
    void set_auto_ca(bool flag) { auto_ca.update(flag); }
    float get_ca_red() { return ca_red.get(); }
    float get_ca_blue() { return ca_blue.get(); }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  


  void ca_correct_PF(VipsRegion* ir, VipsRegion* out, PF::CACorrectPar* par);


  template < OP_TEMPLATE_DEF > 
  class CACorrectProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, CACorrectPar* par)
    {
      //fast_demosaic( in, n, in_first,
      //	     imap, omap, out, par );
    }
  };


  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class CACorrectProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, CACorrectPar* par)
    {
      ca_correct_PF(in[0], out, par);
    }
  };


  ProcessorBase* new_ca_correct();
}


#endif
