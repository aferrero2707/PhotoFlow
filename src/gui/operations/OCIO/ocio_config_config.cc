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

#include "../../../operations/OCIO/ocio_config.hh"

#include "ocio_config_config.hh"


PF::OCIOCSSelector::OCIOCSSelector( OperationConfigGUI* dialog, std::string pn ):
      Gtk::HBox(),
      PF::PFWidget( dialog, pn ), pname(pn), property(NULL), menu(NULL)
{
  label.set_text("----");
  label.set_size_request( -1, 30 );
  frame.add( label );
  ebox.add( frame );
  vbox.pack_start(ebox);

  pack_start(vbox, Gtk::PACK_EXPAND_WIDGET, 5);

  //fill_menu();

  ebox.signal_button_release_event().
      connect( sigc::mem_fun(*this, &PF::OCIOCSSelector::my_button_release_event) );

  show_all_children();
}


void PF::OCIOCSSelector::set_config(OCIO::ConstConfigRcPtr c)
{
  config = c;
  fill_menu();
}


void PF::OCIOCSSelector::init()
{
  if( get_processor() && get_processor()->get_par() ) {
    property = get_processor()->get_par()->get_property( pname );
  }

  PF::PFWidget::init();

  return;
}


void PF::OCIOCSSelector::update( Glib::ustring csname_new )
{
  bool modified = false;
  if( csname != csname_new ) modified = true;
  std::cout<<"OCIOCSSelector::update(): csname="<<csname<<"  csname_new="<<csname_new<<std::endl;
  csname = csname_new;
  label.set_text( csname );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( csname );
  if( enabled && modified ) {
    signal_cs_changed.emit();
    changed();
  }
}


bool PF::OCIOCSSelector::my_button_release_event( GdkEventButton* event )
{
  std::cout<<"OCIOCSSelector::my_button_release_event(): enable="<<enabled<<std::endl;
  if( !enabled ) return false;
  if( menu == NULL ) return false;
  menu->popup(event->button, event->time);
  return false;
}



void PF::OCIOCSSelector::on_item_clicked(Glib::ustring csname_)
{
  update( csname_ );
  std::cout<<"OCIOCSSelector::on_item_clicked(): colorspace \""
      <<csname<<"\" selected"<<std::endl;
  label.set_text( csname );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( csname );
}


void PF::OCIOCSSelector::set_csname(Glib::ustring csname_)
{
  label.set_text( _("Unknown") );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( _("Unknown") );

  std::cout<<"OCIOCSSelector::set_csname(): csname_="<<csname_<<"  config="<<config<<std::endl;
  if( !config ) return;

  bool found = false;
  int ncs = config->getNumColorSpaces();
  for( int i = 0; i < ncs; i++ ) {
    if( csname_ == config->getColorSpaceNameByIndex(i) ) {
      found = true;
      update( csname_ );
      break;
    }
  }
}


void PF::OCIOCSSelector::fill_menu()
{
  int ncs = config->getNumColorSpaces();

  if( menu ) delete menu;
  menu = new Gtk::Menu;
  //menu.reset();
  //if( ncs > 0 ) set_csname( config->getColorSpaceNameByIndex(0) );

  std::map<Glib::ustring, std::set<Glib::ustring>> csnames;
  for( int i = 0; i < ncs; i++ ) {
    const char* cn = config->getColorSpaceNameByIndex(i);
    OCIO::ConstColorSpaceRcPtr cs = config->getColorSpace(cn);

    //int nc = cs->getNumCategories();
    //const char* cat = cs->getCategory(ic);

    const char* cat = cs->getFamily();

    csnames[Glib::ustring(cat)].insert(Glib::ustring(cn));
  }

  for (auto &p : csnames) {
    Gtk::MenuItem* item = new Gtk::MenuItem(p.first);
    menu->append( *item );
    Gtk::Menu* cmenu = new Gtk::Menu;
    item->set_submenu( *cmenu );
    for (auto &c : p.second) {
      Gtk::MenuItem* citem = new Gtk::MenuItem( c );
      cmenu->append( *citem );

      citem->signal_activate().connect(
          sigc::bind<Glib::ustring>(
              sigc::mem_fun(*this,&OCIOCSSelector::on_item_clicked), c ) );

      //citem->signal_activate().connect( sigc::mem_fun(*this, &PFWidget::changed) );
    }
  }
/*
  for( int i = 0; i < ncs; i++ ) {
    Glib::ustring c = config->getColorSpaceNameByIndex(i);
    Gtk::MenuItem* item = new Gtk::MenuItem( c );
    menu.append( *item );
    item->signal_activate().connect(
        sigc::bind<Glib::ustring>(
            sigc::mem_fun(*this,&OCIOCSSelector::on_item_clicked), c) );
  }*/
  menu->show_all();
}


void PF::OCIOCSSelector::get_value()
{
  if( !get_prop() ) return;

  PF::Property<std::string>* strprop = dynamic_cast< PF::Property<std::string>* >( get_prop() );
  if( !strprop ) return;

  std::string str = strprop->get();
  std::cout<<"OCIOCSSelector::get_value(): str="<<str<<std::endl;
  set_csname(str);
}


void PF::OCIOCSSelector::set_value()
{
  if( !get_prop() ) return;

  std::string str = csname.c_str();
  get_prop()->update(str);

  //std::cout<<"OCIOCSSelector::set_value(): camera properties set to \""
  //    <<cam_maker_name<<" / "<<cam_model_name<<"\""<<std::endl;
  //std::cout<<"OCIOCSSelector::set_value(): lens property set to \""<<lens_name<<"\""<<std::endl;
}





PF::OCIOLookSelector::OCIOLookSelector( OperationConfigGUI* dialog, std::string pn ):
      Gtk::HBox(),
      PF::PFWidget( dialog, pn ), pname(pn), property(NULL), menu(NULL)
{
  label.set_text("----");
  label.set_size_request( -1, 30 );
  frame.add( label );
  ebox.add( frame );
  vbox.pack_start(ebox);

  pack_start(vbox, Gtk::PACK_EXPAND_WIDGET, 5);

  //fill_menu();

  ebox.signal_button_release_event().
      connect( sigc::mem_fun(*this, &PF::OCIOLookSelector::my_button_release_event) );

  show_all_children();
}


void PF::OCIOLookSelector::set_config(OCIO::ConstConfigRcPtr c)
{
  config = c;
}


void PF::OCIOLookSelector::set_colorspace(std::string cs)
{
  colorspace = cs;
  fill_menu();
}


void PF::OCIOLookSelector::init()
{
  if( get_processor() && get_processor()->get_par() ) {
    property = get_processor()->get_par()->get_property( pname );
  }

  PF::PFWidget::init();

  return;
}


void PF::OCIOLookSelector::update( Glib::ustring lookname_new )
{
  bool modified = false;
  if( lookname != lookname_new ) modified = true;
  std::cout<<"OCIOLookSelector::update(): lookname="<<lookname<<"  lookname_new="<<lookname_new<<std::endl;
  lookname = lookname_new;
  label.set_text( lookname );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( lookname );
  if( enabled && modified ) {
    signal_look_changed.emit();
    changed();
  }
}


bool PF::OCIOLookSelector::my_button_release_event( GdkEventButton* event )
{
  std::cout<<"OCIOLookSelector::my_button_release_event(): enable="<<enabled<<std::endl;
  if( !enabled ) return false;
  if( menu == NULL ) return false;
  menu->popup(event->button, event->time);
  return false;
}



void PF::OCIOLookSelector::on_item_clicked(Glib::ustring lookname_)
{
  update( lookname_ );
  std::cout<<"OCIOLookSelector::on_item_clicked(): colorspace \""
      <<lookname<<"\" selected"<<std::endl;
  label.set_text( lookname );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( lookname );
}


void PF::OCIOLookSelector::set_lookname(Glib::ustring lookname_)
{
  label.set_text( _("No Look") );
  label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  label.set_tooltip_text( _("No Look") );

  std::cout<<"OCIOLookSelector::set_lookname(): lookname_="<<lookname_<<"  config="<<config<<std::endl;
  if( !config ) return;

  bool found = false;
  int nlook = config->getNumLooks();
  for( int i = 0; i < nlook; i++ ) {
    if( lookname_ == config->getLookNameByIndex(i) ) {
      found = true;
      update( lookname_ );
      break;
    }
  }
}


void PF::OCIOLookSelector::fill_menu()
{
  int nlook = config->getNumLooks();

  if( menu ) delete menu;
  menu = new Gtk::Menu;
  //menu.reset();
  //if( nlook > 0 ) set_lookname( config->getColorSpaceNameByIndex(0) );

  std::set<Glib::ustring> looknames;
  for( int i = 0; i < nlook; i++ ) {
    const char* ln = config->getLookNameByIndex(i);
    OCIO::ConstLookRcPtr look = config->getLook(ln);

    //int nc = look->getNumCategories();
    //const char* cat = look->getCategory(ic);

    std::string cs = look->getProcessSpace();
    if( cs != colorspace ) continue;

    looknames.insert(Glib::ustring(ln));
  }

  Gtk::MenuItem* item = new Gtk::MenuItem("No Look");
  menu->append( *item );

  item->signal_activate().connect(
      sigc::bind<Glib::ustring>(
          sigc::mem_fun(*this,&OCIOLookSelector::on_item_clicked), Glib::ustring("No Look") ) );

  for (auto &p : looknames) {
    Gtk::MenuItem* item = new Gtk::MenuItem(p);
    menu->append( *item );

    item->signal_activate().connect(
        sigc::bind<Glib::ustring>(
            sigc::mem_fun(*this,&OCIOLookSelector::on_item_clicked), p ) );
  }
/*
  for( int i = 0; i < nlook; i++ ) {
    Glib::ustring c = config->getColorSpaceNameByIndex(i);
    Gtk::MenuItem* item = new Gtk::MenuItem( c );
    menu.append( *item );
    item->signal_activate().connect(
        sigc::bind<Glib::ustring>(
            sigc::mem_fun(*this,&OCIOLookSelector::on_item_clicked), c) );
  }*/
  menu->show_all();
}


void PF::OCIOLookSelector::get_value()
{
  if( !get_prop() ) return;

  PF::Property<std::string>* strprop = dynamic_cast< PF::Property<std::string>* >( get_prop() );
  if( !strprop ) return;

  std::string str = strprop->get();
  std::cout<<"OCIOLookSelector::get_value(): str="<<str<<std::endl;
  set_lookname(str);
}


void PF::OCIOLookSelector::set_value()
{
  if( !get_prop() ) return;

  std::string str = lookname.c_str();
  get_prop()->update(str);

  //std::cout<<"OCIOLookSelector::set_value(): camera properties set to \""
  //    <<cam_maker_name<<" / "<<cam_model_name<<"\""<<std::endl;
  //std::cout<<"OCIOLookSelector::set_value(): lens property set to \""<<lens_name<<"\""<<std::endl;
}





PF::OCIOConfigConfigGUI::OCIOConfigConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "OCIO configuration" ),
  img_open( PF::PhotoFlow::Instance().get_icons_dir()+"/libre-folder-open.png" ),
  openButton(/*Gtk::Stock::OPEN*/),
  csin_frame(_("input colorspace")),
  csin_selector( this, "cs_in_name" ),
  in_prof_mode_selector( this, "in_profile_mode", _("gamut: "), 1, 200 ),
  in_trc_type_selector( this, "in_trc_type", _("encoding: "), 1, 200 ),
  csout_frame(_("output colorspace")),
  csout_selector( this, "cs_out_name" ),
  out_prof_mode_selector( this, "out_profile_mode", _("gamut: "), 1, 200 ),
  out_trc_type_selector( this, "out_trc_type", _("encoding: "), 1, 200 ),
  look_frame(_("Look")),
  look_selector( this, "look_name" )
{
  label.set_text( _("file name:") );
  openButton.set_image( img_open );
  fileEntry.set_width_chars(20);

  file_hbox.pack_end( openButton, Gtk::PACK_SHRINK, 0 );
  file_hbox.pack_end( fileEntry, Gtk::PACK_SHRINK, 2 );
  file_hbox.pack_end( label, Gtk::PACK_SHRINK, 0 );
  controlsBox.pack_start( file_hbox, Gtk::PACK_SHRINK, 5 );

  csin_selector.enable();
  //std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/aces_1.0.3/config.ocio";
  //csin_selector.set_config( OCIO::Config::CreateFromFile(configfile.c_str()) );

  in_prof_mode_selector_box.pack_end( in_prof_mode_selector, Gtk::PACK_SHRINK );
  in_trc_type_selector_box.pack_end( in_trc_type_selector, Gtk::PACK_SHRINK );
  csin_vbox.pack_start( in_prof_mode_selector_box, Gtk::PACK_SHRINK, 5 );
  csin_vbox.pack_start( in_trc_type_selector_box, Gtk::PACK_SHRINK, 5 );
  csin_vbox.pack_start( csin_selector, Gtk::PACK_SHRINK, 5 );
  csin_frame.add(csin_vbox);

  controlsBox.pack_start( csin_frame, Gtk::PACK_SHRINK, 5 );


  csout_selector.enable();
  out_prof_mode_selector_box.pack_end( out_prof_mode_selector, Gtk::PACK_SHRINK );
  out_trc_type_selector_box.pack_end( out_trc_type_selector, Gtk::PACK_SHRINK );
  csout_vbox.pack_start( out_prof_mode_selector_box, Gtk::PACK_SHRINK, 5 );
  csout_vbox.pack_start( out_trc_type_selector_box, Gtk::PACK_SHRINK, 5 );
  csout_vbox.pack_start( csout_selector, Gtk::PACK_SHRINK, 5 );
  csout_frame.add(csout_vbox);

  controlsBox.pack_start( csout_frame, Gtk::PACK_SHRINK, 5 );

  look_selector.enable();
  look_vbox.pack_start( look_selector, Gtk::PACK_SHRINK, 5 );
  look_frame.add(look_vbox);
  controlsBox.pack_start( look_frame, Gtk::PACK_SHRINK, 5 );
  add_widget( controlsBox );

  fileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
        &OCIOConfigConfigGUI::on_filename_changed));
  openButton.signal_clicked().connect(sigc::mem_fun(*this,
                &OCIOConfigConfigGUI::on_button_open_clicked) );
  csout_selector.signal_cs_changed.connect(
      sigc::mem_fun(*this, &OCIOConfigConfigGUI::on_colorspace_changed));
}


void PF::OCIOConfigConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::OCIOConfigPar* par =
      dynamic_cast<PF::OCIOConfigPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      fileEntry.set_text( par->get_config_file_name() );
      on_filename_changed();
    }
  }
  OperationConfigGUI::open();
}



void PF::OCIOConfigConfigGUI::on_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_image_folder();
  //if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
    {
      std::cout << "Open clicked." << std::endl;

      //last_dir = dialog.get_current_folder();
      //PF::PhotoFlow::Instance().get_options().set_last_visited_image_folder( last_dir );

      //Notice that this is a std::string, not a Glib::ustring.
      std::string filename = dialog.get_filename();
      std::cout << "File selected: " <<  filename << std::endl;
      fileEntry.set_text( filename.c_str() );
      on_filename_changed();
      break;
    }
  case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      break;
    }
  default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      break;
    }
  }
}


void PF::OCIOConfigConfigGUI::on_filename_changed()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = fileEntry.get_text();
    //std::cout<<"New image file name: "<<filename<<std::endl;
    PF::OCIOConfigPar* par =
      dynamic_cast<PF::OCIOConfigPar*>(get_layer()->get_processor()->get_par());
    if( par && !filename.empty() ) {
      par->set_config_file_name( filename );
      //std::string configfile = PF::PhotoFlow::Instance().get_data_dir() + "/ocio-configs/aces_1.0.3/config.ocio";
      csin_selector.set_config( OCIO::Config::CreateFromFile(filename.c_str()) );
      csout_selector.set_config( OCIO::Config::CreateFromFile(filename.c_str()) );
      look_selector.set_config( OCIO::Config::CreateFromFile(filename.c_str()) );
      look_selector.set_colorspace( csout_selector.get_colorspace() );

      get_layer()->set_dirty( true );
      //std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
  }
}


void PF::OCIOConfigConfigGUI::on_colorspace_changed()
{
  look_selector.set_colorspace( csout_selector.get_colorspace() );
}
