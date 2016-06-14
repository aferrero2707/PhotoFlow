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

#include "emulate_film_user_defined_config.hh"


PF::GmicEmulateFilmUserDefinedConfigGUI::GmicEmulateFilmUserDefinedConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Apply LUT (G'MIC)"  ),
  openButton(Gtk::Stock::OPEN),
  prop_opacity_slider( this, "opacity", "opacity", 1, 0, 1, .01, .1, 1),
  prop_gamma_slider( this, "gamma", "gamma", 0, -1.2, 1.2, .02, .2, 1),
  prop_contrast_slider( this, "contrast", "contrast", 1, 0, 4, .04, .4, 1),
  prop_brightness_slider( this, "brightness", "brightness", 0, -255, 255, 5.10, 51.0, 1),
  prop_hue_slider( this, "hue", "hue", 0, -180, 180, 3.60, 36.0, 1),
  prop_saturation_slider( this, "saturation", "saturation", 0, -1, 1, .02, .2, 1),
  prop_post_normalize_slider( this, "post_normalize", "post_normalize", 0, 0, 1, 1, 5, 1)
{
  fileEntryBox.pack_start( label );
  fileEntryBox.pack_start( fileEntry );
  fileEntryBox.pack_start( openButton );
  //controlsBox.set_size_request(200,-1);

  controlsBox.pack_start( fileEntryBox, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( prop_opacity_slider );
  //controlsBox.pack_start( prop_gamma_slider );
  //controlsBox.pack_start( prop_contrast_slider );
  //controlsBox.pack_start( prop_brightness_slider );
  //controlsBox.pack_start( prop_hue_slider );
  //controlsBox.pack_start( prop_saturation_slider );
  //controlsBox.pack_start( prop_post_normalize_slider );
  
  add_widget( controlsBox );

  fileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
        &GmicEmulateFilmUserDefinedConfigGUI::on_filename_changed));
  openButton.signal_clicked().connect(sigc::mem_fun(*this,
                &GmicEmulateFilmUserDefinedConfigGUI::on_button_open_clicked) );
}



void PF::GmicEmulateFilmUserDefinedConfigGUI::on_filename_changed()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = fileEntry.get_text();
    //std::cout<<"New image file name: "<<filename<<std::endl;
    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    if( par && !filename.empty() ) {
      PF::PropertyBase* prop = par->get_property( "filename" );
      if( prop ) {
        std::cout<<"GmicEmulateFilmUserDefinedConfigGUI::on_filename_changed(): updating filename property..."<<std::endl;
        prop->update( filename );
        std::cout<<"... done."<<std::endl;
        get_layer()->set_dirty( true );
        //std::cout<<"  updating image"<<std::endl;
        get_layer()->get_image()->update();
      }
    }
  }
}



void PF::GmicEmulateFilmUserDefinedConfigGUI::on_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a LUT file",
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



void PF::GmicEmulateFilmUserDefinedConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    if( par ) {
      PF::PropertyBase* prop = par->get_property( "filename" );
      std::string filename = prop->get_str();
      if( !filename.empty() )
        fileEntry.set_text( filename );
    }
  }
  OperationConfigGUI::open();
}
