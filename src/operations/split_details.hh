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

#ifndef SPLIT_DETAILS_H
#define SPLIT_DETAILS_H


#include "../base/processor.hh"


namespace PF 
{

enum split_details_blur_type
{
  SPLIT_DETAILS_BLUR_GAUSS,
  SPLIT_DETAILS_BLUR_WAVELETS
};

  class SplitDetailsPar: public OpParBase
  {
    PropertyBase blur_type;
    Property<int> prop_nscales;
    Property<float> prop_base_scale;

    Property<bool> output_residual_image;

    std::vector<ProcessorBase*> levels;

  public:
    SplitDetailsPar();
    ~SplitDetailsPar() { std::cout<<"~SplitDetailsPar() called."<<std::endl; }

    std::vector<VipsImage*> build_many(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level);
    
    int get_blur_type() { return blur_type.get_enum_value().first; }
  };

  

  template < OP_TEMPLATE_DEF > 
  class SplitDetailsProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };




  ProcessorBase* new_split_details();
}

#endif 


