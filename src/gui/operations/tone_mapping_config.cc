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

#include "tone_mapping_config.hh"



PF::ToneMappingCurveArea::ToneMappingCurveArea(): border_size( 0 ), is_linear( false )
{
  method = TONE_MAPPING_EXP_GAMMA;
  gamma = 1;
  set_size_request(250,150);
}


void PF::ToneMappingCurveArea::set_params(PF::ToneMappingPar* tmpar)
{
  bool filmic2_changed =
      (tmpar->get_filmic2_TS() != TS ||
          tmpar->get_filmic2_TL() != TL ||
          tmpar->get_filmic2_SS() != SS ||
          tmpar->get_filmic2_SL() != SL ||
          tmpar->get_filmic2_SA() != SA) ? true : false;
  bool redraw =
      (tmpar->get_method() != method ||
       tmpar->get_gamma() != gamma ||
       tmpar->get_filmic_A() != A ||
       tmpar->get_filmic_B() != B ||
       tmpar->get_filmic_C() != C ||
       tmpar->get_filmic_D() != D ||
       tmpar->get_filmic_E() != E ||
       tmpar->get_filmic_F() != F ||
       tmpar->get_filmic_W() != W ||
       filmic2_changed) ? true : false;

  method = tmpar->get_method();

  gamma = tmpar->get_gamma();
  exponent = 1.f / gamma;

  A = tmpar->get_filmic_A();
  B = tmpar->get_filmic_B();
  C = tmpar->get_filmic_C();
  D = tmpar->get_filmic_D();
  E = tmpar->get_filmic_E();
  F = tmpar->get_filmic_F();
  W = tmpar->get_filmic_W();

  TS = tmpar->get_filmic2_TS();
  TL = tmpar->get_filmic2_TL();
  SS = tmpar->get_filmic2_SS();
  SL = tmpar->get_filmic2_SL();
  SA = tmpar->get_filmic2_SA();

  if( filmic2_changed ) {
  FilmicToneCurve::CurveParamsUser filmic2_user;
  filmic2_user.m_toeStrength      = TS;
  filmic2_user.m_toeLength        = TL;
  filmic2_user.m_shoulderStrength = SS;
  filmic2_user.m_shoulderLength   = SL;
  if(filmic2_user.m_shoulderLength > 0.9999) filmic2_user.m_shoulderLength = 0.9999;
  filmic2_user.m_shoulderAngle    = SA;
  filmic2_user.m_gamma            = 1;
  FilmicToneCurve::CurveParamsDirect filmic2_direct;
  FilmicToneCurve::CalcDirectParamsFromUser(filmic2_direct, filmic2_user);
  FilmicToneCurve::CreateCurve(filmic2_curve, filmic2_direct);
  }

  if( redraw ) queue_draw();
}



float PF::ToneMappingCurveArea::get_curve_value( float val)
{
  float result = val;
  switch( method ) {
  case TONE_MAPPING_EXP_GAMMA:
    if( gamma != 1 ) {
        result = powf( result, exponent );
    }
    std::cout<<"val="<<val<<"  gamma="<<gamma<<"  result="<<result<<std::endl;
    break;
  case TONE_MAPPING_REINHARD: {
      result = val / (val + 1.f);
    break;
  }
  case TONE_MAPPING_HEJL: {
      float x = MAX(0,val-0.004);
      result = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
      result = powf( result, 2.2f );
    break;
  }
  case TONE_MAPPING_FILMIC: {
    float whiteScale = 1.0f/( ((W*(A*W+C*B)+D*E)/(W*(A*W+B)+D*F))-E/F );
    result *= 2;
    result = ((result * (A*result + C*B) + D*E)/(result * (A*result + B) + D*F)) - E/F;
    result *= whiteScale;
    break;
  }
  case TONE_MAPPING_FILMIC2: {
    result = filmic2_curve.Eval(val);
    break;
  }
  default:
    break;
  }
  return result;
}




#ifdef GTKMM_2
bool PF::ToneMappingCurveArea::on_expose_event(GdkEventExpose* event)
{
  //std::cout<<"CurveArea::on_expose_event() called: trc_type="<<curve.get_trc_type()<<std::endl;
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window )
    return true;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

  if( false && event ) {
    // clip to the area indicated by the expose event so that we only
    // redraw the portion of the window that needs to be redrawn
    cr->rectangle(event->area.x, event->area.y,
        event->area.width, event->area.height);
    cr->clip();
  }
#endif
#ifdef GTKMM_3
bool PF::ToneMappingCurveArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;
#endif

  cr->save();
  cr->set_source_rgba(0.2, 0.2, 0.2, 1.0);
  cr->paint();
  cr->restore();

  // Draw outer rectangle
  cr->set_antialias( Cairo::ANTIALIAS_GRAY );
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->set_line_width( 0.5 );
  cr->rectangle( double(0.5+x0-1), double(0.5+y0-1), double(width+1), double(height+1) );
  cr->stroke ();


  draw_background( cr );

  std::cout<<"CurveArea::on_draw(): is_linear="<<is_linear<<std::endl;

  PF::ICCProfile* labprof = PF::ICCStore::Instance().get_profile(PF::PROF_TYPE_REC2020, PF::PF_TRC_PERCEPTUAL);

  // Draw curve
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector< std::pair<float,float> > vec;
  //std::cout<<"PF::CurveArea::on_expose_event(): width="<<width<<"  height="<<height<<std::endl;
  for( int i = 0; i < width; i++ ) {
    float fi = i;
    fi /= (width-1);
    float fil = labprof->perceptual2linear(fi);
    float yl = get_curve_value( fi );
    float y = labprof->linear2perceptual(yl);
    std::cout<<"  fi="<<fi<<"  fil="<<fil<<"  y="<<y<<"  yl="<<yl<<std::endl;
    vec.push_back( std::make_pair( fi, y ) );
  }

  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->move_to( double(vec[0].first)*width+x0, double(1.0f-vec[0].second)*height+y0 );
  for (unsigned int i=1; i<vec.size(); i++) {
    cr->line_to( double(vec[i].first)*width+x0, double(1.0f-vec[i].second)*height+y0 );
  }
  cr->stroke ();

  return true;
}



void PF::ToneMappingCurveArea::draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;

  // Draw grid
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector<double> ds (2);
  ds[0] = 4;
  ds[1] = 4;
  cr->set_dash (ds, 0);
  cr->move_to( double(0.5+x0+width/4), double(y0) );
  cr->rel_line_to (double(0), double(height) );
  cr->move_to( double(0.5+x0+width/2), double(y0) );
  cr->rel_line_to (double(0), double(height) );
  cr->move_to( double(0.5+x0+width*3/4), double(y0) );
  cr->rel_line_to (double(0), double(height) );
  cr->move_to( double(x0), double(0.5+y0+height/4) );
  cr->rel_line_to (double(width), double(0) );
  cr->move_to( double(x0), double(0.5+y0+height/2) );
  cr->rel_line_to (double(width), double(0) );
  cr->move_to( double(x0), double(0.5+y0+height*3/4) );
  cr->rel_line_to (double(width), double(0) );
  cr->stroke ();
  cr->unset_dash ();

  ds[0] = 2;
  ds[1] = 4;
  cr->set_source_rgb( 0.5, 0.5, 0.5 );
  cr->set_dash (ds, 0);
  for( int i = 1; i <= 7; i += 2 ) {
    cr->move_to( double(0.5+x0+width*i/8), double(y0) );
    cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(x0), double(0.5+y0+height*i/8) );
    cr->rel_line_to (double(width), double(0) );
  }
  cr->stroke ();
  cr->unset_dash ();
}



PF::ToneMappingConfigGUI::ToneMappingConfigGUI( PF::Layer* layer ):
          OperationConfigGUI( layer, "Tone Mapping" ),
          exposureSlider( this, "exposure", _("exposure"), 0, -10, 10, 0.1, 1 ),
          modeSelector( this, "method", "method: ", 0 ),
          gamma_slider( this, "gamma", _("gamma adjustment"), 1, 1, 5, 0.2, 1, 1 ),
          filmic_A_slider( this, "filmic_A", _("shoulder strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_B_slider( this, "filmic_B", _("linear strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_C_slider( this, "filmic_C", _("linear angle"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_D_slider( this, "filmic_D", _("toe strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_E_slider( this, "filmic_E", _("toe num."), 0.5, 0, 0.1, 0.002, 0.01, 1 ),
          filmic_F_slider( this, "filmic_F", _("toe den."), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_W_slider( this, "filmic_W", _("lin. white point"), 10, 1, 100, 2, 10, 1 ),
          filmic2_TS_slider( this, "filmic2_TS", _("toe strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_TL_slider( this, "filmic2_TL", _("toe lenght"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_SS_slider( this, "filmic2_SS", _("shoulder strength"), 0.5, 0, 1, 0.02, 0.1, 0.1 ),
          filmic2_SL_slider( this, "filmic2_SL", _("shoulder lenght"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_SA_slider( this, "filmic2_SA", _("shoulder angle"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          lumi_blend_frac_slider( this, "lumi_blend_frac", _("preserve colors"), 1, 0, 1, 0.02, 0.1, 1 )
{
  gammaControlsBox.pack_start( gamma_slider, Gtk::PACK_SHRINK );

  filmicControlsBox.pack_start( filmic_A_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_B_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_C_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_D_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_E_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_F_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_W_slider, Gtk::PACK_SHRINK );

  filmic2ControlsBox.pack_start( filmic2_TS_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_TL_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SS_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SL_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SA_slider, Gtk::PACK_SHRINK );

  controlsBox.pack_start( curve_area_box, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( exposureSlider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( controlsBox2, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 2 );

  curve_area_box.pack_start( curve_area, Gtk::PACK_SHRINK, 10 );

  controlsBox.pack_start( lumi_blend_frac_slider, Gtk::PACK_SHRINK );


  globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( globalBox );
}




void PF::ToneMappingConfigGUI::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF:PropertyBase* prop = par->get_property( "method" );
    if( !prop )  return;
    PF::ToneMappingPar* tmpar = dynamic_cast<PF::ToneMappingPar*>(par);
    if( !tmpar ) return;

    //std::cout<<"PF::ToneMappingConfigGUI::do_update() called."<<std::endl;

    bool need_update = false;

    curve_area.set_params( tmpar );

    if( prop->get_enum_value().first == PF::TONE_MAPPING_EXP_GAMMA ) {
      if( gammaControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else if( prop->get_enum_value().first == PF::TONE_MAPPING_FILMIC ) {
      if( filmicControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else if( prop->get_enum_value().first == PF::TONE_MAPPING_FILMIC2 ) {
      if( filmic2ControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else {
      if( (gammaControlsBox.get_parent() == &controlsBox2) ||
          (filmicControlsBox.get_parent() == &controlsBox2) ||
          (filmic2ControlsBox.get_parent() == &controlsBox2) )
        need_update = true;
    }

    if( need_update ) {
      if( gammaControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( gammaControlsBox );
      if( filmicControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( filmicControlsBox );
      if( filmic2ControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( filmic2ControlsBox );

      switch( prop->get_enum_value().first ) {
      case PF::TONE_MAPPING_EXP_GAMMA:
        controlsBox2.pack_start( gammaControlsBox, Gtk::PACK_SHRINK );
        gammaControlsBox.show();
        break;
      case PF::TONE_MAPPING_FILMIC:
        controlsBox2.pack_start( filmicControlsBox, Gtk::PACK_SHRINK );
        filmicControlsBox.show();
        break;
      case PF::TONE_MAPPING_FILMIC2:
        controlsBox2.pack_start( filmic2ControlsBox, Gtk::PACK_SHRINK );
        filmic2ControlsBox.show();
        break;
      }
    }
    controlsBox2.show_all_children();
  }

  OperationConfigGUI::do_update();
}

