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


PF::View::~View()
{
  for( unsigned int i = 0; i < nodes.size(); i++ ) {
    if( nodes[i] != NULL ) {
      if( nodes[i]->image != NULL )
	g_object_unref( nodes[i]->image );
      delete nodes[i];
    }
  }
}


void PF::View::set_node( VipsImage* img, unsigned int id, int input_id )
{
  while( nodes.size() <= (id+1) ) nodes.push_back(NULL);
  ViewNode* node = new ViewNode;
  node->image = img;
  node->input_id = input_id;
  if( nodes[id] != NULL ) {
    if( nodes[id]->image != NULL )
      g_object_unref( nodes[id]->image );
    delete nodes[id];
    nodes[id] = NULL;
  }
  nodes[id] = node;
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
  }
}
