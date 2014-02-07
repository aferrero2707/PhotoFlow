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

#include "mainwindow.hh"
#include <iostream>

#include "layertree.hh"

namespace PF {

  MainWindow::MainWindow(char* filename):
#ifdef GTKMM_2
    mainBox(),
    editorBox(),
    viewBox(),
    controlBox(),
#endif
#ifdef GTKMM_3
    mainBox(Gtk::ORIENTATION_HORIZONTAL),
    viewBox(Gtk::ORIENTATION_VERTICAL),
    controlBox(Gtk::ORIENTATION_VERTICAL),
#endif
    buttonOpen("Open"), buttonSave("Save"), buttonExit("Exit")
{
  set_title("Photo Flow");
  // Sets the border width of the window.
  set_border_width(0);
  set_default_size(1200,800);
  
  add(mainBox);

  mainBox.pack_start(topButtonBox, Gtk::PACK_SHRINK);
  mainBox.pack_start(editorBox);

  editorBox.pack1(viewBox);
  editorBox.pack2(controlBox,false,true);

  viewBox.pack_start(topButtonBox, Gtk::PACK_SHRINK);
  viewBox.pack_start(viewerNotebook);

  //VImage* image = new VImage("../testimages/lena512color.jpg");
  //imageArea.set_image("../testimages/lena512color-bis.jpg");
  imageArea.set_image( filename );

  viewerNotebook.append_page(imageArea_scrolledWindow,"Image #1");
  imageArea_scrolledWindow.add(imageArea);

  topButtonBox.pack_start(buttonOpen, Gtk::PACK_SHRINK);
  topButtonBox.pack_start(buttonSave, Gtk::PACK_SHRINK);
  topButtonBox.pack_start(buttonExit, Gtk::PACK_SHRINK);
  topButtonBox.set_border_width(5);
  topButtonBox.set_layout(Gtk::BUTTONBOX_START);
  buttonExit.signal_clicked().connect( sigc::mem_fun(*this,
						     &MainWindow::on_button_exit) );

  //treeFrame.set_border_width(10);
  //treeFrame.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);


  controlBox.pack_start(layersWidget);

  //layersWidget.set_layer_manager( imageArea.get_layer_manager() );
  layersWidget.set_image( imageArea.get_image() );

  /*
  controlBox.pack_start(treeNotebook);

  treeNotebook.set_tab_pos(Gtk::POS_LEFT); 

  treeFrame.add(layerTree);


  layerTree.set_reorderable();
  LayerTree* layertree = new LayerTree( imageArea.get_layer_manager() );
  layerTree.set_model(layertree->get_model());

  layerTree.append_column_editable("V", layertree->get_columns().col_visible);
  layerTree.append_column_editable("Name", layertree->get_columns().col_name);

  treeNotebook.append_page(treeFrame,"Layers");
  Widget* page = treeNotebook.get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)treeNotebook.get_tab_label(*page);
  label->set_angle(90);
  */
  

  show_all_children();


}

  MainWindow::~MainWindow()
  {
  }

  void MainWindow::on_button_clicked()
  {
    std::cout << "Hello World" << std::endl;
  }

  void MainWindow::on_button_exit()
  {
    hide();
  }
}
