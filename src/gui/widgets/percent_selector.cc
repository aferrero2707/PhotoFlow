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
#include <sstream>
#include <string>
#include "percent_selector.hh"


PF::PercentSelector::PercentSelector( OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width ): Gtk::HBox(),
  PF::PFWidget( dialog, pname ), cbox(true), current_active_id(-1)
{
  label.set_text( l.c_str() );

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  //cbox.pack_start(columns.col_name);
  cbox.set_entry_text_column(columns.col_name);

  pack_end( cbox, Gtk::PACK_SHRINK );
  pack_end( label, Gtk::PACK_SHRINK );

  //cbox.set_size_request( 100, -1 );

  if( width > 0 ) {
    Glib::ListHandle< Gtk::CellRenderer* > cells = cbox.get_cells();
    Glib::ListHandle< Gtk::CellRenderer* >::iterator ci = cells.begin();
    //for( ci = cells.begin(); ci != cells.end(); ci++ ) {
    //  (*ci)->set_fixed_size( width, -1 );
    //}
  }
  cbox.set_size_request(width, -1);

  Gtk::TreeModel::iterator ri;
  Gtk::TreeModel::Row row;
  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "100 %";
  row[columns.col_id] = 0;
  row[columns.col_value] = 1;
  cbox.set_active( ri );

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "75 %";
  row[columns.col_id] = 1;
  row[columns.col_value] = 0.75;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "50 %";
  row[columns.col_id] = 2;
  row[columns.col_value] = 0.5;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "25 %";
  row[columns.col_id] = 3;
  row[columns.col_value] = 0.25;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "10 %";
  row[columns.col_id] = 4;
  row[columns.col_value] = 0.1;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "5 %";
  row[columns.col_id] = 5;
  row[columns.col_value] = 0.05;

  //pack_start( vbox, Gtk::PACK_SHRINK );

  //cbox.signal_changed().connect(sigc::mem_fun(*this, &PFWidget::changed));

  Gtk::Entry* entry = cbox.get_entry();
  if (entry)
  {
    entry->set_width_chars(5);

    // The Entry shall receive focus-out events.
    entry->add_events(Gdk::FOCUS_CHANGE_MASK);

    // Alternatively you can connect to m_Combo.signal_changed().
    //entry->signal_changed().connect(sigc::mem_fun(*this,
    //  &ExampleWindow::on_entry_changed) );

    entry->signal_activate().connect(sigc::mem_fun(*this,
      &PFWidget::changed) );

    entry->signal_focus_out_event().connect(sigc::mem_fun(*this,
      &PercentSelector::on_entry_focus_out_event) );
  }

  show_all_children();
}


PF::PercentSelector::PercentSelector( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l, int val, int width ): Gtk::HBox(),
    PF::PFWidget( dialog, processor, pname ), cbox(true), current_active_id(-1)
{
  label.set_text( l.c_str() );

  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  //cbox.pack_start(columns.col_name);
  cbox.set_entry_text_column(columns.col_name);

  pack_start( label, Gtk::PACK_SHRINK );
  pack_start( cbox, Gtk::PACK_SHRINK );

  if( width > 0 ) {
    Glib::ListHandle< Gtk::CellRenderer* > cells = cbox.get_cells();
    Glib::ListHandle< Gtk::CellRenderer* >::iterator ci = cells.begin();
    //for( ci = cells.begin(); ci != cells.end(); ci++ ) {
    //  (*ci)->set_fixed_size( width, -1 );
    //}
    //Gtk::Entry* entry = cbox.get_entry();
    //if (entry) entry->set_size_request(width);
  }
  //cbox.set_size_request(width, -1);

  Gtk::TreeModel::iterator ri;
  Gtk::TreeModel::Row row;
  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "100 %";
  row[columns.col_id] = 0;
  row[columns.col_value] = 1;
  cbox.set_active( ri );

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "75 %";
  row[columns.col_id] = 1;
  row[columns.col_value] = 0.75;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "50 %";
  row[columns.col_id] = 2;
  row[columns.col_value] = 0.5;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "25 %";
  row[columns.col_id] = 3;
  row[columns.col_value] = 0.25;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "10 %";
  row[columns.col_id] = 4;
  row[columns.col_value] = 0.1;

  ri = model->append();
  row = *(ri);
  row[columns.col_name] = "5 %";
  row[columns.col_id] = 5;
  row[columns.col_value] = 0.05;

  //pack_start( vbox, Gtk::PACK_SHRINK );

  cbox.signal_changed().connect(sigc::mem_fun(*this, &PercentSelector::on_value_changed));

  Gtk::Entry* entry = cbox.get_entry();
  if (entry)
  {
    entry->set_width_chars(5);

    // The Entry shall receive focus-out events.
    entry->add_events(Gdk::FOCUS_CHANGE_MASK);

    // Alternatively you can connect to m_Combo.signal_changed().
    //entry->signal_changed().connect(sigc::mem_fun(*this,
    //  &ExampleWindow::on_entry_changed) );

    entry->signal_activate().connect(sigc::mem_fun(*this,
      &PercentSelector::on_entry_activated) );

    entry->signal_focus_out_event().connect(sigc::mem_fun(*this,
      &PercentSelector::on_entry_focus_out_event) );
  }

  show_all_children();
}


void PF::PercentSelector::on_value_changed()
{
  int active_id = -1;
  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      active_id = row[columns.col_id];
    }
  }

  std::cout<<"[PercentSelector::on_value_changed] active_id="<<active_id<<"  current_active_id="<<current_active_id<<std::endl;
  if( (active_id >= 0) && (active_id != current_active_id) ) {
    current_active_id = active_id;
    changed();
  }
}


void PF::PercentSelector::on_entry_activated()
{
  Gtk::Entry*  entry = cbox.get_entry();
  if (entry)
  {
    changed();
    get_value();
  }
}


bool PF::PercentSelector::on_entry_focus_out_event(GdkEventFocus* /* event */)
{
  Gtk::Entry*  entry = cbox.get_entry();
  if (entry)
  {
    changed();
    get_value();
    return true;
  }
  return false;
}


void PF::PercentSelector::get_value()
{
  if( !get_prop() ) return;
  double val;
  get_prop()->get(val);

  val *= 100;
  std::ostringstream ostr;
  ostr << val << " %";
  Gtk::Entry* entry = cbox.get_entry();
  if( entry ) entry->set_text( ostr.str() );
}


void PF::PercentSelector::set_value()
{
  if( !get_prop() ) return;

  std::string value;
  Gtk::Entry* entry = cbox.get_entry();
  if( entry ) value = entry->get_text();

  size_t pos = value.find("%");
  if(pos != std::string::npos) value.erase(pos, 1);

  std::istringstream istr(value);
  double ival;
  istr >> ival;
  ival /= 100;

//#ifndef NDEBUG
      std::cout << "[PercentSelector::get_value]: selected value=" << value << "  ival=" << ival << std::endl;
//#endif
  get_prop()->update(ival);

  double val;
  get_prop()->get(val);
  //#ifndef NDEBUG
        std::cout << "[PercentSelector::get_value]: val=" << val << std::endl;
  //#endif
}
