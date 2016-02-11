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

#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#include "../base/file_util.hh"
#include "../base/imageprocessor.hh"
#include "../base/pf_file_loader.hh"
#include "../gui/layertree.hh"
#include "pluginwindow.hh"


PF::PluginWindow::PluginWindow():
#ifdef GTKMM_2
  mainBox(),
  editorBox(),
  controlBox(),
#endif
#ifdef GTKMM_3
  mainBox(Gtk::ORIENTATION_VERTICAL),
  editorBox(Gtk::ORIENTATION_VERTICAL),
  controlBox(Gtk::ORIENTATION_VERTICAL),
  buttonBox(Gtk::ORIENTATION_HORIZONTAL),
#endif
  buttonOk( _("Ok") ),
  buttonCancel( _("Cancel") ),
  image_editor( NULL )
{
  imgbuf.buf = NULL;
  imgbuf.iccdata = NULL;
  imgbuf.iccsize = 0;

  set_title("Photo Flow");
  // Sets the border width of the window.
  set_border_width(0);
  //set_default_size(120,80);
  set_default_size(1280,700);
  
  add(mainBox);

  //mainBox.pack_start(topButtonBox, Gtk::PACK_SHRINK);
  mainBox.pack_start(editorBox);
  mainBox.pack_start(buttonBox, Gtk::PACK_SHRINK);

  //editorBox.pack_start(viewerNotebook);

  //VImage* image = new VImage("../testimages/lena512color.jpg");
  //imageArea.set_image("../testimages/lena512color-bis.jpg");
  //imageArea.set_image( filename );
  //set_image( filename );

  //viewerNotebook.append_page(buttonTest,"test");
  //imageArea_scrolledWindow.add(imageArea);

  buttonBox.pack_start(buttonOk, Gtk::PACK_SHRINK);
  buttonBox.pack_start(buttonCancel, Gtk::PACK_SHRINK);
  //topButtonBox.pack_start(buttonSaveAs, Gtk::PACK_SHRINK);
  //topButtonBox.pack_start(buttonExport, Gtk::PACK_SHRINK);
  //topButtonBox.pack_start(buttonExit, Gtk::PACK_SHRINK);
  //topButtonBox.set_border_width(5);
  buttonBox.set_layout(Gtk::BUTTONBOX_CENTER);

  buttonOk.signal_clicked().connect( sigc::mem_fun(*this,
      &PluginWindow::on_button_ok) );
  buttonCancel.signal_clicked().connect( sigc::mem_fun(*this,
      &PluginWindow::on_button_cancel) );

  show_all_children();
}

PF::PluginWindow::~PluginWindow()
{
  PF::PhotoFlow::Instance().set_active_image( NULL );
  std::cout<<"~PluginWindow(): deleting image editor"<<std::endl;
  if( image_editor ) delete( image_editor );
  std::cout<<"~PluginWindow(): image editor deleted"<<std::endl;

  /*
  std::cout<<"~PluginWindow(): submitting end request for image processor"<<std::endl;
  ProcessRequestInfo request;
  request.request = PF::PROCESSOR_END;
  PF::ImageProcessor::Instance().submit_request( request );	
  std::cout<<"~PluginWindow(): request submitted"<<std::endl;
  */
  //delete pf_image;
}


void PF::PluginWindow::on_button_ok()
{
  if( image_editor && image_editor->get_image() )
  {
    image_editor->get_image()->export_merged_to_mem( &imgbuf );
    if( imgbuf.buf ) {
      for( int i = 0; i < imgbuf.width*imgbuf.height*3; i++ ) {
        //std::cout<<"imgbuf.buf["<<i<<"]="<<imgbuf.buf[i]<<std::endl;
        //imgbuf.buf[i] *= 65535;
      }
    }
  }

  hide();
}


void PF::PluginWindow::on_button_cancel()
{
  hide();
}


void PF::PluginWindow::on_unmap()
{
  std::string bckname = image_editor->get_image()->get_backup_filename();
  unlink( bckname.c_str() );
  bckname += ".info";
  unlink( bckname.c_str() );
}



#define LOAD_PFI

void
PF::PluginWindow::open_image( std::string filename )
{
  if( image_editor ) delete image_editor;

	char* fullpath = strdup( filename.c_str() );
  image_editor = new PF::ImageEditor( fullpath );
	free(fullpath);
  editorBox.pack_start( *image_editor );
  //image_editor->show();
}


