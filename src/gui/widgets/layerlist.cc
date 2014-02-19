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


PF::LayerList::LayerList( OperationConfigUI* d, std::string l ):
  Gtk::VBox(),
  dialog (d )
{
  label.set_text( l.c_str() );

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  cbox.pack_start(columns.col_name);

  pack_start( label, Gtk::PACK_SHRINK );
  pack_start( cbox, Gtk::PACK_SHRINK );

  //pack_start( vbox, Gtk::PACK_SHRINK );

  cbox.signal_changed().
    connect(sigc::mem_fun(*this,
			  &LayerList::changed));

  show_all_children();
}


void PF::LayerList::update_model()
{
  if( !dialog ) return;
  PF::Layer* layer = dialog->get_layer();
  if(! layer ) return;
  PF::Image* image = layer->get_image();
  if( !image ) return;
  std::list< std::pair<std::string,Layer*> > list;
  image->get_layer_manager().get_parent_layers( layer, list );

  model->clear();

  std::list< std::pair<std::string,Layer*> >::reverse_iterator iter;
  for( iter = list.rbegin(); iter != list.rend(); iter++ ) {
    Gtk::TreeModel::iterator ri = model->append();
    Gtk::TreeModel::Row row = *(ri);
    row[columns.col_name] = (*iter).first;
    row[columns.col_layer] = (*iter).second;
    //if( def.first == (*iter).first) cbox.set_active( ri );
  }
}


void PF::LayerList::changed()
{
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

      std::cout<<"Setting extra input of layer \""<<layer->get_name()
	       <<"\" to \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
      layer->set_input( 0, l->get_id() );
      layer->set_dirty( true );
      std::cout<<"  updating image"<<std::endl;
      image->update();
    }
  }
}
