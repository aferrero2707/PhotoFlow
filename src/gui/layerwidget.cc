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


#include "layerwidget.hh"


PF::LayerWidget::LayerWidget(): Gtk::Notebook(), layer_manager( NULL )
{
  set_tab_pos(Gtk::POS_LEFT);
  Gtk::ScrolledWindow* frame = new Gtk::ScrolledWindow();

  LayerTree* view = new LayerTree( );
  frame->add( *view );
  
  view->set_reorderable();

  append_page(*frame,"Layers");
  Widget* page = get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)get_tab_label(*page);
  label->set_angle(90);

  Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( view->get_column_cell_renderer(0) );
  cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerWidget::on_cell_toggled)); 

  layer_frames.push_back( frame );
  layer_views.push_back( view );
}


PF::LayerWidget::~LayerWidget()
{
}


void PF::LayerWidget::on_cell_toggled( const Glib::ustring& path )
{
  int page = get_current_page();
  if( page < 0 ) return;
  Gtk::TreeModel::iterator iter = layer_views[page]->get_model()->get_iter( path );
  if (iter) {
    PF::LayerTreeColumns& columns = layer_views[page]->get_columns();
    bool visible = (*iter)[columns.col_visible];
    PF::Layer* l = (*iter)[columns.col_layer];
    std::cout<<"Toggled visibility of layer \""<<l->get_name()<<"\": "<<visible<<std::endl;
    l->set_visible( visible );
    l->set_dirty( true );
    layer_manager->rebuild( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );

    signal_redraw.emit();
    std::cout<<"signal_redraw() emitted."<<std::endl;
  }
}

