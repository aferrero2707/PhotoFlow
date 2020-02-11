/* This code is directly derived from the gtkdisp2.cc program included in the 
 * VIPS distribution; credits go therefore to the VIPS authors.
 *
 * 8-bit RGB images only, though it would be easy to fix this.
 *
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

#ifndef PF_SNAPSHOTS_WIDGET_HH
#define PF_SNAPSHOTS_WIDGET_HH

#include <stdio.h>
#include <iostream>

#include <queue>

#include <gtkmm.h>

#include "../base/photoflow.hh"
#include "../base/pipeline.hh"
#include "../base/image.hh"



namespace PF
{

class ImageEditor;

class Snapshot;


class SnapshotsListModel: public Gtk::ListStore
{
protected:
  SnapshotsListModel();

public:
  // Definition of the calumns in the layer list
  class SnapshotsListColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    SnapshotsListColumns()
    { add(col_name); add(col_snapshot); }

    Gtk::TreeModelColumn<Glib::ustring> col_name;
    Gtk::TreeModelColumn<Snapshot*> col_snapshot;
  };


  SnapshotsListColumns columns;

  static Glib::RefPtr<SnapshotsListModel> create();
};




class SnapshotsWidget : public Gtk::VBox
{
  Gtk::HBox buttons_hbox;
  Gtk::Button button_new, button_delete;
  Gtk::ScrolledWindow snapshots_scrollwin;

  Glib::RefPtr<PF::SnapshotsListModel> list_model;
  Gtk::TreeView snapshots_list;

  ImageEditor* editor;

public:
  SnapshotsWidget(ImageEditor* e);
  ~SnapshotsWidget();

  void add_snapshot(Glib::ustring name, PF::Snapshot* snapshot);
};

}

#endif
