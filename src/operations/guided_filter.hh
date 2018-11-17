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

#ifndef PF_GUIDED_FILTER_H
#define PF_GUIDED_FILTER_H

#include "../base/operation.hh"

#include "padded_op.hh"

namespace PF 
{

class GuidedFilterPar: public PaddedOpPar
{
  Property<float> radius;
  Property<float> threshold;
public:
  GuidedFilterPar();

  bool has_intensity() { return false; }
  bool needs_caching();

  float get_radius() { return radius.get(); }
  float get_threshold() { return threshold.get(); }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class GuidedFilterProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class GuidedFilterProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 1 ) return;
    if( ireg[0] == NULL ) return;

    GuidedFilterPar* opar = dynamic_cast<GuidedFilterPar*>(par);
    if( !opar ) return;

    //const int order = 1; // 0,1,2
    const float radius = opar->get_radius();
    const float thresfold = opar->get_threshold();
  }
};


ProcessorBase* new_guided_filter();

}

#endif 


