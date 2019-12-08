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


void PF::Slider::create_widgets( std::string l, double val,
    double min, double max,
    double sincr, double pincr, int size, int layout )
{
#ifdef GTKMM_2
  numentry.set_adjustment( &adjustment );
#endif
#ifdef GTKMM_3
  adjustment = Gtk::Adjustment::create( val, min, max, sincr, pincr, 0 );
  scale.set_adjustment( adjustment );
  spinButton.set_adjustment( adjustment );
  numentry.set_adjustment( adjustment );
#endif

  label.set_text( l.c_str() );
  scale.set_digits(0);
  if( sincr < 1 ) { scale.set_digits(1); spinButton.set_digits(1); }
  if( sincr < 0.1 )  { scale.set_digits(2); spinButton.set_digits(2); }
  scale.set_size_request( /*size*/50, -1 );
  spinButton.set_size_request( 50, -1 );
  spinButton.set_has_frame( false );

  //numentry.set_size_request( 30, 50 );
  //numentry.set_has_frame( false );

  //int layout = 4;

  if( (max-min) < 1000000 ) {
    // Full widget with slider and spin button
    if(layout == 1 ) {
    pack_start( vbox, Gtk::PACK_SHRINK );

    vbox.pack_start( hbox, Gtk::PACK_SHRINK );
    //vbox.set_spacing(-3);

    hbox.pack_start( label, Gtk::PACK_SHRINK );

    //reset_button_align.set( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0 );
    //reset_button_align.add( reset_button );
    vbox2.pack_start( reset_button, Gtk::PACK_EXPAND_WIDGET );
    hbox.pack_end( vbox2, Gtk::PACK_SHRINK );
    hbox.pack_end( numentry, Gtk::PACK_SHRINK, 0 );

    scale.set_value_pos(Gtk::POS_LEFT);
    scale.set_draw_value( false );
    //align.set(0,0.5,0,1);
    //align.add( label );

    //hbox.pack_start( scale, Gtk::PACK_SHRINK );
    //hbox.pack_start( spinButton, Gtk::PACK_SHRINK );
    //set_spacing(-3);
    //pack_start( align );

    vbox.pack_start( scale, Gtk::PACK_SHRINK );
    //vbox2.pack_end( numentry, Gtk::PACK_SHRINK );
    //set_spacing(4);
    //pack_start( spinButton, Gtk::PACK_SHRINK );
    //hbox2.pack_start( vbox, Gtk::PACK_SHRINK );
    //hbox2.set_baseline_position( Gtk::BASELINE_POSITION_CENTER );
    //hbox2.pack_start( numentry, Gtk::PACK_SHRINK, 4 );
    }

    if(layout == 2 ) {
      // label on the left of the slider
      reset_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      reset_align.add( reset_button );
      scale.set_draw_value( false );
      scale_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      scale_align.add( scale );
      numentry_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      numentry_align.add( numentry );
      label_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      label_align.add( label );
      //vbox2.pack_start( reset_button, Gtk::PACK_SHRINK );
      //hbox.pack_start( numentry, Gtk::PACK_SHRINK, 0 );
      //hbox.pack_start( vbox2, Gtk::PACK_SHRINK );
      //pack_end( hbox, Gtk::PACK_SHRINK );
      hbox.pack_start( label_align, Gtk::PACK_SHRINK );
      hbox.pack_start( scale_align, Gtk::PACK_SHRINK );
      hbox.pack_start( numentry_align, Gtk::PACK_SHRINK );
      hbox.pack_start( reset_align, Gtk::PACK_SHRINK );
      align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      align.set_padding(4,4,4,4);
      align.add( hbox );
      pack_end( align, Gtk::PACK_SHRINK );

    } else if(layout == 4 ) {
      // label above the slider
      reset_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      reset_align.add( reset_button );
      scale.set_draw_value( false );
      scale_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      scale_align.add( scale );
      numentry_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      numentry_align.add( numentry );
      label_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      label_align.add( label );
      //vbox2.pack_start( reset_button, Gtk::PACK_SHRINK );
      //hbox.pack_start( numentry, Gtk::PACK_SHRINK, 0 );
      //hbox.pack_start( vbox2, Gtk::PACK_SHRINK );
      //pack_end( hbox, Gtk::PACK_SHRINK );
      hbox.pack_start( label_align, Gtk::PACK_SHRINK, 4 );
      hbox2.pack_start( scale_align, Gtk::PACK_SHRINK );
      hbox.pack_end( reset_align, Gtk::PACK_SHRINK );
      hbox.pack_end( numentry_align, Gtk::PACK_SHRINK );
      vbox.pack_start( hbox, Gtk::PACK_SHRINK );
      vbox.pack_start( hbox2, Gtk::PACK_SHRINK );
      align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      align.set_padding(0,0,10,10);
      align.add( vbox );
      pack_end( align, Gtk::PACK_EXPAND_WIDGET );

    } else if(layout == 3 ) {
      // label above the slider, with the reset button on its left
      reset_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      reset_align.add( reset_button );
      label_align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      label_align.add( label );
      hbox.pack_start( reset_align, Gtk::PACK_SHRINK, 4 );
      hbox.pack_start( label_align, Gtk::PACK_SHRINK, 4 );

      scale.set_draw_value( false );
      scale_align.set( Gtk::ALIGN_START, Gtk::ALIGN_END, 1, 0 );
      scale_align.add( scale );
      numentry_align.set( Gtk::ALIGN_END, Gtk::ALIGN_END, 0, 0 );
      numentry_align.add( numentry );
      hbox2.pack_start( scale_align, Gtk::PACK_EXPAND_WIDGET );
      hbox2.pack_end( numentry_align, Gtk::PACK_SHRINK );
      vbox.set_spacing(-8);
      vbox.pack_start( hbox, Gtk::PACK_SHRINK );
      vbox.pack_start( hbox2, Gtk::PACK_SHRINK );
      //align.set( Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0, 0 );
      //align.set_padding(0,0,1,1);
      //align.add( vbox );
      pack_end( vbox, Gtk::PACK_EXPAND_WIDGET );

    }
  } else {
    //hbox.pack_start( label, Gtk::PACK_SHRINK );
    //hbox.pack_start( spinButton, Gtk::PACK_SHRINK );
    //set_spacing(4);
    hbox.pack_start( label, Gtk::PACK_SHRINK, 5 );
    //pack_start( spinButton, Gtk::PACK_SHRINK );
    hbox.pack_start( numentry, Gtk::PACK_SHRINK, 0 );
    //reset_button_align.set( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0, 0 );
    //reset_button_align.add( reset_button );
    //pack_start( reset_button_align, Gtk::PACK_SHRINK );
    vbox2.pack_start( reset_button, Gtk::PACK_EXPAND_WIDGET );
    hbox.pack_start( vbox2, Gtk::PACK_SHRINK );
    pack_end( hbox, Gtk::PACK_SHRINK );
  }

  //pack_start( hbox, Gtk::PACK_SHRINK );

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

  reset_button.signal_clicked.connect(sigc::mem_fun(*this,
        &PF::Slider::reset) );
  reset_button.signal_clicked.connect(sigc::mem_fun(*this,
        &PF::Slider::changed) );

  show_all_children();

}


PF::Slider::Slider( OperationConfigGUI* dialog, std::string pname, std::string l,
		    double val, double min, double max, double sincr, double pincr,
		    double mult, int size, int layout ):
  PF::PFWidget( dialog, pname ),
#ifdef GTKMM_2
  adjustment( val, min, max, sincr, pincr, 0),
  scale(adjustment),
  spinButton(adjustment),
#endif
  reset_button(PF::PhotoFlow::Instance().get_icons_dir()+"/libre-restore.png",PF::PhotoFlow::Instance().get_icons_dir()+"/libre-restore-pressed.png"),
  multiplier(mult), fun_slider_to_prop(NULL), fun_prop_to_slider(NULL)
{
  create_widgets( l, val, min, max, sincr, pincr, size, layout );
  value_changed.connect( sigc::mem_fun(*this, &PF::Slider::update_gui) );
}


PF::Slider::Slider( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l,
		    double val, double min, double max, double sincr, double pincr,
		    double mult, int size, int layout ):
  PF::PFWidget( dialog, processor, pname ),
#ifdef GTKMM_2
  adjustment( val, min, max, sincr, pincr, 0),
  scale(adjustment),
  spinButton(adjustment),
#endif
  reset_button(PF::PhotoFlow::Instance().get_icons_dir()+"/libre-restore.png",PF::PhotoFlow::Instance().get_icons_dir()+"/libre-restore-pressed.png"),
  multiplier(mult), fun_slider_to_prop(NULL), fun_prop_to_slider(NULL), user_data(NULL)
{
  create_widgets( l, val, min, max, sincr, pincr, size, layout );
  value_changed.connect( sigc::mem_fun(*this, &PF::Slider::update_gui) );
}


PF::Slider::~Slider()
{
  if(user_data) free( user_data );
}


void PF::Slider::get_value()
{
  //std::cout<<"PF::Slider::get_value(): property=\""<<get_prop_name()<<"\"(0x"<<get_prop()<<")"<<std::endl;
  if( !get_prop() ) return;
  double val;
  get_prop()->get(val);
  if(fun_prop_to_slider) val = (*fun_prop_to_slider)(val, get_dialog(), user_data);
#ifdef GTKMM_2
  adjustment.set_value( val*multiplier );
#endif
#ifdef GTKMM_3
  adjustment->set_value( val*multiplier );
  spinButton.update();
  //std::cout<<"PF::Slider::get_value("<<get_prop_name()<<"): spinButton.value="<<spinButton.get_value()<<std::endl;
  //std::cout<<"PF::Slider::get_value("<<get_prop_name()<<"): adjustment.value="<<adjustment->get_value()<<std::endl;
#endif
}


void PF::Slider::set_value()
{
  if( !get_prop() ) return;
#ifdef GTKMM_2
  double val = adjustment.get_value()/multiplier;
#endif
#ifdef GTKMM_3
  double val = adjustment->get_value()/multiplier;
#endif
  if(fun_slider_to_prop) val = (*fun_slider_to_prop)(val, get_dialog(), user_data);
  get_prop()->update(val);
  //std::cout<<"PF::Slider::set_value(): property=\""<<get_prop_name()<<"\"(0x"<<get_prop()<<")  val="<<val<<std::endl;
}
