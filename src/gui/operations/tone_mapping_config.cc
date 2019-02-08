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
  exposure = 1;
  gamma = 1;
  gamma_pivot = 1;
  f2gamma = 0;
  f2midgraylock = false;
  AL_Lmax =  AL_b = 0;
  set_size_request(250,150);
}


void PF::ToneMappingCurveArea::set_params(PF::ToneMappingPar* tmpar)
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
          tmpar->get_gamma() != gamma ||
          tmpar->get_gamma_pivot() != gamma_pivot ||
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
  gamma = tmpar->get_gamma();
  gamma_pivot = pow(tmpar->get_gamma_pivot(), 2.45);
  exponent = 1.f / gamma;

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
  LP_Kstrength = tmpar->get_LP_knee_strength();
  LP_Kmax = ((1.f-LP_slope)*LP_midgray)/(LP_slope/(sqrt(2.0f)*LP_Kstrength)-LP_slope);
  LP_linmax = tmpar->get_LP_lin_max();
  if( LP_linmax < LP_Kmax ) LP_linmax = LP_Kmax + 1.0e-5;
  LP_Ssmooth = tmpar->get_LP_shoulder_smoothness();
  std::cout<<"ToneMappingCurveArea::set_params: LP_Ssmooth="<<LP_Ssmooth<<"  LP_compr="<<LP_compr<<std::endl;
  //std::cout<<"ToneMappingCurveArea::set_params: AL_Tsize="<<AL_Tsize<<"  AL_Tlength="<<AL_Tlength
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



float PF::ToneMappingCurveArea::get_curve_value( float val )
{
  float result = val;
  switch( method ) {
  case TONE_MAPPING_EXP_GAMMA:
    if( gamma != 1 ) {
        result = powf( result/gamma_pivot, exponent ) * gamma_pivot;
    }
    //std::cout<<"val="<<val<<"  gamma="<<gamma<<"  result="<<result<<std::endl;
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
    float midgray = 0.1814;
    //float xmidgray = filmic2_curve.EvalInv(midgray);
    float filmic2_scale = f2midgraylock ? filmic2_curve.EvalInv(midgray)/midgray : 1.f;
    val = powf( val, f2exponent );
    result = filmic2_curve.Eval(val*filmic2_scale);
    result = powf( result, 1.f/f2exponent );
    //std::cout<<"ToneMappingCurveArea::get_curve_value: f2exponent="<<f2exponent<<" val="<<val<<" result="<<result<<std::endl;
    break;
  }
  case TONE_MAPPING_ADAPTIVE_LOG: {
    float Lmax = pow(10.0, AL_Lmax);
    float AL_scale = 1; //pow(AL_b-0.85+1, 5);
    float Ldmax = 100;
    float b = AL_b;
    float Lwmax = Lmax/AL_scale;
    float LLwmax = log10(Lwmax+1);
    float AL_A = 0.01 * Ldmax / LLwmax;
    float E = log(AL_b)/log(0.5);
    float AL_iTlength = 1.0f/AL_Tlength;


    //std::cout<<"ToneMappingCurveArea::get_curve_value: AL_Tsize="<<AL_Tsize
    //    <<"  AL_Tshift="<<AL_Tshift
    //    <<"  AL_Tlength="<<AL_Tlength
    //<<"  AL_iTlength="<<AL_iTlength<<std::endl;
    if( val > AL_Tmax ) {
      float Lw = (AL_Tlength+1-AL_Tshift)*(val-AL_Tlength+AL_Tshift)*Lwmax;
      float LLw1p = log1p(Lw);
      float Rw = Lw/Lwmax;
      float D = log( (pow(Rw,E))*8 + 2 );
      float B = LLw1p / D;
      result = AL_Trange + (1.0f-AL_Trange)*AL_A*B;
    } else {
      float Lw = (val+AL_Tshift)*AL_iTlength;
      result = AL_Tsize * pow(Lw,AL_Texp) - AL_Tvshift;
      //std::cout<<"ToneMappingCurveArea::get_curve_value: val="<<val<<"  Lw="<<Lw<<"  out="<<result<<std::endl;
    }
    break;
  }
  case TONE_MAPPING_LIN_POW: {
    float LP_midgray = pow(0.5,2.45);
    float LP_Ylinmax = ( LP_linmax - LP_midgray ) * LP_slope + LP_midgray;
    float LP_Srange = 1.0f - LP_Ylinmax;
    float LP_Sslope = LP_Srange * LP_slope;
    float LP_Kymax = (LP_Kmax-LP_midgray)*LP_slope + LP_midgray;
    float LP_Kexp = LP_Kmax * LP_slope / LP_Kymax;
    float LP_compr2 = LP_compr*2.5f;

    //val = LP_midgray;

    if( val > LP_linmax ) {
      //float Lw = (LP_linmax - val) / LP_Srange;
      //result = LP_Srange * (1.0f - exp(Lw*LP_slope*LP_compr)) / LP_compr + LP_Ylinmax;
      float X = (val - LP_linmax) * LP_slope;
      //result = log(X*LP_compr+1) / LP_compr + LP_Ylinmax;
      float Y1 = log(X*LP_compr2+1) / LP_compr2 + LP_Ylinmax;
      float Y2 = ( 1.0f - log(X*LP_compr+1) / (X*LP_compr) ) * 2.0f / LP_compr + LP_Ylinmax;
      result = LP_Ssmooth * Y1 + (1.0f - LP_Ssmooth) * Y2;
    } else if( val < LP_Kmax ) {
      //float X = val / LP_Kmax - 1.0f;
      //result = LP_slope*LP_Kmax*( (sqrt(2.0f)*X/sqrt(1.0f+X*X) + 1.0f) / sqrt(2.0f) );
      float X = val / LP_Kmax;
      result = LP_Kymax * pow(X,LP_Kexp);
    } else {
      result = (val - LP_midgray) * LP_slope + LP_midgray;
    }
    //std::cout<<"LIN_POW: "<<val<<" -> "<<result<<std::endl;
    break;
  }
  case TONE_MAPPING_HD: {
    float HD_par[4] = { HD_lin_slope, HD_lin_pivot, HD_lin_Dmin, HD_lin_Dmax };
    result = PF::HD_filmic2(val, HD_par);
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

  //std::cout<<"CurveArea::on_draw(): is_linear="<<is_linear<<std::endl;

  PF::ICCProfile* labprof = PF::ICCStore::Instance().get_profile(PF::PROF_TYPE_REC2020, PF::PF_TRC_PERCEPTUAL);

  // Draw curve
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector< std::pair< std::pair<float,int>, float > > vec;
  //std::cout<<"PF::CurveArea::on_expose_event(): width="<<width<<"  height="<<height<<std::endl;
  float xmax = 2, ymax = 1, exponent = 1;//2.45;
  for( int i = 0; i < width; i++ ) {
    float fi = i;
    fi /= (width-1);
    float fil = labprof->perceptual2linear(fi*xmax);
    //float fil = pow(fi*xmax, exponent);
    int index = 0;
    if( method == TONE_MAPPING_FILMIC2 ) {
      float normX = fil * filmic2_curve.m_invW;
      index = (normX < filmic2_curve.m_x0) ? 1 : ((normX < filmic2_curve.m_x1) ? 2 : 3);
      //std::cout<<"x="<<fil<<"  normX="<<normX
      //    <<"  m_x0="<<filmic2_curve.m_x0<<"  m_x1="<<filmic2_curve.m_x1<<"  index="<<index<<std::endl;
    }
    if( method == TONE_MAPPING_ADAPTIVE_LOG ) {
      index = (fil <= AL_Tmax) ? 1 : 2;
      //std::cout<<"x="<<fil<<"  fi*xmax="<<fi*xmax<<"  fil="<<fil<<"  index="<<index<<std::endl;
    }
    if( method == TONE_MAPPING_LIN_POW ) {
      index = (fil < LP_Kmax) ? 1 : ((fil < LP_linmax) ? 2 : 3);
    }

    float yl = get_curve_value( fil );
    float y = labprof->linear2perceptual(yl);
    //float y = pow(yl, 1.0f/exponent);
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



PF::ToneMappingConfigGUI::ToneMappingConfigGUI( PF::Layer* layer ):
          OperationConfigGUI( layer, "Tone Mapping" ),
          exposureSlider( this, "exposure", _("exposure"), 0, -10, 10, 0.1, 1 ),
          modeSelector( this, "method", "method: ", 0 ),
          gamma_slider( this, "gamma", _("exponent"), 1, 1, 5, 0.2, 1, 1 ),
          gamma_pivot_slider( this, "gamma_pivot", _("pivot"), 1, 0.1, 10, 0.2, 1, 1 ),
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
          LP_compression( this, "LP_compression", _("compression"), 0, 0, 100, 0.5, 2, 10 ),
          LP_slope( this, "LP_slope", _("slope"), 0, 1, 2, 0.05, 0.2, 1 ),
          LP_lin_max( this, "LP_lin_max", _("linear range"), 0, 10, 100, 1, 5, 100 ),
          LP_knee_strength( this, "LP_knee_strength", _("knee strength"), 0, 1, 2, 0.05, 0.2, 1 ),
          LP_shoulder_smoothness( this, "LP_shoulder_smoothness", _("shoulder smoothness"), 0, 0, 100, 1, 5, 100 ),
          HD_slope( this, "HD_slope", _("slope"), 0, 0.1, 2, 0.05, 0.2, 1 ),
          HD_shoulder_range( this, "HD_shoulder_range", _("highlights compression"), 0, 0, 100, 1, 5, 100 ),
          gamut_compression_slider( this, "gamut_compression", _("gamut compression"), 1, 0, 1, 0.02, 0.1, 1 ),
          gamut_compression_exponent_slider( this, "gamut_compression_exponent", _("gamut comp exp"), 1, 1, 10, 0.2, 1, 1 ),
          lumi_blend_frac_slider( this, "lumi_blend_frac", _("hue matching"), 1, 0, 100, 1, 5, 100 )
{
  gammaControlsBox.pack_start( gamma_slider, Gtk::PACK_SHRINK );
  gammaControlsBox.pack_start( gamma_pivot_slider, Gtk::PACK_SHRINK );

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

  HDControlsBox.pack_start( HD_slope, Gtk::PACK_SHRINK );
  HDControlsBox.pack_start( HD_shoulder_range, Gtk::PACK_SHRINK );

  controlsBox.pack_start( curve_area_box, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( exposureSlider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( controlsBox2, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 2 );

  curve_area_box.pack_start( curve_area, Gtk::PACK_SHRINK, 10 );

  controlsBox.pack_start( lumi_blend_frac_slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( gamut_compression_slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( gamut_compression_exponent_slider, Gtk::PACK_SHRINK );


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
    } else if( prop->get_enum_value().first == PF::TONE_MAPPING_ADAPTIVE_LOG ) {
      if( ALControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else if( prop->get_enum_value().first == PF::TONE_MAPPING_LIN_POW ) {
      if( LPControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else if( prop->get_enum_value().first == PF::TONE_MAPPING_HD ) {
      if( HDControlsBox.get_parent() != &controlsBox2 )
        need_update = true;
    } else {
      if( (gammaControlsBox.get_parent() == &controlsBox2) ||
          (filmicControlsBox.get_parent() == &controlsBox2) ||
          (filmic2ControlsBox.get_parent() == &controlsBox2) ||
          (ALControlsBox.get_parent() == &controlsBox2) ||
          (LPControlsBox.get_parent() == &controlsBox2) ||
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
      if( HDControlsBox.get_parent() == &controlsBox2 )
        controlsBox2.remove( HDControlsBox );

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
      case PF::TONE_MAPPING_ADAPTIVE_LOG:
        controlsBox2.pack_start( ALControlsBox, Gtk::PACK_SHRINK );
        ALControlsBox.show();
        break;
      case PF::TONE_MAPPING_LIN_POW:
        controlsBox2.pack_start( LPControlsBox, Gtk::PACK_SHRINK );
        LPControlsBox.show();
        break;
      case PF::TONE_MAPPING_HD:
        controlsBox2.pack_start( HDControlsBox, Gtk::PACK_SHRINK );
        HDControlsBox.show();
        break;
      }
    }
    controlsBox2.show_all_children();
  }

  OperationConfigGUI::do_update();
}

