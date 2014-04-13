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


#include "view.hh"
#include "processor.hh"
#include "layer.hh"
#include "photoflow.hh"


PF::View::~View()
{
  for( unsigned int i = 0; i < nodes.size(); i++ ) {
    if( nodes[i] != NULL ) {
      if( nodes[i]->image != NULL ) {
	g_assert( G_OBJECT( nodes[i]->image )->ref_count > 0 );
	g_object_unref( nodes[i]->image );
      }
      if( nodes[i]->processor != NULL )
	delete( nodes[i]->processor );
      delete nodes[i];
    }
  }
}


PF::ViewNode* PF::View::set_node( Layer* layer, Layer* input_layer )
{
  if( !layer )
    return NULL;

  int id = layer->get_id();
  if( id >= nodes.size() ) {
    while( nodes.size() <= (id+1) ) nodes.push_back(NULL);
    nodes[id] = new PF::ViewNode;
  }
  if( nodes[id] == NULL )
    nodes[id] = new PF::ViewNode;

  PF::ViewNode* node = nodes[id];
  if( node == NULL )
    return NULL;

  node->input_id = ( input_layer != NULL ) ? input_layer->get_id() : -1;

  PF::OpParBase* srcpar = NULL;
  if( layer->get_processor() != NULL )
    srcpar = layer->get_processor()->get_par();

  if( (srcpar != NULL ) ) {
    if( (node->processor == NULL) ||
	(node->processor->get_par() == NULL) ||
	(node->processor->get_par()->get_type() != srcpar->get_type()) ) {
      if( node->processor != NULL )
	delete( node->processor );
      node->processor = PF::PhotoFlow::Instance().
	new_operation_nogui( srcpar->get_type(), NULL );
    }

    if( (node->processor != NULL) &&
	(node->processor->get_par() != NULL) ) {
      g_assert( node->processor->get_par()->import_settings( srcpar ) != false );
    }
  }

  return node;
}


void PF::View::set_image( VipsImage* img, unsigned int id )
{
  if( id >= nodes.size() ) 
    return;
  
  if( nodes[id] != NULL ) {
    if( nodes[id]->image != NULL ) {
      if( G_OBJECT( nodes[id]->image )->ref_count < 1 )
	std::cout<<"!!! View::set_image(): wrong ref_count for node #"<<id<<", image="<<nodes[id]->image<<std::endl;
      g_assert( G_OBJECT( nodes[id]->image )->ref_count > 0 );
      g_object_unref( nodes[id]->image );
    }
    nodes[id]->image = img;
  }
}


void PF::View::remove_node( unsigned int id )
{
  if( id >= nodes.size() ) return;

  if( nodes[id] != NULL ) {
    if( nodes[id]->image != NULL )
      g_object_unref( nodes[id]->image );
    if( nodes[id]->processor != NULL )
      delete( nodes[id]->processor );
    delete nodes[id];
    nodes[id] = NULL;
  }
}


bool PF::View::processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::View::update(): sink #"<<i<<" -> processing="<<sinks[i]->is_processing()<<std::endl;
#endif
    if( sinks[i]->is_processing() ) return true;
  }
  return false;
}


void PF::View::lock_processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::View::update(): locking sink #"<<i<<std::endl;
#endif
    sinks[i]->get_processing_mutex().lock();
#ifndef NDEBUG
    std::cout<<"PF::View::update(): sink #"<<i<<" locked"<<std::endl;
#endif
  }
}


void PF::View::unlock_processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::View::update(): unlocking sink #"<<i<<std::endl;
#endif
    sinks[i]->get_processing_mutex().unlock();
#ifndef NDEBUG
    std::cout<<"PF::View::update(): sink #"<<i<<" unlocked"<<std::endl;
#endif
  }
}


void PF::View::update()
{
#ifndef NDEBUG
  std::cout<<"PF::View::update(): called"<<std::endl;
#endif
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::View::update(): updating sink #"<<i<<std::endl;
#endif
    sinks[i]->update();
#ifndef NDEBUG
    std::cout<<"PF::View::update(): sink #"<<i<<" updated"<<std::endl;
#endif
  }
}
