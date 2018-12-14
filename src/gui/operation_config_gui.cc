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

#include "operation_config_gui.hh"
#include "imageeditor.hh"

#include "../base/new_operation.hh"
#include "../base/photoflow.hh"

#include "../legacy/gui/operations/raw_developer_config.hh"
#include "../legacy/gui/operations/hue_saturation_config.hh"
#include "../legacy/gui/operations/brightness_contrast_config.hh"
#include "operations/raw_developer_config.hh"
#include "operations/clip_config.hh"
#include "operations/white_balance_config.hh"
#include "operations/levels_config.hh"
#include "operations/noise_generator_config.hh"
#include "operations/basic_adjustments_config.hh"
#include "operations/color_correction_config.hh"
#include "operations/hsl_mask_config.hh"
#include "operations/imageread_config.hh"
#include "operations/raw_loader_config.hh"
#include "operations/vips_operation_config.hh"
#include "operations/clone_config.hh"
#include "operations/crop_config.hh"
#include "operations/scale_config.hh"
#include "operations/perspective_config.hh"
#include "operations/gradient_config.hh"
#include "operations/path_mask_config.hh"
#include "operations/uniform_config.hh"
#include "operations/curves_config.hh"
#include "operations/channel_mixer_config.hh"
#include "operations/gaussblur_config.hh"
#include "operations/blur_bilateral_config.hh"
#include "operations/denoise_config.hh"
#include "operations/desaturate_config.hh"
#include "operations/dynamic_range_compressor_config.hh"
#include "operations/sharpen_config.hh"
#include "operations/draw_config.hh"
#include "operations/clone_stamp_config.hh"
#include "operations/convert_colorspace_config.hh"
#include "operations/lensfun_config.hh"
#include "operations/volume_config.hh"
#include "operations/threshold_config.hh"
#include "operations/shadows_highlights_config.hh"
#include "operations/defringe_config.hh"
#include "operations/guided_filter_config.hh"
#include "operations/split_details_config.hh"
#include "operations/wavdec_config.hh"
#include "operations/tone_mapping_config.hh"
#include "operations/local_contrast_config.hh"

#include "operations/gmic/new_gmic_operation_config.hh"


static gboolean config_update_cb (PF::OperationConfigGUI * config)
{
  //std::cout<<"config_update_cb() called."<<std::endl;
  if( config ) {
    config->do_update();
    config->update_notify();
    //std::cout<<"config_update_cb(): update notified."<<std::endl;
  }
  return FALSE;
}


PF::OperationConfigGUI::OperationConfigGUI(PF::Layer* layer, const Glib::ustring& title, bool chsel ):
  PF::OperationConfigUI(layer),
  editor( NULL ),
  blendSelector( this, layer->get_blender(), "blend_mode", "", PF_BLEND_PASSTHROUGH, 55 ),
  blendSelector2( this, layer->get_blender(), "blend_mode", "", PF_BLEND_PASSTHROUGH, 55 ),
  blendSelectorMask( this, layer->get_blender(), "mask_blend_mode", "", PF_BLEND_NORMAL, 55 ),
  blendSelectorMask2( this, layer->get_blender(), "mask_blend_mode", "", PF_BLEND_NORMAL, 55 ),
  intensitySlider( this, "intensity", _("Intensity"), 100, 0, 100, 1, 10, 100),
  intensitySlider2( this, "intensity", _("Intensity"), 100, 0, 100, 1, 10, 100),
  opacitySlider( this, layer->get_blender(), "opacity", "", 100, 0, 100, 1, 10, 100, 100),
  opacitySlider2( this, layer->get_blender(), "opacity", "", 100, 0, 100, 1, 10, 100),
  imap_enabled_box( this, "mask_enabled", _("Enable mask"), true),
  omap_enabled_box( this, layer->get_blender(), "mask_enabled", _("Enable mask"), true),
  test_padding_enable_box( this, "enable_padding", _("enable padding"), false),
  shift_x( this, layer->get_blender(), "shift_x", _("X shift "), 0, -1000000, 1000000, 1, 10, 1),
  shift_y( this, layer->get_blender(), "shift_y", _("Y shift "), 0, -1000000, 1000000, 1, 10, 1),
  has_ch_sel(chsel),
  greychSelector( this, "grey_target_channel", _("Target channel: "), -1 ),
  rgbchSelector( this, "rgb_target_channel", _("Target channel: "), -1 ),
  labchSelector( this, "lab_target_channel", _("Target channel: "), -1 ),
  cmykchSelector( this, "cmyk_target_channel", _("Target channel:"), -1 ),
  input_source_visible(true),
  input_source_expander( _("input source") ),
  input_source_checkbox( this, "previous_layer_is_input", _("process previous layer"), true),
  layer_list( this, "Layer name:"),
  sourceSelector( this, "source_channel", "Source channel: ", 1 ),
  previewButton(_("preview")),
  dialog( NULL ),
  frame( NULL ),
  frame_visible(PF::PhotoFlow::Instance().get_icons_dir()+"/visible_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/visible_inactive.png",true),
  //frame_preview(PF::PhotoFlow::Instance().get_icons_dir()+"/preview_active.png",
  //    PF::PhotoFlow::Instance().get_icons_dir()+"/preview_inactive.png"),
  frame_mask(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_inactive.png",true),
  frame_mask2(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_inactive.png",true),
  frame_edit(PF::PhotoFlow::Instance().get_icons_dir()+"/edit_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/edit_inactive.png",true,false),
  frame_edit2(PF::PhotoFlow::Instance().get_icons_dir()+"/edit_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/edit_inactive.png",true,false),
  frame_sticky(PF::PhotoFlow::Instance().get_icons_dir()+"/pushpin_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/pushpin_inactive.png",true,false),
  frame_sticky2(PF::PhotoFlow::Instance().get_icons_dir()+"/pushpin_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/pushpin_inactive.png",true,false),
  frame_undo(PF::PhotoFlow::Instance().get_icons_dir()+"/undo_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/undo_inactive.png"),
  frame_redo(PF::PhotoFlow::Instance().get_icons_dir()+"/redo_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/redo_inactive.png"),
  frame_reset(PF::PhotoFlow::Instance().get_icons_dir()+"/reset_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/reset_inactive.png"),
  frame_help(PF::PhotoFlow::Instance().get_icons_dir()+"/libre-info.png",PF::PhotoFlow::Instance().get_icons_dir()+"/libre-info-pressed.png"),
  frame_help2(PF::PhotoFlow::Instance().get_icons_dir()+"/libre-info.png",PF::PhotoFlow::Instance().get_icons_dir()+"/libre-info-pressed.png"),
  frame_close(PF::PhotoFlow::Instance().get_icons_dir()+"/close_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/close_inactive.png"),
  frame_expander(PF::PhotoFlow::Instance().get_icons_dir()+"/expand.png",PF::PhotoFlow::Instance().get_icons_dir()+"/collapse.png",true),
  expert_ctrls_expander(_("more"))
{
  vips_semaphore_init( &update_done_sem, 0, (char*)"update_done_sem" );


  Glib::ustring dataPath = PF::PhotoFlow::Instance().get_data_dir();
  Glib::ustring iconsPath = dataPath + "/icons";

  //frame_expander.set_images( iconsPath + "/collapse.png", iconsPath + "/expand.png" );
  //frame_visible.set_images( iconsPath + "/visible_active.png", iconsPath + "/visible_inactive.png" );
  //frame_visible.show_all_children();

  OpParBase* par = get_par();

  controls_box.set_spacing(5);

  frame = new Gtk::Frame;
  //frame->set_size_request(200,-1);
  frame->set_shadow_type( Gtk::SHADOW_NONE );

  controls_frame.set_shadow_type( Gtk::SHADOW_NONE );

  //frame_hbox.pack_start( frame_vbox, Gtk::PACK_EXPAND_WIDGET );
  //frame->add( frame_hbox );
  frame->add( frame_vbox );

  frame_vbox.set_spacing(4);
  frame_vbox.pack_start( frame_top_box_1, Gtk::PACK_SHRINK, 0 );
  //frame_vbox.pack_start( controls_frame, Gtk::PACK_SHRINK, 0 );
  //frame_vbox.pack_start( hline, Gtk::PACK_SHRINK, 5 );

  //blendSelector.set_size_request( -1, 22 );

  //map_buttons.add_button( new PF::ToggleImageButton(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_blend_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_blend_inactive.png",false,true) );
  //map_buttons.add_button( new PF::ToggleImageButton(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_union_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_union_inactive.png",false,false) );
  //map_buttons.add_button( new PF::ToggleImageButton(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_intersection_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_intersection_inactive.png",false,false) );
  //map_buttons.add_button( new PF::ToggleImageButton(PF::PhotoFlow::Instance().get_icons_dir()+"/mask_exclusion_active.png",PF::PhotoFlow::Instance().get_icons_dir()+"/mask_exclusion_inactive.png",false,false) );

  //frame_top_box_1_1.pack_start( map_buttons, Gtk::PACK_SHRINK );

  //frame_box_top.set_spacing(5);

  /********************************************************************************
   * Layer control buttons - main frame
   */
  nameEntry.set_has_frame( false );
  frame_top_buttons_hbox.pack_start( frame_mask, Gtk::PACK_SHRINK, 5 );
  frame_top_buttons_hbox.pack_start( frame_sticky, Gtk::PACK_SHRINK, 5 );
  frame_top_buttons_hbox.pack_start( frame_edit, Gtk::PACK_SHRINK, 5 );
  //frame_top_box_1_1.pack_start( frame_undo, Gtk::PACK_SHRINK, 5 );
  //frame_top_box_1_1.pack_start( frame_redo, Gtk::PACK_SHRINK, 5 );
  frame_top_buttons_hbox.pack_start( frame_reset, Gtk::PACK_SHRINK, 5 );
  frame_top_buttons_hbox.pack_start( frame_help, Gtk::PACK_SHRINK, 5 );

  frame_top_buttons_alignment.add( frame_top_buttons_hbox );
  frame_top_buttons_alignment.set( 0, 0.5, 0, 0 );

  frame_top_box_1_1.pack_start( frame_top_buttons_alignment, Gtk::PACK_SHRINK );

  /********************************************************************************
   * Layer control buttons - aux frame
   */
  nameEntry2.set_has_frame( false );
  aux_top_buttons_hbox.pack_start( frame_mask2, Gtk::PACK_SHRINK, 5 );
  aux_top_buttons_hbox.pack_start( frame_sticky2, Gtk::PACK_SHRINK, 5 );
  //aux_top_buttons_hbox.pack_start( frame_reset2, Gtk::PACK_SHRINK, 5 );
  aux_top_buttons_hbox.pack_start( frame_help2, Gtk::PACK_SHRINK, 5 );

  aux_top_buttons_alignment.add( aux_top_buttons_hbox );
  aux_top_buttons_alignment.set( 0, 0.5, 0, 0 );

  aux_controls_hbox_1.pack_start( aux_top_buttons_alignment, Gtk::PACK_SHRINK );


  /********************************************************************************
   */
  if(par && par->has_opacity() ) {
    //frame_top_box_1_1.pack_start( frame_box_2_padding, Gtk::PACK_EXPAND_WIDGET );
    //frame_top_box_1_1.pack_start( blendSelector, Gtk::PACK_SHRINK );
  }

  //frame_top_box_1_2.pack_start( frame_expander, Gtk::PACK_SHRINK, 5 );
  frame_top_box_1_1.pack_start( nameEntry, Gtk::PACK_EXPAND_WIDGET );
  //frame_top_box_1_2.pack_start( frame_box_1_padding, Gtk::PACK_EXPAND_WIDGET );
  //frame_top_box_1_1.pack_start( frame_close, Gtk::PACK_SHRINK, 5 );

  nameEntry.set_text( _("layer name") );

  aux_controls_hbox_1.pack_start( nameEntry2, Gtk::PACK_EXPAND_WIDGET );
  nameEntry2.set_text( nameEntry.get_text() );


  //frame_top_box_1_2.pack_start( map_buttons, Gtk::PACK_SHRINK, 5 );

  frame_top_vbox_1.pack_start( frame_top_box_1_1, Gtk::PACK_SHRINK, 4 );
  frame_top_vbox_1.pack_start( frame_top_box_1_2, Gtk::PACK_SHRINK );
  frame_top_box_1.pack_start( frame_top_vbox_1, Gtk::PACK_EXPAND_WIDGET );


  if(par && par->has_opacity() ) {
    //opacitySlider.set_width( 200 );
    opacity_box.pack_end( opacitySlider, Gtk::PACK_SHRINK, 0 );
    opacity_box.pack_end( blendSelectorMask, Gtk::PACK_SHRINK, 0 );
    opacity_box.pack_end( blendSelector, Gtk::PACK_SHRINK, 0 );
    frame_top_box_2.pack_start( opacity_box, Gtk::PACK_EXPAND_WIDGET );
    //frame_top_box_2.pack_start( opacitySlider, Gtk::PACK_EXPAND_WIDGET );
    //frame_top_box_2.pack_start( blendSelector, Gtk::PACK_SHRINK );

    aux_opacity_box.pack_end( opacitySlider2, Gtk::PACK_EXPAND_WIDGET, 0 );
    aux_opacity_box.pack_end( blendSelectorMask2, Gtk::PACK_SHRINK, 0 );
    aux_opacity_box.pack_end( blendSelector2, Gtk::PACK_SHRINK, 0 );
    aux_controls_hbox_2.pack_start( aux_opacity_box, Gtk::PACK_EXPAND_WIDGET );
  }
  controls_box.pack_start( frame_top_box_2, Gtk::PACK_SHRINK, 0 );

  if(par && par->has_intensity() ) {
    //intensitySlider.set_width( 200 );
    frame_top_box_3.pack_start( intensitySlider, Gtk::PACK_EXPAND_WIDGET );
  }
  frame_top_box_3.pack_start( frame_box_4_padding, Gtk::PACK_EXPAND_WIDGET );
  //controls_box.pack_start( frame_top_box_4, Gtk::PACK_SHRINK, 0 );

  if(par && par->has_target_channel() ) {
    frame_top_box_3.pack_start( frame_chsel_box, Gtk::PACK_SHRINK, 5 );
  }
  if(par && par->has_opacity() ) {
    //frame_shift_box.pack_start( test_padding_enable_box, Gtk::PACK_SHRINK, 2 );
    if( par && par->has_target_channel() ) {
      frame_shift_box.pack_start( shift_x, Gtk::PACK_SHRINK, 2 );
      frame_shift_box.pack_start( shift_y, Gtk::PACK_SHRINK, 2 );
      frame_top_box_3.pack_start( frame_shift_box, Gtk::PACK_SHRINK, 5 );
    } else {
      frame_top_box_3.pack_start( shift_x, Gtk::PACK_SHRINK, 5 );
      frame_top_box_3.pack_start( shift_y, Gtk::PACK_SHRINK, 5 );
    }
  }

  expert_ctrls_expander.add(frame_top_box_3);
  controls_box.pack_start( expert_ctrls_expander, Gtk::PACK_SHRINK, 0 );

  //middle_padding.set_size_request(-1,5);
  //controls_box.pack_start( middle_padding, Gtk::PACK_SHRINK, 0 );
  controls_box.pack_start( hline, Gtk::PACK_SHRINK, 5 );

  layer_selector_box.pack_start( input_source_checkbox, Gtk::PACK_SHRINK, 5 );
  layer_selector_box.pack_start( layer_list, Gtk::PACK_SHRINK, 5 );
  //layer_selector_box.pack_start( sourceSelector, Gtk::PACK_SHRINK, 5 );
  input_source_expander.add( layer_selector_box );
  input_source_expander.set_expanded( false );
  controls_box.pack_end( input_source_expander, Gtk::PACK_SHRINK, 5 );

  //controls_box.pack_end( layer_selector_checkbox, Gtk::PACK_SHRINK, 5 );
  controls_box.pack_end( hline2, Gtk::PACK_SHRINK, 5 );

/*
#ifdef GTKMM_2
  Gdk::Color bg;
  bg.set_grey_p(0.3);
  controls_evbox.modify_bg( Gtk::STATE_NORMAL, bg );

  bg.set_grey_p(0.22);
  nameEntry.modify_base( Gtk::STATE_NORMAL, bg );
  //nameEntry.set_alignment(1);
#endif
*/
  controls_evbox.add( controls_box );
  controls_frame.add( controls_evbox );


  aux_controls_box.pack_start( aux_controls_hbox_1, Gtk::PACK_SHRINK, 5 );
  aux_controls_box.pack_start( aux_controls_hbox_2, Gtk::PACK_SHRINK, 5 );

  if(false && par && par->has_intensity() ) {
    //aux_controls_box.pack_start( intensitySlider2, Gtk::PACK_SHRINK );
  }
  aux_controls_box.set_size_request(0,80);

  frame_visible.set_tooltip_text( _("toggle layer visibility on/off") );
  frame_mask.set_tooltip_text( _("enable/disable layer mask(s)") );
  frame_mask2.set_tooltip_text( _("enable/disable layer mask(s)") );
  frame_sticky.set_tooltip_text( _("toggle sticky flag on/off") );
  frame_sticky2.set_tooltip_text( _("toggle sticky flag on/off") );
  frame_edit.set_tooltip_text( _("toggle editing flag on/off") );
  frame_edit2.set_tooltip_text( _("toggle editing flag on/off") );
  frame_reset.set_tooltip_text( _("reset tool parameters") );
  frame_help.set_tooltip_text( _("show information on current tool") );
  frame_help2.set_tooltip_text( _("show information on current tool") );

  frame_expander.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::expand) );
  frame_expander.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::collapse) );

  frame_visible.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::show_layer_cb) );
  frame_visible.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::hide_layer_cb) );

  frame_mask.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::enable_masks_cb) );
  frame_mask.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::disable_masks_cb) );
  frame_mask2.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::enable_masks_cb) );
  frame_mask2.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::disable_masks_cb) );

  frame_sticky.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::set_sticky_cb) );
  frame_sticky.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::unset_sticky_cb) );
  frame_sticky2.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::set_sticky_cb) );
  frame_sticky2.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::unset_sticky_cb) );

  /**/
  frame_edit.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::enable_editing_cb) );
  frame_edit.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::disable_editing_cb) );
  frame_edit2.signal_activated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::enable_editing_cb) );
  frame_edit2.signal_deactivated.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::disable_editing_cb) );
  /**/

  frame_reset.signal_clicked.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::parameters_reset_cb) );

  frame_help.signal_clicked.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::show_help_cb) );
  frame_help2.signal_clicked.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::show_help_cb) );

  frame_close.signal_clicked.connect(sigc::mem_fun(*this,
        &OperationConfigGUI::close_config_cb) );

  nameEntry.signal_changed().connect(sigc::mem_fun(*this,
      &OperationConfigGUI::on_layer_name_changed) );
  nameEntry2.signal_changed().connect(sigc::mem_fun(*this,
      &OperationConfigGUI::on_layer_name2_changed) );

  blendSelector.hide();
}


PF::OperationConfigGUI::~OperationConfigGUI()
{
}


PF::OpParBase* PF::OperationConfigGUI::get_par()
{
  PF::OpParBase* par = NULL;
  //std::cout<<"OperationConfigGUI::on_preview_clicked(): active="<<previewButton.get_active()<<std::endl;
  if( get_layer() && get_layer()->get_image() &&
    get_layer()->get_processor() &&
    get_layer()->get_processor()->get_par() ) {
    par = get_layer()->get_processor()->get_par();
  }
  return par;
}


PF::OpParBase* PF::OperationConfigGUI::get_blender()
{
  PF::OpParBase* par = NULL;
  //std::cout<<"OperationConfigGUI::on_preview_clicked(): active="<<previewButton.get_active()<<std::endl;
  if( get_layer() && get_layer()->get_image() &&
    get_layer()->get_blender() &&
    get_layer()->get_blender()->get_par() ) {
    par = get_layer()->get_blender()->get_par();
  }
  return par;
}


void PF::OperationConfigGUI::add_widget( Gtk::Widget& widget )
{
  controls_box.pack_start( widget );

  controls_box.show_all_children();
}


void PF::OperationConfigGUI::on_map()
{
  //std::cout<<"OperationConfigGUI::on_map(\""<<get_layer()->get_name()<<"\") called"<<std::endl;
}


void PF::OperationConfigGUI::on_unmap()
{
  //std::cout<<"OperationConfigGUI::on_unmap(\""<<get_layer()->get_name()<<"\") called"<<std::endl;
}


void PF::OperationConfigGUI::expand()
{
#ifndef NDEBUG
  std::cout<<"OperationConfigGUI::expand() called."<<std::endl;
#endif
  if( controls_frame.get_parent() == NULL ) {
    //std::cout<<"OperationConfigGUI::expand(): editor="<<editor<<std::endl;
    if( editor ) {
      editor->get_layer_widget().get_controls_group().collapse_all();
    }
    frame_vbox.pack_start( controls_frame, Gtk::PACK_SHRINK, 0 );
    controls_frame.show_all_children();
    controls_frame.show();
    frame_expander.set_active(true);
    if( editor && get_layer()) {
      editor->set_edited_layer( get_layer()->get_id() );
    }

    PF::OpParBase* par = get_par();
    if( par ) {
      //par->set_editing_flag( true );
      //get_layer()->get_image()->update();
    }
#ifndef NDEBUG
    std::cout<<"OperationConfigGUI::expand(): controls shown"<<std::endl;
#endif
  }
#ifndef NDEBUG
  std::cout<<"OperationConfigGUI::expand(): calling update()"<<std::endl;
#endif
  update();
#ifndef NDEBUG
  std::cout<<"OperationConfigGUI::expand() finished."<<std::endl;
#endif
}


void PF::OperationConfigGUI::collapse()
{
  //std::cout<<"OperationConfigGUI::collapse() called."<<std::endl;
  if( controls_frame.get_parent() == &frame_vbox ) {
    frame_vbox.remove( controls_frame );
    frame_expander.set_active(false);
    //std::cout<<"OperationConfigGUI::collapse(): controls hidden"<<std::endl;
    //if( editor ) {
    //  editor->set_edited_layer( -1 );
    //}
  }

  PF::OpParBase* par = get_par();
  if( par ) {
//#ifndef NDEBUG
    std::cout<<"OperationConfigGUI::collapse(): resetting editing flag"<<std::endl;
//#endif
    if( par->is_editing() ) {
      disable_editing();
    }
  }
}


bool PF::OperationConfigGUI::is_expanded()
{
  return frame_expander.is_active();
}


void PF::OperationConfigGUI::show_layer()
{
  frame_visible.set_active( true );
  PF::Layer* l = get_layer();
  //std::cout<<"OperationConfigGUI::show_layer(): l="<<l<<std::endl;
  //if(l) std::cout<<"OperationConfigGUI::show_layer(): l->is_enabled()="<<l->is_enabled()<<std::endl;
  if( !l || l->is_enabled() )
    return;
  l->set_enabled( true );
  l->set_dirty( true );

  l->get_image()->update();

  //std::cout<<"Layer \""<<l->get_name()<<"\" shown"<<std::endl;
}


void PF::OperationConfigGUI::hide_layer()
{
  frame_visible.set_active( false );
  PF::Layer* l = get_layer();
  //std::cout<<"OperationConfigGUI::hide_layer(): l="<<l<<std::endl;
  //if(l) std::cout<<"OperationConfigGUI::hide_layer(): l->is_enabled()="<<l->is_enabled()<<std::endl;
  if( !l || !l->is_enabled() )
    return;
  l->set_enabled( false );
  l->set_dirty( true );

  if( frame_sticky.is_active() )
    unset_sticky();

  l->get_image()->update();

  //std::cout<<"Layer \""<<l->get_name()<<"\" hidden"<<std::endl;
}


void PF::OperationConfigGUI::enable_masks()
{
  bool modified = false;
  PF::Layer* l = get_layer();
  /*
  PF::OpParBase* par = get_par();
  if( par && !par->get_mask_enabled() ) {
    par->set_mask_enabled( true );
    modified = true;
  }
  */

  frame_mask.set_active( true );
  frame_mask2.set_active( true );

  PF::OpParBase* blender = get_blender();
  if( blender && !blender->get_mask_enabled() ) {
    blender->set_mask_enabled( true );
    modified = true;
  }
  if( modified ) {
    l->set_dirty( true );
    l->get_image()->update();
  }
}


void PF::OperationConfigGUI::disable_masks()
{
  bool modified = false;
  PF::Layer* l = get_layer();
  /*
  PF::OpParBase* par = get_par();
  if( par && par->get_mask_enabled() ) {
    par->set_mask_enabled( false );
    modified = true;
  }
  */

  frame_mask.set_active( false );
  frame_mask2.set_active( false );

  PF::OpParBase* blender = get_blender();
  if( blender && blender->get_mask_enabled() ) {
    blender->set_mask_enabled( false );
    modified = true;
  }
  if( modified ) {
    l->set_dirty( true );
    l->get_image()->update();
  }
}


void PF::OperationConfigGUI::enable_editing()
{
  if( !get_layer() || !(get_layer()->is_visible()) ) {
    frame_edit.set_active(false);
    return;
  }
  PF::OpParBase* par = get_par();
  if( !par ) {
    frame_edit.set_active(false);
    return;
  }

  frame_edit.set_active( true );
  frame_edit2.set_active( true );
/*
  PF::LayerManager& lm = get_layer()->get_image()->get_layer_manager();

  // First we fill a list with all the layers in the image
  std::list<PF::Layer*> list;
  std::list<PF::Layer*>::iterator li;
  for( li = lm.get_layers().begin(); li != lm.get_layers().end(); li++ ) {
    lm.expand_layer( *li, list );
  }

  // Then we loop over the layers and for each we reset the corresponding button
  for( li = list.begin(); li != list.end(); li++ ) {
    if( (*li) == NULL ) continue;
    // skip the current layer
    if( (*li) == get_layer() ) continue;
    if( (*li)->get_processor() == NULL ) continue;
    if( (*li)->get_processor()->get_par() == NULL ) continue;
    PF::OpParBase* par2 = (*li)->get_processor()->get_par();
    par2->set_editing_flag( false );
    PF::OperationConfigUI* ui = par2->get_config_ui();
    if( !ui ) continue;
    PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( !gui ) continue;
    gui->reset_edit_button();
  }

  editor->set_edited_layer( get_layer()->get_id() );
*/
  //std::cout<<"OperationConfigGUI::enable_editing(\""<<get_layer()->get_name()<<"\"): par->set_editing_flag( true )"<<std::endl;
  par->set_editing_flag( true );

  get_layer()->get_image()->update();
}


void PF::OperationConfigGUI::reset_edit_button()
{
  frame_edit.set_active( false );
  frame_edit2.set_active( false );
}


void PF::OperationConfigGUI::disable_editing()
{
  PF::OpParBase* par = get_par();
  if( !par ) return;

  frame_edit.set_active( false );
  frame_edit2.set_active( false );

  std::cout<<"OperationConfigGUI::disable_editing(): setting editing flag to false"<<std::endl;
  par->set_editing_flag( false );
  //std::cout<<"  updating image"<<std::endl;
  //editor->set_edited_layer( -1 );
  get_layer()->get_image()->update();
}


bool PF::OperationConfigGUI::get_editing_flag()
{
  PF::OpParBase* par = get_par();
  if( !par ) return false;
  if( par->is_editing() ) {
    return( true );
  }
  return( false );
}


void PF::OperationConfigGUI::set_sticky()
{
  //std::cout<<"OperationConfigGUI::set_sticky() called."<<std::endl;
  if( !get_layer() ) return;
  if( !get_layer()->get_image() ) return;

  //if( frame_sticky.is_active() ) return;

  frame_sticky.set_active( true );
  frame_sticky2.set_active( true );

  PF::LayerManager& lm = get_layer()->get_image()->get_layer_manager();

  // First we fill a list with all the layers in the image
  std::list<PF::Layer*> list;
  std::list<PF::Layer*>::iterator li;
  for( li = lm.get_layers().begin(); li != lm.get_layers().end(); li++ ) {
    lm.expand_layer( *li, list );
  }

  // Then we loop over the layers and for each we reset the corresponding button
  for( li = list.begin(); li != list.end(); li++ ) {
    if( (*li) == NULL ) continue;
    // skip the current layer
    if( (*li) == get_layer() ) continue;
    if( (*li)->get_processor() == NULL ) continue;
    if( (*li)->get_processor()->get_par() == NULL ) continue;
    PF::OpParBase* par = (*li)->get_processor()->get_par();
    PF::OperationConfigUI* ui = par->get_config_ui();
    if( !ui ) continue;
    PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( !gui ) continue;
    gui->reset_sticky_button();
  }

  //std::cout<<"OperationConfigGUI::set_sticky(): editor->set_displayed_layer("<<get_layer()->get_id()<<")"<<std::endl;
  editor->set_displayed_layer( get_layer()->get_id() );
}


void PF::OperationConfigGUI::reset_sticky_button()
{
  frame_sticky.set_active( false );
  frame_sticky2.set_active( false );
}


void PF::OperationConfigGUI::unset_sticky()
{
  reset_sticky_button();
  editor->set_displayed_layer( -1 );
}


void PF::OperationConfigGUI::parameters_undo()
{

}


void PF::OperationConfigGUI::parameters_redo()
{

}


void PF::OperationConfigGUI::parameters_reset()
{
  for( unsigned int i = 0; i < controls.size(); i++ )
    controls[i]->reset();
  if( get_layer() && get_layer()->get_image() )
    get_layer()->get_image()->update();
}


void PF::OperationConfigGUI::show_help()
{
  Gtk::Dialog dialog(_("help"), false);
  dialog.set_default_size(300,100);

  Gtk::Frame frame;

  Glib::ustring help;
  if( get_par() ) {
    Glib::ustring helpPath = PF::PhotoFlow::Instance().get_data_dir() + "/help/en/" + get_par()->get_type() + ".hlp";
    std::ifstream file(helpPath.c_str());
    char ch;
    if( !file.fail() ) {
      while(!file.eof()) {
        //std::string tmpStr;
        //std::getline(file, tmpStr);
        //help += tmpStr;
        file.get( ch );
        if( !file.fail() ) help += ch;
      }
    } else {
      help = _("This help is not yet available. Sorry.");
    }
  }

  Gtk::TextView textview;
  Glib::RefPtr< Gtk::TextBuffer > buf = textview.get_buffer ();
  buf->set_text( help );
  textview.set_wrap_mode(Gtk::WRAP_WORD);
  textview.set_left_margin( 5 );
  textview.set_right_margin( 5 );
  textview.set_editable( false );
  textview.set_cursor_visible( false );

  dialog.get_vbox()->pack_start( textview );

  //frame.add( textview );
  //dialog.get_vbox()->pack_start( frame );
  //dialog.get_content_area().pack_start( frame );

  dialog.add_button(_("Close"), Gtk::RESPONSE_OK);

  dialog.show_all_children();

  Gtk::Container* toplevel = controls_box.get_toplevel();
#ifdef GTKMM_2
  if( toplevel && toplevel->is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
#ifdef GTKMM_3
  if( toplevel && toplevel->get_is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
    dialog.set_transient_for( *(dynamic_cast<Gtk::Window*>(toplevel)) );

  dialog.run();
}


void PF::OperationConfigGUI::close_config()
{
  collapse();
  if( editor ) {
    editor->get_layer_widget().get_controls_group().remove_control( this );
  }
}


void PF::OperationConfigGUI::on_layer_name_changed()
{
  //std::cout<<"OperationConfigGUI::on_layer_name_changed() called"<<std::endl;
  if( get_layer() && (get_layer()->get_name() != nameEntry.get_text()) ) {
    get_layer()->set_name( nameEntry.get_text() );
    nameEntry2.set_text( nameEntry.get_text() );
    if( editor ) {
      editor->get_layer_widget().update(true);
      int pos = nameEntry.get_position();
      nameEntry.grab_focus();
      nameEntry.select_region(0,0);
      nameEntry.set_position(pos);
    }
  }
}


void PF::OperationConfigGUI::on_layer_name2_changed()
{
  //std::cout<<"OperationConfigGUI::on_layer_name2_changed() called"<<std::endl;
  if( get_layer() && (get_layer()->get_name() != nameEntry2.get_text()) ) {
    get_layer()->set_name( nameEntry2.get_text() );
    nameEntry.set_text( nameEntry2.get_text() );
    if( editor ) {
      editor->get_layer_widget().update(true);
      int pos = nameEntry2.get_position();
      nameEntry2.grab_focus();
      nameEntry2.select_region(0,0);
      nameEntry2.set_position(pos);
    }
  }
}


void PF::OperationConfigGUI::update_buttons()
{
  PF::OpParBase* par = get_par();
  //std::cout<<"OperationConfigGUI::update_buttons(\""<<get_layer()->get_name()<<"\") called, par="<<par<<std::endl;
  if( par ) {
    if( par->is_map() ) {
      blendSelector.hide();
      blendSelectorMask.show();
      //std::cout<<"  blendSelectorMask.show()"<<std::endl;
      blendSelector2.hide();
      blendSelectorMask2.show();
      //frame_top_buttons_alignment.hide();
      //map_buttons.hide();
      frame_mask.hide();
      frame_edit.show();
      frame_sticky.hide();
    } else {
      blendSelector.show();
      blendSelectorMask.hide();
      //std::cout<<"  blendSelectorMask.hide()"<<std::endl;
      blendSelector2.show();
      blendSelectorMask2.hide();
      //frame_top_buttons_alignment.show();
      //map_buttons.hide();
      frame_mask.show();
      frame_edit.show();
      frame_sticky.show();
    }
  }

  if( get_layer() ) {
    frame_visible.set_active( get_layer()->is_enabled() );
    //std::cout<<"OperationConfigGUI::update_buttons(): frame_visible.set_active("<<get_layer()->is_enabled()<<");"<<std::endl;
  }
  if( get_blender() ) {
    frame_mask.set_active( get_blender()->get_mask_enabled() );
  }
}


void PF::OperationConfigGUI::init()
{
#ifndef NDEBUG
  std::cout<<"OperationConfigGUI::init(\""<<get_layer()->get_name()<<"\") called"<<std::endl;
#endif
  update_buttons();
  for( unsigned int i = 0; i < controls.size(); i++ )
    controls[i]->init();
}


void PF::OperationConfigGUI::open()
{
  init();
  /*
  for( int i = 0; i < controls.size(); i++ )
    controls[i]->init();
  */
  PF::OpParBase* par = get_par();

  if( PF::PhotoFlow::Instance().is_single_win_mode() && frame )
    frame->show_all();

  if( par ) {
    nameEntry.set_text( get_layer()->get_name().c_str() );
    nameEntry2.set_text( get_layer()->get_name().c_str() );

    values_save.clear();
    par->save_properties( values_save );
#ifndef NDEBUG
    std::cout<<"Saved property values:"<<std::endl;
    for( std::list<std::string>::iterator i = values_save.begin();
        i != values_save.end(); i++ ) {
      std::cout<<"  "<<(*i)<<std::endl;
    }
#endif
  }
  PF::OperationConfigUI::open();

  expand();
  //show_all();
  //show();
}



void PF::OperationConfigGUI::reset_ch_selector()
{
  if( greychSelector.get_parent() == &frame_chsel_box )
    frame_chsel_box.remove( greychSelector );
  if( rgbchSelector.get_parent() == &frame_chsel_box )
    frame_chsel_box.remove( rgbchSelector );
  if( labchSelector.get_parent() == &frame_chsel_box )
    frame_chsel_box.remove( labchSelector );
  if( cmykchSelector.get_parent() == &frame_chsel_box )
    frame_chsel_box.remove( cmykchSelector );
}


void PF::OperationConfigGUI::do_update()
{
#ifndef NDEBUG
  std::cout<<"PF::OperationConfigGUI::do_update(\""<<get_layer()->get_name()<<"\") called."<<std::endl;
#endif
  update_buttons();

  layer_list.update_model();

  bool old_inhibit;
  PF::PFWidget* w;

  w = &blendSelector;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &blendSelectorMask;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &blendSelector2;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &blendSelectorMask2;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &opacitySlider;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &opacitySlider2;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &intensitySlider;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  w = &intensitySlider2;
  old_inhibit = w->get_inhibit();
  w->set_inhibit( true ); w->get_value(); w->set_inhibit( old_inhibit );

  if( get_layer() ) {
    nameEntry.set_text( get_layer()->get_name() );
    nameEntry2.set_text( get_layer()->get_name() );
  }

  if( get_par() ) {
    if( get_par()->get_previous_layer_is_input() ) layer_list.hide();
    else layer_list.show();
    if( get_par()->is_editing() ) {
      frame_edit.set_active( true );
      frame_edit2.set_active( true );
    } else {
      frame_edit.set_active( false );
      frame_edit2.set_active( false );
    }
  }

  if( input_source_visible ) input_source_expander.show();
  else input_source_expander.hide();

  // Update target channel selector
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() &&
      get_layer()->get_processor()->get_par()->has_target_channel() ) {
#ifndef NDEBUG
    std::cout<<"OperationConfigDialog::do_update() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif

    PF::colorspace_t cs = PF_COLORSPACE_UNKNOWN;
    PF::Image* image = get_layer()->get_image();
    PF::Pipeline* pipeline = image->get_pipeline(0);
    PF::PipelineNode* node = NULL;
    if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
    if( node && node->processor && node->processor->get_par() ) {
      PF::OpParBase* par = node->processor->get_par();
      cs = PF::convert_colorspace( par->get_interpretation() );
      //std::cout<<"OperationConfigDialog::update() par: "<<par<<std::endl;
    }
    //std::cout<<"OperationConfigDialog::update() for "<<get_layer()->get_name()<<" called, cs: "<<cs<<std::endl;
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      if( greychSelector.get_parent() != &frame_chsel_box ) {
        reset_ch_selector();
        frame_chsel_box.pack_start( greychSelector, Gtk::PACK_EXPAND_PADDING );
        greychSelector.show();
      }
      break;
    case PF_COLORSPACE_RGB:
      if( rgbchSelector.get_parent() != &frame_chsel_box ) {
        reset_ch_selector();
        frame_chsel_box.pack_start( rgbchSelector, Gtk::PACK_EXPAND_PADDING );
        rgbchSelector.show();
      }
      break;
    case PF_COLORSPACE_LAB:
      if( labchSelector.get_parent() != &frame_chsel_box ) {
        reset_ch_selector();
        frame_chsel_box.pack_start( labchSelector, Gtk::PACK_EXPAND_PADDING );
        labchSelector.show();
      }
      break;
    case PF_COLORSPACE_CMYK:
      if( cmykchSelector.get_parent() != &frame_chsel_box ) {
        reset_ch_selector();
        frame_chsel_box.pack_start( cmykchSelector, Gtk::PACK_EXPAND_PADDING );
        cmykchSelector.show();
      }
      break;
    default:
      break;
    }
  }
}


void PF::OperationConfigGUI::update()
{
  //std::cout<<"PF::OperationConfigGUI::update(\""<<get_layer()->get_name()<<"\") called, is_expanded()="
  //    <<is_expanded()<<std::endl;
  if( !is_expanded() ) return;
  //std::cout<<"PF::OperationConfigGUI::update(\""<<get_layer()->get_name()<<"\"): calling do_update()"<<std::endl;
  do_update();

  //gdk_threads_add_idle ((GSourceFunc) config_update_cb, this);
  //std::cout<<"PF::OperationConfigGUI::update(\""<<get_layer()->get_name()<<"\"): waiting for semaphore"<<std::endl;
  //vips_semaphore_down( &update_done_sem );
  //std::cout<<"PF::OperationConfigGUI::update(\""<<get_layer()->get_name()<<"\"): semaphore ready"<<std::endl;
}



void PF::OperationConfigGUI::update_properties()
{
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    controls[i]->set_value();
  }
}


void PF::OperationConfigGUI::enable_preview()
{
  PF::OpParBase* par = get_par();
  if( !par ) return;

  get_layer()->get_image()->lock();
  // Enable all controls
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    controls[i]->set_inhibit( false );
    controls[i]->set_value();
  }
  get_layer()->set_dirty( true );
  //std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
}


void PF::OperationConfigGUI::disable_preview()
{
  PF::OpParBase* par = get_par();
  if( !par ) return;

  get_layer()->get_image()->lock();
  // Inhibit all controls such that they do not modify the
  // underlying properties
  for( unsigned int i = 0; i < controls.size(); i++ )
    controls[i]->set_inhibit( true );

  //std::cout<<"  restoring original values"<<std::endl;
  par->restore_properties( values_save );
  get_layer()->set_dirty( true );
  //std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  //std::cout<<"  image updated"<<std::endl;
  get_layer()->get_image()->unlock();
}


void PF::OperationConfigGUI::screen2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->screen2image( x, y, w, h );
}
void PF::OperationConfigGUI::image2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->image2layer( x, y, w, h );
}
void PF::OperationConfigGUI::screen2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->screen2layer( x, y, w, h );
}
void PF::OperationConfigGUI::image2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->image2screen( x, y, w, h );
}
void PF::OperationConfigGUI::layer2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->layer2image( x, y, w, h );
}
void PF::OperationConfigGUI::layer2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if(editor) editor->layer2screen( x, y, w, h);
}




PF::ProcessorBase* PF::new_operation_with_gui( std::string op_type, PF::Layer* current_layer )
{
  if( !current_layer ) return NULL;
  PF::ProcessorBase* processor = PF::new_operation( op_type, current_layer );
  if( !processor ) return NULL;

  PF::OperationConfigGUI* dialog = NULL;

  if( op_type == "imageread" ) {

    dialog = new PF::ImageReadConfigGUI( current_layer );

  } else if( op_type == "raw_loader" ) {

    dialog = new PF::RawLoaderConfigGUI( current_layer );

  } else if( op_type == "raw_developer_v2" ) {

    dialog = new PF::RawDeveloperConfigGUI( current_layer );

  } else if( op_type == "raw_developer" ) {

    dialog = new PF::RawDeveloperConfigGUIV1( current_layer );

  } else if( op_type == "raw_output_v2" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "RAW output" );

  } else if( op_type == "raw_output" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "RAW output" );

  } else if( op_type == "buffer" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "Buffer" );

  } else if( op_type == "blender" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "Layer Group" );

  } else if( op_type == "clone" ) {

    dialog = new PF::CloneConfigGUI( current_layer );

  } else if( op_type == "clip" ) {

    dialog = new PF::ClipConfigGUI( current_layer );

  } else if( op_type == "white_balance" ) {

    dialog = new PF::WhiteBalanceConfigGUI( current_layer );

  } else if( op_type == "crop" ) {

    dialog = new PF::CropConfigGUI( current_layer );

  } else if( op_type == "scale" ) {

    dialog = new PF::ScaleConfigGUI( current_layer );

  } else if( op_type == "perspective" ) {

    dialog = new PF::PerspectiveConfigGUI( current_layer );

  } else if( op_type == "invert" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "Convert Colors" );

  } else if( op_type == "threshold" ) {

    dialog = new PF::ThresholdConfigGUI( current_layer );

  } else if( op_type == "noise_generator" ) {

    dialog = new PF::NoiseGeneratorConfigGUI( current_layer );

  } else if( op_type == "desaturate" ) {

    dialog = new PF::DesaturateConfigGUI( current_layer );

  } else if( op_type == "uniform" ) {

    dialog = new PF::UniformConfigGUI( current_layer );

  } else if( op_type == "gradient" ) {

    dialog = new PF::GradientConfigGUI( current_layer );

  } else if( op_type == "path_mask" ) {

    dialog = new PF::PathMaskConfigGUI( current_layer );

  } else if( op_type == "levels" ) {

    dialog = new PF::LevelsConfigGUI( current_layer );

  } else if( op_type == "basic_adjustments" ) {

    dialog = new PF::BasicAdjustmentsConfigGUI( current_layer );

  } else if( op_type == "color_correction" ) {

    dialog = new PF::ColorCorrectionConfigGUI( current_layer );

  } else if( op_type == "brightness_contrast" ) {

    dialog = new PF::BrightnessContrastConfigGUI( current_layer );

  } else if( op_type == "hue_saturation" ) {

    dialog = new PF::HueSaturationConfigGUI( current_layer );

  } else if( op_type == "hsl_mask" ) {

    dialog = new PF::HSLMaskConfigGUI( current_layer );

  } else if( op_type == "curves" ) {

    dialog = new PF::CurvesConfigGUI( current_layer );

  } else if( op_type == "channel_mixer" ) {

    dialog = new PF::ChannelMixerConfigGUI( current_layer );

  } else if( op_type == "gaussblur" ) {

    dialog = new PF::GaussBlurConfigGUI( current_layer );

  } else if( op_type == "blur_bilateral" ) {

    dialog = new PF::BlurBilateralConfigGUI( current_layer );

  } else if( op_type == "denoise" ) {

    dialog = new PF::DenoiseConfigGUI( current_layer );

  } else if( op_type == "sharpen" ) {

    dialog = new PF::SharpenConfigGUI( current_layer );

  } else if( op_type == "convert2lab" ) {

    dialog = new PF::OperationConfigGUI( current_layer, "Convert to Lab colororspace" );

  } else if( op_type == "convert_colorspace" ) {

    dialog = new PF::ConvertColorspaceConfigGUI( current_layer );

  } else if( op_type == "dynamic_range_compressor" ) {

    dialog = new PF::DynamicRangeCompressorConfigGUI( current_layer );

  } else if( op_type == "draw" ) {

    dialog = new PF::DrawConfigGUI( current_layer );

  } else if( op_type == "clone_stamp" ) {

    dialog = new PF::CloneStampConfigGUI( current_layer );

  } else if( op_type == "lensfun" ) {

    dialog = new PF::LensFunConfigGUI( current_layer );

  } else if( op_type == "volume" ) {

    dialog = new PF::VolumeConfigGUI( current_layer );

  } else if( op_type == "shadows_highlights" ) {

    dialog = new PF::ShadowsHighlightsConfigGUI( current_layer );

  } else if( op_type == "defringe" ) {

    dialog = new PF::DefringeConfigGUI( current_layer );

  } else if( op_type == "guided_filter" ) {

    dialog = new PF::GuidedFilterConfigGUI( current_layer );

  } else if( op_type == "split_details" ) {

    dialog = new PF::SplitDetailsConfigGUI( current_layer );

  } else if( op_type == "tone_mapping" ) {

    dialog = new PF::ToneMappingConfigGUI( current_layer );

  } else if( op_type == "local_contrast" ) {

    dialog = new PF::LocalContrastConfigGUI( current_layer );

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

    PF::VipsOperationConfigGUI* vips_config =
      new PF::VipsOperationConfigGUI( current_layer );
    vips_config->set_op( vips_op_type.c_str() );
    dialog = vips_config;
  }
  */

  if( processor ) {
    PF::OpParBase* current_op = processor->get_par();
    if( current_op && dialog ) {
      current_op->set_config_ui( dialog );
      //current_op->set_editing_flag(true);
    }
  }

  return processor;
}


