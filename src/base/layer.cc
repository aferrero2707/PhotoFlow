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

#include "layer.hh"


PF::Layer::Layer(int32_t i): id(i)
{
  // A layer is always dirty when created, as it is by definition not included in the
  // VIPS rendering chain yet
  dirty = true;

  visible = true;
}



bool PF::Layer::insert(std::list<PF::Layer*>& list, PF::Layer* l, int32_t lid)
{
  if( lid < 0 ) {
    list.push_back( l );
    return true;
  }

  std::list<Layer*>::iterator it;
  for( it = list.begin(); it != list.end(); ++it )
    if( (*it)->get_id() == lid ) break;

  if( it == list.end() ) return false;

  //list.insert( l, it );
  return true;
}



bool PF::Layer::insert_before(std::list<PF::Layer*>& list, PF::Layer* l, int32_t lid)
{
  std::list<Layer*>::iterator it;
  for( it = list.begin(); it != list.end(); ++it )
    if( (*it)->get_id() == lid ) break;

  if( it == list.end() ) return false;

  //list.insert( l, it );
  return true;
}



bool PF::Layer::sublayers_insert(PF::Layer* l, int32_t lid)
{
  return insert(sublayers,l,lid);
}

bool PF::Layer::sublayers_insert_before(PF::Layer* l, int32_t lid)
{
  return insert_before(sublayers,l,lid);
}


bool PF::Layer::imap_insert(PF::Layer* l, int32_t lid)
{
  return insert(imap_layers,l,lid);
}

bool PF::Layer::imap_insert_before(PF::Layer* l, int32_t lid)
{
  return insert_before(imap_layers,l,lid);
}


bool PF::Layer::omap_insert(PF::Layer* l, int32_t lid)
{
  return insert(omap_layers,l,lid);
}

bool PF::Layer::omap_insert_before(PF::Layer* l, int32_t lid)
{
  return insert_before(omap_layers,l,lid);
}
