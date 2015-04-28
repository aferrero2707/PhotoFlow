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

//#include "../../operations/hue_saturation.hh"

#include "hue_saturation_config.hh"


PF::HueSaturationConfigDialog::HueSaturationConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Hue/Saturation Adjustment" ),
  hueSlider( this, "hue", "Hue", 0, -180, 180, 0.1, 10, 1),
  saturationSlider( this, "saturation", "Saturation", 0, -1, 1, 0.05, 0.2, 1)
{
  controlsBox.pack_start( hueSlider );
  controlsBox.pack_start( saturationSlider );
  
  padding1.set_size_request( 2, 20 );
  padding2.set_size_request( 2, 10 );
  padding3.set_size_request( 2, 10 );

  add_widget( padding1 );

  add_widget( controlsBox );
}
