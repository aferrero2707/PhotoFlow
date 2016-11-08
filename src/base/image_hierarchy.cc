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


#include <iostream>
#include "image_hierarchy.hh"


void PF::image_hierarchy_free(PF::IHArray* array)
{
  if( !array ) return;
  if( array->vec ) g_free(array->vec);
  g_free(array);
}


PF::IHArray* PF::image_hierarchy_new()
{
  PF::IHArray* result = (PF::IHArray*)g_malloc( sizeof(PF::IHArray) );
  if( result ) {
    result->size = 0;
    result->vec = NULL;
  }
  return result;
}


void PF::image_hierarchy_add_element(PF::IHArray* array, VipsImage* el, int padding)
{
  if( !array ) return;
  std::cout<<"image_hierarchy_add_element(): padding="<<padding<<std::endl;
  for(unsigned int i = 0; i < array->size; i++) {
    if( array->vec[i].image == el ) {
      std::cout<<"image_hierarchy_add_element():   found image in array, old padding="<<array->vec[i].padding<<std::endl;
      if( array->vec[i].padding < padding ) {
        array->vec[i].padding = padding;
      }
      return;
    }
  }

  array->vec = (PF::IHElement*)g_realloc( array->vec, sizeof(PF::IHElement)*(array->size+1) );
  if( array->vec == NULL ) {
    array->size = 0;
    return;
  }
  array->vec[array->size].image = el;
  array->vec[array->size].padding = padding;
  array->size += 1;
  std::cout<<"image_hierarchy_add_element():   added element, new size="<<array->size<<std::endl;
}


void PF::image_hierarchy_fill(VipsImage* dest, int padding, std::vector<VipsImage*>& parents)
{
  PF::IHArray* array = PF::image_hierarchy_new();
  std::cout<<"image_hierarchy_fill(): array="<<(void*)array<<" size="<<array->size<<std::endl;

  for( unsigned int i = 0; i < parents.size(); i++ ) {
    PF::IHArray* parray;
    size_t length;

    std::cout<<"image_hierarchy_fill(): i="<<i<<" array="<<array<<std::endl;
    if( vips_image_get_blob( parents[i], "pf-image-hierarchy", (void**)&parray, &length) ) {
      std::cout<<"image_hierarchy_fill(): parent image "<<i<<" does not have hierarchy information"<<std::endl;
      std::cout<<"image_hierarchy_fill(): i="<<i<<" array="<<array<<std::endl;
    } else {
      if( parray == NULL ) {
        std::cout<<"image_hierarchy_fill(): parent image "<<i<<" NULL hierarchy information"<<std::endl;
        std::cout<<"image_hierarchy_fill(): i="<<i<<" array="<<array<<std::endl;
      } else {
        if( length != sizeof(PF::IHArray) ) {
          std::cout<<"image_hierarchy_fill(): parent image "<<i<<" wromg size of hierarchy information"<<std::endl;
          std::cout<<"image_hierarchy_fill(): i="<<i<<" array="<<array<<std::endl;
        } else {
          std::cout<<"image_hierarchy_fill(): adding array from parent image "<<i<<std::endl;

          for( unsigned int ei = 0; ei < parray->size; ei++ ) {
            PF::image_hierarchy_add_element( array, parray->vec[ei].image,
                parray->vec[ei].padding+padding );
            std::cout<<"image_hierarchy_fill(): i="<<i<<" ei="<<ei<<" array="<<array<<std::endl;
          }
        }
      }
    }

    // add the parent image itself
    PF::image_hierarchy_add_element( array, parents[i], padding );
    std::cout<<"image_hierarchy_fill(): i="<<i<<" array="<<array<<std::endl;
  }

  std::cout<<"image_hierarchy_fill(): adding array to image="<<array<<std::endl;
  vips_image_set_blob( dest, "pf-image-hierarchy",
      (VipsCallbackFn) PF::image_hierarchy_free, array, sizeof(PF::IHArray) );
  std::cout<<"image_hierarchy_fill(): array added"<<std::endl;
}



int PF::image_hierarchy_compare_images(VipsImage* i0, VipsImage* i1)
{
  PF::IHArray* parray0;
  PF::IHArray* parray1;
  size_t length;

  if( vips_image_get_blob( i0, "pf-image-hierarchy", (void**)&parray0, &length) ) {
    return 1;
  }
  if( parray0 == NULL ) return 1;
  if( length != sizeof(PF::IHArray) ) return 1;

  if( vips_image_get_blob( i1, "pf-image-hierarchy", (void**)&parray1, &length) ) {
    return 0;
  }
  if( parray1 == NULL ) return 0;
  if( length != sizeof(PF::IHArray) ) return 0;

  int padding_tot0 = 0;
  int padding_tot1 = 0;

  for( unsigned int i = 0; i < parray0->size; i++ ) {
    for( unsigned int j = 0; j < parray1->size; j++ ) {
      if( parray0->vec[i].image != parray1->vec[j].image )
        continue;
      if( parray0->vec[i].padding > parray1->vec[j].padding )
        padding_tot0 += parray0->vec[i].padding - parray1->vec[j].padding;
      else
        padding_tot1 += parray1->vec[j].padding - parray0->vec[i].padding;
    }
  }
  if( padding_tot0 >= padding_tot1 ) return 0;
  else return 1;
}
