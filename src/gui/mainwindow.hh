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

#include <string>

#include <gtkmm.h>

#include "imagearea.hh"
#include "layerwidget.hh"


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
  Gtk::HPaned editorBox;
  Gtk::VBox viewBox;
  Gtk::VBox controlBox;
  Gtk::HButtonBox topButtonBox;
#endif
#ifdef GTKMM_3
  Gtk::Paned mainBox;
  Gtk::Box viewBox;
  Gtk::Box controlBox;
  Gtk::ButtonBox topButtonBox;
#endif
  Gtk::Notebook viewerNotebook;
  Gtk::Button buttonOpen, buttonSave, buttonExit;
  LayerWidget layersWidget;
  //Gtk::ScrolledWindow treeFrame;
  //Gtk::TreeView layerTree;
  Gtk::ScrolledWindow imageArea_scrolledWindow;
  ImageArea imageArea;

  PF::Image* pf_image;

public:
  MainWindow( char* filename );
  virtual ~MainWindow();

  LayerWidget& get_layer_widget() { return layersWidget; }

  void on_button_save_clicked();

  void set_image(std::string filename);
};

}

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
