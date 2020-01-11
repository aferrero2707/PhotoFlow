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

#ifndef PF_MAIN_WINDOW_H
#define PF_MAIN_WINDOW_H

#if defined(__MINGW32__) || defined(__MINGW64__)

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#endif



#include <string>

#include <gtkmm.h>

#include "imageeditor.hh"
#include "exportdialog.hh"


namespace PF {


class MessangerDialog: public Messanger
{
  Gtk::Window* window;
public:
  MessangerDialog(Gtk::Window* w): Messanger(), window(w) {}
  virtual void show(std::string msg);
};


class MainWindow : public Gtk::Window
{
protected:
  //Signal handlers:
  void on_button_clicked();
  void on_button_exit();

  //Member widgets:
#ifdef GTKMM_2
  Gtk::VBox mainBox;
  Gtk::VBox editorBox;
  Gtk::VBox controlBox;
  //Gtk::HButtonBox topButtonBox1, topButtonBox2;
  Gtk::HBox topButtonBox1, topButtonBox2;
#endif
#ifdef GTKMM_3
  Gtk::Box mainBox;
  Gtk::Box editorBox;
  Gtk::Box controlBox;
  Gtk::ButtonBox topButtonBox1, topButtonBox2;
#endif
  Gtk::HBox top_box;
  Gtk::Notebook viewerNotebook;
  Gtk::Frame files_frame, editing_frame;
  Gtk::Image img_open, img_save, img_save_as, img_export, img_settings, img_exit;
  Gtk::Button buttonOpen, buttonSave, buttonSaveAs, buttonExport, buttonExit, buttonSettings;
  Gtk::Image img_load_preset, img_save_preset, img_trash;
  Gtk::Button buttonNewLayer, buttonNewGroup, buttonDelLayer, buttonLoadPreset, buttonSavePreset;

  std::vector<ImageEditor*> image_editors;

  // Global export dialog to remember the settings
  ExportDialog export_dialog;

  //Gtk::ScrolledWindow treeFrame;
  //Gtk::TreeView layerTree;

  Gtk::MenuBar* menubar;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  void make_menus();

public:
  MainWindow();
  virtual ~MainWindow();

  void on_map();

  void on_button_open_clicked();

  bool on_button_save_clicked();
  bool on_button_saveas_clicked();

  void on_button_export_clicked();

  void on_button_settings_clicked();

  bool on_delete_event( GdkEventAny* event );

  bool on_key_press_event(GdkEventKey* event);

  void open_image(std::string filename);

  int get_number_of_images() { return viewerNotebook.get_n_pages(); }

  void update_all_images();

  void set_image_name(Glib::ustring name);

  bool remove_tab( Gtk::Widget* widget, bool immediate );
  void remove_tab_cb( Gtk::Widget* widget, bool immediate );
  bool remove_all_tabs();

  void on_my_switch_page(
#ifdef GTKMM_2
                         GtkNotebookPage* 	page,
#endif
#ifdef GTKMM_3
                         Widget* page,
#endif
                         guint page_num);
};

}

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
