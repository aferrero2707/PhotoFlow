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

#include "../../operations/brightness_contrast.hh"

#include "imageread_config.hh"


PF::ImageReadConfigGUI::ImageReadConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Open an image" ),
  img_open( PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-folder-open.png" ),
  openButton(/*Gtk::Stock::OPEN*/)
{
  label.set_text( _("file name:") );
  openButton.set_image( img_open );
  fileEntry.set_width_chars(20);

  controlsBox.pack_start( label, Gtk::PACK_SHRINK, 0 );
  controlsBox.pack_start( fileEntry, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( openButton, Gtk::PACK_SHRINK, 0 );
  
  add_widget( controlsBox );

  fileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &ImageReadConfigGUI::on_filename_changed));
  openButton.signal_clicked().connect(sigc::mem_fun(*this,
						    &ImageReadConfigGUI::on_button_open_clicked) );
}


void PF::ImageReadConfigGUI::on_filename_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = fileEntry.get_text();
    //std::cout<<"New image file name: "<<filename<<std::endl;
    PF::ImageReaderPar* par = 
      dynamic_cast<PF::ImageReaderPar*>(get_layer()->get_processor()->get_par());
    if( par && !filename.empty() ) {
      par->set_file_name( filename );
      get_layer()->set_dirty( true );
      //std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
  }
}


void PF::ImageReadConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::ImageReaderPar* par = 
      dynamic_cast<PF::ImageReaderPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      fileEntry.set_text( par->get_file_name() );
      //brightnessAdj.set_value( par->get_brightness() );
      //contrastAdj.set_value( par->get_contrast() );
    }
  }
  OperationConfigGUI::open();
}



void PF::ImageReadConfigGUI::on_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
				Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Open clicked." << std::endl;

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
