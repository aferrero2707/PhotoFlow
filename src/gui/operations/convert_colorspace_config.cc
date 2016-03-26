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

#include "../../operations/convert_colorspace.hh"

#include "convert_colorspace_config.hh"



PF::ConvertColorspaceConfigGUI::ConvertColorspaceConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Convert to profile" ),
  outProfileModeSelector( this, "profile_mode", "working profile: ", 1 ),
  outTRCModeSelector( this, "trc_mode", _("encoding: "), 1 ),
  assignButton( this, "assign", _("assign profile"), false ),
  outProfOpenButton(Gtk::Stock::OPEN)
{

  outProfileModeSelectorBox.pack_start( outProfileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfileModeSelectorBox, Gtk::PACK_SHRINK );

  outTRCModeSelectorBox.pack_start( outTRCModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outTRCModeSelectorBox, Gtk::PACK_SHRINK );

  outProfLabel.set_text( _("working profile name:") );
  outProfVBox.pack_start( outProfLabel );
  outProfVBox.pack_start( outProfFileEntry );
  outProfHBox.pack_start( outProfVBox );
  outProfHBox.pack_start( outProfOpenButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfHBox );

  assignButtonBox.pack_start( assignButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( assignButtonBox, Gtk::PACK_SHRINK );

  add_widget( outputControlsBox );


  outProfFileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &ConvertColorspaceConfigGUI::on_out_filename_changed));
  outProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &ConvertColorspaceConfigGUI::on_out_button_open_clicked) );

  get_main_box().show_all_children();
}



void PF::ConvertColorspaceConfigGUI::do_update()
{
  PF::OpParBase* par = get_par();
  PF::ConvertColorspacePar* ccpar = dynamic_cast<PF::ConvertColorspacePar*>( par );
  if( ccpar ) {
    if( ccpar->get_out_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      outTRCModeSelectorBox.hide();
    } else {
      outTRCModeSelectorBox.show();
    }

    if( ccpar->get_out_profile_mode() == PF::OUT_PROF_CUSTOM ) {
      outProfHBox.show();
    } else {
      outProfHBox.hide();
    }
  }
  OperationConfigGUI::do_update();
}


void PF::ConvertColorspaceConfigGUI::on_out_button_open_clicked()
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
      outProfFileEntry.set_text( filename.c_str() );
      on_out_filename_changed();
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



void PF::ConvertColorspaceConfigGUI::on_out_filename_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = outProfFileEntry.get_text();
    if( filename.empty() )
      return;
    std::cout<<"New output profile name: "<<filename<<std::endl;
    PF::ConvertColorspacePar* par = 
      dynamic_cast<PF::ConvertColorspacePar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "profile_name" );
    if( !prop ) 
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}
