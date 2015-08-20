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

#include "vslider.hh"


PF::VSlider::VSlider( OperationConfigGUI* dialog, std::string pname, std::string l,
		    double val, double min, double max, double sincr, double pincr,
		    double mult ):
  Gtk::VBox(),
  PF::PFWidget( dialog, pname ),
#ifdef GTKMM_2
  adjustment( val, min, max, sincr, pincr, 0),
  scale(adjustment),
  spinButton(adjustment),
#endif
  multiplier(mult)
{
#ifdef GTKMM_3
  adjustment = Gtk::Adjustment::create( val, min, max, sincr, pincr, 0 );
  scale.set_adjustment( adjustment );
  spinButton.set_adjustment( adjustment );
#endif

  label.set_text( l.c_str() ); label.set_angle( 90 ); 
  //label.set_alignment( Gtk::ALIGN_END );
  label.set_alignment( 1.0 );
  scale.set_digits(0);
  scale.set_size_request( -1, 100 );
  if( sincr >= 1 ) { spinButton.set_size_request( 48, -1 ); }
  if( sincr < 1 ) { scale.set_digits(1); spinButton.set_digits(1); }
  if( sincr < 0.1 )  { scale.set_digits(2); spinButton.set_digits(2); }
  //spinButton.set_alignment( Gtk::ALIGN_END );
  spinButton.set_alignment( 1.0 );

  if( (max-min) < 1000000 ) {
    // Full widget with slider and spin button
    scale.set_value_pos(Gtk::POS_LEFT);
    scale.set_draw_value( false );
    scale.set_inverted( true );
    align.set(0,0.5,0,1);
    align.add( label );

    hbox.pack_start( align, Gtk::PACK_SHRINK );
    hbox.pack_start( scale, Gtk::PACK_SHRINK );

    pack_start( hbox, Gtk::PACK_SHRINK );
    pack_start( spinButton, Gtk::PACK_SHRINK );
  } else {
    hbox.pack_start( label );
    hbox.pack_start( spinButton );
    pack_start( hbox );
  }

#ifdef GTKMM_2
  adjustment.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));
#endif
#ifdef GTKMM_3
  adjustment->signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));
#endif

  show_all_children();
}


PF::VSlider::VSlider( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l,
		    double val, double min, double max, double sincr, double pincr,
		    double mult ):
  Gtk::VBox(),
  PF::PFWidget( dialog, processor, pname ),
#ifdef GTKMM_2
  adjustment( val, min, max, sincr, pincr, 0),
  scale(adjustment),
  spinButton(adjustment),
#endif
  multiplier(mult)
{
#ifdef GTKMM_3
  adjustment = Gtk::Adjustment::create( val, min, max, sincr, pincr, 0 );
  scale.set_adjustment( adjustment );
  spinButton.set_adjustment( adjustment );
#endif

  label.set_text( l.c_str() );
  scale.set_digits(0);
  if( sincr < 1 ) { scale.set_digits(1); spinButton.set_digits(1); }
  if( sincr < 0.1 )  { scale.set_digits(2); spinButton.set_digits(2); }
  scale.set_size_request( 300, -1 );
  spinButton.set_size_request( 70, -1 );

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

#ifdef GTKMM_2
  adjustment.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));
#endif
#ifdef GTKMM_3
  adjustment->signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));
#endif

  show_all_children();
}


void PF::VSlider::get_value()
{
  //std::cout<<"PF::VSlider::get_value(): property=\""<<get_prop_name()<<"\"(0x"<<get_prop()<<")"<<std::endl;
  if( !get_prop() ) return;
  double val;
  get_prop()->get(val);
  //std::cout<<"PF::VSlider::get_value(): value="<<val<<std::endl;
#ifdef GTKMM_2
  adjustment.set_value( val*multiplier );
#endif
#ifdef GTKMM_3
  adjustment->set_value( val*multiplier );
#endif
}


void PF::VSlider::set_value()
{
  if( !get_prop() ) return;
#ifdef GTKMM_2
  double val = adjustment.get_value()/multiplier;
#endif
#ifdef GTKMM_3
  double val = adjustment->get_value()/multiplier;
#endif
  get_prop()->update(val);
}
