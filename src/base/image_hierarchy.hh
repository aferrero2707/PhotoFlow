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


#ifndef PF_IMAGE_HIERARCHY__HH
#define PF_IMAGE_HIERARCHY__HH

#include <vips/vips.h>
#include <vector>

namespace PF
{
  struct IHElement
  {
    VipsImage* image;
    int padding;
  };

  struct IHArray
  {
    unsigned int size;
    IHElement* vec;
  };

  void image_hierarchy_free(IHArray* array);
  IHArray* image_hierarchy_new();
  void image_hierarchy_add_element(IHArray* array, VipsImage* el, int padding);
  void image_hierarchy_fill(VipsImage* dest, int padding, std::vector<VipsImage*>& parents);
  int image_hierarchy_compare_images(VipsImage* i0, VipsImage* i1);
}

#endif
