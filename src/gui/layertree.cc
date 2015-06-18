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

#include "layertree.hh"
#include "../base/image.hh"

static const struct {
  guint  	 width;
  guint  	 height;
  guint  	 bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  guint8 	 pixel_data[24 * 15 * 3 + 1];
} icon_meter = {
  24, 15, 3,
  "\376\376\376\377\377\377\376\376\376\376\376\376\376\376\376\376\376\376"
  "\377\376\376\376\377\377\376\376\376\377\376\376\376\376\376\376\376\376"
  "\376\377\377\376\375\375\377\376\376\376\376\376\376\376\376\377\377\377"
  "\376\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376\376"
  "\376\376\376\377\377\377\377\377\377\377\377\377\377\376\376\377\376\376"
  "\377\377\377\376\377\377\376\377\377\377\377\377\377\367\367\377\357\357"
  "\377\353\353\377\365\365\377\374\374\376\377\377\376\377\377\377\377\377"
  "\377\376\376\377\376\376\377\377\377\377\377\377\377\377\377\376\376\376"
  "\377\377\377\377\377\377\377\376\376\377\377\377\377\377\377\377\377\377"
  "\377\353\353\377\217\217\377HH\377\27\27\377\0\0\377\0\0\377\0\0\377\0\0"
  "\377\21\21\377==\377\200\200\377\325\325\377\377\377\377\377\377\377\377"
  "\377\377\376\376\377\377\377\377\377\377\376\376\376\377\377\377\377\373"
  "\373\377\377\377\377\356\356\377LL\377\0\0\376\0\0\376\0\0\377\12\12\377"
  "\13\13\377\0\0\377\0\0\377\0\0\377\0\0\377\0\0\376\0\0\376\0\0\37788\377"
  "\322\322\377\377\377\377\375\375\377\376\376\376\376\376\376\376\376\377"
  "\377\377\376\377\377\376\203\203\376\4\4\376ZZ\377\263\263\376\320\320\376"
  "\344\344\377\351\351\376\342\342\376\311\311\376\244\244\376LL\377\0\0\376"
  "\0\0\377\0\0\377\1\1\376\0\0\376\0\0\376gg\376\377\377\377\377\377\376\376"
  "\376\376\377\377\377\377\377\376``\376\177\177\376\377\377\376\377\377\377"
  "\377\377\376\377\377\376\377\377\377\377\377\376\377\377\376\377\377\376"
  "\377\377\376\377\377\377\361\361\377mm\355\5\5\360\0\0\377\0\0\376\0\0\376"
  "\0\0\376&&\377\364\364\376\377\377\376\370\370\377\224\224\377\346\346\377"
  "\377\377\377\372\372\377\374\374\377\377\377\376\377\377\376\376\376\377"
  "\377\377\377\377\377\377\377\377\377\376\376\376\373\373\375\374\374\377"
  "\377\377hQQ\377,,\377\0\0\377\0\0\377\1\1\377\0\0\377\37\37\376\364\364\377"
  "\332\332\377\377\377\377\377\377\377\376\376\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\377\377SSS\233\241\241\377\377\377\376;;\377\0\0"
  "\377\0\0\377\0\0\377\0\0\377!!\376\377\377\377\377\377\377\376\376\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377\377"
  "\377\377\377\377\376\376\376\376\376\376\377\377\377\237\237\237\0\0\0\377"
  "\377\377\371\377\377\377\377\377\377--\377\0\0\377\0\0\377\0\0\376ww\376"
  "\376\376\377\377\377\376\376\376\376\376\376\376\376\376\376\376\376\377"
  "\377\377\376\376\376\376\376\376\377\377\377\376\376\376\376\376\376\377"
  "\377\377\332\332\332\0\0\0zzz\377\377\377\375\375\375\376\377\377\376\355"
  "\355\376\0\0\376\32\32\377\322\322\376\377\377\376\376\376\377\377\377\376"
  "\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376\376\376"
  "\376\376\377\377\377\376\376\376\376\376\376\377\377\377\25\25\25\0\0\0\330"
  "\330\330\377\377\377\377\377\377\376\376\376\376\377\377\376\311\311\376"
  "\377\377\377\377\377\376\376\376\376\376\376\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377"
  "\377\377\377\377\377\377\377\377QQQ\0\0\0""444\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\375\375\377\377\377\377\374\374\377\376\376\376"
  "\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\263"
  "\263\263\0\0\0\0\0\0\262\262\262\377\377\377\376\376\376\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376"
  "\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\376\376\376\376\376\376\377\377\377\377\377\377\321\321\321\0\0"
  "\0+++\377\377\377\375\375\375\376\376\376\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\376\376\376\376\376\376\377\377"
  "\377\376\376\376\376\376\376\376\376\376\376\376\376\377\377\377\376\376"
  "\376\376\376\376\377\377\377\377\377\377\371\371\371\325\325\325\377\377"
  "\377\377\377\377\377\377\377\376\376\376\377\377\377\376\376\376\376\376"
  "\376\376\376\376\376\376\376\377\377\377\376\376\376",
};



static const struct {
  guint  	 width;
  guint  	 height;
  guint  	 bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  guint8 	 pixel_data[24 * 16 * 3 + 1];
} icon_gradient = {
  24, 16, 3,
  "\376\376\376\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\10\10\10\31\32\32\40\40\40,,,777BBBMMMXXXcccoooz{{\205\205\205"
  "\220\220\220\234\234\234\247\247\247\263\263\263\275\275\275\311\310\311"
  "\324\324\324\340\340\340\352\353\353\366\366\366\377\377\377\377\377\377"
  "\10\10\10\31\31\31\40\40\40+++667BBBLLLXXXcccooo{{z\204\204\205\220\220\220"
  "\234\234\234\247\247\247\263\263\263\275\275\275\311\310\311\324\324\324"
  "\340\340\340\353\353\353\366\366\366\377\377\377\377\377\377\4\4\3\25\25"
  "\26\34\34\34(((334???JJJUUUaaammmxxx\203\203\203\216\216\216\232\233\232"
  "\246\246\246\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337"
  "\353\353\353\365\365\365\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34"
  "'((333???JJJUUUaa`mmmxxy\203\203\202\216\216\216\232\232\232\246\246\246"
  "\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337\353\352\352"
  "\365\366\366\377\377\377\377\377\377\4\3\4\25\26\25\34\34\34'((333???JJI"
  "UUUaaammmxxx\203\203\203\216\216\216\232\232\232\246\246\246\261\261\261"
  "\274\274\274\310\310\307\323\323\323\337\337\337\352\352\352\366\365\366"
  "\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34('(333???JJIUUUaaammmxxx"
  "\202\203\203\216\216\216\232\232\232\246\246\246\261\261\261\274\274\274"
  "\310\307\310\323\323\323\337\337\337\352\353\353\366\365\365\377\377\377"
  "\377\377\377\4\4\4\25\26\25\34\34\34(((433???JIJUUUaaammmxxy\203\203\202"
  "\216\216\216\233\232\232\246\246\246\261\262\261\274\274\274\307\310\310"
  "\323\323\323\337\337\337\353\352\352\365\365\365\377\377\377\377\377\377"
  "\4\4\4\26\25\25\34\34\34(((333???IIJUUUaaammmxxx\203\203\203\216\216\216"
  "\233\232\233\246\246\246\261\261\261\274\274\274\307\307\310\323\323\323"
  "\337\337\337\352\353\353\366\365\365\377\377\377\377\377\377\4\3\4\25\26"
  "\25\34\34\34(((333???IIJUUUaaammmyyy\202\202\202\216\216\216\232\232\232"
  "\246\246\246\261\261\261\274\274\274\310\310\307\323\323\323\337\337\337"
  "\353\353\352\365\365\366\377\377\377\377\377\377\3\4\4\26\25\25\34\34\34"
  "(((333???IIIUUUa`ammmxxx\203\203\203\216\216\216\232\233\232\246\246\246"
  "\261\261\261\274\274\274\310\310\310\323\323\323\337\337\337\353\353\353"
  "\365\365\365\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34(((333???JII"
  "UUUaaammmxxy\203\203\202\216\216\216\232\232\232\246\246\246\261\261\261"
  "\274\274\274\307\310\310\323\323\323\337\337\337\353\353\352\365\365\365"
  "\377\377\377\377\377\377\4\4\4\25\25\25\34\34\34(((333???JIIUUUaaammmxyy"
  "\203\202\203\216\216\216\232\232\232\246\246\246\261\261\261\274\274\274"
  "\310\310\310\323\323\323\337\337\337\353\352\353\365\366\365\377\377\377"
  "\377\377\377\4\4\4\26\26\26\34\34\34(((444???JJJVVVaaammmyyy\203\203\203"
  "\217\217\217\233\232\232\246\246\246\262\262\262\274\274\274\310\310\307"
  "\323\323\323\337\337\337\353\353\353\365\365\365\377\377\377\377\377\377"
  "\0\0\0\4\4\4\13\13\13\30\30\30$$$111<<<IIIUUUbbbonnzzz\206\206\206\222\223"
  "\223\237\237\237\254\254\254\267\267\267\304\304\304\320\320\320\335\335"
  "\335\351\351\351\365\365\365\377\377\377\377\377\377\252\252\252\260\260"
  "\260\262\262\262\266\266\266\272\272\272\276\276\276\301\301\301\305\306"
  "\305\312\311\311\315\315\315\321\321\321\325\325\325\331\330\330\335\335"
  "\335\340\340\340\345\344\344\350\350\350\354\354\354\360\360\360\364\364"
  "\364\370\370\370\373\373\373\377\377\377",
};


PF::LayerTreeModel::LayerTreeModel()
{
  set_column_types( columns );
}


Glib::RefPtr<PF::LayerTreeModel> PF::LayerTreeModel::create()
{
  return Glib::RefPtr<LayerTreeModel>( new PF::LayerTreeModel() );
}


PF::LayerTree::LayerTree( bool is_map ): 
  layers( NULL ),
  map_flag( is_map )
{
  treeModel = PF::LayerTreeModel::create();
  treeView.set_model(treeModel);
  treeView.append_column_editable("V", treeModel->columns.col_visible);
  treeView.append_column("Name", treeModel->columns.col_name);
  treeView.append_column("map1", treeModel->columns.col_omap);
  treeView.append_column("map2", treeModel->columns.col_imap);

  treeView.enable_model_drag_source();
  treeView.enable_model_drag_dest();

  treeView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

  Gtk::CellRendererToggle* cell = 
    dynamic_cast<Gtk::CellRendererToggle*>( treeView.get_column_cell_renderer(0) );
  cell->signal_toggled().connect( sigc::mem_fun(*this, &PF::LayerTree::on_cell_toggled) ); 

  treeModel->signal_dnd_done.
    connect( sigc::mem_fun(*this, &PF::LayerTree::update_model_cb) ); 

  add( treeView );

  set_size_request(280,0);
  /*
  Gtk::TreeModel::Row row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer";
  row[columns.col_layer] = NULL;
  row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer 2";
  row[columns.col_layer] = NULL;
  row = *(treeModel->append());
  row[columns.col_visible] = true;
  row[columns.col_name] = "new layer 3";
  row[columns.col_layer] = NULL;
  */
  //update_model();
}



PF::LayerTree::~LayerTree()
{
}


void PF::LayerTree::on_cell_toggled( const Glib::ustring& path )
{
  Gtk::TreeModel::iterator iter = treeView.get_model()->get_iter( path );
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    //PF::LayerTreeColumns& columns = columns;
    bool enabled = (*iter)[treeModel->columns.col_visible];
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    if( !l ) return;
#ifndef NDEBUG
    std::cout<<"Toggled visibility of layer \""<<l->get_name()<<"\": "<<enabled<<std::endl;
#endif
    l->set_enabled( enabled );
    l->set_dirty( true );
    //layer_manager->rebuild( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );
    l->get_image()->update();
  }
}



void PF::LayerTree::update_model( Gtk::TreeModel::Row parent_row )
{
  //PF::LayerTreeColumns& columns = columns;
  PF::Layer* parent_layer = parent_row[treeModel->columns.col_layer];
  if( !parent_layer ) return;

  Gtk::TreeModel::Row row = *(treeModel->append(parent_row.children()));
  row[treeModel->columns.col_visible] = false;
  row[treeModel->columns.col_name] = std::string( "" );
  row[treeModel->columns.col_layer] = NULL;

  std::list<Layer*> sublayers = parent_layer->get_sublayers();
  for( std::list<Layer*>::iterator li = sublayers.begin();
       li != sublayers.end(); li++ ) {
    PF::Layer* l = *li;
    row = *(treeModel->prepend(parent_row.children()));
    row[treeModel->columns.col_visible] = l->is_enabled();
    row[treeModel->columns.col_name] = l->get_name();
    row[treeModel->columns.col_layer] = l;
    if( l->get_processor()->get_par()->has_intensity() )
      //row[columns.col_imap] = Gdk::Pixbuf::create_from_file("/home/aferrero/Projects/PhotoFlow/PhotoFlow_VIPS/build/icons/meter.png");
      row[treeModel->columns.col_imap] = Gdk::Pixbuf::create_from_data(icon_meter.pixel_data,Gdk::COLORSPACE_RGB, 
							    false, 8, icon_meter.width, icon_meter.height, 
							    icon_meter.width*3);
    if( l->get_processor()->get_par()->has_opacity() )
      //row[columns.col_omap] = Gdk::Pixbuf::create_from_file("/home/aferrero/Projects/PhotoFlow/PhotoFlow_VIPS/build/icons/gradient.png");
      row[treeModel->columns.col_omap] = Gdk::Pixbuf::create_from_data(icon_gradient.pixel_data,Gdk::COLORSPACE_RGB, 
							    false, 8, icon_gradient.width, icon_gradient.height, 
							    icon_gradient.width*3);

    if( l->is_group() ) {
      update_model( row );
    }
  }
}



void PF::LayerTree::update_model()
{
  treeModel->clear();
  std::list<PF::Layer*>::iterator li;
  for( li = layers->begin(); li != layers->end(); li++ ) {
    PF::Layer* l = *li;
    if( !l ) continue;
    if( !l->get_processor() ) {
      std::cout<<"LayerTree::update_model(): NULL processor for layer \""<<l->get_name()<<"\""<<std::endl;
      continue;
    }
    if( !l->get_processor()->get_par() ) {
      std::cout<<"LayerTree::update_model(): NULL operation for layer \""<<l->get_name()<<"\""<<std::endl;
      continue;
    }
    Gtk::TreeModel::Row row = *(treeModel->prepend());
    row[treeModel->columns.col_visible] = l->is_enabled();
    row[treeModel->columns.col_name] = l->get_name();
    row[treeModel->columns.col_layer] = l;
    if( l->get_processor()->get_par()->has_intensity() )
      //row[columns.col_imap] = Gdk::Pixbuf::create_from_file("/home/aferrero/Projects/PhotoFlow/PhotoFlow_VIPS/build/icons/meter.png");
      row[treeModel->columns.col_imap] = Gdk::Pixbuf::create_from_data(icon_meter.pixel_data,Gdk::COLORSPACE_RGB, 
							    false, 8, icon_meter.width, icon_meter.height, 
							    icon_meter.width*3);
    if( l->get_processor()->get_par()->has_opacity() )
      //row[columns.col_omap] = Gdk::Pixbuf::create_from_file("/home/aferrero/Projects/PhotoFlow/PhotoFlow_VIPS/build/icons/gradient.png");
      row[treeModel->columns.col_omap] = Gdk::Pixbuf::create_from_data(icon_gradient.pixel_data,Gdk::COLORSPACE_RGB, 
							    false, 8, icon_gradient.width, icon_gradient.height, 
							    icon_gradient.width*3);

    if( l->is_group() ) {
      update_model( row );
    }
  }
  treeView.expand_all();

  signal_updated.emit();

  /*
  if (!image) {
    treeModel->clear();
    return;
  }

  const std::vector<PF::Layer*>& layers = image->get_layers();
  Gtk::TreeModel::Children children = treeModel->children();
  Gtk::TreeModel::iterator iter;
  int layerid;
  for (iter=children.begin(), layerid=0; iter != children.end(); iter++, layerid++) {
    if (layerid >= layers.size()) break;
    bool visible = layers[layerid]->is_enabled();
    const std::string& name = layers[layerid]->get_name();
    (*iter)[columns.col_visible] = visible;
    (*iter)[columns.col_name] = name;
    (*iter)[columns.col_layer] = layers[layerid];
  }

  if ( layerid >= layers.size() && iter != children.end() ) {
    // clear list items that do not correspond anymore to layers
    for( ; iter != children.end();) {
      iter = treeModel->erase(iter);
    }
  }

  if (layerid < layers.size()) {
    // Append additional layers at the end of the list
    for (; layerid < layers.size(); layerid++) {
      bool visible = layers[layerid]->is_enabled();
      const std::string& name = layers[layerid]->get_name();
      Gtk::TreeModel::Row row = *(treeModel->append());
      row[columns.col_visible] = visible;
      row[columns.col_name] = name;
      row[columns.col_layer] = layers[layerid];
    }
  }
  */
}



PF::Layer* PF::LayerTree::get_selected_layer()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    iter = get_model()->get_iter( sel_rows[0] );
  }
  //Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    return l;
  }
  return NULL;
}



int PF::LayerTree::get_selected_layer_id()
{
  int result = -1;
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  std::vector<Gtk::TreeModel::Path> sel_rows = 
    refTreeSelection->get_selected_rows();
  Gtk::TreeModel::iterator iter;
  if( !sel_rows.empty() ) {
    iter = get_model()->get_iter( sel_rows[0] );
  }
  //Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if(iter) {//If anything is selected
    Gtk::TreeModel::Row row = *iter;
    PF::Layer* l = (*iter)[treeModel->columns.col_layer];
    if(l) result = l->get_id();
  }

  return( result );
}



bool PF::LayerTree::get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter)
{
  for(  Gtk::TreeModel::iterator it = rows.begin();
        it != rows.end(); it++ ) {
    //Gtk::TreeModel::Row row = *it;
    PF::Layer* l = (*it)[treeModel->columns.col_layer];
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



bool PF::LayerTree::get_row(int id, Gtk::TreeModel::iterator& iter)
{
  Glib::RefPtr<Gtk::TreeStore> model = get_model();
  const Gtk::TreeModel::Children rows = model->children();
  return get_row( id, rows, iter );
}


void PF::LayerTree::select_row(int id)
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
    get_tree().get_selection();
  Gtk::TreeModel::iterator iter;
  if( get_row( id, iter ) ) {
    refTreeSelection->select( iter );
  }
}



