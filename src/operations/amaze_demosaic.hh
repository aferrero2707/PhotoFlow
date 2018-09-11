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

#ifndef AMAZE_DEMOSAIC_H
#define AMAZE_DEMOSAIC_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../rt/rtengine/rawimagesource.hh"
#include "demosaic_common.hh"



namespace PF
{

  class AmazeDemosaicPar: public DemosaicBasePar
  {
  public:
    AmazeDemosaicPar();
  };

  


  void amaze_demosaic_PF(VipsRegion* in, VipsRegion* out, AmazeDemosaicPar* par);


  template < OP_TEMPLATE_DEF > 
  class AmazeDemosaicProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, AmazeDemosaicPar* par) 
    {
      //fast_demosaic( in, n, in_first,
      //	     imap, omap, out, par );
    }
  };


  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class AmazeDemosaicProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) > 
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, AmazeDemosaicPar* par) 
    {
      amaze_demosaic_PF(in[0], out, par);
    }
  };

  ProcessorBase* new_amaze_demosaic();
}


#endif
