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

#include "image.hh"


void PF::Image::update()
{
  std::vector<View>::iterator vi;
  for( vi = views.begin(); vi != views.end(); vi++ ) {
    layer_manager.rebuild( *vi, PF::PF_COLORSPACE_RGB, 100, 100 );
  }

  signal_modified.emit();
  std::cout<<"signal_redraw() emitted."<<std::endl;
}
