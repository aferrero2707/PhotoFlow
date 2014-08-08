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

#include "../../operations/unsharp_mask.hh"

#include "unsharp_mask_config.hh"


PF::UnsharpMaskConfigDialog::UnsharpMaskConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Gaussian Blur" ),
  radiusSlider( this, "radius", "Radius", 5, 0, 1000, 0.1, 1, 1),
  amountSlider( this, "amount", "Amount", 100, 0, 500, 1, 10, 1)
{
  controlsBox.pack_start( radiusSlider );
  controlsBox.pack_start( amountSlider );
  
  add_widget( controlsBox );
}



void PF::UnsharpMaskConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    //amountSlider.init();
    //radiusSlider.init();
  }
  OperationConfigDialog::open();
}
