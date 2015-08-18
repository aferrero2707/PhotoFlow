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


void PF::ControlsGroup::add_control(Gtk::Frame* control)
{
  for( unsigned int i = 0; i < controls.size(); i++ ) {
    if( controls[i] == control )
      return;
  }
  controls.push_back( control );
  pack_start( *control, Gtk::PACK_SHRINK );
  editor->update_controls();
}


void PF::ControlsGroup::remove_control(Gtk::Frame* control)
{
  std::cout<<"ControlsGroup::remove_control() called."<<std::endl;
  if( control->get_parent() == this ) {
    std::vector<Gtk::Frame*> new_controls;
    for( unsigned int i = 0; i < controls.size(); i++ ) {
      std::cout<<"  controls["<<i<<"]="<<controls[i]<<" (control="<<control<<")"<<std::endl;
      if( controls[i] == control )
        continue;
      std::cout<<"  new_controls.push_back("<<controls[i]<<")"<<std::endl;
      new_controls.push_back( controls[i] );
    }
    std::cout<<"ControlsGroup::remove_control(): controls.size()="<<controls.size()<<"  new_controls.size()="<<new_controls.size()<<std::endl;
    controls = new_controls;
    remove( *control );
  }
  editor->update_controls();
}


void PF::ControlsGroup::set_controls( std::vector<Gtk::Frame*>& new_controls)
{
}


PF::LayerWidget::LayerWidget( Image* img, ImageEditor* ed ):
  Gtk::VBox(), 
  image( img ), editor( ed ),
  controls_group( ed ),
  buttonAdd("+"),
  buttonAddGroup("G+"),
  buttonDel("-"),
  buttonPresetLoad( _("Load") ),
  buttonPresetSave( _("Save") ),
  operationsDialog( image, this )
{
  set_size_request(250,200);
  notebook.set_tab_pos(Gtk::POS_LEFT);
  //Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();

  LayerTree* view = new LayerTree( editor );
  //view->signal_updated.connect(sigc::mem_fun(this, &LayerWidget::modified) );
  //frame->add( *view );
  
  //view->set_reorderable();

  notebook.append_page(*view,_("Layers"));
  Widget* page = notebook.get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
  label->set_angle(90);

  top_box.pack_start(notebook);

  buttonAdd.set_size_request(30,20);
  buttonAdd.set_tooltip_text( _("Add a new layer") );
  buttonAddGroup.set_size_request(30,20);
  buttonAddGroup.set_tooltip_text( _("Add a new layer group") );
  buttonDel.set_size_request(30,20);
  buttonDel.set_tooltip_text( _("Remove selected layers") );

  buttonPresetLoad.set_tooltip_text( _("Load an existing preset") );
  buttonPresetSave.set_tooltip_text( _("Save the selected layers as a preset") );

  buttonbox.pack_start(buttonAdd, Gtk::PACK_SHRINK);
  buttonbox.pack_start(buttonAddGroup, Gtk::PACK_SHRINK);
  buttonbox.pack_start(buttonDel, Gtk::PACK_SHRINK);
  buttonbox.pack_start(buttonPresetLoad/*, Gtk::PACK_SHRINK*/);
  buttonbox.pack_start(buttonPresetSave/*, Gtk::PACK_SHRINK*/);
  //buttonbox.set_layout(Gtk::BUTTONBOX_START);

  top_box.pack_start(buttonbox, Gtk::PACK_SHRINK);

  controls_scrolled_window.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  controls_scrolled_window.set_shadow_type( Gtk::SHADOW_NONE );
  controls_scrolled_window.set_size_request( 250, 0 );
  //controls_scrolled_window.add( controls_group );

  //layers_panel.pack1( top_box, true, true );
  //layers_panel.pack2( controls_scrolled_window, true, true );
  //pack_start(layers_panel);
  pack_start( top_box );

  /*
    Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
    cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 
  */

  view->get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) ); 

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
  if( refTreeSelection->count_selected_rows() == 0 ) {
    Gtk::TreeModel::Children children = layer_views[page]->get_model()->children();
    refTreeSelection->select( children.begin() );
    return;
  }

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
      /*
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if(dialog) {
        Gtk::Window* toplevel = dynamic_cast<Gtk::Window*>(get_toplevel());
        if( toplevel )
            dialog->set_transient_for( *toplevel );
        dialog->open();
        dialog->enable_editing();
      }
      */
      PF::OperationConfigGUI* gui = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( gui && gui->get_frame() ) {
        controls_group.add_control( gui->get_frame() );
        gui->open();
      }
      controls_group.show_all_children();
    }
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
#ifndef NDEBUG
  std::cout<<"PF::LayerWidget::remove_tab() called."<<std::endl;
#endif
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
#ifndef NDEBUG
  std::cout<<"PF::LayerWidget::remove_tab() page #"<<page<<" removed."<<std::endl;
#endif
}



void PF::LayerWidget::on_button_add()
{
  operationsDialog.open();
  //operationsDialog.set_transient_for( (get_window()) );
}



void PF::LayerWidget::add_layer( PF::Layer* layer )
{
  int page = notebook.get_current_page();
  if( page < 0 ) page = 0;

  bool is_map = layer_views[page]->is_map();
  layer->get_processor()->get_par()->
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

  layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
/*
  if( layer->get_processor() && layer->get_processor()->get_par() ) {
    PF::OperationConfigGUI* ui = dynamic_cast<PF::OperationConfigGUI*>( layer->get_processor()->get_par()->get_config_ui() );
    if( ui ) ui->set_editor( editor );
  }
*/


  update();
  layer_views[page]->unselect_all();
  select_row( layer->get_id() );
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

    image->remove_layer( l );
    image->get_layer_manager().modified();
    removed = true;
  }

  if( removed ) update();
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

  //if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Save clicked." << std::endl;

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

  //if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Save clicked." << std::endl;

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

