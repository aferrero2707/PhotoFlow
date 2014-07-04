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

#ifndef GTKMM_EXAMPLE_HELLOWORLD_H
#define GTKMM_EXAMPLE_HELLOWORLD_H

#if defined(__MINGW32__) || defined(__MINGW64__)

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#endif



#include <string>

#include <gtkmm.h>

#include "imageeditor.hh"


namespace PF {

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
  Gtk::HButtonBox topButtonBox;
#endif
#ifdef GTKMM_3
  Gtk::Box mainBox;
  Gtk::Box editorBox;
  Gtk::Box controlBox;
  Gtk::ButtonBox topButtonBox;
#endif
  Gtk::Notebook viewerNotebook;
  Gtk::Button buttonOpen, buttonSave, buttonExport, buttonExit, buttonTest;

  std::vector<ImageEditor*> image_editors;

  //Gtk::ScrolledWindow treeFrame;
  //Gtk::TreeView layerTree;

public:
  MainWindow();
  virtual ~MainWindow();

  void on_button_open_clicked();

  void on_button_save_clicked();

  void on_button_export_clicked();

  void open_image(std::string filename);
};

}

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
