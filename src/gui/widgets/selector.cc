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

#include "selector.hh"


PF::Selector::Selector( OperationConfigGUI* dialog, std::string pname, std::string l, int val ):
  Gtk::HBox(),
  PF::PFWidget( dialog, pname )
{
  label.set_text( l.c_str() );

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  cbox.pack_start(columns.col_name);

  pack_start( label, Gtk::PACK_SHRINK );
  pack_start( cbox, Gtk::PACK_SHRINK );

  cbox.set_size_request( 150, -1 );

  //pack_start( vbox, Gtk::PACK_SHRINK );

  cbox.signal_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));

  show_all_children();
}


PF::Selector::Selector( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l, int val ):
  Gtk::HBox(),
  PF::PFWidget( dialog, processor, pname )
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
			  &PFWidget::changed));

  show_all_children();
}


void PF::Selector::get_value()
{
  if( !get_prop() ) return;
  if( !get_prop()->is_enum() ) return;

  model->clear();

  std::pair< int, std::pair<std::string,std::string> > def = 
    get_prop()->get_enum_value();
#ifndef NDEBUG
  std::cout<<"PF::Selector::get_value(): current value=\""<<def.second.second<<"\""<<std::endl;
#endif
  std::map< int, std::pair<std::string,std::string> > values = 
    get_prop()->get_enum_values();
  std::map< int, std::pair<std::string,std::string> >::iterator iter;
  for( iter = values.begin(); iter != values.end(); iter++ ) {
    if( !check_value((*iter).first,(*iter).second.first,(*iter).second.second) )
      continue;
    Gtk::TreeModel::iterator ri = model->append();
    Gtk::TreeModel::Row row = *(ri);
    row[columns.col_name] = (*iter).second.second.c_str();
    row[columns.col_id] = (*iter).first;
    row[columns.col_value] = (*iter).second.first.c_str();
    if( def.first == (*iter).first) cbox.set_active( ri );
  }
}


void PF::Selector::set_value()
{
  if( !get_prop() ) return;

  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      Glib::ustring value = row[columns.col_value];

#ifndef NDEBUG
      std::cout << "selected value=" << value << std::endl;
#endif
      std::string str = value.c_str();
      get_prop()->update(str);
    }
  }
}
