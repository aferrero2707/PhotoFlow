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

#include "outmode_slider.hh"


PF::OutModeSlider::OutModeSlider( OperationConfigDialog* dialog, std::string pname, std::string l, 
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

  //label1.set_size_request(100,-1);
  //label2.set_size_request(100,-1);
  //label3.set_size_request(100,-1);

#ifdef GTKMM_3
  label1.set_halign( Gtk::ALIGN_START );
  label2.set_halign( Gtk::ALIGN_CENTER );
  label3.set_halign( Gtk::ALIGN_END );
#endif
  label1.set_text( "keep color/luminance" );
  label2.set_text( "normal" );
  label3.set_text( "preserve colors" );
  scale.set_digits(0);
  if( sincr < 1 ) { scale.set_digits(1); spinButton.set_digits(1); }
  if( sincr < 0.1 )  { scale.set_digits(2); spinButton.set_digits(2); }
  scale.set_size_request( 250, -1 );
  //spinButton.set_size_request( 70, -1 );

  // Full widget with slider and spin button
  scale.set_value_pos(Gtk::POS_LEFT);
  scale.set_draw_value( false );
  align.set(0,0.5,0,1);
  //align.add( label1 );
  //align.add( label2 );
  //align.add( label3 );

  //Gtk::PackOptions label_pack = Gtk::PACK_EXPAND_WIDGET;
  Gtk::PackOptions label_pack = Gtk::PACK_SHRINK;



  //labelsBox1.pack_start( vline1, Gtk::PACK_SHRINK );
  labelsBox1.pack_start( label1, label_pack );
  //labelsBox1.pack_start( padding1, Gtk::PACK_EXPAND_WIDGET );
  //labelsBox1.set_size_request(120,-1);

  //labelsBox2.pack_start( padding2, Gtk::PACK_EXPAND_WIDGET );
  labelsBox2.pack_start( label3, label_pack );
  //labelsBox2.pack_start( vline3, Gtk::PACK_SHRINK );
  //labelsBox2.set_size_request(120,-1);

  labelsBox.pack_start( labelsBox1, Gtk::PACK_SHRINK );
  //labelsBox.pack_start( padding3, Gtk::PACK_EXPAND_WIDGET );
  //labelsBox.pack_start( vline2, Gtk::PACK_SHRINK );
  //labelsBox.pack_start( padding4, Gtk::PACK_EXPAND_WIDGET );
  //labelsBox.pack_start( labelsBox2, Gtk::PACK_EXPAND_WIDGET );
  //labelsBox.set_size_request(250,-1);
  labelsBoxOuter.pack_start( labelsBox, Gtk::PACK_SHRINK );


  //labelsBox.pack_start( label2, label_pack );
  //labelsBox.pack_start( padding2, Gtk::PACK_EXPAND_WIDGET );
  //labelsBox.pack_start( vline3, Gtk::PACK_SHRINK );
  //labelsBox.pack_start( label3, label_pack );
  //labelsBox.pack_start( vline4, Gtk::PACK_SHRINK );
  
  vbox.pack_start( labelsBoxOuter );
  hbox.pack_start( scale );
  hbox.pack_start( spinButton );
  vbox.pack_start( hbox );

  //spinButtonBox.pack_start( spinButton, Gtk::PACK_SHRINK );
  //spinButtonBox.pack_start( spinButtonPadding, Gtk::PACK_EXPAND_WIDGET );
  //vbox.pack_start( spinButtonBox );
  
  //pack_start( align );
  pack_start( vbox );

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


void PF::OutModeSlider::get_value()
{
  //std::cout<<"PF::OutModeSlider::get_value(): property=\""<<get_prop_name()<<"\"(0x"<<get_prop()<<")"<<std::endl;
  if( !get_prop() ) return;
  double val;
  get_prop()->get(val);
  //std::cout<<"PF::OutModeSlider::get_value(): value="<<val<<std::endl;
#ifdef GTKMM_2
  adjustment.set_value( val*multiplier );
#endif
#ifdef GTKMM_3
  adjustment->set_value( val*multiplier );
#endif
}


void PF::OutModeSlider::set_value()
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
