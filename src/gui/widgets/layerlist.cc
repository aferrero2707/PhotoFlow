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


PF::LayerList::LayerList( OperationConfigGUI* d, std::string l, int iid, bool compact ):
  Gtk::HBox(),
  dialog( d ),
  input_id(iid),
  inhibit( false )
{
  label.set_text( l.c_str() );
  label2.set_text( "sub-image" );

  image_num.set_range(0,99);
  //image_num.set_value(5);

  //set_spacing(5);

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  cbox.pack_start(columns.col_name);

  cbox.set_size_request( 150, -1 );

  if(!compact) vbox.pack_start( label, Gtk::PACK_SHRINK );
  vbox.pack_start( cbox, Gtk::PACK_SHRINK );

  if(!compact) vbox2.pack_start( label2, Gtk::PACK_SHRINK );
  if(!compact) vbox2.pack_start( image_num, Gtk::PACK_SHRINK );

  if(!compact) pack_end( vbox2, Gtk::PACK_SHRINK );
  pack_end( vbox, Gtk::PACK_EXPAND_WIDGET );
  if(compact) pack_end( label, Gtk::PACK_SHRINK );

  //image_num.signal_changed().
  image_num.signal_activate().
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
  //return;
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
  std::pair< std::pair<int32_t,int32_t>,bool> iddef = image->get_layer_manager().get_default_input_layer( layer );
  PF::Layer* ldef = image->get_layer_manager().get_layer( iddef.first.first );
  PF::Layer* container = image->get_layer_manager().get_container_layer( layer );

  int lid_prev = -1;
  bool blended_prev = true;
  Gtk::TreeModel::iterator active_iter = cbox.get_active();
  if( active_iter ) {
    Gtk::TreeModel::Row row = *active_iter;
    if( row ) {
      PF::Layer* active_layer = row[columns.col_layer];
      if( active_layer ) {
        lid_prev = active_layer->get_id();
        blended_prev = row[columns.col_blended];
      }
    }
  }

  inhibit = true;
  model->clear();

  int lid = -1;
  int imgid = 0;
  bool blended = false;
  if( layer->get_inputs().size() > input_id ) {
    lid = layer->get_inputs()[input_id].first.first;
    imgid = layer->get_inputs()[input_id].first.second;
    blended = layer->get_inputs()[input_id].second;
  }

  int active_lid = -1;
  int active_blended = true;
  int first_lid = -1;
  int last_lid = -1;
  Gtk::TreeModel::iterator ri = model->append();
  Gtk::TreeModel::Row row = *(ri);
  std::string defname = "default";
  if( ldef ) {
    defname += " (";
    defname += ldef->get_name();
    if( iddef.second ) defname += ", blended";
    defname += ")";
  }
  row[columns.col_name] = defname;
  row[columns.col_layer] = NULL;
  row[columns.col_blended] = iddef.second;

  std::list< std::pair<std::string,Layer*> >::reverse_iterator iter;
  for( iter = list.rbegin(); iter != list.rend(); iter++ ) {

    if( (*iter).second != container ) {
      ri = model->append();
      row = *(ri);
      row[columns.col_name] = (*iter).first + ", blended";
      row[columns.col_layer] = (*iter).second;
      row[columns.col_blended] = true;
      if( (*iter).second ) {
        if( ((int)((*iter).second->get_id()) == lid) && (blended == true) ) {
          cbox.set_active( model->children().size()-1 );
          image_num.set_value( imgid );
          active_lid = (*iter).second->get_id();
          active_blended = true;
        }
        last_lid = (*iter).second->get_id();
        if( first_lid < 0 )
          first_lid = (*iter).second->get_id();
      }
    }

    ri = model->append();
    row = *(ri);
    row[columns.col_name] = (*iter).first;
    row[columns.col_layer] = (*iter).second;
    row[columns.col_blended] = false;
    if( (*iter).second ) {
      if( ((int)((*iter).second->get_id()) == lid) && (blended == false) ) {
				cbox.set_active( model->children().size()-1 );
        image_num.set_value( imgid );
				active_lid = (*iter).second->get_id();
        active_blended = false;
      }
      last_lid = (*iter).second->get_id();
			if( first_lid < 0 )
				first_lid = (*iter).second->get_id();
    }
  }
  if( active_lid < 0 ) {
    // No layer matching any of the extra inputs has been found.
    // Either there are no extra inputs yet defined, or the list of layers has changed
    cbox.set_active( 0 );
    if( false && first_lid >= 0 ) {
      // There are however some layers that have been inserted in the list of potential
      // sources, therefore we pick the first one
      cbox.set_active( 0 );
      active_lid = first_lid;
      //cbox.set_active( model->children().size()-1 );
      //active_lid = last_lid;
    }
  }

  ri = cbox.get_active();
  if( ri ) {
    row = *ri;
    if( row ) {
      std::string cname = row[columns.col_name];
      cbox.set_tooltip_text(std::string(_("select the input layer for this tool\n\ncurrent: \""))+cname+"\"");
      label.set_tooltip_text(std::string(_("select the input layer for this tool\n\ncurrent: \""))+cname+"\"");
    }
  }

  //std::cout<<"LayerList: lid_prev="<<lid_prev<<"  active_lid="<<active_lid<<"  blended_prev="<<blended_prev
  //    <<"  active_blended="<<active_blended<<std::endl;
  if( lid_prev != active_lid || blended_prev != active_blended ){
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

  //std::cout<<"LayerList::changed() called, inhibit="<<inhibit<<std::endl;

  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      PF::Layer* l = row[columns.col_layer];
      bool blended = row[columns.col_blended];

      if( l ) {
        std::vector< std::pair< std::pair<int32_t,int32_t>,bool> >& inputs = layer->get_inputs();
        if( (inputs.size() > input_id) && (inputs[input_id].first.first == (int)(l->get_id()))
            && (inputs[input_id].first.second == image_num.get_value())
            && (inputs[input_id].second == blended) ) {
          std::cout<<"LayerList::changed(): extra input of layer \""<<layer->get_name()
           <<"\" is unmodified."<<std::endl;
          std::string cname = row[columns.col_name];
          cbox.set_tooltip_text(cname);
          label.set_tooltip_text(cname);
          return;
        }

#ifndef NDEBUG
        std::cout<<"LayerList::changed(): setting extra input of layer \""<<layer->get_name()
	           <<"\" to \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
#endif
        std::string cname = row[columns.col_name];
        cbox.set_tooltip_text(cname);
        label.set_tooltip_text(cname);
        layer->set_input( input_id, l->get_id(), image_num.get_value(), row[columns.col_blended] );
#ifndef NDEBUG
        std::cout<<"LayerList::changed(): setting dirty flag of layer \""<<layer->get_name()
             <<"\" to true"<<std::endl;
#endif
      } else {
        std::string cname = row[columns.col_name];
        cbox.set_tooltip_text(cname);
        label.set_tooltip_text(cname);
        layer->set_input( input_id, -1, image_num.get_value(), row[columns.col_blended] );
      }

      layer->set_dirty( true );
			if( !inhibit ) {
				//std::cout<<"LayerList::changed(): updating image"<<std::endl;
				image->update();
			}
    }
  }
}
