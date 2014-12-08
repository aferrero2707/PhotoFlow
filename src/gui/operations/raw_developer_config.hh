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

#include "../operation_config_dialog.hh"
#include "../../operations/raw_developer.hh"


namespace PF {

  class RawDeveloperConfigDialog: public OperationConfigDialog
  {
    Gtk::Notebook notebook;
    Gtk::HBox wbHBox;
    Gtk::VBox wbControlsBox;
    Gtk::VBox exposureControlsBox;
    Gtk::VBox demoControlsBox;
    Gtk::VBox outputControlsBox;
    
    Selector wbModeSelector;
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

    Selector profileModeSelector;
    Gtk::HBox profileModeSelectorBox;

    Gtk::HBox camProfHBox;
    Gtk::VBox camProfVBox;
    Gtk::Label camProfLabel;
    Gtk::Entry camProfFileEntry;
    Gtk::Button camProfOpenButton;

    Gtk::HBox gammaModeHBox;
    Gtk::VBox gammaModeVBox;
    Selector gammaModeSelector;
    Slider inGammaLinSlider;
    Slider inGammaExpSlider;

    Selector outProfileModeSelector;
    Gtk::HBox outProfileModeSelectorBox;

    Gtk::HBox outProfHBox;
    Gtk::VBox outProfVBox;
    Gtk::Label outProfLabel;
    Gtk::Entry outProfFileEntry;
    Gtk::Button outProfOpenButton;

    
  public:
    RawDeveloperConfigDialog( Layer* l );
    
    void do_update();

    void on_cam_button_open_clicked();
    void on_cam_filename_changed();
    void on_out_button_open_clicked();
    void on_out_filename_changed();

    void spot_wb( double x, double y );
    void color_spot_wb( double x, double y );
    
    //bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
  };

}

#endif
