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

#ifndef PF_PLUGIN_WINDOW_H
#define PF_PLUGIN_WINDOW_H

#if defined(__MINGW32__) || defined(__MINGW64__)

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#endif



#include <string>

#include <gtkmm.h>

#include "../gui/imageeditor.hh"


namespace PF {

class PluginWindow : public Gtk::Window
{
protected:
  //Signal handlers:
  //void on_button_clicked();
  //void on_button_exit();

  //Member widgets:
#ifdef GTKMM_2
  Gtk::VBox mainBox;
  Gtk::VBox editorBox;
  Gtk::VBox controlBox;
  Gtk::HButtonBox buttonBox;
#endif
#ifdef GTKMM_3
  Gtk::Box mainBox;
  Gtk::Box editorBox;
  Gtk::Box controlBox;
  Gtk::ButtonBox buttonBox;
#endif
  Gtk::Notebook viewerNotebook;
  Gtk::Button buttonOk, buttonCancel, buttonSettings;

  ImageEditor* image_editor;

  std::string last_dir;

  ImageBuffer imgbuf;

  //Gtk::ScrolledWindow treeFrame;
  //Gtk::TreeView layerTree;

public:
  PluginWindow();
  virtual ~PluginWindow();

  void on_button_export_clicked();

  void on_button_ok();
  void on_button_cancel();
  void on_button_settings_clicked();

  void on_unmap();

  ImageBuffer& get_image_buffer() { return imgbuf; }

  void open_image(std::string filename);
  void update_image();
  void run()
  {
    show_all();
    gtk_main();
  }
};

}

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
