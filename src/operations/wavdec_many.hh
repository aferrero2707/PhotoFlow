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

#ifndef WAVDEC_MANY_H
#define WAVDEC_MANY_H


#include "../base/processor.hh"


namespace PF 
{

enum wavdec_many_blur_type
{
  WAVDEC_MANY_BLUR_GAUSS,
  WAVDEC_MANY_BLUR_WAVELETS
};

  class WavDecManyPar: public OpParBase
  {
    PropertyBase blur_type;
    Property<int> prop_nscales;
    Property<float> prop_base_scale;
    //Property<float> prop_detail_scale;

    std::vector<ProcessorBase*> levels;

  public:
    WavDecManyPar();
    ~WavDecManyPar() { std::cout<<"~WavDecManyPar() called."<<std::endl; }

    std::vector<VipsImage*> build_many(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class WavDecManyProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_wavdec_many();
}

#endif 


