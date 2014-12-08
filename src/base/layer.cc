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

#include "image.hh"


PF::Layer::Layer(int32_t i, bool c): 
  id(i), 
  processor( NULL ), 
  blender( NULL ), 
  cached( c ), 
  cache_buffer( NULL ),
  image( NULL )
{
  // A layer is always dirty when created, as it is by definition not included in the
  // VIPS rendering chain yet
  dirty = true;
  modified_flag = true;

  visible = true;

  normal = true;

  if( cached )
    cache_buffer = new CacheBuffer();
}



void PF::Layer::set_processor(ProcessorBase* p) 
{ 
  processor = p; 
  //processor->get_par()->signal_modified.connect(sigc::mem_fun(this, &PF::Layer::set_modified) );
  //processor->get_par()->signal_modified.connect(sigc::mem_fun(image, &PF::Image::set_modified) );
}


void PF::Layer::set_blender(ProcessorBase* p) 
{ 
  blender = p; 
  //blender->get_par()->signal_modified.connect(sigc::mem_fun(this, &PF::Layer::set_modified) );
  //blender->get_par()->signal_modified.connect(sigc::mem_fun(image, &PF::Image::set_modified) );
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

  it++;
  list.insert( it, l );
  return true;
}



bool PF::Layer::insert_before(std::list<PF::Layer*>& list, PF::Layer* l, int32_t lid)
{
  std::list<Layer*>::iterator it;
  for( it = list.begin(); it != list.end(); ++it )
    if( (*it)->get_id() == lid ) break;

  if( it == list.end() ) return false;

  list.insert( it, l );
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


void PF::Layer::remove_input(int32_t lid)
{
  bool found = false;
  do {
    for( unsigned int i = 0; i < extra_inputs.size(); i++) {
      if( extra_inputs[i].first == lid ) {
        extra_inputs.erase( extra_inputs.begin()+i );
        found = true;
        break;
      }
    }
  } while( found );
}



bool PF::Layer::save( std::ostream& ostr, int level )
{
  for(int i = 0; i < level; i++) ostr<<"  ";
  ostr<<"<layer name=\""<<name<<"\" id=\""<<id<<"\" visible=\""<<visible<<"\" normal=\""<<normal<<"\" extra_inputs=\"";
  int n;
  for( int i=0, n=0; i < extra_inputs.size(); i++ ) {
    int32_t id = extra_inputs[i].first;
    if( id < 0 ) continue;
    PF::Layer* l = image->get_layer_manager().get_layer( id );
    if( !l ) continue;
    if( n>0 ) ostr<<" ";
    ostr<<l->get_id()<<" "<<extra_inputs[i].second;
    n++;
  }
  ostr<<"\">"<<std::endl;

  if( processor && processor->get_par() )
    processor->get_par()->save( ostr, level+1 );

  if( blender && blender->get_par() )
    blender->get_par()->save( ostr, level+1 );

  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"<sublayers type=\"child\">"<<std::endl;
  for( std::list<Layer*>::iterator li = sublayers.begin();
       li != sublayers.end(); li++ ) {
    (*li)->save( ostr, level+2 );
  }
  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"</sublayers>"<<std::endl;

  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"<sublayers type=\"imap\">"<<std::endl;
  for( std::list<Layer*>::iterator li = imap_layers.begin();
       li != imap_layers.end(); li++ ) {
    (*li)->save( ostr, level+2 );
  }
  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"</sublayers>"<<std::endl;

  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"<sublayers type=\"omap\">"<<std::endl;
  for( std::list<Layer*>::iterator li = omap_layers.begin();
       li != omap_layers.end(); li++ ) {
    (*li)->save( ostr, level+2 );
  }
  for(int i = 0; i < level+1; i++) ostr<<"  ";
  ostr<<"</sublayers>"<<std::endl;

  for(int i = 0; i < level; i++) ostr<<"  ";
  ostr<<"</layer>"<<std::endl;

  return true;
}
