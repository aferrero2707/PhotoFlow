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

#ifndef PF_CLAHE_H
#define PF_CLAHE_H

#include <string>

#include "../base/property.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"

namespace PF 
{

  class CLAHEPar: public OpParBase
  {
    Property<int> width;
    Property<int> slope;

  public:
    CLAHEPar();

    bool has_opacity() { return false; }
    bool has_intensity() { return false; }

    /* Function to derive the output area from the input area
    */
    virtual void transform(const VipsRect* rin, VipsRect* rout, int /*id*/)
    {
      rout->left = rin->left;
      rout->top = rin->top;
      rout->width = rin->width;
      rout->height = rin->height;
    }

    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const VipsRect* rout, VipsRect* rin, int /*id*/)
    {
      rin->left = rout->left;
      rin->top = rout->top;
      rin->width = rout->width;
      rin->height = rout->height;
    }


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class CLAHEProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_clahe();
}

#endif 


