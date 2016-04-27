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


#include "../base/file_util.hh"
#include "../base/fileutils.hh"
#include "../base/pf_file_loader.hh"
#include "../operations/buffer.hh"
#include "../operations/blender.hh"
#include "tablabelwidget.hh"
#include "layerwidget.hh"
#include "imageeditor.hh"


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


void PF::ControlsGroup::add_control(PF::OperationConfigGUI* gui)
{
  collapse_all();
  for( unsigned int i = 0; i < guis.size(); i++ ) {
    if( guis[i] == gui ) {
      guis[i]->expand();
      return;
    }
  }
  guis.push_back( gui );
  Gtk::Frame* control = gui->get_frame();
  controls.push_back( control );
  pack_start( *control, Gtk::PACK_SHRINK );
  editor->update_controls();
}


void PF::ControlsGroup::remove_control(PF::OperationConfigGUI* gui)
{
  Gtk::Frame* control = gui->get_frame();
  std::cout<<"ControlsGroup::remove_control() called."<<std::endl;
  if( control->get_parent() == this ) {
    std::vector<PF::OperationConfigGUI*> new_guis;
    std::vector<Gtk::Frame*> new_controls;
    for( unsigned int i = 0; i < controls.size(); i++ ) {
      std::cout<<"  controls["<<i<<"]="<<controls[i]<<" (control="<<control<<")"<<std::endl;
      if( controls[i] == control )
        continue;
      std::cout<<"  new_controls.push_back("<<controls[i]<<")"<<std::endl;
      new_guis.push_back( guis[i] );
      new_controls.push_back( controls[i] );
    }
    std::cout<<"ControlsGroup::remove_control(): controls.size()="<<controls.size()<<"  new_controls.size()="<<new_controls.size()<<std::endl;
    guis = new_guis;
    controls = new_controls;
    remove( *control );
  }
  editor->update_controls();
}


void PF::ControlsGroup::collapse_all()
{
  for( unsigned int i = 0; i < guis.size(); i++ ) {
    guis[i]->collapse();
  }
}


//void PF::ControlsGroup::set_controls( std::vector<Gtk::Frame*>& new_controls)
//{
//}


PF::LayerWidget::LayerWidget( Image* img, ImageEditor* ed ):
  Gtk::VBox(), 
  image( img ), editor( ed ),
  controls_group( ed ),
  buttonAdd( _("New Adjustment") ),
  buttonAddGroup("G+"),
  buttonDel("-"),
  buttonPresetLoad( _("Load preset") ),
  buttonPresetSave( _("Save preset") ),
  operationsDialog( image, this ),
  add_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/add-layer.png", "", image, this),
  group_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/group.png", "", image, this),
  trash_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/trash.png", "", image, this),
  insert_image_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-file-image.png", "", image, this),
  curves_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/curves.png", "curves", image, this),
  uniform_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/bucket-fill.png", "uniform", image, this),
  gradient_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/gradient.png", "gradient", image, this),
  path_mask_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/path-mask.png", "path_mask", image, this),
  desaturate_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/desaturate.png", "desaturate", image, this),
  crop_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/crop.png", "crop", image, this),
  basic_edits_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/basic-edits.png", "levels", image, this),
  draw_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/draw.png", "draw", image, this),
  clone_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/clone.png", "clone_stamp", image, this),
  scale_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/scale.png", "scale", image, this),
  perspective_button(PF::PhotoFlow::Instance().get_data_dir()+"/icons/tools/perspective.png", "perspective", image, this)
{
  set_size_request(250,-1);
  notebook.set_tab_pos(Gtk::POS_LEFT);
  //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();

  add_button.set_tooltip_text( _("new layer") );
  group_button.set_tooltip_text( _("new group layer") );
  trash_button.set_tooltip_text( _("delete layer") );
  insert_image_button.set_tooltip_text( _("insert image as layer") );
  basic_edits_button.set_tooltip_text( _("basic editing") );
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
  tool_buttons_box.pack_start( basic_edits_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( curves_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( uniform_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( gradient_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( desaturate_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( crop_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( perspective_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( scale_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( path_mask_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( draw_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( clone_button, Gtk::PACK_SHRINK, 2 );
  tool_buttons_box.pack_start( trash_button, Gtk::PACK_SHRINK, 2 );


  LayerTree* view = new LayerTree( editor );
  //view->signal_updated.connect(sigc::mem_fun(this, &LayerWidget::modified) );
  //frame->add( *view );
  
  //view->set_reorderable();

  notebook.append_page(*view,_("Layers"));
  Widget* page = notebook.get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
  label->set_angle(90);

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

  main_box.pack_start(tool_buttons_box, Gtk::PACK_SHRINK);
  main_box.pack_start(vbox, Gtk::PACK_EXPAND_WIDGET);
  vbox.set_spacing(4);
  vbox.pack_start(notebook, Gtk::PACK_EXPAND_WIDGET);
  vbox.pack_start( buttonbox, Gtk::PACK_SHRINK );
  top_box.pack_start( main_box, Gtk::PACK_EXPAND_WIDGET );
  //top_box.pack_start( buttonbox, Gtk::PACK_SHRINK );
  pack_start( top_box );

  /*
    Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
    cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 
  */

  view->get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) ); 
  view->get_tree().signal_row_expanded().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_expanded) );
  view->get_tree().signal_row_collapsed().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_collapsed) );

  //view->get_tree().signal_button_release_event().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_button_event) );

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = view->get_tree().get_selection();
  refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &PF::LayerWidget::on_selection_changed));

  notebook.signal_switch_page().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_switch_page) ); 

  //layer_frames.push_back( frame );
  layer_views.push_back( view );

  layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
  //image->get_layer_manager().signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
  image->signal_updated.connect(sigc::mem_fun(this, &LayerWidget::update_idle) );

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
}


PF::LayerWidget::~LayerWidget()
{
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
      signal_active_layer_changed.emit( layer_id );
  }
  return true;
}
*/


void PF::LayerWidget::on_selection_changed()
{
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_selection_chaged() called."<<std::endl;
#endif
  int page = notebook.get_current_page();
  if( page < 0 ) return;
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
  if( row_it == selected_rows.end() )
    return;

  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( *row_it );
  if (iter) {
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
#ifndef NDEBUG
    std::cout<<"Selected row "<<l->get_name()<<std::endl;
#endif

    if( PF::PhotoFlow::Instance().is_single_win_mode() ) {
      PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
      if( ui ) {
        PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
        if( gui && editor ) {
          editor->set_aux_controls( &(gui->get_aux_controls()) );
        }
      }
    }
  }

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
    signal_active_layer_changed.emit( layer_id );

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
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( path );
  if (iter) {
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    std::cout<<"Activated row "<<l->get_name()<<std::endl;
    if( column == layer_views[page]->get_tree().get_column(IMAP_COL_NUM) ) {
      if( !l->get_processor()->get_par()->has_intensity() )
        return;
      std::cout<<"Activated IMap column of row "<<l->get_name()<<std::endl;
      //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();
      
      int tab_id = get_map_tab( &(l->get_imap_layers()) );
      if( tab_id >= 0 ) {
        notebook.set_current_page( tab_id );
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

      VTabLabelWidget* tabwidget = 
        new VTabLabelWidget( std::string("intensity (")+l->get_name()+")",
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
      view->show_all();
      notebook.set_current_page( -1 );
      //frame->show();

      return;
    }
    if( column == layer_views[page]->get_tree().get_column(OMAP_COL_NUM) ) {
      if( !l->get_processor()->get_par()->has_opacity() )
        return;
      std::cout<<"Activated OMap column of row "<<l->get_name()<<std::endl;
      //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();
      
      int tab_id = get_map_tab( &(l->get_omap_layers()) );
      if( tab_id >= 0 ) {
        notebook.set_current_page( tab_id );
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

      VTabLabelWidget* tabwidget = 
        new VTabLabelWidget( std::string("opacity (")+l->get_name()+")",
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

      return;
    }

    PF::OperationConfigUI* ui = l->get_processor()->get_par()->get_config_ui();
    if( ui ) {
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui && gui->get_frame() ) {
        controls_group.add_control( gui );
        gui->open();
        gui->expand();
      }
      controls_group.show_all_children();
    }
  }
}


void PF::LayerWidget::on_row_expanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path )
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  if (iter) {
    //std::cout<<"LayerWidget::on_row_expanded() called"<<std::endl;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    l->set_expanded( true );
    //std::cout<<"LayerWidget::on_row_expanded(): layer expanded flag set"<<std::endl;
    layer_views[page]->get_tree().columns_autosize();
  }
}


void PF::LayerWidget::on_row_collapsed( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path )
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  if (iter) {
    //std::cout<<"LayerWidget::on_row_collapsed() called"<<std::endl;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    l->set_expanded( false );
    //std::cout<<"LayerWidget::on_row_collapsed(): layer expanded flag reset"<<std::endl;
    layer_views[page]->get_tree().columns_autosize();
  }
}


int PF::LayerWidget::get_selected_layer_id()
{
  int result = -1;

  int page = notebook.get_current_page();
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
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  for(  Gtk::TreeModel::iterator it = rows.begin();
        it != rows.end(); it++ ) {
    //Gtk::TreeModel::Row row = *it;
    PF::LayerTreeModel::LayerTreeColumns& columns = layer_views[page]->get_columns();
    PF::Layer* l = (*it)[columns.col_layer];
    if(l && (l->get_id()==id)) {
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
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();
  const Gtk::TreeModel::Children rows = model->children();
  return get_row( id, rows, iter );
}


void PF::LayerWidget::select_row(int id)
{
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  Gtk::TreeModel::iterator iter;
  if( get_row( id, iter ) ) {
    refTreeSelection->select( iter );
    signal_active_layer_changed.emit( id );
  }
}



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



void PF::LayerWidget::on_button_add()
{
  operationsDialog.open();
  //operationsDialog.set_transient_for( (get_window()) );
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
    std::cout << "Open clicked." << std::endl;

    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_image_folder( last_dir );

    //Notice that this is a std::string, not a Glib::ustring.
    std::string filename = dialog.get_filename();
    std::cout << "File selected: " <<  filename << std::endl;
    char* fullpath = realpath( filename.c_str(), NULL );
    if(!fullpath)
      return;
    insert_image( fullpath );
    free(fullpath);
    break;
        }
  case(Gtk::RESPONSE_CANCEL):
        {
    std::cout << "Cancel clicked." << std::endl;
    break;
        }
  default:
    std::cout << "Unexpected button clicked." << std::endl;
    break;
  }
}



void PF::LayerWidget::add_layer( PF::Layer* layer )
{
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  bool is_map = layer_views[page]->is_map();
  layer->get_processor()->get_par()->
    set_map_flag( is_map );
  layer->get_blender()->get_par()->
    set_map_flag( is_map );
    
#ifndef NDEBUG
  std::cout<<"LayerWidget::add_layer(): layer_views.size()="<<layer_views.size()<<std::endl;
#endif

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    std::cout<<"Selected path: "<<sel_rows[0].to_string()<<std::endl;
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
      
      std::cout<<"Adding layer \""<<layer->get_name()
               <<" above layer \""<<l->get_name()<<"\""<<std::endl;
      
      //image->get_layer_manager().insert_layer( layer, l->get_id() );
      PF::insert_layer( *(layer_views[page]->get_layers()), layer, l->get_id() );
      image->get_layer_manager().modified();
    }
  } else {
    // Nothing selected, we add the layer on top of the stack
    //image->get_layer_manager().insert_layer( layer );
    PF::insert_layer( *(layer_views[page]->get_layers()), layer, -1 );
    image->get_layer_manager().modified();
  }

  //layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
/*
  if( layer->get_processor() && layer->get_processor()->get_par() ) {
    PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( layer->get_processor()->get_par()->get_config_ui() );
    if( ui ) ui->set_editor( editor );
  }
*/


  update();
  layer_views[page]->unselect_all();
  select_row( layer->get_id() );

  PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
  if( ui ) {
    PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( gui && gui->get_frame() ) {
      controls_group.add_control( gui );
      gui->open();
    }
    controls_group.show_all_children();
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
    PF::Layer* layer = layer_manager.new_layer();
    if( !layer ) return;

    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "imageread", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( "image file" );

    add_layer( limg );
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

    PF::ProcessorBase* blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
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
    PF::ProcessorBase* proc2 = PF::PhotoFlow::Instance().new_operation( "raw_developer", limg2 );
    limg2->set_processor( proc2 );
    limg2->set_name( "RAW developer" );
    gl->sublayers_insert( limg2, -1 );

    add_layer( gl );
  }
}


void PF::LayerWidget::insert_preset( std::string filename )
{
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

#ifndef NDEBUG
  std::cout<<"LayerWidget::add_layer(): layer_views.size()="<<layer_views.size()<<std::endl;
#endif

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    std::cout<<"Selected path: "<<sel_rows[0].to_string()<<std::endl;
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
      
      std::cout<<"Adding preset \""<<filename<<"\""
               <<" above layer \""<<l->get_name()<<"\""<<std::endl;
      
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



void PF::LayerWidget::detach_controls( Layer* l )
{
  if( !l ) return;
  std::cout<<"LayerWidget::detach_controls(\""<<l->get_name()<<"\") called."<<std::endl;
  PF::ProcessorBase* processor = l->get_processor();
  if( processor ) {
    PF::OpParBase* par = processor->get_par();
    if( par ) {
      PF::OperationConfigUI* ui = par->get_config_ui();
      PF::OperationConfigGUI* gui =
          dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui ) {
        get_controls_group().remove_control( gui );
        std::cout<<"LayerWidget::detach_controls(\""<<l->get_name()<<"\"): controls removed."<<std::endl;
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
  std::cout<<"LayerWidget::detach_controls( std::list<Layer*>& layers ): layers.size()="<<layers.size()<<std::endl;
  for( std::list<Layer*>::iterator li = layers.begin(); li != layers.end(); li++ ) {
    if( !(*li) ) continue;
    detach_controls( *li );
  }
}



void PF::LayerWidget::unset_sticky_and_editing( Layer* l )
{
  if( !l ) return;
  std::cout<<"LayerWidget::unset_sticky_and_editing(\""<<l->get_name()<<"\") called."<<std::endl;

  if( editor ) {
    //if( editor->get_active_layer() == l->get_id() )
    //  editor->set_active_layer(-1);
    if( editor->get_displayed_layer() == l->get_id() )
      editor->set_displayed_layer(-1);
  }
  unset_sticky_and_editing( l->get_omap_layers() );
  unset_sticky_and_editing( l->get_imap_layers() );
  unset_sticky_and_editing( l->get_sublayers() );
}



void PF::LayerWidget::unset_sticky_and_editing( std::list<Layer*>& layers )
{
  std::cout<<"LayerWidget::unset_sticky_and_editing( std::list<Layer*>& layers ): layers.size()="<<layers.size()<<std::endl;
  for( std::list<Layer*>::iterator li = layers.begin(); li != layers.end(); li++ ) {
    if( !(*li) ) continue;
    unset_sticky_and_editing( *li );
  }
}



void PF::LayerWidget::remove_layers()
{
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  Glib::RefPtr<Gtk::TreeStore> model = layer_views[page]->get_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;

  // Clear the selection, since we are going to remove all selected layers
  //layer_views[page]->select_row( -1 );
  refTreeSelection->unselect_all();

  if( !sel_rows.empty() ) {
    std::cout<<"Selected path: "<<sel_rows[0].to_string()<<std::endl;
    //iter = model->get_iter( sel_rows[0] );
    signal_active_layer_changed.emit(-1);
  }
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

    if( editor ) {
      std::cout<<"editor->get_active_layer()="<<editor->get_active_layer()<<"  l->get_id()="<<l->get_id()<<std::endl;
    }
    if( editor && (editor->get_active_layer() == l->get_id()) ) {
      std::cout<<"editor->set_active_layer( -1 );"<<std::endl;
      editor->set_active_layer( -1 );
    }

    //std::cout<<"Calling unset_sticky_and_editing(\""<<l->get_name()<<"\")"<<std::endl;
    unset_sticky_and_editing( l );
    //std::cout<<"Calling detach_controls(\""<<l->get_name()<<"\")"<<std::endl;
    detach_controls( l );
    //std::cout<<"Calling close_map_tabs(\""<<l->get_name()<<"\")"<<std::endl;
    close_map_tabs( l );

    //std::cout<<"Calling image->remove_layer(\""<<l->get_name()<<"\")"<<std::endl;
    image->remove_layer( l );
    image->get_layer_manager().modified();
    removed = true;
  }

  if( removed )
    update();
}



void PF::LayerWidget::on_button_add_group()
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  
  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  if( !layer ) return;
  layer->set_name( _("New Group Layer") );
  layer->set_normal( false );

  PF::ProcessorBase* processor = new_buffer();
  layer->set_processor( processor );

  PF::ProcessorBase* blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  layer->set_blender( blender );

  PF::OperationConfigGUI* dialog =
    new PF::OperationConfigGUI( layer, Glib::ustring(_("Group Layer Config")) );
  processor->get_par()->set_config_ui( dialog );

  add_layer( layer );

  //dialog->update();
  dialog->open();
}



void PF::LayerWidget::on_button_del()
{
  remove_layers();
}



void PF::LayerWidget::on_button_load()
{
  Gtk::FileChooserDialog dialog("Open preset",
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
      std::cout << "File selected: " <<  filename << std::endl;
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
  int page = notebook.get_current_page();
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
      std::cout << "File selected: " <<  filename << std::endl;
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
      for( int rj = 0; rj < sel_rows.size(); rj++ ) {
        Gtk::TreeModel::iterator iter2 = model->get_iter( sel_rows[rj] );
        if( !iter2 ) continue;
        if( parent != iter2 ) continue;
        selected = true;
        break;
      }
      if( selected ) {
        PF::Layer* pl = (*parent)[columns.col_layer];
        if(l) std::cout<<"PF::LayerWidget::on_button_save(): container of layer \""<<l->get_name()<<"\" is selected... skipped."<<std::endl;
        continue;
      }
    }
    if( !l ) continue;

    int level = 1;
    if( !l->save( of, level ) ) return;      
    std::cout<<"PF::LayerWidget::on_button_save(): layer \""<<l->get_name()<<"\" saved."<<std::endl;
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
#ifndef NDEBUG
  std::cout<<"LayerWidget::on_switch_page( "<<page_num<<" ) called."<<std::endl;
  std::cout<<"Selected layer id: "<<layer_id<<std::endl;
#endif
  if( layer_id >= 0 )
    signal_active_layer_changed.emit( layer_id );
}

