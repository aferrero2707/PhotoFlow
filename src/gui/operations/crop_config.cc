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

#include "../../operations/crop.hh"

#include "crop_config.hh"


PF::CropConfigDialog::CropConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Crop" ),
  cropLeftSlider( this, "crop_left", "Crop left", 0, 0, 10000000, 1, 10, 1),
  cropTopSlider( this, "crop_top", "Crop top", 0, 0, 10000000, 1, 10, 1),
  cropWidthSlider( this, "crop_width", "Crop width", 0, 0, 10000000, 1, 10, 1),
  cropHeightSlider( this, "crop_height", "Crop height", 0, 0, 10000000, 1, 10, 1)
{
  controlsBox.pack_start( cropLeftSlider );
  controlsBox.pack_start( cropTopSlider );
  controlsBox.pack_start( cropWidthSlider );
  controlsBox.pack_start( cropHeightSlider );
  
  add_widget( controlsBox );
}



void PF::CropConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    cropLeftSlider.init();
    cropTopSlider.init();
    cropWidthSlider.init();
    cropHeightSlider.init();
  }
  OperationConfigDialog::open();
}
