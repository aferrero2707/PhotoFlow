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

#include "slider.hh"


PF::Slider::Slider( OperationConfigUI* dialog, std::string pname, std::string l, 
		    double val, double min, double max, double sincr, double pincr,
		    double mult ):
  Gtk::VBox(),
  PF::PFWidget( dialog, pname ),
  adjustment( val, min, max, sincr, pincr, 0),
  scale(adjustment),
  spinButton(adjustment),
  multiplier(mult)
{
  label.set_text( l.c_str() );
  scale.set_digits(0);
  if( sincr < 1 ) { scale.set_digits(1); spinButton.set_digits(1); }
  if( sincr < 0.1 )  { scale.set_digits(2); spinButton.set_digits(2); }
  scale.set_size_request( 200, -1 );
  spinButton.set_size_request( 50, -1 );

  if( (max-min) < 500 ) {
    // Full widget with slider and spin button
    scale.set_value_pos(Gtk::POS_LEFT);
    scale.set_draw_value( false );
    align.set(0,0.5,0,1);
    align.add( label );

    hbox.pack_start( scale );
    hbox.pack_start( spinButton );

    pack_start( align );
  } else {
    hbox.pack_start( label );
    hbox.pack_start( spinButton );
  }

  pack_start( hbox );

  adjustment.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));

  show_all_children();
}


void PF::Slider::get_value()
{
  if( !get_prop() ) return;
  double val;
  get_prop()->get(val);
  adjustment.set_value( val*multiplier );
}


void PF::Slider::set_value()
{
  if( !get_prop() ) return;
  double val = adjustment.get_value()/multiplier;
  get_prop()->update(val);
}
