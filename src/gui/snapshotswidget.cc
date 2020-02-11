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


#include "imageeditor.hh"
#include "snapshotswidget.hh"





PF::SnapshotsListModel::SnapshotsListModel()
{
  set_column_types( columns );
}


Glib::RefPtr<PF::SnapshotsListModel> PF::SnapshotsListModel::create()
{
  return Glib::RefPtr<SnapshotsListModel>( new PF::SnapshotsListModel() );
}



PF::SnapshotsWidget::SnapshotsWidget(ImageEditor* e): editor(e),
button_new(_("new")),
button_delete(_("delete"))
{
  list_model = PF::SnapshotsListModel::create();
  snapshots_list.set_model(list_model);
  snapshots_list.append_column("Name", list_model->columns.col_name);
  snapshots_list.set_headers_visible(false);

  add_snapshot("snapshot 1", NULL);
  add_snapshot("snapshot 2", NULL);
  add_snapshot("snapshot 3", NULL);

  snapshots_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  snapshots_scrollwin.add(snapshots_list);

  buttons_hbox.pack_start(button_new, Gtk::PACK_EXPAND_WIDGET);
  buttons_hbox.pack_start(button_delete, Gtk::PACK_EXPAND_WIDGET);

  pack_start(buttons_hbox, Gtk::PACK_SHRINK);
  pack_start(snapshots_scrollwin, Gtk::PACK_EXPAND_WIDGET);
  show_all_children();
}

void PF::SnapshotsWidget::add_snapshot(Glib::ustring name, PF::Snapshot* snapshot)
{
  Gtk::TreeModel::iterator iter = list_model->append();
  Gtk::TreeModel::Row row = *(iter);
  row[list_model->columns.col_name] = name;
  row[list_model->columns.col_snapshot] = snapshot;
}

PF::SnapshotsWidget::~SnapshotsWidget()
{

}
