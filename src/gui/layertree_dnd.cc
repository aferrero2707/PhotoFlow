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

#include "layertree.hh"
#include "../base/image.hh"


static void remove_list( std::list<PF::Layer*>& list, const std::list<PF::Layer*>& remove )
{
  for( std::list<PF::Layer*>::const_iterator li = remove.begin();
       li != remove.end(); li++ ) {
    list.remove( *li );
  }
}


bool PF::LayerTreeModel::row_draggable_vfunc( const Gtk::TreeModel::Path& path ) const
{
  PF::LayerTreeModel* this2 = const_cast<PF::LayerTreeModel*>(this);
  const_iterator iter = this2->get_iter( path );
  //const_iterator iter = get_iter( path );
  if( iter ) {
    Row row = *iter;
    if( row[columns.col_layer] != NULL ) return true;
    else return false;
  }
  return Gtk::TreeStore::row_draggable_vfunc( path );
}


// Check if plist contains all the layers on which "layer"
// as well as its children (maps and sublayers) depend
static bool check_inputs( std::list<PF::Layer*> layers, std::list<PF::Layer*> plist )
{
  std::cout<<"check_inputs(): dragged layers:"<<std::endl;
  for( std::list<PF::Layer*>::iterator li = layers.begin();
       li != layers.end(); li++ ) {
    if( (*li) == NULL ) continue;
    std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }

  // Expand the dragged layers list and remove all the dragged layers from the list
  // of parents of the drop destination
  std::list<PF::Layer*> list;
  for( std::list<PF::Layer*>::iterator li = layers.begin();
       li != layers.end(); li++ ) {
    if( (*li) == NULL ) continue;
    PF::Image* image = (*li)->get_image();
    g_assert( image != NULL );
    image->get_layer_manager().expand_layer( *li, list );
  }
  std::cout<<"check_inputs(): dragged layers (expanded):"<<std::endl;
  for( std::list<PF::Layer*>::iterator li = list.begin();
       li != list.end(); li++ ) {
    if( (*li) == NULL ) continue;
    std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }

  remove_list( plist, list );

  std::cout<<"check_inputs(): input layers (optimized):"<<std::endl;
  for( std::list<PF::Layer*>::iterator li = plist.begin();
       li != plist.end(); li++ ) {
    if( (*li) == NULL ) continue;
    std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }

  // Loop over the dragged layers and check that all extra inputs are 
  // in the list of parents of the drop destination
  for( std::list<PF::Layer*>::iterator li = list.begin();
       li != list.end(); li++ ) {
    if( (*li) == NULL ) continue;
    for( unsigned int i = 0; i < (*li)->get_extra_inputs().size(); i++ ) {
      int32_t id = (*li)->get_extra_inputs()[i].first;
      bool found = false;
      std::cout<<"check_inputs(): checking extra input \""<<(*li)->get_name()<<"\"["<<i<<"]="<<id<<std::endl;
      for( std::list<PF::Layer*>::iterator lj = plist.begin();
           lj != plist.end(); lj++ ) {
        if( ((*lj) != NULL) && ((*lj)->get_id() == id) ) {
          std::cout<<"check_inputs(): found."<<std::endl;
          found = true;
          break;
        }
      }
      if( !found ) return false;
    }
  }
  return true;
}


// Check if any of the layers in plist (as well as its children (maps and sublayers)) depend on the dragged layers
// The dragged layers are supposed to go above all the layers in plist
static bool check_inputs2( std::list<PF::Layer*> layers, std::list< PF::Layer*> plist )
{
  // Expand the dragged layers list and remove all the dragged layers from the list
  // of parents of the drop destination
  std::list<PF::Layer*> drag_list;
  for( std::list<PF::Layer*>::iterator li = layers.begin();
       li != layers.end(); li++ ) {
    if( (*li) == NULL ) continue;
    PF::Image* image = (*li)->get_image();
    g_assert( image != NULL );
    image->get_layer_manager().expand_layer( *li, drag_list );
  }
  std::cout<<"check_inputs2(): dragged layers (expanded):"<<std::endl;
  for( std::list<PF::Layer*>::iterator li = drag_list.begin();
       li != drag_list.end(); li++ ) {
    if( (*li) == NULL ) continue;
    std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }

  std::cout<<"check_inputs2(): plist.size() before optimization:"<<plist.size()<<std::endl;
  remove_list( plist, drag_list );
  std::cout<<"check_inputs2(): plist.size() after optimization:"<<plist.size()<<std::endl;
  std::cout<<"check_inputs2(): input layers (optimized):"<<std::endl;
  for( std::list<PF::Layer*>::iterator li = plist.begin();
       li != plist.end(); li++ ) {
    if( (*li) == NULL ) continue;
    std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }


  // loop on plist and check if any of the extra inputs of each layer corresponds to one of the dragged layers
  for( std::list<PF::Layer*>::iterator li = plist.begin();
       li != plist.end(); li++ ) {
    if( (*li) == NULL ) continue;
    for( unsigned int i = 0; i < (*li)->get_extra_inputs().size(); i++ ) {
      bool found = false;
      int32_t id = (*li)->get_extra_inputs()[i].first;
      std::cout<<"check_inputs2(): checking extra input \""<<(*li)->get_name()<<"\"["<<i<<"]="<<id<<std::endl;
      for( std::list<PF::Layer*>::iterator lj = drag_list.begin();
           lj != drag_list.end(); lj++ ) {
        if( (*lj) == NULL ) continue;
        std::cout<<"check_inputs2(): checking dragged layer \""<<(*lj)->get_name()<<"\"="<<(*lj)->get_id()<<std::endl;
        if( (*lj)->get_id() == id ) {
          std::cout<<"check_inputs2(): found."<<std::endl;
          found = true;
          break;
        }
      }
      if( found ) return false;
    }
  }

  return true;
}


PF::Layer* PF::LayerTreeModel::get_dest_layer(const Gtk::TreeModel::Path& dest,
                                              bool& drop_into) const
{
  drop_into = false;
  PF::LayerTreeModel* this2 = const_cast<PF::LayerTreeModel*>(this);

  const_iterator dest_row_iter = this2->get_iter( dest );
  if( !(dest_row_iter) ) {
    Gtk::TreeModel::Path dest_parent = dest;
    dest_parent.up();
    if( !dest_parent.empty() ) {
      const_iterator parent_row_iter = this2->get_iter( dest_parent );
      if( (parent_row_iter) ) {
        PF::Layer* pl = (*parent_row_iter)[columns.col_layer];
        if( pl ) {
          std::cout<<"get_dest_layer(): pl=\""<<pl->get_name()<<"\""<<std::endl;
          drop_into = true;
          return pl;
        }
      }
    }
  } else {
    Row row = *dest_row_iter;
    PF::Layer* l = row[columns.col_layer];
    if( l ) std::cout<<"get_dest_layer(): l=\""<<l->get_name()<<"\""<<std::endl;
    else std::cout<<"get_dest_layer(): l=NULL"<<std::endl;
    Gtk::TreeModel::Path dest_parent = dest;
    dest_parent.up();
    if( !dest_parent.empty() ) {
      const_iterator parent_row_iter = this2->get_iter( dest_parent );
      if( (parent_row_iter) ) {
        PF::Layer* pl = (*parent_row_iter)[columns.col_layer];
        if( pl ) {
          std::cout<<"get_dest_layer(): pl=\""<<pl->get_name()<<"\""<<std::endl;
        }
      }
    }
    return l;
  }

  return NULL;
}


PF::Layer* PF::LayerTreeModel::get_parent_layer(const Gtk::TreeModel::Path& dest) const
{
  PF::LayerTreeModel* this2 = const_cast<PF::LayerTreeModel*>(this);

  Gtk::TreeModel::Path dest_parent = dest;
  if( !dest_parent.up() ) return NULL;

  if( !dest_parent.empty() ) {
    const_iterator parent_row_iter = this2->get_iter( dest_parent );
    if( (parent_row_iter) ) {
      PF::Layer* pl = (*parent_row_iter)[columns.col_layer];
      return pl;
    }
  }

  return NULL;
}


bool PF::LayerTreeModel::row_drop_possible_vfunc( const Gtk::TreeModel::Path& dest,
                                                  const Gtk::SelectionData& selection_data) const
{
  /* Destination layer
   */
  std::cout<<"row_drop_possible_vfunc()"<<std::endl;
  bool drop_into;
  PF::Layer* dest_layer = get_dest_layer( dest, drop_into );
  if( drop_into ) {
    if( dest_layer )
      std::cout<<std::endl<<"Dest layer: \""<<dest_layer->get_name()<<"\"  drop_into="<<drop_into<<std::endl;
    else
      std::cout<<std::endl<<"Dest layer NULL,  drop_into="<<drop_into<<std::endl;
    return false;
  }
  PF::Layer* parent_layer = get_parent_layer( dest  );
  //if( !dest_layer && parent_layer ) return false;
  if( dest_layer )
    std::cout<<std::endl<<"Dest layer: \""<<dest_layer->get_name()<<"\"  drop_into="<<drop_into<<std::endl;
  else if( parent_layer ) {
    std::cout<<std::endl<<"Parent layer: \""<<parent_layer->get_name()<<"\""<<std::endl;
    dest_layer = parent_layer;
  }

  PF::Layer* src_layer = NULL;
  PF::Layer* group_layer = NULL;


  /* Source layer
   */
  PF::LayerTreeModel* this2 = const_cast<PF::LayerTreeModel*>(this);
  Glib::RefPtr<Gtk::TreeModel> refThis = 
    Glib::RefPtr<Gtk::TreeModel>( const_cast<PF::LayerTreeModel*>(this) );
  refThis->reference();
  Gtk::TreeModel::Path path_dragged_row;
  Gtk::TreeModel::Path::get_from_selection_data( selection_data,
                                                 refThis, path_dragged_row );

  if( path_dragged_row.empty() ) {
    std::cout<<"Dragged source empty!!!"<<std::endl;
    return Gtk::TreeStore::row_drop_possible_vfunc( dest, selection_data );
  }
  const_iterator dragged_row_iter = this2->get_iter( path_dragged_row );
  if( dragged_row_iter ) {
    Row row = *dragged_row_iter;
    PF::Layer* l = row[columns.col_layer];
    src_layer = l;
    if( l != NULL ) {
      std::cout<<"Dragged layer: \""<<l->get_name()<<"\""<<std::endl;
    }
  }


  if( (src_layer == NULL) || (dest_layer == NULL) ) return false;

  if( src_layer->get_id() == dest_layer->get_id() )
    return false;

  // The drop operation is only possible if two conditions are fulfilled:
  // 1. all the extra inputs of the dragged layer remain below it
  // 2. the dragged layer does not go above any layer that has it as extra input

  bool can_drop = true;
  PF::Image* image = src_layer->get_image();
  g_assert( image != NULL );
  /*
  std::list<PF::Layer*>* clist = image->get_layer_manager().get_list( src_layer );
  std::list<PF::Layer*>::iterator li;
  PF::Layer* previous_layer = NULL;
  for(li = clist->begin(); li != clist->end(); ++li) {
    PF::Layer* l = *li;
    if( l->get_id() == src_layer->get_id() ) break;
    previous_layer = l;
  }
  if( previous_layer ) {
    std::cout<<"Layer below dragged: \""<<previous_layer->get_name()<<"\""<<std::endl;
    if( previous_layer->get_id() == dest_layer->get_id() ) 
      return false;
  } else {
    std::cout<<"Layer below dragged: NULL"<<std::endl;
  }
  */

  std::list<PF::Layer*> plist;
  image->get_layer_manager().get_input_layers( dest_layer, plist );
  /**/
  std::cout<<"Parents of layer \""<<dest_layer->get_name()<<"\":"<<std::endl;
  for( std::list< PF::Layer*>::iterator li = plist.begin();
       li != plist.end(); li++ ) {
    if( (*li) != NULL ) std::cout<<"  \""<<(*li)->get_name()<<"\""<<std::endl;
  }
  /**/
  // Add dest_layer and all its children to plist
  image->get_layer_manager().expand_layer( dest_layer, plist );
  

  std::list<PF::Layer*> source_layers;
  source_layers.push_back( src_layer );

  // Condition #1
  can_drop = check_inputs( source_layers, plist );
  std::cout<<"can_drop #1="<<can_drop<<std::endl;

  // Condition #2
  can_drop = check_inputs2( source_layers, plist );
  std::cout<<"can_drop #2="<<can_drop<<std::endl;

  if( !can_drop ) return false;

  return true;
}



bool PF::LayerTreeModel::drag_data_received_vfunc( const Gtk::TreeModel::Path& dest,
                                                   const Gtk::SelectionData& selection_data)
{
  PF::Layer* src_layer = NULL;
  PF::Layer* dest_layer = NULL;
  PF::Layer* group_layer = NULL;

  PF::LayerTreeModel* this2 = const_cast<PF::LayerTreeModel*>(this);

  const_iterator dest_row_iter = this2->get_iter( dest );
  if( !(dest_row_iter) ) {
    std::cout<<"Drag end: dest_row_iter not valid"<<std::endl;
    return Gtk::TreeStore::drag_data_received_vfunc( dest, selection_data );
  }

  /* Source layer
   */
  Glib::RefPtr<Gtk::TreeModel> refThis = 
    Glib::RefPtr<Gtk::TreeModel>( const_cast<PF::LayerTreeModel*>(this) );
  refThis->reference();
  Gtk::TreeModel::Path path_dragged_row;
  Gtk::TreeModel::Path::get_from_selection_data( selection_data,
                                                 refThis, path_dragged_row );

  if( path_dragged_row.empty() ) {
    std::cout<<"Dragged source empty!!!"<<std::endl;
    return Gtk::TreeStore::drag_data_received_vfunc( dest, selection_data );
  }
  const_iterator dragged_row_iter = this2->get_iter( path_dragged_row );
  if( dragged_row_iter ) {
    Row row = *dragged_row_iter;
    PF::Layer* l = row[columns.col_layer];
    src_layer = l;
    if( l != NULL ) {
      std::cout<<"Drag end: dragged layer: \""<<l->get_name()<<"\""<<std::endl;
    }
  }


  /* Destination layer
   */
  //Gtk::TreeModel::Path dest_parent = dest;
  //bool is_child = dest_parent.up();

  Row row = *dest_row_iter;
  PF::Layer* l = row[columns.col_layer];
  dest_layer = l;
  if( l != NULL ) {
    std::cout<<"Drag end: destination layer: \""<<l->get_name()<<"\""<<std::endl;
    //std::cout<<"  is_child="<<is_child<<std::endl;
  } else {
    group_layer = get_parent_layer( dest  );
  }
 

  std::cout<<"Drag end: src="<<src_layer<<"  dest="<<dest_layer<<"  group="<<group_layer<<std::endl;

  //return Gtk::TreeStore::drag_data_received_vfunc( dest, selection_data );
  if( src_layer == NULL ) return false;
  if( (dest_layer == NULL) && (group_layer == NULL) ) return false;


  PF::Image* image = src_layer->get_image();
  g_assert( image != NULL );

  // Now we have to move the dragged layer in the new position 
  // and refresh the layer tree model
  // First of all we have to make sure that no image update occurs during
  // the reconfiguration of the layers
  image->lock();
  if( !(image->get_layer_manager().remove_layer(src_layer)) ) {
    image->unlock();
    return false;
  }

  
  bool result = Gtk::TreeStore::drag_data_received_vfunc( dest, selection_data );
  if( dest_layer ) {
    // If the destination layer is defined, then we insert the dragged layer
    // above it
    std::cout<<"Inserting \""<<src_layer->get_name()<<"\" above \""
             <<dest_layer->get_name()<<"\""<<std::endl;
    std::list<Layer*>* list = image->get_layer_manager().get_list( dest_layer );
    if( list == NULL ) {
    image->unlock();
      return false;
    }
    if( !(insert_layer(*list, src_layer, dest_layer->get_id())) ) {
    image->unlock();
      return false;
    }
  } else {
    // Otherwise, if the group layer is defined we insert the dragged
    // layer on top of the group's sublayers
    if( group_layer ) {
      if( !(insert_layer(group_layer->get_sublayers(), 
                         src_layer, -1)) ) {
    image->unlock();
        return false;
      }
    } else {
    image->unlock();
      return false;
    }
  }
  

    image->unlock();
  image->update();
  // Now that the layers have been reconfigured, we emit the signal_drop_done
  // to notify the LayerTree that the model has to be updated
  signal_dnd_done.emit();

  if( !result ) return false;
  return true;
}



