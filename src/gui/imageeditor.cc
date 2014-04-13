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

#include "imageeditor.hh"


PF::ImageEditor::ImageEditor( Image* img ):
  image( img ),
  imageArea( image->get_view(0) ),
  layersWidget( image ),
  buttonZoomIn( "Zoom +" ),
  buttonZoomOut( "Zoom -" ),
  buttonShowMerged( "show merged layers" ),
  buttonShowActive( "show active layer" )
{
  imageArea_scrolledWindow.add( imageArea );

  radioBox.pack_start( buttonShowMerged );
  radioBox.pack_start( buttonShowActive );

  Gtk::RadioButton::Group group = buttonShowMerged.get_group();
  buttonShowActive.set_group(group);

  controlsBox.pack_end( radioBox, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomOut, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomIn, Gtk::PACK_SHRINK );

  imageBox.pack_start( imageArea_scrolledWindow );
  imageBox.pack_start( controlsBox, Gtk::PACK_SHRINK );

  pack1( imageBox, true, false );

  pack2( layersWidget, false, false );

  buttonZoomIn.signal_clicked().connect( sigc::mem_fun(*this,
						       &PF::ImageEditor::zoom_in) );
  buttonZoomOut.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_out) );
  //set_position( get_allocation().get_width()-200 );

  show_all_children();
}


PF::ImageEditor::~ImageEditor()
{
  if( image )
    delete image;
}



void PF::ImageEditor::zoom_out()
{
  PF::View* view = image->get_view(0);
  if( !view ) return;
  int level = view->get_level();
  view->set_level( level + 1 );
  image->update();
}


void PF::ImageEditor::zoom_in()
{
  PF::View* view = image->get_view(0);
  if( !view ) return;
  int level = view->get_level();
  if( level > 0 ) {
    view->set_level( level - 1 );
    image->update();
  }
}
