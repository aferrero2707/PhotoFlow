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
#include <glib.h>

#include "../base/file_util.hh"
#include "../base/fileutils.hh"
#include "../base/pf_file_loader.hh"
#include "../operations/buffer.hh"
#include "../operations/blender.hh"
#include "tablabelwidget.hh"
#include "layerwidget.hh"
#include "imageeditor.hh"


class MaskTabWidget: public Gtk::HBox
{
  Gtk::Image image;
  PF::ToggleImageButton button;
  Gtk::CheckButton check_button;
  Gtk::Label label;
  Gtk::Widget* widget;

public:
  MaskTabWidget():
    HBox(),
    image(),
    button(PF::PhotoFlow::Instance().get_icons_dir()+"/preview_active.png", PF::PhotoFlow::Instance().get_icons_dir()+"/preview_inactive.png"),
    label( _("Layer mask   ") )
  {
    image.set( PF::PhotoFlow::Instance().get_icons_dir()+"/preview_active.png" );
    //button.add( image );
    //button.set_relief( Gtk::RELIEF_NONE );
    pack_start( label, Gtk::PACK_SHRINK );
    pack_start( check_button, Gtk::PACK_SHRINK );
    pack_start( image, Gtk::PACK_SHRINK );

    check_button.signal_toggled().connect( sigc::mem_fun(*this,
                                                   &MaskTabWidget::on_button_toggled) );

    show_all();
  }

  void on_button_toggled()
  {
    if( check_button.get_active() ) signal_show_mask.emit();
    else signal_hide_mask.emit();
  }

  sigc::signal<void> signal_show_mask;
  sigc::signal<void> signal_hide_mask;
};




PF::ControlsGroup::ControlsGroup( ImageEditor* e ): editor(e)
{
  set_spacing(10);
}


void PF::ControlsGroup::clear()
{
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    if( controls[i] && (controls[i]->get_parent() == this) )
      remove( *(controls[i]) );
  }
  controls.clear();
  editor->update_controls();
}


void PF::ControlsGroup::populate()
{
  //editor->get_image()->rebuild_lock();
  //editor->get_image()->rebuild_done_wait( false );
  // Make sure the image is not being rebuilt
  editor->get_image()->lock();

  // temporarely remove all controls
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    if( controls[i] && (controls[i]->get_parent() == this) )
      remove( *(controls[i]) );
  }

  // get a flattened copy of the layers tree
  std::list<Layer*> layers;
  editor->get_image()->get_layer_manager().get_flattened_layers_tree( layers );

  // loop over the layers, and re-insert in the controls group those that are in the guis list
#ifndef NDEBUG
  std::cout<<"ControlsGroup::update(): layers.size()="<<layers.size()<<std::endl;
#endif
  for( std::list<Layer*>::reverse_iterator li = layers.rbegin(); li != layers.rend(); li++ ) {
    PF::Layer* l = *li;
    PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
    if( ui ) {
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
        if( gui ) {
        for( unsigned int i = 0; i < guis.size(); i++ ) {
          if( guis[i] == gui ) {
            Gtk::Frame* control = gui->get_frame();
            pack_start( *control, Gtk::PACK_SHRINK );
            PF::OpParBase* par = gui->get_par();
            if( gui->is_expanded() && par ) {
#ifndef NDEBUG
              std::cout<<"ControlsGroup::populate(): setting editing flag"<<std::endl;
#endif
              par->set_editing_flag( true );
              //get_layer()->get_image()->update();
            }
            break;
          }
        }
      }
    }
  }

  //editor->get_image()->rebuild_unlock();
  editor->get_image()->unlock();
}


void PF::ControlsGroup::update()
{
#ifndef NDEBUG
  std::cout<<"ControlsGroup::update() called"<<std::endl;
#endif
  editor->get_image()->lock();
  for( unsigned int i = 0; i < guis.size(); i++ ) {
    guis[i]->update();
  }
  editor->get_image()->unlock();
#ifndef NDEBUG
  std::cout<<"ControlsGroup::update() finished"<<std::endl;
#endif
}


void PF::ControlsGroup::add_control(PF::Layer* layer, PF::OperationConfigGUI* gui)
{
#ifndef NDEBUG
  std::cout<<std::endl<<std::endl;
  std::cout<<"ControlsGroup::add_control() called."<<std::endl;
#endif
  //editor->get_image()->rebuild_lock();
  //editor->get_image()->rebuild_done_wait( false );
  // Make sure the image is not being rebuilt
  editor->get_image()->lock();
  for( unsigned int i = 0; i < guis.size(); i++ ) {
    if( guis[i] == gui ) {
      guis[i]->expand();
      //editor->get_image()->rebuild_unlock();
      editor->get_image()->unlock();
      return;
    }
  }
  collapse_all();
  bool needs_update = remove_all_controls();
  guis.push_back( gui );
  Gtk::Frame* control = gui->get_frame();
  controls.push_back( control );
  //pack_end( *control, Gtk::PACK_SHRINK );
  //editor->get_image()->rebuild_unlock();
  PF::OpParBase* par = gui->get_par();
  if( par ) {
    par->set_editing_flag( true );
    //get_layer()->get_image()->update();
  }
  //editor->get_image()->update();
  editor->get_image()->unlock();
  populate();
  editor->update_controls();
#ifndef NDEBUG
  std::cout<<"ControlsGroup::add_control(): needs_update="<<needs_update
      <<" gui->has_editing_mode()="<<gui->has_editing_mode()<<std::endl;
#endif
  if( needs_update || gui->has_editing_mode() )
    editor->get_image()->update();
}


void PF::ControlsGroup::remove_control(PF::OperationConfigGUI* gui)
{
  Gtk::Frame* control = gui->get_frame();
#ifndef NDEBUG
  std::cout<<"ControlsGroup::remove_control() called."<<std::endl;
#endif
  if( control->get_parent() == this ) {
    PF::OpParBase* par = gui->get_par();
    if( par ) {
#ifndef NDEBUG
      std::cout<<"ControlsGroup::remove_control(): resetting editing flag"<<std::endl;
#endif
      par->set_editing_flag( false );
      //get_layer()->get_image()->update();
    }
    std::vector<PF::OperationConfigGUI*> new_guis;
    std::vector<Gtk::Widget*> new_controls;
    for( unsigned int i = 0; i < controls.size(); i++ ) {
#ifndef NDEBUG
      std::cout<<"  controls["<<i<<"]="<<controls[i]<<" (control="<<control<<")"<<std::endl;
#endif
      if( controls[i] == control )
        continue;
#ifndef NDEBUG
      std::cout<<"  new_controls.push_back("<<controls[i]<<")"<<std::endl;
#endif
      new_guis.push_back( guis[i] );
      new_controls.push_back( controls[i] );
    }
#ifndef NDEBUG
    std::cout<<"ControlsGroup::remove_control(): controls.size()="<<controls.size()<<"  new_controls.size()="<<new_controls.size()<<std::endl;
#endif
    guis = new_guis;
    controls = new_controls;
    remove( *control );
  }
  editor->update_controls();
}


bool PF::ControlsGroup::remove_all_controls()
{
  bool result = false;
  std::vector<PF::OperationConfigGUI*> tmp_guis = guis;
  for( unsigned int i = 0; i < tmp_guis.size(); i++ ) {
    if( tmp_guis[i]->has_editing_mode() ) result = true;
    remove_control( tmp_guis[i] );
  }
  return result;
}


void PF::ControlsGroup::collapse_all()
{
  for( unsigned int i = 0; i < guis.size(); i++ ) {
    guis[i]->collapse();
  }
}




PF::AuxControlsGroup::AuxControlsGroup( ImageEditor* e ): editor(e), controls(NULL), gui(NULL)
{
  //set_spacing(10);
  //set_size_request(-1,60);
}


void PF::AuxControlsGroup::clear()
{
  if(!controls) return;
  remove(*controls);
  controls = NULL;
  gui = NULL;
  editor->update_controls();
}


void PF::AuxControlsGroup::update()
{
  editor->get_image()->lock();
  gui->update();
  editor->get_image()->unlock();
}


void PF::AuxControlsGroup::set_control(PF::Layer* layer, PF::OperationConfigGUI* g)
{
  // Make sure the image is not being rebuilt
  editor->get_image()->lock();
  if( g == gui ) {
    editor->get_image()->unlock();
    return;
  }

  clear();

  gui = g;
  controls = &(gui->get_aux_controls());
  pack_start( *controls, Gtk::PACK_SHRINK );

  show_all();

  editor->get_image()->unlock();
  editor->update_controls();
}



PF::ControlsDialog::ControlsDialog( ImageEditor* e ):
    Gtk::Dialog(), editor(e), gui(NULL), top_box(NULL), close_button(_("Close")), x(-1), y(-1)
{
  //close_button_box.pack_end( close_button, Gtk::PACK_SHRINK, 5 );
#ifdef GTKMM_3
  get_content_area()->pack_end( notebook, Gtk::PACK_SHRINK );
#else
  get_vbox()->pack_end( notebook, Gtk::PACK_SHRINK );
#endif

  show_all_children();
  set_deletable ( false );

  close_button.signal_clicked().connect( sigc::mem_fun(*this,
      &PF::ControlsDialog::close) );

  add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
}


void PF::ControlsDialog::set_controls(PF::Layer* l)
{
  bool needs_update = false;
  if( gui && gui->get_frame()) {
    gui->collapse();
/*
#ifdef GTKMM_3
    get_content_area()->remove( *(gui->get_frame()) );
#else
    get_vbox()->remove( *(gui->get_frame()) );
#endif
*/
    for(int pi = 0; pi < notebook.get_n_pages(); pi++) notebook.remove_page(-1);
    PF::OpParBase* par = gui->get_par();
    if( par ) {
      par->set_editing_flag( false );
    }
    if( gui->has_editing_mode() ) needs_update = true;
  }

  //std::cout<<"ControlsDialog::set_controls(): l="<<l<<std::endl;
  if( !l ) {if(needs_update) editor->get_image()->update(); return;}
  //std::cout<<"ControlsDialog::set_controls(): l->get_processor()="<<l->get_processor()<<std::endl;
  if( !l->get_processor() ) {if(needs_update) editor->get_image()->update(); return;}
  //std::cout<<"ControlsDialog::set_controls(): l->get_processor()->get_par()="<<l->get_processor()->get_par()<<std::endl;
  if( !l->get_processor()->get_par() ) {if(needs_update) editor->get_image()->update(); return;}
  //std::cout<<"ControlsDialog::set_controls(): l->get_processor()->get_par()->get_config_ui()="<<l->get_processor()->get_par()->get_config_ui()<<std::endl;
  if( !l->get_processor()->get_par()->get_config_ui() ) {
    if(needs_update) editor->get_image()->update(); return;
  }
  PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
  gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
  //std::cout<<"ControlsDialog::set_controls(): gui="<<gui<<std::endl;
  if( !gui ) {if(needs_update) editor->get_image()->update(); return;}
  //std::cout<<"ControlsDialog::set_controls(): gui->get_frame()="<<gui->get_frame()<<std::endl;
  if( !gui->get_frame() ) {if(needs_update) editor->get_image()->update(); return;}

  gui->open();
  gui->expand();

  PF::OpParBase* par = gui->get_par();
  if( par ) {
    par->set_editing_flag( true );
  }
  if( gui->has_editing_mode() ) needs_update = true;

  Gtk::Widget* controls = gui->get_frame();
  //std::cout<<"ControlsDialog::set_controls(\""<<l->get_name()<<"\"): controls="<<controls<<std::endl;
  if( controls ) {
/*
#ifdef GTKMM_3
    get_content_area()->pack_start( *controls, Gtk::PACK_SHRINK );
#else
    get_vbox()->pack_start( *controls, Gtk::PACK_SHRINK );
#endif
*/
    notebook.append_page(*controls, _("controls"));
    notebook.append_page(gui->get_blend_box(), _("in/out"));
    //notebook.append_page(gui->get_input_box(), _("input"));
    //controls->show_all();
  }

  if( top_box ) delete top_box;
  top_box = new Gtk::HBox();

  top_box->pack_start( gui->get_top_box(), Gtk::PACK_SHRINK, 5 );
  top_box->pack_end( close_button, Gtk::PACK_SHRINK, 5 );
  top_box->show_all();

  #ifdef GTKMM_3
  get_content_area()->pack_start( *top_box, Gtk::PACK_SHRINK );
#else
  get_vbox()->pack_start( *top_box, Gtk::PACK_SHRINK );
#endif


  set_title(l->get_processor()->get_par()->get_default_name());


  if( needs_update || gui->has_editing_mode() )
    editor->get_image()->update();
}


void PF::ControlsDialog::update()
{
#ifndef NDEBUG
  std::cout<<"ControlsDialog::update() called"<<std::endl;
#endif
  editor->get_image()->lock();
  gui->update();
  editor->get_image()->unlock();
#ifndef NDEBUG
  std::cout<<"ControlsDialog::update() finished"<<std::endl;
#endif
}


void PF::ControlsDialog::on_hide()
{
  std::cout<<"ControlsDialog::on_hide() called."<<std::endl;
  get_position(x,y);
  //visible = false;
  Gtk::Dialog::on_hide();
}


bool PF::ControlsDialog::on_delete_event( GdkEventAny* any_event )
{
  std::cout<<"ControlsDialog::on_delete_event() called."<<std::endl;
  //if(editor) editor->get_layer_widget().controls_dialog_close(gui);
  close();
  return true;
}


void PF::ControlsDialog::open()
{
  //std::cout<<"ControlsDialog::open(): x="<<x<<"  y="<<y<<std::endl;
  if(x>=0 && y>=0) move(x,y);
  show();
  visible=true;
}

void PF::ControlsDialog::close()
{
  hide();
  visible=false;
}


bool PF::ControlsDialog::on_key_press_event(GdkEventKey* event)
{
  //std::cout<<"ControlsDialog: event->type="<<event->type<<"  event->state="<<(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK))<<std::endl;
  if( event->type == GDK_KEY_PRESS &&
      (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK)) == (GDK_CONTROL_MASK) ) {
    if( event->keyval == 'e' ) {
      //std::cout<<"ControlsDialog: Ctrl+e pressed"<<std::endl;
      if( gui ) {
        if( gui->get_editing_flag() )
          gui->disable_editing();
        else
          gui->enable_editing();
        return true;
      }
    }
  }

  // Forward the event to the image editor
  //return( editor->on_key_press_event(event) );
  return Gtk::Dialog::on_key_press_event(event);
}





PF::LayerWidget::LayerWidget( Image* img, ImageEditor* ed ):
  Gtk::VBox(), 
  image( img ), editor( ed ),
  layers_view( ed, false ),
  mask_view( ed, true ),
  mask_view_back_button(_("back")),
  mask_view_show_label1(_("show")),
  mask_view_show_label2(_("mask")),
  selected_layer_id( -1 ),
  controls_group( ed ),
  aux_controls_group( ed ),
  buttonAdd( _("New Adjustment") ),
  buttonAddGroup("G+"),
  buttonDel("-"),
  buttonPresetLoad( _("Load preset") ),
  buttonPresetSave( _("Save preset") ),
  operationsDialog( image, this ),
  controls_dialog_visible(false), controls_dialog_x(-1), controls_dialog_y(-1),
  //add_button(PF::PhotoFlow::Instance().get_icons_dir()+"/add-layer.png", "", image, this),
  add_button(PF::PhotoFlow::Instance().get_icons_dir()+"/plus.png", "", image, this),
  group_button(PF::PhotoFlow::Instance().get_icons_dir()+"/group.png", "", image, this),
  trash_button(PF::PhotoFlow::Instance().get_icons_dir()+"/trash.png", "", image, this),
  insert_image_button(PF::PhotoFlow::Instance().get_icons_dir()+"/libre-file-image.png", "", image, this),
  curves_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/curves.png", "curves", image, this),
  uniform_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/bucket-fill.png", "uniform", image, this),
  gradient_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/gradient.png", "gradient", image, this),
  path_mask_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/path-mask.png", "path_mask", image, this),
  desaturate_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/desaturate.png", "desaturate", image, this),
  crop_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/crop.png", "crop", image, this),
  basic_edits_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/basic-edits.png", "basic_adjustments", image, this),
  levels_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/basic-edits.png", "levels", image, this),
  draw_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/draw.png", "draw", image, this),
  clone_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/clone.png", "clone_stamp", image, this),
  scale_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/scale.png", "scale", image, this),
  perspective_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/perspective.png", "perspective", image, this),
  relight_button(PF::PhotoFlow::Instance().get_icons_dir()+"/tools/flash.png", "relight", image, this)
{
  floating_tool_dialogs = PF::PhotoFlow::Instance().get_options().get_ui_floating_tool_dialogs();

  set_size_request(250,-1);
  //notebook.set_tab_pos(Gtk::POS_LEFT);
  //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();

  add_button.set_tooltip_text( _("new layer") );
  group_button.set_tooltip_text( _("new group layer") );
  trash_button.set_tooltip_text( _("delete layer") );
  insert_image_button.set_tooltip_text( _("insert image as layer") );
  basic_edits_button.set_tooltip_text( _("basic adjustments") );
  levels_button.set_tooltip_text( _("levels") );
  curves_button.set_tooltip_text( _("curves tool") );
  uniform_button.set_tooltip_text( _("uniform fill") );
  gradient_button.set_tooltip_text( _("gradient tool") );
  desaturate_button.set_tooltip_text( _("desaturate tool") );
  crop_button.set_tooltip_text( _("crop tool") );
  draw_button.set_tooltip_text( _("freehand drawing") );
  clone_button.set_tooltip_text( _("clone stamp tool") );
  perspective_button.set_tooltip_text( _("perspective correction") );
  scale_button.set_tooltip_text( _("scale/rotate tool") );
  path_mask_button.set_tooltip_text( _("path tool") );

  tool_buttons_box.pack_start( add_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( group_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( insert_image_button, Gtk::PACK_SHRINK, 2 );
  //tool_buttons_box.pack_start( levels_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( basic_edits_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( curves_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( relight_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( uniform_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( gradient_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( desaturate_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( crop_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( perspective_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( scale_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( path_mask_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( draw_button, Gtk::PACK_SHRINK, 2 );
  //tool_buttons_box.pack_start( clone_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( trash_button, Gtk::PACK_SHRINK, 20 );


  //LayerTree* view = new LayerTree( editor );
  hbox.set_spacing(4);
  if( floating_tool_dialogs ) {
    vbox.pack_start( aux_controls_group, Gtk::PACK_SHRINK );
    hbox.pack_start( tool_buttons_box, Gtk::PACK_SHRINK );
  }
  hbox.pack_start( layers_view, Gtk::PACK_EXPAND_WIDGET );
  vbox.pack_start( hbox, Gtk::PACK_EXPAND_WIDGET );

  mask_view_top_box.pack_start( mask_view_back_button, Gtk::PACK_SHRINK, 4 );
  mask_view_show_label_box.pack_start( mask_view_show_label1, Gtk::PACK_SHRINK, 0 );
  mask_view_show_label_box.pack_start( mask_view_show_label2, Gtk::PACK_SHRINK, 0 );
  mask_view_top_box.pack_end( mask_view_show_label_box, Gtk::PACK_SHRINK, 4 );
  mask_view_top_box.pack_end( mask_view_show_button, Gtk::PACK_SHRINK, 4 );
  mask_view_box.pack_start( mask_view_top_box, Gtk::PACK_SHRINK, 4 );
  mask_view_box.pack_start( mask_view, Gtk::PACK_EXPAND_WIDGET );
  hbox.pack_start( mask_view_box, Gtk::PACK_EXPAND_WIDGET );

  //view->signal_updated.connect(sigc::mem_fun(this, &LayerWidget::modified) );
  //frame->add( *view );
  
  //view->set_reorderable();

  //notebook.append_page(*view,_("Layers"));
  //Widget* page = notebook.get_nth_page(-1);
  //Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
  //label->set_angle(90);

  //LayerTree* maskview = new LayerTree( editor, true );
  //MaskTabWidget* tabwidget = new MaskTabWidget();
  //tabwidget->signal_close.connect( sigc::bind<bool>(sigc::mem_fun(*this, &PF::MainWindow::remove_tab), false) );
  //notebook.append_page(*maskview, *tabwidget);

  //top_box.pack_start(buttonAdd, Gtk::PACK_SHRINK);
  //top_box.pack_start(buttonbox, Gtk::PACK_SHRINK);

  //top_box.pack_start(notebook);

  buttonAdd.set_size_request(-1,30);
  buttonAdd.set_tooltip_text( _("Add a new layer") );
  buttonAddGroup.set_size_request(30,20);
  buttonAddGroup.set_tooltip_text( _("Add a new layer group") );
  buttonDel.set_size_request(30,20);
  buttonDel.set_tooltip_text( _("Remove selected layers") );

  buttonPresetLoad.set_tooltip_text( _("Load an existing preset") );
  buttonPresetLoad.set_size_request(108, 26);
  buttonPresetSave.set_tooltip_text( _("Save the selected layers as a preset") );
  buttonPresetSave.set_size_request(108, 26);

  buttonbox.set_spacing(5);
  //buttonbox.set_border_width(4);
  //buttonbox.pack_start(buttonAdd, Gtk::PACK_SHRINK);
  //buttonbox.pack_start(buttonAddGroup, Gtk::PACK_SHRINK);
  //buttonbox.pack_start(buttonDel, Gtk::PACK_SHRINK);
  buttonbox.pack_end(buttonPresetSave, Gtk::PACK_SHRINK);
  buttonbox.pack_end(buttonPresetLoad, Gtk::PACK_SHRINK);
  //buttonbox.set_layout(Gtk::BUTTONBOX_START);

  controls_scrolled_window.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  controls_scrolled_window.set_shadow_type( Gtk::SHADOW_NONE );
  controls_scrolled_window.set_size_request( 250, 0 );
  //controls_scrolled_window.add( controls_group );

  //layers_panel.pack1( top_box, true, true );
  //layers_panel.pack2( controls_scrolled_window, true, true );
  //pack_start(layers_panel);

  //main_box.pack_start(tool_buttons_box, Gtk::PACK_SHRINK);
  main_box.pack_start(vbox, Gtk::PACK_EXPAND_WIDGET);
  vbox.set_spacing(4);
  //vbox.pack_start(notebook, Gtk::PACK_EXPAND_WIDGET);
  //vbox.pack_start( buttonbox, Gtk::PACK_SHRINK );
  //vbox.pack_start( tool_buttons_box, Gtk::PACK_SHRINK );
  top_box.pack_start( main_box, Gtk::PACK_EXPAND_WIDGET );
  //top_box.pack_start( buttonbox, Gtk::PACK_SHRINK );
  pack_start( top_box );

#define ADD_TOOL_ITEM( a, b ) { \
    Gtk::MenuItem* tool_item = new Gtk::MenuItem( a ); \
    submenu->append( *tool_item ); \
    tool_item->signal_activate().connect( \
        sigc::bind<std::string>( \
            sigc::mem_fun(*this,&PF::LayerWidget::add_tool), b ) ); \
}


  Gtk::MenuItem* item = new Gtk::MenuItem(_("color"));
  tools_menu.append( *item );
  Gtk::Menu* submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  Gtk::MenuItem* tool_item = new Gtk::MenuItem( _("Color profile conversion") );
  submenu->append( *tool_item );
  tool_item->signal_activate().connect(
      sigc::bind<std::string>(
          sigc::mem_fun(*this,&PF::LayerWidget::add_tool), "convert_colorspace" ) );
  ADD_TOOL_ITEM(_("White Balance"), "white_balance")
  ADD_TOOL_ITEM( _("Basic Adjustments"), "basic_adjustments" );
  ADD_TOOL_ITEM( _("Color Adjustments"), "color_correction" );
  ADD_TOOL_ITEM( _("Desaturate"), "desaturate" );
  ADD_TOOL_ITEM( _("Curves"), "curves" );
  ADD_TOOL_ITEM( _("Channel selector"), "clone" );
  ADD_TOOL_ITEM( _("Relight"), "relight" );
  ADD_TOOL_ITEM( _("Shadows/Highlights"), "shadows_highlights_v2" );
  ADD_TOOL_ITEM( _("Tone mapping"), "tone_mapping_v2" );
  ADD_TOOL_ITEM( _("Dynamic range compressor"), "dynamic_range_compressor_v2" );
  ADD_TOOL_ITEM( _("Shadows/Highlights old"), "shadows_highlights" );
  ADD_TOOL_ITEM( _("Tone mapping old"), "tone_mapping" );
  ADD_TOOL_ITEM( _("Channel Mixer"), "channel_mixer" );
  //ADD_TOOL_ITEM( "Brightness/Contrast"), "brightness_contrast" );
  ADD_TOOL_ITEM( _("Noise"), "noise_generator" );
  ADD_TOOL_ITEM( _("Clip values"), "clip" );
  ADD_TOOL_ITEM( _("Apply LUT"), "gmic_emulate_film_user_defined" );
  ADD_TOOL_ITEM( _("Emulate film [color slide]"), "gmic_emulate_film_colorslide" );
  ADD_TOOL_ITEM( _("Emulate film [B&W]"), "gmic_emulate_film_bw" );
  ADD_TOOL_ITEM( _("Emulate film [instant consumer]"), "gmic_emulate_film_instant_consumer" );
  ADD_TOOL_ITEM( _("Emulate film [instant pro]"), "gmic_emulate_film_instant_pro" );
  ADD_TOOL_ITEM( _("Emulate film [negative color]"), "gmic_emulate_film_negative_color" );
  ADD_TOOL_ITEM( _("Emulate film [negative new]"), "gmic_emulate_film_negative_new" );
  ADD_TOOL_ITEM( _("Emulate film [negative old]"), "gmic_emulate_film_negative_old" );
  ADD_TOOL_ITEM( _("Emulate film [print films]"), "gmic_emulate_film_print_films" );
  ADD_TOOL_ITEM( _("Emulate film [various]"), "gmic_emulate_film_various" );

  item = new Gtk::MenuItem(_("detail"));
  tools_menu.append( *item );
  submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  ADD_TOOL_ITEM( _("Sharpen"), "sharpen" );
  ADD_TOOL_ITEM( _("Local contrast"), "local_contrast_v2" );
  ADD_TOOL_ITEM( _("Gaussian blur"), "gaussblur" );
  ADD_TOOL_ITEM( _("Guided filter"), "guided_filter" );
  ADD_TOOL_ITEM( _("Bilateral blur"), "blur_bilateral" );
  //ADD_TOOL_ITEM( _("Median filter"), "median_filter" );
  //ADD_TOOL_ITEM( _("CLAHE"), "clahe" );
  ADD_TOOL_ITEM( _("Gradient Norm"), "gmic_gradient_norm" );
  ADD_TOOL_ITEM( _("Split Details"), "split_details" );
  ADD_TOOL_ITEM( _("Defringe"), "defringe" );
  //ADD_TOOL_ITEM( _("Multi-level decomposition"), "gmic_split_details" );
  ADD_TOOL_ITEM( _("Noise reduction"), "denoise" );

  item = new Gtk::MenuItem(_("geometry"));
  tools_menu.append( *item );
  submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  ADD_TOOL_ITEM( _("Crop image"), "crop" );
  ADD_TOOL_ITEM( _("Scale & rotate image"), "scale" );
  ADD_TOOL_ITEM( _("Perspective correction"), "perspective" );
//#if !defined(__MINGW32__) && !defined(__MINGW64__)
  ADD_TOOL_ITEM( _("Optical corrections"), "lensfun" );
//#endif

  //#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef PF_DISABLE_GMIC
  item = new Gtk::MenuItem(_("G'MIC"));
  tools_menu.append( *item );
  submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  //ADD_TOOL_ITEM( "G'MIC Interpreter"), "gmic" );
  ADD_TOOL_ITEM( _("Dream Smoothing"), "gmic_dream_smooth" );
  ADD_TOOL_ITEM( _("Gradient Norm"), "gmic_gradient_norm" );
  //too generic ADD_TOOL_ITEM( _("Convolve"), "gmic_convolve" );
  //crashes ADD_TOOL_ITEM( _("Extract Foreground"), "gmic_extract_foreground" );
  //slow ADD_TOOL_ITEM( _("Inpaint [patch-based]"), "gmic_inpaint" );
  //RT algorithm is better? ADD_TOOL_ITEM( _("Despeckle"), "gmic_gcd_despeckle" );
  //crashes? ADD_TOOL_ITEM( _("Iain's Noise Reduction"), "gmic_iain_denoise" );
  ADD_TOOL_ITEM( _("Sharpen [richardson-lucy]"), "gmic_sharpen_rl" );
  //ADD_TOOL_ITEM( _("Denoise"), "gmic_denoise" );
  //ADD_TOOL_ITEM( _("Smooth [non-local means]"), "gmic_smooth_nlmeans" );
  ADD_TOOL_ITEM( _("Smooth [anisotropic]"), "gmic_smooth_anisotropic" );
  ADD_TOOL_ITEM( _("Smooth [bilateral]"), "gmic_blur_bilateral" );
  //ADD_TOOL_ITEM( _("Smooth [diffusion]"), "gmic_smooth_diffusion" );
  //ADD_TOOL_ITEM( _("Smooth [mean-curvature]"), "gmic_smooth_mean_curvature" );
  //ADD_TOOL_ITEM( _("Smooth [median]"), "gmic_smooth_median" );
  //ADD_TOOL_ITEM( _("Smooth [patch-based]"), "gmic_denoise" );
  //ADD_TOOL_ITEM( _("Smooth [selective gaussian]"), "gmic_smooth_selective_gaussian" );
  //ADD_TOOL_ITEM( _("Smooth [total variation]"), "gmic_smooth_total_variation" );
  //ADD_TOOL_ITEM( _("Smooth [wavelets]"), "gmic_smooth_wavelets_haar" );
  //ADD_TOOL_ITEM( _("Smooth [guided]"), "gmic_smooth_guided" );
  //ADD_TOOL_ITEM( _("Tone mapping"), "gmic_tone_mapping" );
  ADD_TOOL_ITEM( _("Transfer colors [advanced]"), "gmic_transfer_colors" );
  ADD_TOOL_ITEM( _("G'MIC Interpreter"), "gmic" );
#endif


  item = new Gtk::MenuItem(_("mask"));
  tools_menu.append( *item );
  submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  ADD_TOOL_ITEM( _("Uniform Fill"), "uniform");
  ADD_TOOL_ITEM( _("Invert"), "invert" );
  ADD_TOOL_ITEM( _("Threshold"), "threshold" );
  ADD_TOOL_ITEM( _("Curves"), "curves" );
  ADD_TOOL_ITEM( _("Gradient"), "gradient");
  ADD_TOOL_ITEM( _("Path"), "path_mask");
  ADD_TOOL_ITEM( _("H/S/L Mask"), "hsl_mask" );
  ADD_TOOL_ITEM( _("Gaussian blur"), "gaussblur" );
  //#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef PF_DISABLE_GMIC
  ADD_TOOL_ITEM( _("Gradient Norm"), "gmic_gradient_norm" );
#endif
  ADD_TOOL_ITEM( _("Draw"), "draw" );



  item = new Gtk::MenuItem(_("misc"));
  tools_menu.append( *item );
  submenu = new Gtk::Menu;
  item->set_submenu( *submenu );
  ADD_TOOL_ITEM( _("Draw"), "draw" );
  //ADD_TOOL_ITEM( _("Clone stamp"), "clone_stamp" );
  ADD_TOOL_ITEM( _("Channel selector"), "clone" );
  ADD_TOOL_ITEM( _("Buffer layer"), "buffer" );
  ADD_TOOL_ITEM( _("Digital watermark"), "gmic_watermark_fourier" );

  tools_menu.show_all();

  /*
    Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
    cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 
  */

  layers_view.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) );
  layers_view.get_tree().signal_row_expanded().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_expanded) );
  layers_view.get_tree().signal_row_collapsed().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_collapsed) );

  mask_view.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) );
  mask_view.get_tree().signal_row_expanded().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_expanded) );
  mask_view.get_tree().signal_row_collapsed().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_collapsed) );


  //layers_view.get_tree().signal_button_release_event().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_button_event) );

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = layers_view.get_tree().get_selection();
  refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));

  refTreeSelection = mask_view.get_tree().get_selection();
  refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));

  //notebook.signal_switch_page().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_switch_page) );

  //layer_frames.push_back( frame );
  layer_views.push_back( &layers_view );
  layer_views.push_back( &mask_view );
  active_view = 0;

  layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
  //image->get_layer_manager().signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
  image->signal_updated.connect(sigc::mem_fun(this, &LayerWidget::update_async) );

  signal_update.connect( sigc::bind(sigc::mem_fun(*this, &LayerWidget::update),false) );

  mask_view_back_button.signal_clicked().connect( sigc::mem_fun(*this,
                                                    &PF::LayerWidget::switch_to_layers_view) );
  mask_view_show_button.signal_toggled().connect(sigc::mem_fun(*this,
        &LayerWidget::toggle_mask));


  buttonAdd.signal_clicked().connect( sigc::mem_fun(*this,
                                                    &PF::LayerWidget::on_button_add) );
  buttonAddGroup.signal_clicked().connect( sigc::mem_fun(*this,
                                                         &PF::LayerWidget::on_button_add_group) );
  buttonDel.signal_clicked().connect( sigc::mem_fun(*this,
                                                    &PF::LayerWidget::on_button_del) );

  add_button.signal_clicked.connect( sigc::mem_fun(*this,
      &PF::LayerWidget::on_button_add) );
  group_button.signal_clicked.connect( sigc::mem_fun(*this,
      &PF::LayerWidget::on_button_add_group) );
  trash_button.signal_clicked.connect( sigc::mem_fun(*this,
      &PF::LayerWidget::on_button_del) );
  insert_image_button.signal_clicked.connect( sigc::mem_fun(*this,
      &PF::LayerWidget::on_button_add_image) );

  buttonPresetLoad.signal_clicked().
    connect(sigc::mem_fun(*this,
                          &PF::LayerWidget::on_button_load) );
  buttonPresetSave.signal_clicked().
    connect(sigc::mem_fun(*this,
                          &PF::LayerWidget::on_button_save) );


  add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

  show_all();
  //switch_to_layers_view();
  //switch_to_mask_view();
}


PF::LayerWidget::~LayerWidget()
{
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i;
  for(i = controls_dialogs.begin(); i != controls_dialogs.end(); i++) {
    i->second->get_position(controls_dialog_x,controls_dialog_y);
    i->second->hide();
    //i->second->set_controls(NULL);
    delete i->second;
  }
}


/*
bool PF::LayerWidget::on_button_event( GdkEventButton* button )
{
  return true;
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_button_event(): button "<<button->button<<" clicked."<<std::endl;
#endif
  if( button->button == 1 ) {
    int layer_id = get_selected_layer_id();
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_button_event(): selected layer id="<<layer_id<<std::endl;
#endif
    if( layer_id >= 0 )
      signal_edited_layer_changed.emit( layer_id );
  }
  return true;
}
*/


void PF::LayerWidget::update_controls()
{
  controls_group.update();
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i;
  for(i = controls_dialogs.begin(); i != controls_dialogs.end(); i++) {
    if( i->second ) i->second->update();
  }
}



void PF::LayerWidget::on_map()
{
  switch_to_layers_view();
  Gtk::VBox::on_map();
}




PF::OperationConfigGUI* PF::LayerWidget::get_selected_layer_ui()
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return NULL;
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      layer_views[page]->get_tree().get_selection();
  int layer_id = get_selected_layer_id();

  std::vector<Gtk::TreeModel::Path> selected_rows = refTreeSelection->get_selected_rows();
  std::vector<Gtk::TreeModel::Path>::iterator row_it = selected_rows.begin();
  if( row_it == selected_rows.end() ) {
    return NULL;
  }

  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( *row_it );
  if (iter) {
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return NULL;
    PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
    if( ui ) {
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
      return gui;
    }
  }
  return NULL;
}




void PF::LayerWidget::on_selection_changed()
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return;
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_chaged() called, page="<<page<<std::endl;
#endif
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      layer_views[page]->get_tree().get_selection();
  /*
          if( refTreeSelection->count_selected_rows() == 0 ) {
            Gtk::TreeModel::Children children = layer_views[page]->get_model()->children();
            refTreeSelection->select( children.begin() );
            return;
          }
   */
  int layer_id = get_selected_layer_id();
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_changed(): selected layer id="<<layer_id<<std::endl;
#endif

  std::vector<Gtk::TreeModel::Path> selected_rows = refTreeSelection->get_selected_rows();
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_chaged(): "<<selected_rows.size()<<" selected rows."<<std::endl;
#endif
  std::vector<Gtk::TreeModel::Path>::iterator row_it = selected_rows.begin();
  if( row_it == selected_rows.end() ) {
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_selection_changed(): calling controls_group.remove_all_controls()"<<std::endl;
#endif
    controls_group.remove_all_controls();
    aux_controls_group.clear();
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_selection_changed(): emitting signal_edited_layer_changed(-1)"<<std::endl;
#endif
    signal_edited_layer_changed.emit( -1 );
    return;
  }

  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( *row_it );
  if (iter) {
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
#ifndef NDEBUG
    std::cout<<"Selected row "<<l->get_name()<<std::endl;
    std::cout<<"LayerWidget::on_selection_changed(): emitting signal_edited_layer_changed("<<layer_id<<")"<<std::endl;
#endif
    signal_edited_layer_changed.emit( layer_id );
    if( PF::PhotoFlow::Instance().is_single_win_mode() ) {
      PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
      if( ui ) {
        PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
        if( gui && editor ) {
          editor->set_aux_controls( &(gui->get_aux_controls()) );
        }
        if( gui ) {
          if( gui->get_frame() && !floating_tool_dialogs ) {
            controls_group.add_control( l, gui );
          }
#ifndef NDEBUG
          std::cout<<"LayerWidget::on_selection_changed(): calling aux_controls_group.set_control( l, gui )"<<std::endl;
#endif
          aux_controls_group.set_control( l, gui );
          gui->open();
          gui->expand();
        }
      }
    }

    if( page == 0 ) {
      selected_layer_id = layer_id;
      LayerTree* view = layer_views[1];
      //view->get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) );

      //view->get_tree().signal_button_release_event().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_button_event) );
      //Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = view->get_tree().get_selection();
      //refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));

      /*
      Gtk::CellRendererToggle* cell =
      dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
      cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) );
       */

      view->set_layers( &(l->get_omap_layers()) );
      view->update_model();
      //Widget* page = notebook.get_nth_page(-1);
      //Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
      //label->set_angle(90);
      //view->show_all();
      //notebook.set_current_page( 1 );
      //frame->show();
    }

  } else {
    if( page == 0 )
      selected_layer_id = -1;
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_selection_changed(): calling controls_group.remove_all_controls()"<<std::endl;
#endif
    controls_group.remove_all_controls();
    aux_controls_group.clear();
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_selection_changed(): emitting signal_edited_layer_changed(-1)"<<std::endl;
#endif
    signal_edited_layer_changed.emit( -1 );
  }

#ifndef NDEBUG
    std::cout<<"LayerWidget::on_selection_changed(): calling editor->set_selected_layer("<<selected_layer_id<<")"<<std::endl;
#endif
  editor->set_selected_layer( selected_layer_id );

  return;

  /*
  if( !PF::PhotoFlow::Instance().is_single_win_mode() ) return;

  controls_group.clear();

  int page = notebook.get_current_page();
  if( page < 0 ) return;
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      layer_views[page]->get_tree().get_selection();
  if( refTreeSelection->count_selected_rows() == 0 )
    return;

  int layer_id = get_selected_layer_id();
//#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_changed(): selected layer id="<<layer_id<<std::endl;
//#endif
  if( layer_id >= 0 )
    signal_edited_layer_changed.emit( layer_id );

  std::vector<Gtk::TreeModel::Path> selected_rows = refTreeSelection->get_selected_rows();
//#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_chaged(): "<<selected_rows.size()<<" selected rows."<<std::endl;
//#endif
  std::vector<Gtk::TreeModel::Path>::iterator row_it = selected_rows.begin();
  while( row_it != selected_rows.end() ) {
    Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( *row_it );
    if (iter) {
      PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
      bool visible = (*iter)[columns.col_visible];
      PF::Layer* l = (*iter)[columns.col_layer];
      if( !l ) return;
//#ifndef NDEBUG
      std::cout<<"Selected row "<<l->get_name()<<std::endl;
//#endif

      if( PF::PhotoFlow::Instance().is_single_win_mode() ) {
        PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
        if( ui ) {
          PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
          if( gui && gui->get_frame() ) {
            controls_group.add_control( gui->get_frame() );
          }
        }
      }
    }
    row_it++;
  }
  controls_group.show_all_children();
  */
}


void PF::LayerWidget::on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return;
  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( path );
  if (iter) {
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_row_activated: activated row "<<l->get_name()<<std::endl;
#endif
    if( column == layer_views[page]->get_tree().get_column(LAYER_COL_NUM) && floating_tool_dialogs ) {
      // close all dialogs
      if( l && l->get_processor() && l->get_processor()->get_par() &&
          l->get_processor()->get_par()->get_config_ui() ) {

        controls_dialog_open(l);
      }
    }
    if( column == layer_views[page]->get_tree().get_column(IMAP_COL_NUM) ) {
      if( !l->get_processor()->get_par()->has_intensity() )
        return;
#ifndef NDEBUG
      std::cout<<"Activated IMap column of row "<<l->get_name()<<std::endl;
#endif
      //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();
      
      int tab_id = -1;//get_map_tab( &(l->get_imap_layers()) );
      if( tab_id >= 0 ) {
        //notebook.set_current_page( tab_id );
        return;
      }

      LayerTree* view = new LayerTree( editor, true );
      //frame->add( *view );
      
      //view->set_reorderable();
      
      view->get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) ); 

      //view->get_tree().signal_button_release_event().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_button_event) );
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = view->get_tree().get_selection();
      refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));

      /*
        Gtk::CellRendererToggle* cell = 
        dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
        cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 
      */
      /*
      HTabLabelWidget* tabwidget =
        new HTabLabelWidget( std::string(_("intensity ("))+l->get_name()+")",
                            view );
      tabwidget->signal_close.connect( sigc::mem_fun(*this, &PF::LayerWidget::remove_tab) ); 
      notebook.append_page( *view, *tabwidget );

      int pagenum = notebook.get_n_pages();
      layer_views.push_back(view);
      view->set_layers( &(l->get_imap_layers()) );
      view->update_model();
      Widget* page = notebook.get_nth_page(-1);
      //Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
      //label->set_angle(90);
      //view->show_all();
      notebook.set_current_page( -1 );
      //frame->show();
       */
      return;
    }
    if( column == layer_views[page]->get_tree().get_column(OMAP_COL_NUM) ) {
      if( !l->get_processor()->get_par()->has_opacity() )
        return;
#ifndef NDEBUG
      std::cout<<"Activated OMap column of row "<<l->get_name()<<std::endl;
#endif
      //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();
      
      int tab_id = -1;//get_map_tab( &(l->get_omap_layers()) );
      if( tab_id >= 0 ) {
        //notebook.set_current_page( tab_id );
        return;
      }
      switch_to_mask_view();
/*
      LayerTree* view = new LayerTree( editor, true );
      //frame->add( *view );
      
      view->set_reorderable();
      
      view->get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) ); 

      //view->get_tree().signal_button_release_event().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_button_event) );
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = view->get_tree().get_selection();
      refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));
*/
      /*
        Gtk::CellRendererToggle* cell = 
        dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
        cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 
      */
      /*
      HTabLabelWidget* tabwidget =
        new HTabLabelWidget( std::string(_("opacity ("))+l->get_name()+")",
                            view );
      tabwidget->signal_close.connect( sigc::mem_fun(*this, &PF::LayerWidget::remove_tab) ); 
      notebook.append_page( *view, *tabwidget );

      int pagenum = notebook.get_n_pages();
      layer_views.push_back(view);
      view->set_layers( &(l->get_omap_layers()) );
      view->update_model();
      Widget* page = notebook.get_nth_page(-1);
      //Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
      //label->set_angle(90);
      view->show_all();
      notebook.set_current_page( -1 );
      //frame->show();
       */

      return;
    }

    /*
    PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
    if( ui ) {
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui && gui->get_frame() ) {
        controls_group.add_control( l, gui );
        gui->open();
        gui->expand();
      }
      controls_group.show_all_children();
    }
    */
  }
}


void PF::LayerWidget::on_row_expanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path )
{
//  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return;
  if (iter) {
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_row_expanded() called"<<std::endl;
#endif
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    if( l->is_expanded() ) return;
    l->set_expanded( true );
    //std::cout<<"LayerWidget::on_row_expanded(): layer expanded flag set"<<std::endl;
    layer_views[page]->get_tree().columns_autosize();
    layer_views[page]->set_tree_modified();
  }
}


void PF::LayerWidget::on_row_collapsed( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path )
{
//  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return;
  if (iter) {
#ifndef NDEBUG
    std::cout<<"LayerWidget::on_row_collapsed() called"<<std::endl;
#endif
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    if( !l->is_expanded() ) return;
    l->set_expanded( false );
    //std::cout<<"LayerWidget::on_row_collapsed(): layer expanded flag reset"<<std::endl;
    layer_views[page]->get_tree().columns_autosize();
    layer_views[page]->set_tree_modified();
  }
}


int PF::LayerWidget::get_selected_layer_id()
{
  int result = -1;

//  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  return layer_views[page]->get_selected_layer_id();

  /*
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];
    if(l) result = l->get_id();
  }

  return( result );
  */
}



bool PF::LayerWidget::get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter)
{
//  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  for(  Gtk::TreeModel::iterator it = rows.begin();
        it != rows.end(); it++ ) {
    //Gtk::TreeModel::Row row = *it;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*it)[columns.col_layer];
    if(l && ((int)(l->get_id())==id)) {
      iter = it;
      return true;
    }
    Gtk::TreeModel::Children children = it->children();
    if( !children.empty() ) {
      if( get_row( id, children, iter ) )
        return true;
    }
  }
  return false;
}



bool PF::LayerWidget::get_row(int id, Gtk::TreeModel::iterator& iter)
{
  //  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();
  const Gtk::TreeModel::Children rows = model->children();
  return get_row( id, rows, iter );
}


void PF::LayerWidget::select_row(int id)
{
  //  int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  Gtk::TreeModel::iterator iter;
  if( get_row( id, iter ) ) {
    refTreeSelection->select( iter );
#ifndef NDEBUG
    std::cout<<"LayerWidget::select_row("<<id<<"): emitting signal_edited_layer_changed"<<std::endl;
#endif
    signal_edited_layer_changed.emit( id );
  }
}


/*
void PF::LayerWidget::remove_tab( Gtk::Widget* widget )
{
//#ifndef NDEBUG
  std::cout<<"PF::LayerWidget::remove_tab() called."<<std::endl;
//#endif
  int page = notebook.page_num( *widget );
  if( page < 0 ) return;
  if( page >= notebook.get_n_pages() ) return;

  Gtk::Widget* tabwidget = notebook.get_tab_label( *widget );

  Gtk::Widget* widget2 = notebook.get_nth_page( page );
  if( widget != widget2 ) return;

  for( unsigned int i = 0; i < layer_views.size(); i++ ) {
    if( layer_views[i] != widget ) continue;
    layer_views.erase( layer_views.begin()+i );
    break;
  }

  notebook.remove_page( page );
  delete( widget );
  if( tabwidget )
    delete( tabwidget );
//#ifndef NDEBUG
  std::cout<<"PF::LayerWidget::remove_tab() page #"<<page<<" removed."<<std::endl;
//#endif
}
*/


void PF::LayerWidget::controls_dialog_open(PF::Layer* l)
{
  // hide the currently opened dialog, if required in the global options
  if( PF::PhotoFlow::Instance().get_options().get_ui_multiple_tool_dialogs() == false )
    controls_dialog_hide();

  // look for an existing dialog for the selected tool
  PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
  PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i =
      controls_dialogs.find(gui);
  // create the dialog if not existing yet
  PF::ControlsDialog* dialog = NULL;
  if( i == controls_dialogs.end() ) {
    dialog = new ControlsDialog(editor);
    dialog->set_gravity( Gdk::GRAVITY_STATIC );
    dialog->set_controls(l);
    if( gui ) {
      controls_dialogs.insert( std::make_pair(gui, dialog) );
    }
    Gtk::Container* toplevel = get_toplevel();
    Gtk::Window* toplevelwin = NULL;
#ifdef GTKMM_2
    if( toplevel && toplevel->is_toplevel() )
#endif
#ifdef GTKMM_3
      if( toplevel && toplevel->get_is_toplevel() )
#endif
        toplevelwin = dynamic_cast<Gtk::Window*>(toplevel);
    if( toplevelwin ) dialog->set_transient_for(*toplevelwin);
  } else {
    dialog = i->second;
  }
  controls_dialog_visible = true;
  dialog->open();
}


void PF::LayerWidget::controls_dialog_show()
{
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i;
  for(i = controls_dialogs.begin(); i != controls_dialogs.end(); i++) {
    if( i->second && i->second->is_visible() ) i->second->open();
  }
}

void PF::LayerWidget::controls_dialog_hide()
{
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i;
  for(i = controls_dialogs.begin(); i != controls_dialogs.end(); i++) {
    if( i->second ) i->second->hide();
  }
}

void PF::LayerWidget::controls_dialog_delete(PF::Layer* l)
{
#ifndef NDEBUG
    std::cout<<"LayerWidget::controls_dialog_delete: deleting controls of layer \""<<l->get_name()<<"\""<<std::endl;
#endif
  // look for an existing dialog for the selected tool
  PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
  PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*>::iterator i =
      controls_dialogs.find(gui);
  if( i != controls_dialogs.end() ) {
    i->second->get_position(controls_dialog_x,controls_dialog_y);
    i->second->hide();
    i->second->set_controls(NULL);
    delete i->second;
    controls_dialogs.erase(gui);
  }

  controls_dialog_delete( l->get_omap_layers() );
  controls_dialog_delete( l->get_imap_layers() );
  controls_dialog_delete( l->get_sublayers() );
}





void PF::LayerWidget::controls_dialog_delete( std::list<Layer*>& layers )
{
#ifndef NDEBUG
  std::cout<<"LayerWidget::controls_dialog_delete( std::list<Layer*>& layers ): layers.size()="<<layers.size()<<std::endl;
#endif
  for( std::list<Layer*>::iterator li = layers.begin(); li != layers.end(); li++ ) {
    if( !(*li) ) continue;
    controls_dialog_delete( *li );
  }
}



void PF::LayerWidget::on_button_add()
{
  std::cout<<"LayerWidget::on_button_add: tools_menu.popup()"<<std::endl;
  tools_menu.popup(1, 0);
  return;

  Gtk::Container* toplevel = get_toplevel();
#ifdef GTKMM_2
  if( toplevel && toplevel->is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
#ifdef GTKMM_3
  if( toplevel && toplevel->get_is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
    operationsDialog.set_transient_for( *(dynamic_cast<Gtk::Window*>(toplevel)) );
  operationsDialog.open();
}



void PF::LayerWidget::on_button_add_image()
{
  Gtk::FileChooserDialog dialog( _("Open image"),
      Gtk::FILE_CHOOSER_ACTION_OPEN);

  Gtk::Container* toplevel = get_toplevel();
#ifdef GTKMM_2
  if( toplevel && toplevel->is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
#ifdef GTKMM_3
  if( toplevel && toplevel->get_is_toplevel() && dynamic_cast<Gtk::Window*>(toplevel) )
#endif
    dialog.set_transient_for( *(dynamic_cast<Gtk::Window*>(toplevel)) );

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Glib::RefPtr<Gtk::FileFilter> filter_pfi = Gtk::FileFilter::create();
  //filter_pfi->set_name("Photoflow files");
  //filter_pfi->add_pattern("*.pfi");
  //dialog.add_filter(filter_pfi);

#ifdef GTKMM_2
  Gtk::FileFilter filter_tiff;
  filter_tiff.set_name( _("Image files") );
  filter_tiff.add_mime_type("image/tiff");
  filter_tiff.add_mime_type("image/jpeg");
  filter_tiff.add_mime_type("image/png");
  filter_tiff.add_mime_type("image/x-3fr");
  filter_tiff.add_mime_type("image/x-adobe-dng");
  filter_tiff.add_mime_type("image/x-arw;image/x-bay");
  filter_tiff.add_mime_type("image/x-canon-cr2");
  filter_tiff.add_mime_type("image/x-canon-crw");
  filter_tiff.add_mime_type("image/x-cap");
  filter_tiff.add_mime_type("image/x-cr2");
  filter_tiff.add_mime_type("image/x-crw");
  filter_tiff.add_mime_type("image/x-dcr");
  filter_tiff.add_mime_type("image/x-dcraw");
  filter_tiff.add_mime_type("image/x-dcs");
  filter_tiff.add_mime_type("image/x-dng");
  filter_tiff.add_mime_type("image/x-drf");
  filter_tiff.add_mime_type("image/x-eip");
  filter_tiff.add_mime_type("image/x-erf");
  filter_tiff.add_mime_type("image/x-fff");
  filter_tiff.add_mime_type("image/x-fuji-raf");
  filter_tiff.add_mime_type("image/x-iiq");
  filter_tiff.add_mime_type("image/x-k25");
  filter_tiff.add_mime_type("image/x-kdc");
  filter_tiff.add_mime_type("image/x-mef");
  filter_tiff.add_mime_type("image/x-minolta-mrw");
  filter_tiff.add_mime_type("image/x-mos");
  filter_tiff.add_mime_type("image/x-mrw");
  filter_tiff.add_mime_type("image/x-nef");
  filter_tiff.add_mime_type("image/x-nikon-nef");
  filter_tiff.add_mime_type("image/x-nrw");
  filter_tiff.add_mime_type("image/x-olympus-orf");
  filter_tiff.add_mime_type("image/x-orf");
  filter_tiff.add_mime_type("image/x-panasonic-raw");
  filter_tiff.add_mime_type("image/x-pef");
  filter_tiff.add_mime_type("image/x-pentax-pef");
  filter_tiff.add_mime_type("image/x-ptx");
  filter_tiff.add_mime_type("image/x-pxn");
  filter_tiff.add_mime_type("image/x-r3d");
  filter_tiff.add_mime_type("image/x-raf");
  filter_tiff.add_mime_type("image/x-raw");
  filter_tiff.add_mime_type("image/x-rw2");
  filter_tiff.add_mime_type("image/x-rwl");
  filter_tiff.add_mime_type("image/x-rwz");
  filter_tiff.add_mime_type("image/x-sigma-x3f");
  filter_tiff.add_mime_type("image/x-sony-arw");
  filter_tiff.add_mime_type("image/x-sony-sr2");
  filter_tiff.add_mime_type("image/x-sony-srf");
  filter_tiff.add_mime_type("image/x-sr2");
  filter_tiff.add_mime_type("image/x-srf");
  filter_tiff.add_mime_type("image/x-x3f");
  filter_tiff.add_mime_type("image/x-exr");
#ifdef WIN32
  /*filter_tiff.add_pattern("*.EXR");*/
  filter_tiff.add_pattern("*.3FR");
  filter_tiff.add_pattern("*.ARI");
  filter_tiff.add_pattern("*.ARW");
  filter_tiff.add_pattern("*.CAP");
  filter_tiff.add_pattern("*.CINE");
  filter_tiff.add_pattern("*.CR2");
  filter_tiff.add_pattern("*.CRW");
  filter_tiff.add_pattern("*.CS1");
  filter_tiff.add_pattern("*.DC2");
  filter_tiff.add_pattern("*.DCR");
  filter_tiff.add_pattern("*.DNG");
  filter_tiff.add_pattern("*.EFR");
  filter_tiff.add_pattern("*.FFF");
  filter_tiff.add_pattern("*.IA");
  filter_tiff.add_pattern("*.IIQ");
  filter_tiff.add_pattern("*.K25");
  filter_tiff.add_pattern("*.KC2");
  filter_tiff.add_pattern("*.KDC");
  filter_tiff.add_pattern("*.MDC");
  filter_tiff.add_pattern("*.MEF");
  filter_tiff.add_pattern("*.MOS");
  filter_tiff.add_pattern("*.MRW");
  filter_tiff.add_pattern("*.NEF");
  filter_tiff.add_pattern("*.NRW");
  filter_tiff.add_pattern("*.ORF");
  filter_tiff.add_pattern("*.ORI");
  filter_tiff.add_pattern("*.PEF");
  filter_tiff.add_pattern("*.PXN");
  filter_tiff.add_pattern("*.QTK");
  filter_tiff.add_pattern("*.R3D");
  filter_tiff.add_pattern("*.RAF");
  filter_tiff.add_pattern("*.RAW");
  filter_tiff.add_pattern("*.RDC");
  filter_tiff.add_pattern("*.RW2");
  filter_tiff.add_pattern("*.RWL");
  filter_tiff.add_pattern("*.SR2");
  filter_tiff.add_pattern("*.SRF");
  filter_tiff.add_pattern("*.SRW");
  filter_tiff.add_pattern("*.STI");
  filter_tiff.add_pattern("*.X3F");
#endif
  filter_tiff.add_pattern("*.pfi");
  Gtk::FileFilter filter_all;
  filter_all.set_name( _("All files") );
  filter_all.add_pattern("*.*");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_tiff = Gtk::FileFilter::create();
  filter_tiff->set_name( _("Image files") );
  filter_tiff->add_mime_type("image/tiff");
  filter_tiff->add_mime_type("image/jpeg");
  filter_tiff->add_mime_type("image/png");
  filter_tiff->add_mime_type("image/x-3fr");
  filter_tiff->add_mime_type("image/x-adobe-dng");
  filter_tiff->add_mime_type("image/x-arw;image/x-bay");
  filter_tiff->add_mime_type("image/x-canon-cr2");
  filter_tiff->add_mime_type("image/x-canon-crw");
  filter_tiff->add_mime_type("image/x-cap");
  filter_tiff->add_mime_type("image/x-cr2");
  filter_tiff->add_mime_type("image/x-crw");
  filter_tiff->add_mime_type("image/x-dcr");
  filter_tiff->add_mime_type("image/x-dcraw");
  filter_tiff->add_mime_type("image/x-dcs");
  filter_tiff->add_mime_type("image/x-dng");
  filter_tiff->add_mime_type("image/x-drf");
  filter_tiff->add_mime_type("image/x-eip");
  filter_tiff->add_mime_type("image/x-erf");
  filter_tiff->add_mime_type("image/x-fff");
  filter_tiff->add_mime_type("image/x-fuji-raf");
  filter_tiff->add_mime_type("image/x-iiq");
  filter_tiff->add_mime_type("image/x-k25");
  filter_tiff->add_mime_type("image/x-kdc");
  filter_tiff->add_mime_type("image/x-mef");
  filter_tiff->add_mime_type("image/x-minolta-mrw");
  filter_tiff->add_mime_type("image/x-mos");
  filter_tiff->add_mime_type("image/x-mrw");
  filter_tiff->add_mime_type("image/x-nef");
  filter_tiff->add_mime_type("image/x-nikon-nef");
  filter_tiff->add_mime_type("image/x-nrw");
  filter_tiff->add_mime_type("image/x-olympus-orf");
  filter_tiff->add_mime_type("image/x-orf");
  filter_tiff->add_mime_type("image/x-panasonic-raw");
  filter_tiff->add_mime_type("image/x-pef");
  filter_tiff->add_mime_type("image/x-pentax-pef");
  filter_tiff->add_mime_type("image/x-ptx");
  filter_tiff->add_mime_type("image/x-pxn");
  filter_tiff->add_mime_type("image/x-r3d");
  filter_tiff->add_mime_type("image/x-raf");
  filter_tiff->add_mime_type("image/x-raw");
  filter_tiff->add_mime_type("image/x-rw2");
  filter_tiff->add_mime_type("image/x-rwl");
  filter_tiff->add_mime_type("image/x-rwz");
  filter_tiff->add_mime_type("image/x-sigma-x3f");
  filter_tiff->add_mime_type("image/x-sony-arw");
  filter_tiff->add_mime_type("image/x-sony-sr2");
  filter_tiff->add_mime_type("image/x-sony-srf");
  filter_tiff->add_mime_type("image/x-sr2");
  filter_tiff->add_mime_type("image/x-srf");
  filter_tiff->add_mime_type("image/x-x3f");
  filter_tiff->add_mime_type("image/x-exr");
#ifdef WIN32
  /*filter_tiff->add_pattern("*.exr");*/
  filter_tiff->add_pattern("*.3fr");
  filter_tiff->add_pattern("*.ari");
  filter_tiff->add_pattern("*.arw");
  filter_tiff->add_pattern("*.cap");
  filter_tiff->add_pattern("*.cine");
  filter_tiff->add_pattern("*.cr2");
  filter_tiff->add_pattern("*.crw");
  filter_tiff->add_pattern("*.cs1");
  filter_tiff->add_pattern("*.dc2");
  filter_tiff->add_pattern("*.dcr");
  filter_tiff->add_pattern("*.dng");
  filter_tiff->add_pattern("*.erf");
  filter_tiff->add_pattern("*.fff");
  filter_tiff->add_pattern("*.ia");
  filter_tiff->add_pattern("*.iiq");
  filter_tiff->add_pattern("*.k25");
  filter_tiff->add_pattern("*.kc2");
  filter_tiff->add_pattern("*.kdc");
  filter_tiff->add_pattern("*.mdc");
  filter_tiff->add_pattern("*.mef");
  filter_tiff->add_pattern("*.mos");
  filter_tiff->add_pattern("*.mrw");
  filter_tiff->add_pattern("*.nef");
  filter_tiff->add_pattern("*.nrw");
  filter_tiff->add_pattern("*.orf");
  filter_tiff->add_pattern("*.ori");
  filter_tiff->add_pattern("*.pef");
  filter_tiff->add_pattern("*.pxn");
  filter_tiff->add_pattern("*.qtk");
  filter_tiff->add_pattern("*.r3d");
  filter_tiff->add_pattern("*.raf");
  filter_tiff->add_pattern("*.raw");
  filter_tiff->add_pattern("*.rdc");
  filter_tiff->add_pattern("*.rw2");
  filter_tiff->add_pattern("*.rwl");
  filter_tiff->add_pattern("*.sr2");
  filter_tiff->add_pattern("*.srf");
  filter_tiff->add_pattern("*.srw");
  filter_tiff->add_pattern("*.sti");
  filter_tiff->add_pattern("*.x3f");
#endif
  filter_tiff->add_pattern("*.pfi");
  Glib::RefPtr<Gtk::FileFilter> filter_all = Gtk::FileFilter::create();
  filter_all->set_name( _("All files") );
  filter_all->add_pattern("*.*");
#endif
  dialog.add_filter(filter_tiff);
  dialog.add_filter(filter_all);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_image_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
        {
#ifndef NDEBUG
    std::cout << "Open clicked." << std::endl;
#endif
    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_image_folder( last_dir );

    //Notice that this is a std::string, not a Glib::ustring.
    std::string filename = dialog.get_filename();
#ifndef NDEBUG
    std::cout << "File selected: " <<  filename << std::endl;
#endif
    char* fullpath = realpath( filename.c_str(), NULL );
    if(!fullpath)
      return;
    insert_image( fullpath );
    free(fullpath);
    break;
        }
  case(Gtk::RESPONSE_CANCEL):
        {
#ifndef NDEBUG
    std::cout << "Cancel clicked." << std::endl;
#endif
    break;
        }
  default:
#ifndef NDEBUG
    std::cout << "Unexpected button clicked." << std::endl;
#endif
    break;
  }
}



void PF::LayerWidget::add_tool( std::string op_type )
{
  std::cout<<"OperationsTreeDialog::add_layer(): image="<<image<<std::endl;
  if( !image ) return;

  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  std::cout<<"OperationsTreeDialog::add_layer(): layer="<<layer<<std::endl;
  if( !layer ) return;


  PF::ProcessorBase* processor =
      PF::PhotoFlow::Instance().new_operation( op_type.c_str(), layer );
  if( !processor || !processor->get_par() ) return;
  PF::OperationConfigUI* ui = dynamic_cast<PF::OperationConfigUI*>( processor->get_par()->get_config_ui() );
  if( processor->get_par()->get_default_name().empty() )
    layer->set_name( _("New Layer") );
  else
    layer->set_name( processor->get_par()->get_default_name() );

  if( processor ) {
    add_layer( layer );
    if( ui ) {
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if(dialog) {
        if( dialog ) {
          dialog->open();
        }
      }
    }
  }
}



void PF::LayerWidget::add_layer( PF::Layer* layer, bool do_update )
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  layer_views[page]->set_tree_modified();

  bool is_map = layer_views[page]->is_map();
  layer->get_processor()->get_par()->
    set_map_flag( is_map );
  layer->get_blender()->get_par()->
    set_map_flag( is_map );
    
#ifndef NDEBUG
  std::cout<<std::endl<<std::endl;
  std::cout<<"LayerWidget::add_layer(): layer_views.size()="<<layer_views.size()<<"  page="<<page<<std::endl;
#endif

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows =
      refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
#ifndef NDEBUG
    std::cout<<"LayerWidget::add_layer(): Selected path: "<<sel_rows[0].to_string()<<std::endl;
#endif
    iter = model->get_iter( sel_rows[0] );
  }
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];

    Gtk::TreeModel::iterator parent = row.parent();
    if( parent ) {
      // this is a sub-layer of a group layer
      PF::Layer* pl = (*parent)[columns.col_layer];
      if( !pl ) return;
      pl->sublayers_insert( layer, l ? l->get_id() : -1 );
      image->get_layer_manager().modified();
    } else {

#ifndef NDEBUG
      std::cout<<"LayerWidget::add_layer(): Adding layer \""<<layer->get_name()
                       <<" above layer \""<<l->get_name()<<"\""<<std::endl;
#endif

      //image->get_layer_manager().insert_layer( layer, l->get_id() );
      PF::insert_layer( *(layer_views[page]->get_layers()), layer, l->get_id() );
    }
  } else {
    // Nothing selected, we add the layer on top of the stack
#ifndef NDEBUG
    std::cout<<"LayerWidget::add_layer(): Adding layer \""<<layer->get_name()
                     <<"\" on top of stack"<<std::endl;
#endif
    //image->get_layer_manager().insert_layer( layer );
    PF::insert_layer( *(layer_views[page]->get_layers()), layer, -1 );
#ifndef NDEBUG
    std::cout<<"LayerWidget::add_layer(): layer_views[page]->get_layers()->size()="<<layer_views[page]->get_layers()->size()<<std::endl;
#endif
  }

  if( do_update ) {

    //layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
    /*
  if( layer->get_processor() && layer->get_processor()->get_par() ) {
    PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( layer->get_processor()->get_par()->get_config_ui() );
    if( ui ) ui->set_editor( editor );
  }
     */
#ifndef NDEBUG
    std::cout<<"LayerWidget::add_layer(): calling update() to update layers tree"<<std::endl;
#endif
    update();
    //std::cout<<"LayerWidget::add_layer(): image update request submitted"<<std::endl;

    layer_views[page]->unselect_all();
    //std::cout<<"LayerWidget::add_layer(): calling select_row("<<layer->get_id()<<")"<<std::endl;
    select_row( layer->get_id() );

    PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
    if( ui ) {
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui ) {
        gui->enable_editing();
        aux_controls_group.set_control( layer, gui );
        if( gui->get_frame() ) {
          if( floating_tool_dialogs ) {
            controls_dialog_open(layer);
          } else {
            controls_group.add_control( layer, gui );
#ifndef NDEBUG
            std::cout<<"LayerWidget::add_layer(): calling gui->open()"<<std::endl;
#endif
            gui->open();
#ifndef NDEBUG
            std::cout<<"LayerWidget::add_layer(): calling gui->expand()"<<std::endl;
#endif
            gui->expand();
          }
        }
      }
      controls_group.show_all_children();
    }
#ifndef NDEBUG
    std::cout<<"LayerWidget::add_layer(): calling image->get_layer_manager().modified()"<<std::endl;
#endif
    image->get_layer_manager().modified();
  }
}


void PF::LayerWidget::insert_image( std::string filename )
{
  std::string ext;
  if( !PF::getFileExtensionLowcase( "/", filename, ext ) ) return;

  if( ext == "pfi" ) {
  } else if( ext=="tiff" || ext=="tif" || ext=="jpg" || ext=="jpeg" || ext=="png" || ext=="exr" ) {

    std::cout<<"Inserting raster image "<<filename<<std::endl;

    if( !image ) return;

    PF::LayerManager& layer_manager = image->get_layer_manager();
    PF::Layer* gl = layer_manager.new_layer();
    if( !gl ) return;
    gl->set_name( _("image") );
    gl->set_normal( false );

    PF::ProcessorBase* processor = new_buffer();
    gl->set_processor( processor );

    PF::ProcessorBase* blender = new_blender();
    gl->set_blender( blender );

    PF::OperationConfigGUI* dialog =
      new PF::OperationConfigGUI( gl, Glib::ustring(_("Group Layer Config")) );
    processor->get_par()->set_config_ui( dialog );

    //PF::Layer* layer = layer_manager.new_layer();
    //if( !layer ) return;

    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "imageread", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( _("image file") );
    gl->sublayers_insert( limg, -1 );

    add_layer( gl );
  } else {

    std::cout<<"Inserting raw image "<<filename<<std::endl;

    if( !image ) return;

    PF::LayerManager& layer_manager = image->get_layer_manager();
    PF::Layer* gl = layer_manager.new_layer();
    if( !gl ) return;
    gl->set_name( _("RAW image") );
    gl->set_normal( false );

    PF::ProcessorBase* processor = new_buffer();
    gl->set_processor( processor );

    PF::ProcessorBase* blender = new_blender();
    gl->set_blender( blender );

    PF::OperationConfigGUI* dialog =
      new PF::OperationConfigGUI( gl, Glib::ustring(_("Group Layer Config")) );
    processor->get_par()->set_config_ui( dialog );

    // RAW loader layer
    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "raw_loader", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( "RAW loader" );
    gl->sublayers_insert( limg, -1 );

    // RAW processor
    PF::Layer* limg2 = layer_manager.new_layer();
    PF::ProcessorBase* proc2 = PF::PhotoFlow::Instance().new_operation( "raw_developer_v2", limg2 );
    limg2->set_processor( proc2 );
    limg2->set_name( "RAW developer" );
    gl->sublayers_insert( limg2, -1 );

    add_layer( gl );
  }
}


void PF::LayerWidget::insert_preset( std::string filename )
{
  //int page = notebook.get_current_page();
  int page = active_view;

  layer_views[page]->set_tree_modified();

#ifndef NDEBUG
  std::cout<<"LayerWidget::insert_preset(): layer_views.size()="<<layer_views.size()<<std::endl;
#endif

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
#ifndef NDEBUG
    std::cout<<"Selected path: "<<sel_rows[0].to_string()<<std::endl;
#endif
    iter = model->get_iter( sel_rows[0] );
  }
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];

    Gtk::TreeModel::iterator parent = row.parent();
    if( parent ) {
      // this is a sub-layer of a group layer
      PF::Layer* pl = (*parent)[columns.col_layer];
      if( !pl ) return;
      PF::insert_pf_preset( filename, image, l, &(pl->get_sublayers()), layer_views[page]->is_map() );
      image->get_layer_manager().modified();
    } else {
      
#ifndef NDEBUG
      std::cout<<"Adding preset \""<<filename<<"\""
               <<" above layer \""<<l->get_name()<<"\""<<std::endl;
#endif
      
      //image->get_layer_manager().insert_layer( layer, l->get_id() );
      PF::insert_pf_preset( filename, image, l, layer_views[page]->get_layers(), layer_views[page]->is_map() );
      //PF::insert_layer( *(layer_views[page]->get_layers()), layer, l->get_id() );
      image->get_layer_manager().modified();
    }
  } else {
    // Nothing selected, we add the layer on top of the stack
    //image->get_layer_manager().insert_layer( layer );
    PF::insert_pf_preset( filename, image, NULL, layer_views[page]->get_layers(), layer_views[page]->is_map() );
    //PF::insert_layer( *(layer_views[page]->get_layers()), layer, -1 );
    image->get_layer_manager().modified();
  }

  //layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );


  update();
}

/*
int PF::LayerWidget::get_map_tab( std::list<Layer*>* map_layers )
{
  for( int i = notebook.get_n_pages()-1; i>=1; i-- ) {
    Widget* page = notebook.get_nth_page(i);
    LayerTree* view = dynamic_cast<LayerTree*>( page );
    if( !view ) continue;
    std::list<Layer*>* view_layers = view->get_layers();
    if( view_layers == map_layers ) return i;
  }
  return -1;
}



void PF::LayerWidget::close_map_tabs( Layer* l )
{
  if( !l ) return;
  std::cout<<"LayerWidget::close_map_tabs(\""<<l->get_name()<<"\") called."<<std::endl;
  std::list<Layer*>& omap_layers = l->get_omap_layers();
  std::list<Layer*>& imap_layers = l->get_imap_layers();
  std::list<Layer*> map_layers = omap_layers;
  map_layers.insert(map_layers.end(), imap_layers.begin(), imap_layers.end());

  for( int i = notebook.get_n_pages()-1; i>=1; i-- ) {
    Widget* page = notebook.get_nth_page(i);
    LayerTree* view = dynamic_cast<LayerTree*>( page );
    if( !view ) continue;
    std::list<Layer*>* view_layers = view->get_layers();
    bool match = false;
    if( view_layers == &omap_layers ) match = true;
    if( view_layers == &imap_layers ) match = true;
    if( match ) remove_tab( page );
  }

  for( std::list<Layer*>::iterator li = l->get_sublayers().begin(); li != l->get_sublayers().end(); li++ ) {
    close_map_tabs( *li );
  }

  for( std::list<Layer*>::iterator li = map_layers.begin(); li != map_layers.end(); li++ ) {
    close_map_tabs( *li );
  }
}
*/


void PF::LayerWidget::detach_controls( Layer* l )
{
  if( !l ) return;
#ifndef NDEBUG
  std::cout<<"LayerWidget::detach_controls(\""<<l->get_name()<<"\") called."<<std::endl;
#endif
  PF::ProcessorBase* processor = l->get_processor();
  if( processor ) {
    PF::OpParBase* par = processor->get_par();
    if( par ) {
      PF::OperationConfigUI* ui = par->get_config_ui();
      PF::OperationConfigGUI* gui =
          dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui ) {
        get_controls_group().remove_control( gui );
#ifndef NDEBUG
        std::cout<<"LayerWidget::detach_controls(\""<<l->get_name()<<"\"): controls removed."<<std::endl;
#endif
        if( editor ) {
          if( editor->get_aux_controls() == &(gui->get_aux_controls()) )
            editor->set_aux_controls( NULL );
        }
      }
    }
  }
  detach_controls( l->get_omap_layers() );
  detach_controls( l->get_imap_layers() );
  detach_controls( l->get_sublayers() );
}



void PF::LayerWidget::detach_controls( std::list<Layer*>& layers )
{
#ifndef NDEBUG
  std::cout<<"LayerWidget::detach_controls( std::list<Layer*>& layers ): layers.size()="<<layers.size()<<std::endl;
#endif
  for( std::list<Layer*>::iterator li = layers.begin(); li != layers.end(); li++ ) {
    if( !(*li) ) continue;
    detach_controls( *li );
  }
}



void PF::LayerWidget::unset_sticky_and_editing( Layer* l )
{
  if( !l ) return;
#ifndef NDEBUG
  std::cout<<"LayerWidget::unset_sticky_and_editing(\""<<l->get_name()<<"\") called."<<std::endl;
#endif

  if( editor ) {
    //if( editor->get_edited_layer() == l->get_id() )
    //  editor->set_edited_layer(-1);
    if( editor->get_displayed_layer() == (int)(l->get_id()) )
      editor->set_displayed_layer(-1);
  }
  unset_sticky_and_editing( l->get_omap_layers() );
  unset_sticky_and_editing( l->get_imap_layers() );
  unset_sticky_and_editing( l->get_sublayers() );
}



void PF::LayerWidget::unset_sticky_and_editing( std::list<Layer*>& layers )
{
#ifndef NDEBUG
  std::cout<<"LayerWidget::unset_sticky_and_editing( std::list<Layer*>& layers ): layers.size()="<<layers.size()<<std::endl;
#endif
  for( std::list<Layer*>::iterator li = layers.begin(); li != layers.end(); li++ ) {
    if( !(*li) ) continue;
    unset_sticky_and_editing( *li );
  }
}



void PF::LayerWidget::remove_layers()
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  layer_views[page]->set_tree_modified();

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;


  // make sure that we wait for completion of any image update before continuing
  bool force_synced_update = image->get_force_synced_update();
  image->set_force_synced_update( true );

#ifndef NDEBUG
  if( !sel_rows.empty() )
    std::cout<<"Selected path: "<<sel_rows[0].to_string()<<std::endl;
#endif
  // Clear the selection, since we are going to remove all selected layers
  //layer_views[page]->select_row( -1 );
  refTreeSelection->unselect_all();

  /*
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  */

  bool removed = false;
  for( unsigned int ri = 0; ri < sel_rows.size(); ri++ ) {
    iter = model->get_iter( sel_rows[ri] );
    if( !iter ) continue;
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];

#ifndef NDEBUG
    std::cout<<"Calling unset_sticky_and_editing(\""<<l->get_name()<<"\")"<<std::endl;
#endif
    unset_sticky_and_editing( l );
#ifndef NDEBUG
    std::cout<<"Calling detach_controls(\""<<l->get_name()<<"\")"<<std::endl;
#endif
    detach_controls( l );
    //std::cout<<"Calling close_map_tabs(\""<<l->get_name()<<"\")"<<std::endl;
    //close_map_tabs( l );
#ifndef NDEBUG
    std::cout<<"Calling controls_dialog_delete(\""<<l->get_name()<<"\")"<<std::endl;
#endif
    controls_dialog_delete(l);

#ifndef NDEBUG
    std::cout<<"LayerWidget::remove_layers(): calling image->remove_layer(\""<<l->get_name()<<"\")"<<std::endl;
#endif
    image->remove_layer( l );
#ifndef NDEBUG
    std::cout<<"LayerWidget::remove_layers(): calling image->get_layer_manager().modified()"<<std::endl;
#endif
    removed = true;
  }

  if( removed ) {
    image->get_layer_manager().modified();
    signal_edited_layer_changed.emit(-1);
    if( editor ) {
      //std::cout<<"LayerWidget::remove_layers(): editor->get_edited_layer()="<<editor->get_edited_layer()<<"  l->get_id()="<<l->get_id()<<std::endl;
      //if( editor && (editor->get_edited_layer() == (int)(l->get_id())) ) {
#ifndef NDEBUG
      std::cout<<"LayerWidget::remove_layers(): editor->set_edited_layer( -1 );"<<std::endl;
#endif
      editor->set_edited_layer( -1 );
      //}
    }
#ifndef NDEBUG
    std::cout<<"LayerWidget::remove_layers(): calling update()"<<std::endl;
#endif
    update();
#ifndef NDEBUG
    std::cout<<"LayerWidget::remove_layers(): update() finished"<<std::endl;
#endif
  }

  // restore the previous state
  image->set_force_synced_update( force_synced_update );
}



void PF::LayerWidget::switch_to_layers_view()
{
  aux_controls_group.show();
  layers_view.set_tree_modified();
  layers_view.update_model();
  layers_view.show();
  mask_view_box.hide();
  active_view = 0;
  if( mask_view_show_button.get_active() )
    mask_view_show_button.set_active(false);
  on_selection_changed();
}



void PF::LayerWidget::switch_to_mask_view()
{
  layers_view.hide();
  mask_view_box.show();
  active_view = 1;
  on_selection_changed();
}




void PF::LayerWidget::toggle_mask()
{
#ifndef NDEBUG
  std::cout<<"Toggling mask visualization "<<(mask_view_show_button.get_active()?"ON":"OFF")<<std::endl;
#endif
  editor->set_display_mask( mask_view_show_button.get_active() );
}




void PF::LayerWidget::on_button_add_group()
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) return;
  
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
      layer_views[page]->get_tree().get_selection();

#ifndef NDEBUG
  std::cout<<"LayerWidget::on_button_add_group(): refTreeSelection->count_selected_rows()="<<refTreeSelection->count_selected_rows()<<std::endl;
#endif

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();
  std::vector<Gtk::TreeModel::Path> sel_rows =
      refTreeSelection->get_selected_rows();
  std::vector<PF::Layer*> sel_layers;
  Gtk::TreeModel::iterator iter;

  for( int ri = sel_rows.size()-1; ri >= 0; ri-- ) {
    iter = model->get_iter( sel_rows[ri] );
    if( !iter ) continue;
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*iter)[columns.col_layer];
    sel_layers.push_back(l);
  }

  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  if( !layer ) return;
  layer->set_name( _("Group Layer") );
  layer->set_normal( false );

  layer_views[page]->set_tree_modified();

  PF::ProcessorBase* processor = new_buffer();
  layer->set_processor( processor );

  PF::ProcessorBase* blender = new_blender();
  layer->set_blender( blender );

  PF::OperationConfigGUI* dialog =
      new PF::OperationConfigGUI( layer, Glib::ustring(_("Group Layer Config")) );
  processor->get_par()->set_config_ui( dialog );

  add_layer( layer, false );

  if( sel_layers.size() > 1 ) {
    image->lock();

    for( unsigned int li = 0; li < sel_layers.size(); li++ ) {
      PF::Layer* l = sel_layers[li];

      std::list<Layer*> children;
      layer_manager.get_child_layers( l, children );
      for( std::list<Layer*>::iterator i = children.begin(); i != children.end(); i++ ) {
        if( !(*i) ) continue;
        (*i)->get_processor()->get_par()->modified();
      }
      l->get_processor()->get_par()->modified();

      layer_manager.remove_layer( l );
#ifndef NDEBUG
      std::cout<<"LayerWidget::on_button_add_group(): layer \""<<l->get_name()<<"\" removed"<<std::endl;
#endif
      layer->sublayers_insert( l, -1 );
#ifndef NDEBUG
      std::cout<<"LayerWidget::on_button_add_group(): layer \""<<l->get_name()<<"\" added to group"<<std::endl;
#endif
    }
    //layer_manager.modified();
    image->unlock();
  }

  //layer_manager.modified();

  //image->get_layer_manager().modified();

  //layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
  /*
if( layer->get_processor() && layer->get_processor()->get_par() ) {
  PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( layer->get_processor()->get_par()->get_config_ui() );
  if( ui ) ui->set_editor( editor );
}
   */
  //std::cout<<"LayerWidget::on_button_add_group(): submitting image update request..."<<std::endl;
  //update();
  //std::cout<<"LayerWidget::on_button_add_group(): image update request submitted"<<std::endl;

#ifndef NDEBUG
  std::cout<<"LayerWidget::on_button_add_group(): calling update() to update layers tree"<<std::endl;
#endif
  update();
  //std::cout<<"LayerWidget::add_layer(): image update request submitted"<<std::endl;

  layer_views[page]->unselect_all();
  //std::cout<<"LayerWidget::add_layer(): calling select_row("<<layer->get_id()<<")"<<std::endl;
  select_row( layer->get_id() );

  PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
  if( ui ) {
    PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( gui && gui->get_frame() ) {
      controls_group.add_control( layer, gui );
#ifndef NDEBUG
      std::cout<<"LayerWidget::on_button_add_group(): calling gui->open()"<<std::endl;
#endif
      gui->open();
#ifndef NDEBUG
      std::cout<<"LayerWidget::on_button_add_group(): calling gui->expand()"<<std::endl;
#endif
      gui->expand();
    }
    controls_group.show_all_children();
  }
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_button_add_group(): calling image->get_layer_manager().modified()"<<std::endl;
#endif
  image->get_layer_manager().modified();

  /*
  layer_views[page]->unselect_all();
  select_row( layer->get_id() );

  PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
  if( ui ) {
    PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( gui && gui->get_frame() ) {
      controls_group.add_control( layer, gui );
      gui->open();
    }
    controls_group.show_all_children();
  }
  //dialog->update();
  dialog->open();
  */
}



void PF::LayerWidget::on_button_del()
{
  delete_selected_layers();
}



void PF::LayerWidget::delete_selected_layers()
{
  remove_layers();
}


void PF::LayerWidget::cut_selected_layers()
{
  gchar* bufname = g_build_filename( PF::PhotoFlow::Instance().get_cache_dir().c_str(), "copy_buffer.pfp", NULL );
  save_preset(bufname);
  g_free( bufname );
  delete_selected_layers();
}


void PF::LayerWidget::copy_selected_layers()
{
  gchar* bufname = g_build_filename( PF::PhotoFlow::Instance().get_cache_dir().c_str(), "copy_buffer.pfp", NULL );
  save_preset(bufname);
  g_free( bufname );
}


void PF::LayerWidget::paste_layers()
{
  gchar* bufname = g_build_filename( PF::PhotoFlow::Instance().get_cache_dir().c_str(), "copy_buffer.pfp", NULL );
  insert_preset(bufname);
  g_free( bufname );
}


void PF::LayerWidget::on_button_load()
{
  Gtk::FileChooserDialog dialog(_("Open preset"),
				Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

#ifdef GTKMM_2
  Gtk::FileFilter filter_pfp;
  filter_pfp.set_name( _("Photoflow presets") );
  filter_pfp.add_pattern("*.pfp");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_pfp = Gtk::FileFilter::create();
  filter_pfp->set_name( _("Photoflow presets") );
  filter_pfp->add_pattern("*.pfp");
#endif
  dialog.add_filter(filter_pfp);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_preset_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Save clicked." << std::endl;

      last_dir = dialog.get_current_folder();
      PF::PhotoFlow::Instance().get_options().set_last_visited_preset_folder( last_dir );

      //Notice that this is a std::string, not a Glib::ustring.
      filename = dialog.get_filename();
#ifndef NDEBUG
      std::cout << "File selected: " <<  filename << std::endl;
#endif
      break;
    }
  case(Gtk::RESPONSE_CANCEL): 
    {
      std::cout << "Cancel clicked." << std::endl;
      return;
    }
  default: 
    {
      std::cout << "Unexpected button clicked." << std::endl;
      return;
    }
  }

  insert_preset( filename );
}



void PF::LayerWidget::on_button_save()
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  if( sel_rows.empty() ) return;

  Gtk::FileChooserDialog dialog( _("Save preset as..."),
				Gtk::FILE_CHOOSER_ACTION_SAVE);
  //dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

#ifdef GTKMM_2
  Gtk::FileFilter filter_pfp;
  filter_pfp.set_name( _("Photoflow presets") );
  filter_pfp.add_pattern("*.pfp");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_pfp = Gtk::FileFilter::create();
  filter_pfp->set_name( _("Photoflow presets") );
  filter_pfp->add_pattern("*.pfp");
#endif
  dialog.add_filter(filter_pfp);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_preset_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Save clicked." << std::endl;

      last_dir = dialog.get_current_folder();
      PF::PhotoFlow::Instance().get_options().set_last_visited_preset_folder( last_dir );

      //Notice that this is a std::string, not a Glib::ustring.
      filename = dialog.get_filename();
      std::string extension;
      if( get_file_extension(filename, extension) ) {
        if( extension != "pfp" )
          filename += ".pfp";
      }
#ifndef NDEBUG
      std::cout << "File selected: " <<  filename << std::endl;
#endif
      break;
    }
  case(Gtk::RESPONSE_CANCEL): 
    {
      std::cout << "Cancel clicked." << std::endl;
      return;
    }
  default: 
    {
      std::cout << "Unexpected button clicked." << std::endl;
      return;
    }
  }

  save_preset(filename);
}


void PF::LayerWidget::save_preset(std::string filename)
{
  //int page = notebook.get_current_page();
  int page = active_view;
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows =
    refTreeSelection->get_selected_rows();

  std::ofstream of;
  of.open( filename.c_str() );
  if( !of ) return;

  of<<"<preset version=\""<<PF_FILE_VERSION<<"\">"<<std::endl;
  for( int ri = sel_rows.size()-1; ri >= 0; ri-- ) {
    Gtk::TreeModel::iterator iter = model->get_iter( sel_rows[ri] );
    if( !iter ) continue;
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = row[columns.col_layer];

    Gtk::TreeModel::iterator parent = row.parent();
    if( parent ) {
      bool selected = false;
      for( unsigned int rj = 0; rj < sel_rows.size(); rj++ ) {
        Gtk::TreeModel::iterator iter2 = model->get_iter( sel_rows[rj] );
        if( !iter2 ) continue;
        if( parent != iter2 ) continue;
        selected = true;
        break;
      }
      if( selected ) {
        PF::Layer* pl = (*parent)[columns.col_layer];
#ifndef NDEBUG
        if(l) std::cout<<"PF::LayerWidget::on_button_save(): container of layer \""<<l->get_name()<<"\" is selected... skipped."<<std::endl;
#endif
        continue;
      }
    }
    if( !l ) continue;

    int level = 1;
    if( !l->save( of, level ) ) return;      
#ifndef NDEBUG
    std::cout<<"PF::LayerWidget::on_button_save(): layer \""<<l->get_name()<<"\" saved."<<std::endl;
#endif
  }
  of<<"</preset>"<<std::endl;
}



#ifdef GTKMM_2
void PF::LayerWidget::on_switch_page(_GtkNotebookPage* page, guint page_num)
#endif
#ifdef GTKMM_3
  void PF::LayerWidget::on_switch_page(Widget* page, guint page_num)
#endif
{
  int layer_id = get_selected_layer_id();
  on_selection_changed();
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_switch_page( "<<page_num<<" ) called."<<std::endl;
  std::cout<<"Selected layer id: "<<layer_id<<std::endl;
#endif
  if( layer_id >= 0 )
    signal_edited_layer_changed.emit( layer_id );
}


bool PF::LayerWidget::on_key_press_event(GdkEventKey* event)
{
#ifndef NDEBUG
  std::cout<<"LayerWidget: event->type="<<event->type<<"  event->state="<<(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK))<<std::endl;
  std::cout<<"  GDK_KEY_PRESS="<<GDK_KEY_PRESS<<std::endl;
  std::cout<<"  GDK_SHIFT_MASK="<<GDK_SHIFT_MASK<<std::endl;
  std::cout<<"  GDK_CONTROL_MASK="<<GDK_CONTROL_MASK<<std::endl;
  std::cout<<"  GDK_MOD2_MASK="<<GDK_MOD2_MASK<<std::endl;
#endif
  if( event->type == GDK_KEY_PRESS &&
      (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK)) == (GDK_MOD2_MASK+GDK_SHIFT_MASK) ) {
    if( event->keyval == 'N' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+N pressed"<<std::endl;
      on_button_add();
      return true;
    }
    if( event->keyval == 'G' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+G pressed"<<std::endl;
      on_button_add_group();
      return true;
    }
    if( event->keyval == 'D' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+D pressed"<<std::endl;
      on_button_del();
      return true;
    }
    if( event->keyval == 'M' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+M pressed"<<std::endl;
      //notebook.set_current_page(1);
      switch_to_mask_view();
      return true;
    }
    if( event->keyval == 'L' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+L pressed"<<std::endl;
      //notebook.set_current_page(0);
      switch_to_layers_view();
      return true;
    }
    if( event->keyval == 'O' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+O pressed"<<std::endl;
      //notebook.set_current_page(0);
      on_button_load();
      return true;
    }
    if( event->keyval == 'S' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+S pressed"<<std::endl;
      //notebook.set_current_page(0);
      on_button_save();
      return true;
    }
    if( event->keyval == 'C' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+C pressed"<<std::endl;
      //notebook.set_current_page(0);
      copy_selected_layers();
      return true;
    }
    if( event->keyval == 'X' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+X pressed"<<std::endl;
      //notebook.set_current_page(0);
      cut_selected_layers();
      return true;
    }
    if( event->keyval == 'V' ) {
      std::cout<<"LayerWidget: Ctrl+Shift+V pressed"<<std::endl;
      //notebook.set_current_page(0);
      paste_layers();
      return true;
    }
  }

  if( event->type == GDK_KEY_PRESS &&
      (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK)) == (GDK_CONTROL_MASK) ) {
    if( event->keyval == 'e' ) {
      std::cout<<"LayerWidget: Ctrl+e pressed"<<std::endl;
      PF::OperationConfigGUI* gui = get_selected_layer_ui();
      if( !gui ) return false;
      if( gui->get_editing_flag() )
        gui->disable_editing();
      else
        gui->enable_editing();
      return true;
    }
  }

  return false;
}
