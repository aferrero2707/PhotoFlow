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
  outProfileModeSelector( this, "profile_mode", _("type: "), 1 ),
  outProfileTypeSelector( this, "profile_type", _("gamut: "), 1 ),
  outTRCTypeSelector( this, "trc_type", _("encoding: "), 1 ),
  intentSelector( this, "rendering_intent", _("intent: "), 1 ),
  clip_negative_checkbox( this, "clip_negative", _("clip negative values"), true ),
  clip_overflow_checkbox( this, "clip_overflow", _("clip overflow values"), true ),
  gamut_mapping_checkbox( this, "gamut_mapping", _("gamut mapping"), false ),
  bpcButton( this, "bpc", _("black point compensation"), false ),
  adaptationStateSlider( this, "adaptation_state", _("adapt. state"), 0, 0, 1, 0.01, 0.05, 1 ),
  gamutWarningLabel( _("gamut warning") ),
  assignButton( this, "assign", _("assign profile"), false ),
  outProfOpenButton(Gtk::Stock::OPEN)
{

  outProfileModeSelectorBox.pack_start( outProfileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfileModeSelectorBox, Gtk::PACK_SHRINK );

  //outProfileTypeSelectorBox.pack_start( outProfileTypeSelector, Gtk::PACK_SHRINK );
  //outputControlsBox.pack_start( outProfileTypeSelectorBox, Gtk::PACK_SHRINK );

  outTRCTypeSelectorBox.pack_start( outTRCTypeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outTRCTypeSelectorBox, Gtk::PACK_SHRINK );

  outProfLabel.set_text( _("working profile name:") );
  outProfVBox.pack_start( outProfLabel, Gtk::PACK_SHRINK, 2 );
  outProfVBox.pack_start( outProfFileEntry, Gtk::PACK_SHRINK, 2 );
  outProfHBox.pack_start( outProfVBox );
  outProfHBox.pack_start( outProfOpenButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfHBox, Gtk::PACK_SHRINK, 2 );

  intentSelectorBox.pack_start( intentSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( intentSelectorBox, Gtk::PACK_SHRINK, 2 );

  gamut_mapping_box.pack_end( gamut_mapping_checkbox, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( gamut_mapping_box, Gtk::PACK_SHRINK, 2 );

  clip_negative_box.pack_end( clip_negative_checkbox, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( clip_negative_box, Gtk::PACK_SHRINK, 2 );
  clip_overflow_box.pack_end( clip_overflow_checkbox, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( clip_overflow_box, Gtk::PACK_SHRINK, 2 );

  bpcButtonBox.pack_end( bpcButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( bpcButtonBox, Gtk::PACK_SHRINK, 2 );

  adaptationStateBox.pack_end( adaptationStateSlider, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( adaptationStateBox, Gtk::PACK_SHRINK, 2 );

  assignButtonBox.pack_end( assignButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( assignButtonBox, Gtk::PACK_SHRINK, 2 );

  gamutWarningButtonBox.pack_end( gamutWarningButton, Gtk::PACK_SHRINK );
  gamutWarningButtonBox.pack_end( gamutWarningLabel, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( gamutWarningButtonBox, Gtk::PACK_SHRINK, 2 );

  add_widget( outputControlsBox );


  outProfFileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &ConvertColorspaceConfigGUI::on_out_filename_changed));
  outProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
                 &ConvertColorspaceConfigGUI::on_out_button_open_clicked) );

  gamutWarningButton.signal_clicked().connect(sigc::mem_fun(*this,
                 &ConvertColorspaceConfigGUI::on_gamut_warning_toggled) );

  get_main_box().show_all_children();
}



void PF::ConvertColorspaceConfigGUI::open()
{
  PF::OpParBase* par = get_par();
  PF::ConvertColorspacePar* ccpar = dynamic_cast<PF::ConvertColorspacePar*>( par );
  if( ccpar ) {
    outProfFileEntry.set_text( ccpar->get_out_profile_name() );
  }
  OperationConfigGUI::open();
}



void PF::ConvertColorspaceConfigGUI::do_update()
{
  PF::OpParBase* par = get_par();
  PF::ConvertColorspacePar* ccpar = dynamic_cast<PF::ConvertColorspacePar*>( par );
  if( ccpar ) {
    if( ccpar->get_out_profile_type() == PF::PROF_TYPE_FROM_SETTINGS ) {
      //outProfileTypeSelectorBox.hide();
      outTRCTypeSelectorBox.hide();
      outProfHBox.hide();
    } else if( ccpar->get_out_profile_type() == PF::PROF_TYPE_FROM_DISK ) {
      //outProfileTypeSelectorBox.hide();
      outTRCTypeSelectorBox.hide();
      outProfHBox.show();
    } else {//if( ccpar->get_out_profile_mode() == PF::PROF_MODE_CUSTOM ) {
      //outProfileTypeSelectorBox.show();
      outTRCTypeSelectorBox.show();
      outProfHBox.hide();
    }

    if( ccpar->get_intent() == INTENT_ABSOLUTE_COLORIMETRIC ) {
      adaptationStateBox.show();
    } else {
      adaptationStateBox.hide();
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

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_icc_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Open clicked." << std::endl;

      last_dir = dialog.get_current_folder();
      PF::PhotoFlow::Instance().get_options().set_last_visited_icc_folder( last_dir );

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
    if( !par ) return;
    PropertyBase* prop = par->get_property( "profile_name" );
    if( !prop ) return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}



void PF::ConvertColorspaceConfigGUI::on_gamut_warning_toggled()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::ConvertColorspacePar* par =
      dynamic_cast<PF::ConvertColorspacePar*>(get_layer()->get_processor()->get_par());
    if( !par ) return;
    par->set_gamut_warning( gamutWarningButton.get_active() );

    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}
