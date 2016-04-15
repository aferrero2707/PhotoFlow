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
openButton(Gtk::Stock::OPEN),
inProfileModeSelector( this, "in_profile_mode", "input profile: ", 1 ),
inTRCModeSelector( this, "in_trc_mode", _("encoding: "), 1 ),
outProfileModeSelector( this, "out_profile_mode", "working profile: ", 1 ),
outTRCModeSelector( this, "out_trc_mode", _("encoding: "), 1 ),
inProfOpenButton(Gtk::Stock::OPEN),
outProfOpenButton(Gtk::Stock::OPEN)
{
  label.set_text( "file name:" );

  controlsBox.pack_start( label );
  controlsBox.pack_start( fileEntry );
  controlsBox.pack_start( openButton );
  controlsBox.set_size_request(200,-1);
  
  outputControlsBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  spacing1.set_size_request(0,20);
  outputControlsBox.pack_start( spacing1, Gtk::PACK_SHRINK );

  inProfileModeSelectorBox.pack_start( inProfileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( inProfileModeSelectorBox, Gtk::PACK_SHRINK );
  inTRCModeSelectorBox.pack_start( inTRCModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( inTRCModeSelectorBox, Gtk::PACK_SHRINK );

  inProfLabel.set_text( _("input profile name:") );
  inProfVBox.pack_start( inProfLabel );
  //inProfFileEntry.set_width_chars(20);
  inProfVBox.pack_start( inProfFileEntry );
  inProfHBox.pack_start( inProfVBox, Gtk::PACK_EXPAND_WIDGET );
  inProfHBox.pack_start( inProfOpenButton, Gtk::PACK_SHRINK );
  inProfHBox.set_size_request(250,-1);
  outputControlsBox.pack_start( inProfHBox, Gtk::PACK_SHRINK );

  spacing2.set_size_request(0,20);
  outputControlsBox.pack_start( spacing2, Gtk::PACK_SHRINK );

  outProfileModeSelectorBox.pack_start( outProfileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfileModeSelectorBox, Gtk::PACK_SHRINK );
  outTRCModeSelectorBox.pack_start( outTRCModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outTRCModeSelectorBox, Gtk::PACK_SHRINK );

  outProfLabel.set_text( _("working profile name:") );
  outProfVBox.pack_start( outProfLabel );
  outProfFileEntry.set_width_chars(25);
  outProfVBox.pack_start( outProfFileEntry );
  outProfHBox.pack_start( outProfVBox );
  outProfHBox.pack_start( outProfOpenButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfHBox );

  add_widget( outputControlsBox );

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



void PF::ImageReadConfigGUI::do_update()
{
  PF::OpParBase* par = get_par();
  PF::ImageReaderPar* irpar = dynamic_cast<PF::ImageReaderPar*>( par );
  if( irpar ) {
    if( irpar->get_in_profile_mode() == PF::OUT_PROF_EMBEDDED ||
        irpar->get_in_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      inTRCModeSelectorBox.hide();
    } else {
      inTRCModeSelectorBox.show();
    }

    if( irpar->get_in_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      inProfHBox.show();
    } else {
      inProfHBox.hide();
    }

    if( irpar->get_out_profile_mode() == PF::OUT_PROF_EMBEDDED ||
        irpar->get_out_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      outTRCModeSelectorBox.hide();
    } else {
      outTRCModeSelectorBox.show();
    }

    if( irpar->get_out_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      outProfHBox.show();
    } else {
      outProfHBox.hide();
    }
  }
  OperationConfigGUI::do_update();
}


void PF::ImageReadConfigGUI::on_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
				Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_image_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Open clicked." << std::endl;

      last_dir = dialog.get_current_folder();
      PF::PhotoFlow::Instance().get_options().set_last_visited_image_folder( last_dir );

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
