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

//#include "../../../operations/gmic/iain_denoise.hh"

#include "iain_denoise_config.hh"


PF::GmicIainDenoiseConfigDialog::GmicIainDenoiseConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Iain's Noise Reduction (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_luma_slider( this, "luma", "luma", 3, 0, 20, .20, 2.0, 1),
  prop_chroma_slider( this, "chroma", "chroma", 3, 0, 20, .20, 2.0, 1),
  prop_despeckle_slider( this, "despeckle", "despeckle", 1, 0, 4, .04, .4, 1),
  prop_highlights_slider( this, "highlights", "highlights", 0, -50000, 50000, 1000.00, 10000.0, 1),
  prop_shadows_slider( this, "shadows", "shadows", 0, -32000, 32000, 640.00, 6400.0, 1),
  prop_recover_details_selector( this, "recover_details", "recover_details", 1),
  prop_recovery_amount_slider( this, "recovery_amount", "recovery_amount", 0.1, 1, 10, .09, .9, 1),
  prop_adjust_fine_details_slider( this, "adjust_fine_details", "adjust_fine_details", 0, -500, 500, 10.00, 100.0, 1),
  prop_adjust_medium_details_slider( this, "adjust_medium_details", "adjust_medium_details", 0, -500, 500, 10.00, 100.0, 1),
  prop_adjust_large_details_slider( this, "adjust_large_details", "adjust_large_details", 0, -500, 500, 10.00, 100.0, 1),
  prop_detail_emphasis_slider( this, "detail_emphasis", "detail_emphasis", 1.35, 1, 4, .03, .3, 1),
  prop_sharpen_edges_slider( this, "sharpen_edges", "sharpen_edges", 0, 0, 4, .04, .4, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_luma_slider );
  controlsBox.pack_start( prop_chroma_slider );
  controlsBox.pack_start( prop_despeckle_slider );
  controlsBox.pack_start( prop_highlights_slider );
  controlsBox.pack_start( prop_shadows_slider );
  controlsBox.pack_start( prop_recover_details_selector );
  controlsBox.pack_start( prop_recovery_amount_slider );
  controlsBox.pack_start( prop_adjust_fine_details_slider );
  controlsBox.pack_start( prop_adjust_medium_details_slider );
  controlsBox.pack_start( prop_adjust_large_details_slider );
  controlsBox.pack_start( prop_detail_emphasis_slider );
  controlsBox.pack_start( prop_sharpen_edges_slider );
  
  add_widget( controlsBox );
}



void PF::GmicIainDenoiseConfigDialog::open()
{
  OperationConfigDialog::open();
}
