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

#ifndef PF_UNSHARP_MASK_H
#define PF_UNSHARP_MASK_H

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"
#include "gaussblur.hh"

#define CLIP_T(T,VAL) (T)( MIN(MAX(VAL,FormatInfo<T>::MIN),FormatInfo<T>::MAX) )

namespace PF 
{

  class UnsharpMaskPar: public OpParBase
  {
    Property<float> radius, amount;
		ProcessorBase* blur;
  public:
    UnsharpMaskPar();
    float get_radius() { return radius.get(); }
    float get_amount() { return amount.get(); }

    bool needs_caching() { return true; }
      

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  



  template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UnsharpMaskProc
  {
    UnsharpMaskPar* par;
		float amount;
  public:
    UnsharpMaskProc(UnsharpMaskPar* p): par(p) { 
			amount = 1;
			if(par) amount = 0.01f*par->get_amount();
		}
    
    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout) 
    {
      for( int ch = CHMIN; ch <= CHMAX; ch++ ) {
				typename FormatInfo<T>::SIGNED diff = ((typename FormatInfo<T>::SIGNED)p[first][x+ch]) - ((typename FormatInfo<T>::SIGNED)p[first+1][x+ch]);
				double val = amount*intensity*diff + p[first][x+ch];
				if( val < FormatInfo<T>::MIN ) val = FormatInfo<T>::MIN;
				else if( val > FormatInfo<T>::MAX ) val = FormatInfo<T>::MAX;
				pout[x+ch] = (T)(val);
			}
    }
  };


  template < OP_TEMPLATE_DEF > 
  class UnsharpMask: public OperationPTP< OP_TEMPLATE_IMP, UnsharpMaskPar, UnsharpMaskProc >
  {
  };


  ProcessorBase* new_unsharp_mask();

}

#endif 


