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

#include "operation_config_dialog.hh"

#include "../base/new_operation.hh"

#include "../gui/operations/raw_developer_config.hh"
#include "../gui/operations/brightness_contrast_config.hh"
#include "../gui/operations/hue_saturation_config.hh"
#include "../gui/operations/imageread_config.hh"
#include "../gui/operations/vips_operation_config.hh"
#include "../gui/operations/clone_config.hh"
#include "../gui/operations/crop_config.hh"
#include "../gui/operations/scale_config.hh"
#include "../gui/operations/gradient_config.hh"
#include "../gui/operations/uniform_config.hh"
#include "../gui/operations/curves_config.hh"
#include "../gui/operations/channel_mixer_config.hh"
#include "../gui/operations/gaussblur_config.hh"
#include "../gui/operations/denoise_config.hh"
#include "../gui/operations/desaturate_config.hh"
#include "../gui/operations/sharpen_config.hh"
#include "../gui/operations/draw_config.hh"
#include "../gui/operations/clone_stamp_config.hh"
#include "../gui/operations/convert_colorspace_config.hh"
#include "../gui/operations/lensfun_config.hh"

#include "operations/gmic/new_gmic_operation_config.hh"

static bool is_blend_mode_row_separator(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::iterator& iter)
{
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      //Glib::ustring value = row[2];
      //if( value > 1000 ) return true;
    }
  }
  return false;
}


static gboolean dialog_update_cb (PF::OperationConfigDialog * dialog)
{
  if( dialog ) 
    dialog->do_update();
  return FALSE;
}


PF::OperationConfigDialog::OperationConfigDialog(PF::Layer* layer, const Glib::ustring& title, bool chsel):
  PF::OperationConfigUI(layer),
#ifdef GTKMM_2
  Gtk::Dialog(title, false, false),
#endif
#ifdef GTKMM_3
  Gtk::Dialog(title, false),
#endif
  //intensityAdj( 100, 0, 100, 1, 10, 0),
  //opacityAdj( 100, 0, 100, 1, 10, 0),
  //intensityScale(intensityAdj),
  //opacityScale(opacityAdj),
#ifdef GTKMM_3
  mainBox(Gtk::ORIENTATION_VERTICAL),
  mainHBox(Gtk::ORIENTATION_HORIZONTAL),
  chselBox(Gtk::ORIENTATION_HORIZONTAL),
  topBox(Gtk::ORIENTATION_VERTICAL),
  nameBox(Gtk::ORIENTATION_HORIZONTAL),
  controlsBox(Gtk::ORIENTATION_HORIZONTAL),
  controlsBoxLeft(Gtk::ORIENTATION_VERTICAL),
  controlsBoxRight(Gtk::ORIENTATION_VERTICAL),
#endif
  intensitySlider( this, "intensity", "Intensity", 100, 0, 100, 1, 10, 100),
  opacitySlider( this, layer->get_blender(), "opacity", "Opacity", 100, 0, 100, 1, 10, 100),
  blendSelector( this, layer->get_blender(), "blend_mode", "Blend mode: ", PF_BLEND_PASSTHROUGH ),
  shift_x( this, layer->get_blender(), "shift_x", "X shift", 0, -1000000, 1000000, 1, 10, 1),
  shift_y( this, layer->get_blender(), "shift_y", "Y shift", 0, -1000000, 1000000, 1, 10, 1),
  has_ch_sel(chsel),
  greychSelector( this, "grey_target_channel", "Target channel: ", -1 ),
  rgbchSelector( this, "rgb_target_channel", "Target channel: ", -1 ),
  labchSelector( this, "lab_target_channel", "Target channel: ", -1 ),
  cmykchSelector( this, "cmyk_target_channel", "Target channel:", -1 )
{
  //set_keep_above(true);
  add_button("OK",1);
  add_button("Cancel",0);

  signal_response().connect(sigc::mem_fun(*this,
					  &OperationConfigDialog::on_button_clicked) );

  OpParBase* par = NULL;
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() )
    par = get_layer()->get_processor()->get_par();

  lname.set_text( "name:" );
  nameBox.pack_start( lname, Gtk::PACK_SHRINK );

  nameEntry.set_text( "New Layer" );
  nameBox.pack_start( nameEntry, Gtk::PACK_SHRINK );

  //blendSelector.set_row_separator_func( is_blend_mode_row_separator );
  if(par && par->has_opacity() )
    nameBox.pack_end( blendSelector );

  topBox.pack_start( nameBox, Gtk::PACK_SHRINK );

  shiftBox.pack_start( shift_x, Gtk::PACK_EXPAND_PADDING, 10 );
  shiftBox.pack_start( shift_y, Gtk::PACK_EXPAND_PADDING, 10 );

  if(par && par->has_intensity() )
    controlsBoxLeft.pack_start( intensitySlider, Gtk::PACK_EXPAND_PADDING, 10 );
  if(par && par->has_opacity() ) {
    controlsBoxLeft.pack_start( opacitySlider, Gtk::PACK_EXPAND_PADDING, 10 );
    controlsBoxLeft.pack_start( shiftBox, Gtk::PACK_EXPAND_PADDING, 10 );
  }
  controlsBox.pack_start( controlsBoxLeft, Gtk::PACK_SHRINK );
  topBox.pack_start( controlsBox );

  //greychSelector.init(); 
  //chselBox.pack_start( greychSelector, Gtk::PACK_SHRINK );
  //rgbchSelector.init(); 
  //chselBox.pack_start( rgbchSelector, Gtk::PACK_SHRINK );
  //labchSelector.init(); 
  //chselBox.pack_start( labchSelector, Gtk::PACK_SHRINK );
  //cmykchSelector.init(); 
  //chselBox.pack_start( cmykchSelector, Gtk::PACK_SHRINK );

  if(has_ch_sel && par && par->has_opacity() )
    topBox.pack_start( chselBox );

  //topFrame.set_label( "layer options" );
  topFrame.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);

  topFrame.add( topBox );

  get_vbox()->pack_start( topFrame );

  mainHBox.pack_start( mainBox, Gtk::PACK_SHRINK, false, false );

  get_vbox()->pack_start( mainHBox, Gtk::PACK_SHRINK, false, false );

  signal_focus_in_event().connect( sigc::mem_fun(*this,
						 &OperationConfigDialog::focus_in_cb) );
  signal_focus_out_event().connect( sigc::mem_fun(*this,
					     &OperationConfigDialog::focus_out_cb) );

  /*
  intensityAdj.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &OperationConfigDialog::on_intensity_value_changed));

  opacityAdj.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &OperationConfigDialog::on_opacity_value_changed));
  */

  // nameEntry.show();
  // nameBox.show();
  // topBox.show();
  // mainBox.show();

  /*
  add_control( &intensitySlider );
  add_control( &opacitySlider );
  add_control( &blendSelector );
  add_control( &greychSelector );
  add_control( &rgbchSelector );
  add_control( &labchSelector );
  add_control( &cmykchSelector );
  */

  show_all_children();
}


PF::OperationConfigDialog::~OperationConfigDialog()
{
}


void PF::OperationConfigDialog::add_widget( Gtk::Widget& widget )
{
  mainBox.pack_start( widget );

  show_all_children();
}


void PF::OperationConfigDialog::init()
{
  for( int i = 0; i < controls.size(); i++ )
    controls[i]->init();
}


void PF::OperationConfigDialog::open()
{
  init();
  /*
  for( int i = 0; i < controls.size(); i++ )
    controls[i]->init();
  */

  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    nameEntry.set_text( get_layer()->get_name().c_str() );
    
    values_save.clear();
    get_layer()->get_processor()->get_par()->save_properties( values_save );
#ifndef NDEBUG
    std::cout<<"Saved property values:"<<std::endl;
    for( std::list<std::string>::iterator i = values_save.begin();
	 i != values_save.end(); i++ ) {
      std::cout<<"  "<<(*i)<<std::endl;
    }
#endif
  }
  PF::OperationConfigUI::open();
  show_all();
  //show();
}



void PF::OperationConfigDialog::on_map()
{
  Gtk::Dialog::on_map();
}


void PF::OperationConfigDialog::on_unmap()
{
  Gtk::Dialog::on_unmap();
  disable_editing();
}


void PF::OperationConfigDialog::enable_editing()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    get_layer()->get_processor()->get_par()->set_editing_flag( true );
    std::cout<<"  Editing flag set to true"<<std::endl;
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}


void PF::OperationConfigDialog::disable_editing()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    get_layer()->get_processor()->get_par()->set_editing_flag( false );
    std::cout<<"  Editing flag set to false"<<std::endl;
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}


void PF::OperationConfigDialog::reset_ch_selector()
{
  if( greychSelector.get_parent() == &chselBox )
    chselBox.remove( greychSelector );
  if( rgbchSelector.get_parent() == &chselBox )
    chselBox.remove( rgbchSelector );
  if( labchSelector.get_parent() == &chselBox )
    chselBox.remove( labchSelector );
  if( cmykchSelector.get_parent() == &chselBox )
    chselBox.remove( cmykchSelector );
}


void PF::OperationConfigDialog::do_update()
{
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
#ifndef NDEBUG
    std::cout<<"OperationConfigDialog::update() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif
    /*
    if( greychSelector.get_parent() == &chselBox )
      chselBox.remove( greychSelector );
    if( rgbchSelector.get_parent() == &chselBox )
      chselBox.remove( rgbchSelector );
    if( labchSelector.get_parent() == &chselBox )
      chselBox.remove( labchSelector );
    if( cmykchSelector.get_parent() == &chselBox )
      chselBox.remove( cmykchSelector );
    */
    //greychSelector.hide();
    //rgbchSelector.hide();
    //labchSelector.hide();
    //cmykchSelector.hide();
    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::colorspace_t cs = PF::convert_colorspace( par->get_interpretation() );
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      if( greychSelector.get_parent() != &chselBox ) {
        reset_ch_selector();
        chselBox.pack_start( greychSelector, Gtk::PACK_SHRINK );
        greychSelector.show();
      }
      break;
    case PF_COLORSPACE_RGB:
      if( rgbchSelector.get_parent() != &chselBox ) {
        reset_ch_selector();
        chselBox.pack_start( rgbchSelector, Gtk::PACK_SHRINK );
        rgbchSelector.show();
      }
      break;
    case PF_COLORSPACE_LAB:
      if( labchSelector.get_parent() != &chselBox ) {
        reset_ch_selector();
        chselBox.pack_start( labchSelector, Gtk::PACK_SHRINK );
        labchSelector.show();
      }
      break;
    case PF_COLORSPACE_CMYK:
      if( cmykchSelector.get_parent() != &chselBox ) {
        reset_ch_selector();
        chselBox.pack_start( cmykchSelector, Gtk::PACK_SHRINK );
        cmykchSelector.show();
      }
      break;
    default:
      break;
    }
  }

  /*
  // force our program to redraw the entire clock.
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win) {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),
		     get_allocation().get_height());
    win->invalidate_rect(r, false);
  }
  queue_draw();
  */
}



void PF::OperationConfigDialog::update()
{
  //std::cout<<"PF::OperationConfigDialog::update() called."<<std::endl;
  gdk_threads_add_idle ((GSourceFunc) dialog_update_cb, this);
}



void PF::OperationConfigDialog::update_properties()
{
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    controls[i]->set_value();
  }
}


void PF::OperationConfigDialog::on_button_clicked(int id)
{
  switch(id) {
  case 0:
    if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
      PF::OpParBase* par = get_layer()->get_processor()->get_par();
      std::cout<<"  restoring original values"<<std::endl;
      par->restore_properties( values_save );
      init();
      get_layer()->set_dirty( true );
      std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
    //hide_all();
    hide();
    break;
  case 1:
    if( get_layer() ) 
      get_layer()->set_name( nameEntry.get_text().c_str() );
    //hide_all();
    hide();
    break;
  }
}


/*
void PF::OperationConfigDialog::on_intensity_value_changed()
{
  double val = intensityAdj.get_value();
  std::cout<<"New intensity value: "<<val<<std::endl;
  if( get_image() && get_layer() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    get_layer()->get_processor()->get_par()->set_intensity( val/100. );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_image()->update();
  }
}


void PF::OperationConfigDialog::on_opacity_value_changed()
{
  double val = opacityAdj.get_value();
  std::cout<<"New opacity value: "<<val<<std::endl;
  if( get_image() && get_layer() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    get_layer()->get_processor()->get_par()->set_opacity( val/100. );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_image()->update();
  }
}
*/



PF::ProcessorBase* PF::new_operation_with_gui( std::string op_type, PF::Layer* current_layer )
{
  if( !current_layer ) return NULL;
  PF::ProcessorBase* processor = PF::new_operation( op_type, current_layer );
  if( !processor ) return NULL;

  PF::OperationConfigDialog* dialog = NULL;

  if( op_type == "imageread" ) { 

    dialog = new PF::ImageReadConfigDialog( current_layer );

  } else if( op_type == "raw_loader" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "RAW loader" );

  } else if( op_type == "raw_developer" ) {

    dialog = new PF::RawDeveloperConfigDialog( current_layer );

  } else if( op_type == "raw_output" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "RAW output" );

  } else if( op_type == "buffer" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "Buffer" );

  } else if( op_type == "blender" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "Layer Group" );

  } else if( op_type == "clone" ) {

    dialog = new PF::CloneConfigDialog( current_layer );

  } else if( op_type == "crop" ) {

    dialog = new PF::CropConfigDialog( current_layer );

  } else if( op_type == "scale" ) {

    dialog = new PF::ScaleConfigDialog( current_layer );

  } else if( op_type == "invert" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "Invert Image" );

  } else if( op_type == "desaturate" ) {

    dialog = new PF::DesaturateConfigDialog( current_layer );

  } else if( op_type == "uniform" ) {

    dialog = new PF::UniformConfigDialog( current_layer );

  } else if( op_type == "gradient" ) {

    dialog = new PF::GradientConfigDialog( current_layer );

  } else if( op_type == "brightness_contrast" ) {

    dialog = new PF::BrightnessContrastConfigDialog( current_layer );

  } else if( op_type == "hue_saturation" ) {

    dialog = new PF::HueSaturationConfigDialog( current_layer );

  } else if( op_type == "curves" ) {
      
    dialog = new PF::CurvesConfigDialog( current_layer );

  } else if( op_type == "channel_mixer" ) {
      
    dialog = new PF::ChannelMixerConfigDialog( current_layer );

  } else if( op_type == "gaussblur" ) {
      
    dialog = new PF::GaussBlurConfigDialog( current_layer );

  } else if( op_type == "denoise" ) {
      
    dialog = new PF::DenoiseConfigDialog( current_layer );

  } else if( op_type == "sharpen" ) {
      
    dialog = new PF::SharpenConfigDialog( current_layer );

  } else if( op_type == "convert2lab" ) {

    dialog = new PF::OperationConfigDialog( current_layer, "Convert to Lab colororspace" );

  } else if( op_type == "convert_colorspace" ) {

    dialog = new PF::ConvertColorspaceConfigDialog( current_layer );

  } else if( op_type == "draw" ) {

    dialog = new PF::DrawConfigDialog( current_layer );

  } else if( op_type == "clone_stamp" ) {

    dialog = new PF::CloneStampConfigDialog( current_layer );

  } else if( op_type == "lensfun" ) {

    dialog = new PF::LensFunConfigDialog( current_layer );

  }

  if( !dialog ) {
    // Try with G'MIC
    dialog = PF::new_gmic_operation_config( op_type, current_layer );
  }
  /*
  } else { // it must be a VIPS operation...

    int pos = op_type.find( "vips-" );
    if( pos != 0 ) return NULL;
    std::string vips_op_type;
    vips_op_type.append(op_type.begin()+5,op_type.end());

    PF::VipsOperationConfigDialog* vips_config = 
      new PF::VipsOperationConfigDialog( current_layer );
    vips_config->set_op( vips_op_type.c_str() );
    dialog = vips_config;
  }
  */

  if( processor ) {
    PF::OpParBase* current_op = processor->get_par();
    if( current_op && dialog )
      current_op->set_config_ui( dialog );
  }

  return processor;
}
