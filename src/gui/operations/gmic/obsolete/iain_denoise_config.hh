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

#ifndef GMIC_IAIN_DENOISE_CONFIG_DIALOG_HH
#define GMIC_IAIN_DENOISE_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../operation_config_gui.hh"


namespace PF {

  class GmicIainDenoiseConfigGUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    
    Slider iterations_slider;
    Slider prop_luma_slider;
    Slider prop_chroma_slider;
    Slider prop_despeckle_slider;
    Slider prop_highlights_slider;
    Slider prop_shadows_slider;
    Selector prop_recover_details_selector;
    Slider prop_recovery_amount_slider;
    Slider prop_adjust_fine_details_slider;
    Slider prop_adjust_medium_details_slider;
    Slider prop_adjust_large_details_slider;
    Slider prop_detail_emphasis_slider;
    Slider prop_sharpen_edges_slider;
       
  public:
    GmicIainDenoiseConfigGUI( Layer* l );
    
    void open();
  };

}

#endif
