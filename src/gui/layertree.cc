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

PF::LayerTree::LayerTree( ): layers( NULL )//: image(NULL)
{
  treeModel = Gtk::TreeStore::create(columns);
  set_model(treeModel);
  append_column_editable("V", get_columns().col_visible);
  append_column("Name", get_columns().col_name);

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


void PF::LayerTree::update_model()
{
  treeModel->clear();
  std::list<PF::Layer*>::iterator li;
  for( li = layers->begin(); li != layers->end(); li++ ) {
    PF::Layer* l = *li;
    Gtk::TreeModel::Row row = *(treeModel->prepend());
    row[columns.col_visible] = l->is_visible();
    row[columns.col_name] = l->get_name();
    row[columns.col_layer] = l;
  }

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
    bool visible = layers[layerid]->is_visible();
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
      bool visible = layers[layerid]->is_visible();
      const std::string& name = layers[layerid]->get_name();
      Gtk::TreeModel::Row row = *(treeModel->append());
      row[columns.col_visible] = visible;
      row[columns.col_name] = name;
      row[columns.col_layer] = layers[layerid];
    }
  }
  */
}
