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

#include "../../operations/blur_bilateral.hh"

#include "blur_bilateral_config.hh"


PF::BlurBilateralConfigGUI::BlurBilateralConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Bilateral Blur" ),
  sigmasSlider( this, "sigma_s", "Spatial variance", 25, 0, 100, 0.1, 1, 1),
  sigmarSlider( this, "sigma_r", "Value variance", 20, 0, 100, 0.1, 1, 1)
{
  controlsBox.pack_start( sigmasSlider );
  controlsBox.pack_start( sigmarSlider );
  
  add_widget( controlsBox );
}



void PF::BlurBilateralConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    //radiusSlider.init();
  }
  OperationConfigGUI::open();
}
