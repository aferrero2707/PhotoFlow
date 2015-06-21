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

#include "../../operations/gaussblur.hh"

#include "gaussblur_config.hh"


PF::GaussBlurConfigDialog::GaussBlurConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, _("Gaussian Blur") ),
	modeSelector( this, "preview_mode", _("Preview mode: "), PF_BLUR_FAST ),
  radiusSlider( this, "radius", _("Radius"), 5, 0, 1000, 0.1, 1, 1)
{
  controlsBox.pack_start( modeSelector );
  controlsBox.pack_start( radiusSlider );
  
  add_widget( controlsBox );
}



void PF::GaussBlurConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    radiusSlider.init();
  }
  OperationConfigDialog::open();
}
