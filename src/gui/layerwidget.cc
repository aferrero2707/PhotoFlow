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


#include "../operations/blender.hh"
#include "layerwidget.hh"


PF::LayerWidget::LayerWidget( Image* img ): 
  image( img ),
  Gtk::VBox(), 
  buttonAdd("+"),
  buttonAddGroup("G+"),
  buttonDel("-"),
  operationsDialog( image, this )
{
  notebook.set_tab_pos(Gtk::POS_LEFT);
  Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();

  LayerTree* view = new LayerTree( );
  frame->add( *view );
  
  view->set_reorderable();

  notebook.append_page(*frame,"Layers");
  Widget* page = notebook.get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)notebook.get_tab_label(*page);
  label->set_angle(90);

  pack_start(notebook);

  buttonbox.pack_start(buttonAdd/*, Gtk::PACK_SHRINK*/);
  buttonbox.pack_start(buttonAddGroup/*, Gtk::PACK_SHRINK*/);
  buttonbox.pack_start(buttonDel/*, Gtk::PACK_SHRINK*/);
  buttonbox.set_layout(Gtk::BUTTONBOX_START);

  pack_start(buttonbox, Gtk::PACK_SHRINK);

  Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
  cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled) ); 

  layer_frames.push_back( frame );
  layer_views.push_back( view );

  layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
  image->get_layer_manager().signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );

  view->signal_row_activated().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_row_activated) ); 

  buttonAdd.signal_clicked().connect( sigc::mem_fun(*this,
						    &PF::LayerWidget::on_button_add) );
  buttonAddGroup.signal_clicked().connect( sigc::mem_fun(*this,
							 &PF::LayerWidget::on_button_add_group) );
  buttonDel.signal_clicked().connect( sigc::mem_fun(*this,
						    &PF::LayerWidget::on_button_del) );
}


PF::LayerWidget::~LayerWidget()
{
}


void PF::LayerWidget::on_cell_toggled( const Glib::ustring& path )
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( path );
  if (iter) {
    PF::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    std::cout<<"Toggled visibility of layer \""<<l->get_name()<<"\": "<<visible<<std::endl;
    l->set_visible( visible );
    l->set_dirty( true );
    //layer_manager->rebuild( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );
    image->update();
  }
}



void PF::LayerWidget::on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( path );
  if (iter) {
    PF::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    if( !l ) return;
    std::cout<<"Activated row "<<l->get_name()<<std::endl;

    PF::OperationConfigUI* dialog = l->get_processor()->get_par()->get_config_ui();
    if(dialog) dialog->open();
  }
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
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    layer_views[page]->get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::LayerTreeColumns& columns = layer_views[page]->get_columns();
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
      
      image->get_layer_manager().insert_layer( layer, l->get_id() );
      image->get_layer_manager().modified();
    }
  } else {
    // Nothing selected, we add the layer on top of the stack
    image->get_layer_manager().insert_layer( layer );
    image->get_layer_manager().modified();
  }

  layer->signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );


  update();
}



void PF::LayerWidget::on_button_add_group()
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;
  
  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  if( !layer ) return;
  layer->set_name( "New Group Layer" );
  layer->set_normal( false );

  PF::ProcessorBase* processor = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  layer->set_processor( processor );

  add_layer( layer );

  PF::OperationConfigDialog* dialog = 
    new PF::OperationConfigDialog( layer, Glib::ustring("Group Layer Config") );
  processor->get_par()->set_config_ui( dialog );
  dialog->update();
  dialog->open();
}



void PF::LayerWidget::on_button_del()
{
}
