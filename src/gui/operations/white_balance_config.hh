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

#ifndef WHITE_BALANCE_CONFIG_DIALOG_HH
#define WHITE_BALANCE_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/white_balance.hh"


namespace PF {

class WBSelector2: public Selector
{
  std::string maker, model;
public:
  WBSelector2(OperationConfigGUI* dialog, std::string pname, std::string l, int val):
    Selector( dialog, pname, l, val )
  {
  }
  WBSelector2(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val):
    Selector( dialog, processor, pname, l, val )
  {
  }

  void set_maker_model( std::string ma, std::string mo)
  {
    bool modified = false;
    if( maker != ma || model != mo )
      modified = true;
    maker = ma; model = mo;
    if( modified ) get_value();
  }

  bool check_value( int id, const std::string& name, const std::string& val );
};

  class WhiteBalanceConfigGUI: public OperationConfigGUI
  {
    Gtk::HBox wbHBox;
    Gtk::VBox wbControlsBox;
    
    WBSelector2 wbModeSelector;
    Slider wbTempSlider;
    Slider wbTintSlider;
    //Slider wbRedSlider;
    //Slider wbGreenSlider;
    //Slider wbBlueSlider;
    Slider* wbRedSliders[WB_LAST];
    Slider* wbGreenSliders[WB_LAST];
    Slider* wbBlueSliders[WB_LAST];
    Gtk::VBox wbSliderBoxes[WB_LAST];
    Gtk::VBox wbSliderBox;

    Slider wbRedCorrSlider;
    Slider wbGreenCorrSlider;
    Slider wbBlueCorrSlider;

    Slider wb_target_L_slider;
    Slider wb_target_a_slider;
    Slider wb_target_b_slider;
    Gtk::Label wb_best_match_label;
    Gtk::HBox wbTargetBox;


    double XYZ_to_CAM[3][3], CAM_to_XYZ[3][3];
    float preset_wb[3];
    
    void temp2mul(double TempK, double tint, double mul[3]);
    void mul2temp(float coeffs[3], double *TempK, double *tint);

    bool ignore_temp_tint_change;
    void temp_tint_changed();


  public:
    WhiteBalanceConfigGUI( Layer* l );
    
    void do_update();

    void spot_wb( double x, double y );
    void color_spot_wb( double x, double y );
    
    //bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
  };

}

#endif
