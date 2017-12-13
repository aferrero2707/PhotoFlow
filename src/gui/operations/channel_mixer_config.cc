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

#include "../../operations/channel_mixer.hh"

#include "channel_mixer_config.hh"


PF::ChannelMixerConfigGUI::ChannelMixerConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Channel Mixer" ),
  red_mix_slider( this, "red_mix", "Red %", 33, -200, 200, 5, 20, 100),
  green_mix_slider( this, "green_mix", "Green %", 34, -200, 200, 5, 20, 100),
  blue_mix_slider( this, "blue_mix", "Blue %", 33, -200, 200, 5, 20, 100)
{
  controlsBox.pack_start( red_mix_slider );
  controlsBox.pack_start( green_mix_slider );
  controlsBox.pack_start( blue_mix_slider );
  
  
  add_widget( controlsBox );
}
