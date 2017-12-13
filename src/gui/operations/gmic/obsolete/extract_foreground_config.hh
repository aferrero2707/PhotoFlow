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

#ifndef GMIC_EXTRACT_FOREGROUND_CONFIG_DIALOG_HH
#define GMIC_EXTRACT_FOREGROUND_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../../operations/gmic/extract_foreground.hh"

#include "../../operation_config_gui.hh"

#include "../../widgets/layerlist.hh"


namespace PF {

  class GmicExtractForegroundConfigGUI: public OperationConfigGUI
  {
    Gtk::Button updateButton;
    Gtk::RadioButton editPointsButton, showMaskButton, showBlendButton;
    Gtk::VBox controlsBox;
    
    LayerList layer_list;

    GmicExtractForegroundPar* get_par();
       
  public:
    GmicExtractForegroundConfigGUI( Layer* l );
    
    void do_update()
    {
      layer_list.update_model();
      OperationConfigGUI::do_update();
    }

    void init()
    {
      layer_list.update_model();
      OperationConfigGUI::init();
    }

    void on_update();
    void on_edit_points();
    void on_show_mask();
    void on_show_blend();
    void open();

    //bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
    //bool pointer_motion_event( int button, double x, double y, int mod_key );

    virtual bool modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                                 float scale, int xoffset, int yoffset );
  };

}

#endif
