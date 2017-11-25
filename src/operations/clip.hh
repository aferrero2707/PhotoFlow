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

#ifndef PF_CLIP_HH
#define PF_CLIP_HH

#include <iostream>

#include "../base/format_info.hh"
#include "../base/pixel_processor.hh"

namespace PF 
{

  class ClipPar: public PixelProcessorPar
  {
    Property<bool> clip_negative, clip_overflow;

  public:
    ClipPar();

    bool get_clip_negative() { return clip_negative.get(); }
    bool get_clip_overflow() { return clip_overflow.get(); }

    bool has_intensity() { return false; }
    bool has_target_channel() { return false; }
  };

  

  template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class ClipProc
  {
    ClipPar* par;
    int pos;
  public:
    ClipProc(ClipPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout)
    {
      T* pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        pout[pos] = pp[pos];
      }
    }
  };

  
  template < colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class ClipProc<float, CS, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    ClipPar* par;
    int pos;
  public:
    ClipProc(ClipPar* p): par(p) {}

    void process(float**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, float* pout)
    {
      float* pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        pout[pos] = pp[pos];
        if( par->get_clip_negative() ) pout[pos] = MAX( pout[pos], 0.f );
        if( par->get_clip_overflow() ) pout[pos] = MIN( pout[pos], 1.f );
     }
    }
  };

  
  template < OP_TEMPLATE_DEF > 
  class ClipOp: public PixelProcessor< OP_TEMPLATE_IMP, ClipPar, ClipProc >
  {
  };


  ProcessorBase* new_clip();
}

#endif 


