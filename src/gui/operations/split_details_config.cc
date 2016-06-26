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
#include "../../operations/split_details.hh"

#include "split_details_config.hh"


PF::SplitDetailsConfigGUI::SplitDetailsConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Multi-Scale Decomposition"  ),
  blur_type_selector( this, "blur_type", _("blur type: "), PF::SPLIT_DETAILS_BLUR_GAUSS ),
  prop_nscales_slider( this, "nscales", "nscales", 4, 1, 10, 1, 5, 1),
  prop_base_scale_slider( this, "base_scale", "base scale %", 1, 0, 1000000, .01, 1, 1)
  //prop_detail_scale_slider( this, "detail_scale", "detail scale %", 0.01, 0, 1000000, .01, 1, 1)
{
  controlsBox.pack_start( blur_type_selector, Gtk::PACK_SHRINK, 5 );
  controlsBox.pack_start( prop_nscales_slider, Gtk::PACK_SHRINK, 5 );
  controlsBox.pack_start( prop_base_scale_slider, Gtk::PACK_SHRINK, 5 );
  //controlsBox.pack_start( prop_detail_scale_slider );
  
  blur_type_selector.get_combo_box()->signal_changed().connect(sigc::mem_fun(*this,&PF::SplitDetailsConfigGUI::blur_type_selector_changed));

  add_widget( controlsBox );
}

void PF::SplitDetailsConfigGUI::blur_type_selector_changed()
{
  PF::SplitDetailsPar* lpar = dynamic_cast<PF::SplitDetailsPar*>( get_par() );
  if ( lpar->get_blur_type() == PF::SPLIT_DETAILS_BLUR_GAUSS )
    prop_base_scale_slider.show();
  else
    prop_base_scale_slider.hide();

}

void PF::SplitDetailsConfigGUI::do_update()
{
  PF::OperationConfigGUI::do_update();
  
  blur_type_selector_changed();
}


