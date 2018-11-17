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

#ifndef PF_PADDED_OP_H
#define PF_PADDED_OP_H

namespace PF 
{

class PaddedOpPar: public OpParBase
{
  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
  {
    int pad = get_padding(0);
    rout->left = rin->left+pad;
    rout->top = rin->top+pad;
    rout->width = rin->width-pad*2;
    rout->height = rin->height-pad*2;
  }

  /* Function to derive the area to be read from input images,
     based on the requested output area
  */
  virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
  {
    int pad = get_padding(0);
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
  }


public:
  PaddedOpPar();

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};

}

#endif 


