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

#include "exposure_slider.hh"


PF::ExposureSlider::ExposureSlider( OperationConfigGUI* dialog, std::string pname, std::string l, 
				    double val, double min, double max, double sincr, double pincr):
  PF::Slider( dialog, pname, l, val, min, max, sincr, pincr, 1, 250, 3 )
{
}


void PF::ExposureSlider::get_value()
{
  if( !get_prop() ) return;
  float val;
  get_prop()->get(val);
  float exp = log2f( val );
  //#ifdef GTKMM_2
  //get_adjustment().set_value( exp );
  //#endif
  //#ifdef GTKMM_3
  get_adjustment()->set_value( exp );
  //#endif
}


void PF::ExposureSlider::set_value()
{
  if( !get_prop() ) return;
  //#ifdef GTKMM_2
  //float exp = get_adjustment().get_value();
  //#endif
  //#ifdef GTKMM_3
  float exp = get_adjustment()->get_value();
  //#endif
  float val = powf( 2.0f, exp );
  //std::cout<<"PF::ExposureSlider::set_value(): val="<<val<<std::endl;
  get_prop()->update(val);
}
