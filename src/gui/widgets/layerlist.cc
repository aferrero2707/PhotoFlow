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

#include "layerlist.hh"

#include "../operation_config_gui.hh"


PF::LayerList::LayerList( OperationConfigGUI* d, std::string l ):
  Gtk::HBox(),
  dialog( d ),
  inhibit( false )
{
  label.set_text( l.c_str() );
  label2.set_text( "sub-image" );

  //image_num.set_range(0,99);

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  cbox.pack_start(columns.col_name);

  cbox.set_size_request( 100, -1 );

  vbox.pack_start( label, Gtk::PACK_SHRINK );
  vbox.pack_start( cbox, Gtk::PACK_SHRINK );
  pack_start( vbox, Gtk::PACK_SHRINK );

  vbox2.pack_start( label2, Gtk::PACK_SHRINK );
  vbox2.pack_start( image_num, Gtk::PACK_SHRINK );
  pack_start( vbox2, Gtk::PACK_SHRINK );

  image_num.signal_changed().
    connect(sigc::mem_fun(*this,
        &LayerList::changed));

  cbox.signal_changed().
    connect(sigc::mem_fun(*this,
        &LayerList::changed));

  show_all_children();
}


void PF::LayerList::update_model()
{
  //std::cout<<"LayerList::update_model() called"<<std::endl;
  if( !dialog ) return;
  PF::Layer* layer = dialog->get_layer();
  if(! layer ) return;
#ifndef NDEBUG
  const char* layer_name = layer->get_name().c_str();
#endif
  PF::Image* image = layer->get_image();
  if( !image ) return;
  std::list< std::pair<std::string,Layer*> > list;
  image->get_layer_manager().get_parent_layers( layer, list );

  int lid_prev = -1;
  Gtk::TreeModel::iterator active_iter = cbox.get_active();
  if( active_iter ) {
    Gtk::TreeModel::Row row = *active_iter;
    if( row ) {
      PF::Layer* active_layer = row[columns.col_layer];
      if( active_layer )
        lid_prev = active_layer->get_id();
    }
  }

  inhibit = true;
  model->clear();

  int lid = -1;
  int imgid = 0;
  bool blended = false;
  if( layer->get_extra_inputs().size() > 0 ) {
    lid = layer->get_extra_inputs()[0].first.first;
    imgid = layer->get_extra_inputs()[0].first.second;
    blended = layer->get_extra_inputs()[0].second;
  }

  int active_lid = -1;
  int first_lid = -1;
  int last_lid = -1;
  std::list< std::pair<std::string,Layer*> >::reverse_iterator iter;
  for( iter = list.rbegin(); iter != list.rend(); iter++ ) {
    Gtk::TreeModel::iterator ri = model->append();
    Gtk::TreeModel::Row row = *(ri);
    row[columns.col_name] = (*iter).first + " (blended)";
    row[columns.col_layer] = (*iter).second;
    row[columns.col_blended] = true;
    if( (*iter).second ) {
      if( ((*iter).second->get_id() == lid) && (blended == true) ) {
				cbox.set_active( model->children().size()-1 );
				image_num.set_value( imgid );
				active_lid = (*iter).second->get_id();
      }
      last_lid = (*iter).second->get_id();
			if( first_lid < 0 )
				first_lid = (*iter).second->get_id();
    }
    ri = model->append();
    row = *(ri);
    row[columns.col_name] = (*iter).first;
    row[columns.col_layer] = (*iter).second;
    row[columns.col_blended] = false;
    if( (*iter).second ) {
      if( ((*iter).second->get_id() == lid) && (blended == false) ) {
				cbox.set_active( model->children().size()-1 );
        image_num.set_value( imgid );
				active_lid = (*iter).second->get_id();
      }
      last_lid = (*iter).second->get_id();
			if( first_lid < 0 )
				first_lid = (*iter).second->get_id();
    }
  }
  if( active_lid < 0 ) {
    // No layer matching any of the extra inputs has been found.
    // Either there are no extra inputs yet defined, or the list of layers has changed
    cbox.set_active( -1 );
    if( false && first_lid >= 0 ) {
      // There are however some layers that have been inserted in the list of potential
      // sources, therefore we pick the first one
      cbox.set_active( 0 );
      active_lid = first_lid;
      //cbox.set_active( model->children().size()-1 );
      //active_lid = last_lid;
    }
  }

  if( lid_prev != active_lid ){
    changed();
  }
  inhibit = false;
}


void PF::LayerList::changed()
{
  //if( inhibit ) return;
  if( !dialog ) return;
  PF::Layer* layer = dialog->get_layer();
  if(! layer ) return;
  PF::Image* image = layer->get_image();
  if( !image ) return;

  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      PF::Layer* l = row[columns.col_layer];

      std::vector< std::pair< std::pair<int32_t,int32_t>,bool> >& inputs = layer->get_extra_inputs();
      if( (inputs.size() > 0) && (inputs[0].first.first == l->get_id())
          && (inputs[0].first.second == image_num.get_value()) ) {
        //std::cout<<"LayerList::changed(): extra input of layer \""<<layer->get_name()
        // <<"\" is unmodified."<<std::endl;
        return;
      }

#ifndef NDEBUG
      std::cout<<"LayerList::changed(): setting extra input of layer \""<<layer->get_name()
	       <<"\" to \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
#endif
      layer->set_input( 0, l->get_id(), image_num.get_value(), row[columns.col_blended] );
#ifndef NDEBUG
      std::cout<<"LayerList::changed(): setting dirty flag of layer \""<<layer->get_name()
         <<"\" to true"<<std::endl;
#endif
      layer->set_dirty( true );
			if( !inhibit ) {
				//std::cout<<"LayerList::changed(): updating image"<<std::endl;
				image->update();
			}
    }
  }
}
