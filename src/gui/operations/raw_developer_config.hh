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

#ifndef RAW_DEVELOPER_CONFIG_DIALOG_HH
#define RAW_DEVELOPER_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/raw_developer.hh"


namespace PF {

class WBSelector: public Selector
{
  std::string maker, model;
public:
  WBSelector(OperationConfigGUI* dialog, std::string pname, std::string l, int val):
    Selector( dialog, pname, l, val )
  {
  }
  WBSelector(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val):
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

  class RawDeveloperConfigGUI: public OperationConfigGUI
  {
    Gtk::Notebook notebook;
    Gtk::HBox wbHBox;
    Gtk::VBox wbControlsBox;
    Gtk::VBox exposureControlsBox;
    Gtk::VBox lensControlsBox;
    Gtk::VBox demoControlsBox;
    Gtk::VBox outputControlsBox;
    
    WBSelector wbModeSelector;
    Slider wbTempSlider;
    Slider wbTintSlider;
    Slider wbRedSlider;
    Slider wbGreenSlider;
    Slider wbBlueSlider;
    Slider wbRedCorrSlider;
    Slider wbGreenCorrSlider;
    Slider wbBlueCorrSlider;

    Slider wb_target_L_slider;
    Slider wb_target_a_slider;
    Slider wb_target_b_slider;
    Gtk::Label wb_best_match_label;
    Gtk::HBox wbTargetBox;

		// Demosaicing method selector
		Selector demoMethodSelector;
		// False color suppression steps slider
    Slider fcsSlider;

    ExposureSlider exposureSlider;
    Slider saturationLevelSlider;
    Slider blackLevelSlider;
    Selector hlrecoModeSelector;
    Gtk::Label black_level_label;
    Gtk::Label white_level_label;
    Gtk::Alignment black_level_label_align;
    Gtk::Alignment white_level_label_align;
    Gtk::HSeparator separator;

    CheckBox enable_ca_checkbox;
    CheckBox auto_ca_checkbox;
    Slider ca_red_slider, ca_blue_slider;

    Selector profileModeSelector;
    Gtk::HBox profileModeSelectorBox;

    Gtk::HBox camProfHBox;
    Gtk::VBox camProfVBox;
    Gtk::Label camProfLabel;
    Gtk::Entry camProfFileEntry;
    Gtk::Button camProfOpenButton;

    Gtk::HBox camDCPProfHBox;
    Gtk::VBox camDCPProfVBox;
    Gtk::Label camDCPProfLabel;
    Gtk::Entry camDCPProfFileEntry;
    Gtk::Button camDCPProfOpenButton;

    Gtk::HBox gammaModeHBox;
    Gtk::VBox gammaModeVBox;
    Selector gammaModeSelector;
    Slider inGammaLinSlider;
    Slider inGammaExpSlider;

    Selector outProfileModeSelector;
    Gtk::HBox outProfileModeSelectorBox;

    Selector outProfileTypeSelector;
    Gtk::HBox outProfileTypeSelectorBox;

    Selector outTRCTypeSelector;
    Gtk::HBox outTRCTypeSelectorBox;

    Gtk::HBox outProfHBox;
    Gtk::VBox outProfVBox;
    Gtk::Label outProfLabel;
    Gtk::Entry outProfFileEntry;
    Gtk::Button outProfOpenButton;

    Gtk::VBox inProfBox, outProfBox;
    Gtk::Frame inProfFrame, outProfFrame;


    CheckBox clip_negative_checkbox;
    CheckBox clip_overflow_checkbox;


    double XYZ_to_CAM[3][3], CAM_to_XYZ[3][3];
    float preset_wb[3];
    
    void temp2mul(double TempK, double tint, double mul[3]);
    void mul2temp(float coeffs[3], double *TempK, double *tint);

    bool ignore_temp_tint_change;
    void temp_tint_changed();


  public:
    RawDeveloperConfigGUI( Layer* l );
    
    void do_update();

    void on_cam_button_open_clicked();
    void on_cam_filename_changed();
    void on_cam_dcp_button_open_clicked();
    void on_cam_dcp_filename_changed();
    void on_out_button_open_clicked();
    void on_out_filename_changed();

    void spot_wb( double x, double y );
    void color_spot_wb( double x, double y );
    
    //bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
  };

}

#endif
