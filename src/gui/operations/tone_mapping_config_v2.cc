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

#include "tone_mapping_config_v2.hh"



PF::ToneMappingCurveAreaV2::ToneMappingCurveAreaV2(): border_size( 0 ), is_linear( false )
{
  method = TONE_MAPPING_LIN_EXP;
  exposure = 1;
  gamma = 1;
  gamma_pivot = 1;
  f2gamma = 0;
  f2midgraylock = false;
  AL_Lmax =  AL_b = 0;
  set_size_request(250,150);
}


void PF::ToneMappingCurveAreaV2::set_params(PF::ToneMappingParV2* tmpar)
{
  bool filmic2_changed =
      (f2midgraylock != tmpar->get_filmic2_preserve_midgray() ||
          tmpar->get_filmic2_gamma() != f2gamma ||
          tmpar->get_filmic2_TS() != TS ||
          tmpar->get_filmic2_TL() != TL ||
          tmpar->get_filmic2_SS() != SS ||
          tmpar->get_filmic2_SL() != SL ||
          tmpar->get_filmic2_SA() != SA) ? true : false;
  bool redraw =
      (tmpar->get_method() != method ||
          //tmpar->get_gamma() != gamma ||
          //tmpar->get_gamma_pivot() != gamma_pivot ||
          tmpar->get_exposure() != exposure ||
       tmpar->get_filmic_A() != A ||
       tmpar->get_filmic_B() != B ||
       tmpar->get_filmic_C() != C ||
       tmpar->get_filmic_D() != D ||
       tmpar->get_filmic_E() != E ||
       tmpar->get_filmic_F() != F ||
       tmpar->get_filmic_W() != W ||
       tmpar->get_AL_Lmax() != AL_Lmax ||
       tmpar->get_AL_b() != AL_b ||
       tmpar->get_AL_Tsize() != AL_Tsize_par ||
       tmpar->get_AL_Tlength() != AL_Tlength_par ||
       tmpar->get_AL_Tstrength() != AL_Tstrength_par ||
       tmpar->get_LP_slope() != LP_slope ||
       tmpar->get_LP_compression() != LP_compr ||
       tmpar->get_LP_lin_max() != LP_linmax ||
       tmpar->get_LP_knee_strength() != LP_Kstrength ||
       tmpar->get_LP_shoulder_smoothness() != LP_Ssmooth ||
       tmpar->get_HD_slope() != HD_lin_slope ||
       tmpar->get_HD_shoulder_range() != HD_SR ||
       tmpar->get_HD_toe_range() != HD_TR ||
       filmic2_changed) ? true : false;

  method = tmpar->get_method();

  exposure = tmpar->get_exposure();
  //gamma = tmpar->get_gamma();
  //gamma_pivot = pow(tmpar->get_gamma_pivot(), 2.45);
  exponent = 1.f / gamma;

  sh_compr = tmpar->get_sh_compr();
  hl_compr = tmpar->get_hl_compr();

  A = tmpar->get_filmic_A();
  B = tmpar->get_filmic_B();
  C = tmpar->get_filmic_C();
  D = tmpar->get_filmic_D();
  E = tmpar->get_filmic_E();
  F = tmpar->get_filmic_F();
  W = tmpar->get_filmic_W();

  f2midgraylock = tmpar->get_filmic2_preserve_midgray();
  f2gamma = tmpar->get_filmic2_gamma();
  f2exponent = (f2gamma >= 0) ? 1.f/(f2gamma+1) : (1.f-f2gamma);
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

  AL_Lmax = tmpar->get_AL_Lmax();
  AL_b = tmpar->get_AL_b();
  float Lmax = pow(10.0, AL_Lmax);
  float AL_scale = 1; //pow(AL_b-0.85+1, 5);
  float Ldmax = 100;
  float Lwmax = Lmax/AL_scale;
  float LLwmax = log10(Lwmax+1);
  float AL_A = 0.01 * Ldmax / LLwmax;

  AL_Tsize_par = tmpar->get_AL_Tsize();
  AL_Tlength_par = tmpar->get_AL_Tlength();
  AL_Tstrength_par = tmpar->get_AL_Tstrength();
  AL_Tsize = pow(AL_Tsize_par,2.5);
  float AL_Tslope = (Lwmax*AL_A)/(log(2)*2);
  AL_Tlength = AL_Tlength_par*AL_Tsize/AL_Tslope + 1.0e-15;
  float AL_iTlength = 1/AL_Tlength;
  AL_Texp = AL_Tslope / (AL_Tsize*AL_iTlength);
  //AL_Tshift = AL_Tsize/AL_Tslope + 1.0e-15;

  AL_Trange = AL_Tstrength_par*AL_Tsize;
  AL_Tshift = pow( (1.f-AL_Tstrength_par), 1.f/AL_Texp ) * AL_Tlength;
  AL_Tmax = AL_Tlength - AL_Tshift;
  AL_Tvshift = (1.f-AL_Tstrength_par) * AL_Tsize;

  float LP_midgray = pow(0.5,2.45);
  LP_slope = tmpar->get_LP_slope();
  LP_compr = tmpar->get_LP_compression();
  LP_linmax = pow(tmpar->get_LP_lin_max(),2.45);
  LP_Kstrength = tmpar->get_LP_knee_strength();
  LP_Kmax = ((1.f-LP_slope)*LP_midgray)/(LP_slope/(sqrt(2.0f)*LP_Kstrength)-LP_slope);
  LP_Ssmooth = tmpar->get_LP_shoulder_smoothness();
  std::cout<<"ToneMappingCurveAreaV2::set_params: LP_Ssmooth="<<LP_Ssmooth<<"  LP_compr="<<LP_compr<<std::endl;
  //std::cout<<"ToneMappingCurveAreaV2::set_params: AL_Tsize="<<AL_Tsize<<"  AL_Tlength="<<AL_Tlength
  //    <<"  AL_Tshift="<<AL_Tshift<<"  AL_Tmax="<<AL_Tmax<<"  AL_Tvshift="<<AL_Tvshift<<std::endl;

  float LE_midgray = pow(0.5,2.45);
  LE_gain = tmpar->get_LE_gain();
  LE_slope = tmpar->get_LE_slope();
  LE_compr = tmpar->get_LE_compression();
  LE_linmax = pow(tmpar->get_LE_lin_max(),2.45);
  LE_Kstrength = tmpar->get_LE_knee_strength() * ( (LE_slope-1)*1 + 1 );
  LE_Kmax = ((1.f-LE_slope)*LE_midgray)/(LE_slope/(sqrt(2.0f)*LE_Kstrength)-LE_slope);
  LE_Sslope = tmpar->get_LE_shoulder_slope();
  LE_Sslope2 = tmpar->get_LE_shoulder_slope2();
  std::cout<<"ToneMappingCurveAreaV2::set_params: LE_linmax="<<LE_linmax<<"  LE_Sslope="<<LE_Sslope<<"  LE_compr="<<LE_compr<<std::endl;
  //std::cout<<"ToneMappingCurveAreaV2::set_params: AL_Tsize="<<AL_Tsize<<"  AL_Tlength="<<AL_Tlength
  //    <<"  AL_Tshift="<<AL_Tshift<<"  AL_Tmax="<<AL_Tmax<<"  AL_Tvshift="<<AL_Tvshift<<std::endl;


  float HD_fog = 0.;
  float HD_max = 4;
  HD_lin_slope = tmpar->get_HD_slope() / HD_max;
  HD_lin_pivot = log10(1.0f/pow(0.5,2.45));
  HD_SR = tmpar->get_HD_shoulder_range();
  HD_TR = tmpar->get_HD_toe_range();
  HD_lin_Dmin = HD_SR * HD_max;
  HD_lin_Dmax = HD_max * (1.0f - HD_TR);

  if( redraw ) queue_draw();
}



float PF::ToneMappingCurveAreaV2::get_curve_value( float val )
{
  float result = val;
  float val0 = val;

  float log_pivot = pow(0.5,2.45);
  /*
  float d_compr = hl_compr - sh_compr;
  float nRGB = val/log_pivot;
  float ex = (nRGB > 1) ? nRGB - 1 : log(nRGB + 1.0e-15);
  float ex2 = atan(ex);
  float exponent = ex2 * d_compr / M_PI + sh_compr + d_compr/2;
  //float norm = (nRGB > 1) ? exp( (1-nRGB)*1 ) * (log_scale_sh2-log_scale_hl2)+log_scale_hl2 : log_scale_sh2;
  float norm2 = pow(log_pivot,1.0f/(exponent+1));
  result = pow(val,1.0f/(exponent+1))*log_pivot/norm2;
  */
  //result = SH_HL_mapping_pow( val, sh_compr, hl_compr, log_pivot );
  if( sh_compr > 0 || hl_compr > 0 ) {
    result = SH_HL_mapping( val, sh_compr, hl_compr, log_pivot );
  } else {
    result = val;
  }
  float delta = (val > -1.0e-15 && val < 1.0e-15) ? 1 : result / val;
  //clip( exposure*RGB[k], RGB[k] );

  /*
  std::cout<<"SH_HL_mapping_log("<<log_pivot<<"): "<<SH_HL_mapping_log(log_pivot, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  std::cout<<"SH_HL_mapping_log("<<0.1<<"): "<<SH_HL_mapping_log(0.1, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  std::cout<<"SH_HL_mapping_log("<<1<<"): "<<SH_HL_mapping_log(1, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  std::cout<<"SH_HL_mapping_log("<<2<<"): "<<SH_HL_mapping_log(2, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  std::cout<<"SH_HL_mapping_log("<<10<<"): "<<SH_HL_mapping_log(10, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  std::cout<<"SH_HL_mapping_log("<<100<<"): "<<SH_HL_mapping_log(100, 100, hl_compr, log_pivot, 0.1 )<<std::endl;
  */

  float LE_linmax2 = LE_linmax;
  if( sh_compr > 0 || hl_compr > 0 ) SH_HL_mapping( LE_linmax, sh_compr, hl_compr, log_pivot, true );
  //float LE_linmax2 = LE_linmax; //SH_HL_mapping_log( LE_linmax, sh_compr, hl_compr, log_pivot, 0.1 );

  val = result;

  switch( method ) {
  case TONE_MAPPING_LIN_EXP: {
    float LE_slope2 = LE_slope * LE_gain;
    float LE_midgray = pow(0.5,2.45);
    float LE_Ymidgray = LE_midgray * LE_gain;
    float LE_Ylinmax = ( LE_linmax2 - LE_midgray ) * LE_slope2 + LE_Ymidgray;
    float LE_Srange = 1.0f - LE_Ylinmax;
    float LE_Kymax = (LE_Kmax-LE_midgray)*LE_slope + LE_midgray;
    float LE_Kexp = LE_Kmax * LE_slope / LE_Kymax;

    //std::cout<<"get_curve_value: LE_Kmax="<<LE_Kmax<<"  LE_linmax2="<<LE_linmax2<<std::endl;

    //std::cout<<"lin+exp: RGB["<<k<<"] in:  "<<RGB[k]<<std::endl;
    if( val > LE_linmax2 ) {
      // shoulder
      float X = (val - LE_linmax2) * LE_slope2 * LE_compr / LE_Srange;
      //float XD = pow(X,LE_Sslope2) * LE_Sslope /* LE_compr */ + 1;
      //result = 1.0f - LE_Srange * exp( -X / XD );
      //result = (LE_Srange - LE_Srange * exp( -X / XD )) / LE_compr + LE_Ylinmax;

      result = (LE_Srange - LE_Srange * exp( -log(X*(LE_Sslope+1.0e-10)+1)/(LE_Sslope+1.0e-10) )) / LE_compr + LE_Ylinmax;
    } else if( val < LE_Kmax ) {
      // knee
      float X = val / LE_Kmax;
      result = (X>=0) ? LE_Kymax * pow(X,LE_Kexp) : -LE_Kymax * pow(-X,LE_Kexp);
      result *= LE_gain;
      //std::cout<<"val="<<val<<"  result="<<result<<std::endl;
    } else {
      // linear part
      result = (val - LE_midgray) * LE_slope2 + LE_Ymidgray;
    }
    //result *= delta;
    //std::cout<<"lin+log: RGB["<<k<<"] out: "<<value<<std::endl;
    //std::cout<<"LIN_POW: "<<LE_midgray<<" -> "<<RGB[3]<<std::endl;
    break;
  }
  default:
    break;
  }


  return result;
}




#ifdef GTKMM_2
bool PF::ToneMappingCurveAreaV2::on_expose_event(GdkEventExpose* event)
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
bool PF::ToneMappingCurveAreaV2::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
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

  //std::cout<<"CurveArea::on_draw(): is_linear="<<is_linear<<std::endl;

  PF::ICCProfile* labprof = PF::ICCStore::Instance().get_profile(PF::PROF_TYPE_REC2020, PF::PF_TRC_PERCEPTUAL);

  // Draw curve
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector< std::pair< std::pair<float,int>, float > > vec;
  //std::cout<<"PF::CurveArea::on_expose_event(): width="<<width<<"  height="<<height<<std::endl;
  //std::cout<<"PF::CurveArea::on_expose_event(): get_curve_value(1)="<<get_curve_value(1)<<std::endl;
  //std::cout<<"SH_HL_mapping("<<1<<"): "<<SH_HL_mapping(1, sh_compr, hl_compr, pow(0.5,2.45), true )<<std::endl;
  float xmax = 2, ymax = 1, exponent = 2.45;
  for( int i = 0; i < width; i++ ) {
    float fi = i;
    fi /= (width-1);
    //fi *= xmax;
    //float fil = labprof->perceptual2linear(fi*xmax);
    //float fil = fi*xmax; //
    float fil = pow(fi*xmax, exponent);
    int index = 0;
    if( method == TONE_MAPPING_LIN_EXP ) {
      index = (fil < LE_Kmax) ? 1 : ((fil < LE_linmax) ? 2 : 3);
    }

    float yl = get_curve_value( fil );
    //float y = labprof->linear2perceptual(yl);
    //float y = yl; //
    float y = pow(yl, 1.0f/exponent);
    //std::cout<<"  fi="<<fi<<"  fil="<<fil<<"  yl="<<yl<<"  y="<<y<<std::endl;
    vec.push_back( std::make_pair( std::make_pair(fi,index), y/ymax ) );
  }

  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->set_line_width( 2 );
  for (unsigned int i=1; i<vec.size(); i++) {
    //std::cout<<"  fi="<<vec[i].first.first<<"  index="<<vec[i].first.second<<std::endl;
    switch( vec[i].first.second ) {
    case 1: cr->set_source_rgb( 0.9, 0.0, 0.0 ); break;
    case 2: cr->set_source_rgb( 0.0, 0.9, 0.0 ); break;
    case 3: cr->set_source_rgb( 0.2, 0.2, 0.9 ); break;
    default: cr->set_source_rgb( 0.9, 0.9, 0.9 ); break;
    }
    cr->move_to( double(vec[i-1].first.first)*width+x0, double(1.0f-vec[i-1].second)*height+y0 );
    cr->line_to( double(vec[i].first.first)*width+x0, double(1.0f-vec[i].second)*height+y0 );
    cr->stroke ();
  }

  return true;
}



void PF::ToneMappingCurveAreaV2::draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
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
  cr->unset_dash ();
  cr->move_to( double(0.5+x0+width/2), double(y0) );
  cr->rel_line_to (double(0), double(height) );
  cr->stroke ();
  cr->set_dash (ds, 0);
  cr->move_to( double(0.5+x0+width/4), double(y0) );
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


double LE_gain_slider_to_prop(double& val)
{
  return pow(10,val);
}

double LE_gain_prop_to_slider(double& val)
{
  return log10(val);
}



PF::ToneMappingConfigGUI_V2::ToneMappingConfigGUI_V2( PF::Layer* layer ):
          OperationConfigGUI( layer, "Tone Mapping" ),
          exposureSlider( this, "exposure", _("exposure"), 0, -10, 10, 0.1, 1 ),
          modeSelector( this, "method", "method: ", 0 ),
          log_frame( _("tone adjustment") ),
          sh_compr_slider( this, "sh_compr", _("shadows"), 1, 0, 100, 0.5, 5, 1 ),
          hl_compr_slider( this, "hl_compr", _("highlights"), 1, 0, 100, 0.5, 5, 1 ),
          log_pivot_slider( this, "log_pivot", _("pivot"), 1, 0.1, 1, 0.2, 1, 1 ),
          gamut_compression_checkbox( this, "gamut_compression", _("compress gamut"), true ),
          hue_protection_checkbox( this, "hue_protection", _("protect hues"), true ),
          filmic_A_slider( this, "filmic_A", _("shoulder strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_B_slider( this, "filmic_B", _("linear strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_C_slider( this, "filmic_C", _("linear angle"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_D_slider( this, "filmic_D", _("toe strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_E_slider( this, "filmic_E", _("toe num."), 0.5, 0, 0.1, 0.002, 0.01, 1 ),
          filmic_F_slider( this, "filmic_F", _("toe den."), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic_W_slider( this, "filmic_W", _("lin. white point"), 10, 1, 100, 2, 10, 1 ),
          filmic2_preserve_midgray_checkbox( this, "filmic2_preserve_midgray", _("preserve mid gray"), false ),
          filmic2_gamma_slider( this, "filmic2_gamma", _("gamma"), 1.0, -5, 5, 0.02, 0.1, 1 ),
          filmic2_TS_slider( this, "filmic2_TS", _("toe strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_TL_slider( this, "filmic2_TL", _("shadow contrast"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_SS_slider( this, "filmic2_SS", _("shoulder strength"), 0.5, 0, 1, 0.02, 0.1, 0.1 ),
          filmic2_SL_slider( this, "filmic2_SL", _("shoulder lenght"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          filmic2_SA_slider( this, "filmic2_SA", _("shoulder angle"), 0.5, 0, 1, 0.02, 0.1, 1 ),
          AL_Lmax_slider( this, "AL_Lmax", _("L max"), 2, 0, 10, 0.1, 1, 1 ),
          AL_b_slider( this, "AL_b", _("contrast"), 0.85, 0, 1, 0.01, 0.1, 1 ),
          AL_Tsize( this, "AL_Tsize", _("toe size"), 0, 0, 100, 0.5, 5, 100 ),
          AL_Tlength( this, "AL_Tlength", _("toe length"), 0, 0, 100, 0.5, 5, 1 ),
          AL_Tstrength( this, "AL_Tstrength", _("toe strength"), 0, 0, 100, 1, 5, 100 ),
          LP_compression( this, "LP_compression", _("compression"), 0, 0, 100, 0.5, 2, 1 ),
          LP_slope( this, "LP_slope", _("slope"), 0, 1, 2, 0.05, 0.2, 1 ),
          LP_lin_max( this, "LP_lin_max", _("linear range"), 0, 10, 100, 1, 5, 100 ),
          LP_knee_strength( this, "LP_knee_strength", _("knee strength"), 0, 1, 2, 0.05, 0.2, 1 ),
          LP_shoulder_smoothness( this, "LP_shoulder_smoothness", _("shoulder smoothness"), 0, 0, 100, 1, 5, 100 ),
          LE_frame( _("contrast curve") ),
          LE_gain( this, "LE_gain", _("mid tones"), 0, -1, 1, 0.05, 0.2, 1 ),
          LE_compression( this, "LE_compression", _("clip point"), 0, 0, 100, 1, 5, 100 ),
          LE_slope( this, "LE_slope", _("slope"), 0, 1, 10, 0.05, 0.2, 1 ),
          LE_lin_max( this, "LE_lin_max", _("linear range"), 0, 10, 100, 1, 5, 100 ),
          LE_knee_strength( this, "LE_knee_strength", _("knee strength"), 0, 1, 2, 0.05, 0.2, 1 ),
          LE_shoulder_slope( this, "LE_shoulder_slope", _("shoulder slope"), 0, 0, 100, 1, 5, 10 ),
          LE_shoulder_slope2( this, "LE_shoulder_slope2", _("shoulder slope 2"), 0, 0, 100, 1, 5, 100 ),
          HD_slope( this, "HD_slope", _("slope"), 0, 0.1, 2, 0.05, 0.2, 1 ),
          HD_shoulder_range( this, "HD_shoulder_range", _("highlights compression"), 0, 0, 100, 1, 5, 100 ),
          gamut_compression_slider( this, "gamut_compression_amount", _("gamut compression"), 1, 0, 1, 0.02, 0.1, 1 ),
          gamut_compression_exponent_slider( this, "gamut_compression_exponent", _("gamut comp exp"), 1, 1, 10, 0.2, 1, 1 ),
          lumi_blend_frac_slider( this, "lumi_blend_frac", _("hue matching"), 1, 0, 100, 1, 5, 100 ),
          shadows_frame( _("range compression") ),
          midtones_frame( _("tone curve") ),
          highlights_frame( _("tone curve") ),
          saturation_scaling_slider( this, "saturation_scaling", _("sat scaling"), 1, 0, 100, 0.5, 5, 100 ),
          hl_desaturation_slider( this, "hl_desaturation", _("hl desaturation"), 1, 0, 100, 0.5, 5, 100 ),
          local_contrast_frame( _("local contrast") ),
          local_contrast_slider( this, "local_contrast_amount", _("amount"), 1, 0, 100, 0.5, 5, 100 ),
          local_contrast_radius_slider( this, "local_contrast_radius", _("radius"), 1, 0, 100, 0.5, 5, 1 ),
          local_contrast_threshold_slider( this, "local_contrast_threshold", _("threshold"), 1, 0.5, 100, 0.5, 5, 1000 )
{
  //gammaControlsBox.pack_start( gamma_slider, Gtk::PACK_SHRINK );
  //gammaControlsBox.pack_start( gamma_pivot_slider, Gtk::PACK_SHRINK );

  filmicControlsBox.pack_start( filmic_A_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_B_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_C_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_D_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_E_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_F_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_W_slider, Gtk::PACK_SHRINK );

  filmic2ControlsBox.pack_start( filmic2_preserve_midgray_checkbox, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_gamma_slider, Gtk::PACK_SHRINK );
  //filmic2ControlsBox.pack_start( filmic2_TS_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_TL_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SS_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SL_slider, Gtk::PACK_SHRINK );
  filmic2ControlsBox.pack_start( filmic2_SA_slider, Gtk::PACK_SHRINK );

  ALControlsBox.pack_start( AL_Lmax_slider, Gtk::PACK_SHRINK );
  ALControlsBox.pack_start( AL_b_slider, Gtk::PACK_SHRINK );
  ALControlsBox.pack_start( AL_Tsize, Gtk::PACK_SHRINK );
  ALControlsBox.pack_start( AL_Tlength, Gtk::PACK_SHRINK );
  ALControlsBox.pack_start( AL_Tstrength, Gtk::PACK_SHRINK );

  LPControlsBox.pack_start( LP_slope, Gtk::PACK_SHRINK );
  LPControlsBox.pack_start( LP_lin_max, Gtk::PACK_SHRINK );
  LPControlsBox.pack_start( LP_compression, Gtk::PACK_SHRINK );
  LPControlsBox.pack_start( LP_shoulder_smoothness, Gtk::PACK_SHRINK );
  LPControlsBox.pack_start( LP_knee_strength, Gtk::PACK_SHRINK );

  //LEControlsBox.pack_start( LE_gain, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_slope, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_lin_max, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_compression, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_shoulder_slope, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_shoulder_slope2, Gtk::PACK_SHRINK );
  //LEControlsBox.pack_start( LE_knee_strength, Gtk::PACK_SHRINK );
  //LE_frame.add(LEControlsBox);

  HDControlsBox.pack_start( HD_slope, Gtk::PACK_SHRINK );
  HDControlsBox.pack_start( HD_shoulder_range, Gtk::PACK_SHRINK );

  controlsBox.pack_start( curve_area_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( exposureSlider, Gtk::PACK_SHRINK, 2 );

  LE_gain.set_conversion_functions(LE_gain_slider_to_prop, LE_gain_prop_to_slider);
  shadows_box.pack_start( sh_compr_slider, Gtk::PACK_SHRINK );
  //shadows_box.pack_start( LE_gain, Gtk::PACK_SHRINK );
  shadows_box.pack_start( hl_compr_slider, Gtk::PACK_SHRINK );
  shadows_frame.add( shadows_box );
  controlsBox.pack_start( shadows_frame, Gtk::PACK_SHRINK, 2 );

  //midtones_frame.add( midtones_box );
  //controlsBox.pack_start( midtones_frame, Gtk::PACK_SHRINK, 2 );

  highlights_box.pack_start( LE_slope, Gtk::PACK_SHRINK );
  highlights_box.pack_start( LE_knee_strength, Gtk::PACK_SHRINK );
  highlights_box.pack_start( LE_compression, Gtk::PACK_SHRINK );
  highlights_box.pack_start( LE_lin_max, Gtk::PACK_SHRINK );
  highlights_box.pack_start( LE_shoulder_slope, Gtk::PACK_SHRINK );
  highlights_frame.add( highlights_box );
  controlsBox.pack_start( highlights_frame, Gtk::PACK_SHRINK, 2 );

  //gammaControlsBox.pack_start( LE_gain, Gtk::PACK_SHRINK );
  //gammaControlsBox.pack_start( LE_compression, Gtk::PACK_SHRINK );
  //gammaControlsBox.pack_start( LE_slope, Gtk::PACK_SHRINK );
  //gammaControlsBox.pack_start( hl_compr_slider, Gtk::PACK_SHRINK );
  //gammaControlsBox.pack_start( log_pivot_slider, Gtk::PACK_SHRINK );
  //log_frame.add(gammaControlsBox);
  //controlsBox.pack_start( log_frame, Gtk::PACK_SHRINK, 2 );

  local_contrast_box.pack_start( local_contrast_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_radius_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_threshold_slider, Gtk::PACK_SHRINK );
  local_contrast_frame.add(local_contrast_box);
  controlsBox.pack_start( local_contrast_frame, Gtk::PACK_SHRINK, 2 );

  //controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( controlsBox2, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 2 );

  curve_area_box.pack_start( curve_area, Gtk::PACK_SHRINK, 10 );

  //controlsBox.pack_start( lumi_blend_frac_slider, Gtk::PACK_SHRINK );
  /controlsBox.pack_start( saturation_scaling_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( hl_desaturation_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( hue_protection_checkbox, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( gamut_compression_checkbox, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( gamut_compression_slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( gamut_compression_exponent_slider, Gtk::PACK_SHRINK );


  globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( globalBox );
}




void PF::ToneMappingConfigGUI_V2::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF:PropertyBase* prop = par->get_property( "method" );
    if( !prop )  return;
    PF::ToneMappingParV2* tmpar = dynamic_cast<PF::ToneMappingParV2*>(par);
    if( !tmpar ) return;

    //std::cout<<"PF::ToneMappingConfigGUI_V2::do_update() called."<<std::endl;

    bool need_update = false;

    curve_area.set_params( tmpar );

    OperationConfigGUI::do_update();
    return;

    if( prop->get_enum_value().first == PF::TONE_MAPPING_LIN_EXP ) {
      if( LEControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else {
      if( (gammaControlsBox.get_parent() == &controlsBox2) ||
          (filmicControlsBox.get_parent() == &controlsBox2) ||
          (filmic2ControlsBox.get_parent() == &controlsBox2) ||
          (ALControlsBox.get_parent() == &controlsBox2) ||
          (LPControlsBox.get_parent() == &controlsBox2) ||
          (LE_frame.get_parent() == &controlsBox2) ||
          (HDControlsBox.get_parent() == &controlsBox2) )
        need_update = true;
    }

    if( need_update ) {
      if( gammaControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( gammaControlsBox );
      if( filmicControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( filmicControlsBox );
      if( filmic2ControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( filmic2ControlsBox );
      if( ALControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( ALControlsBox );
      if( LPControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( LPControlsBox );
      if( LE_frame.get_parent() == &controlsBox2 )
        controlsBox2.remove( LE_frame );
      if( HDControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( HDControlsBox );

      switch( prop->get_enum_value().first ) {
      case PF::TONE_MAPPING_LIN_EXP:
        controlsBox2.pack_start( LE_frame, Gtk::PACK_SHRINK );
        LE_frame.show();
        break;
      }
    }
    controlsBox2.show_all_children();
  }

  OperationConfigGUI::do_update();
}

