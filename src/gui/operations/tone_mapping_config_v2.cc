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
  set_size_request(250,150);
}


void PF::ToneMappingCurveAreaV2::set_params(PF::ToneMappingParV2* tmpar)
{
  bool redraw = true;
  //    (true || tmpar->get_LE_gain() != LE_gain ) ? true : false;


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

  //if( redraw )
    queue_draw();
}



float PF::ToneMappingCurveAreaV2::get_curve_value( float val )
{
  float result = val;
  float val0 = val;

  float LE_linmax2 = LE_linmax;
  val = result;

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
    index = (fil < LE_Kmax) ? 1 : ((fil < LE_linmax) ? 2 : 3);

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


double LE_max_slider_to_prop(double& val)
{
  return pow(10,val/10);
}

double LE_max_prop_to_slider(double& val)
{
  return log10(val)*10;
}



PF::ToneMappingConfigGUI_V2::ToneMappingConfigGUI_V2( PF::Layer* layer ):
          OperationConfigGUI( layer, "Tone Mapping" ),
          hue_protection_checkbox( this, "hue_protection", _("protect hues"), true ),
          LE_frame( _("contrast curve") ),
          LE_gain( this, "LE_gain", _("mid tones"), 0, -1, 1, 0.05, 0.2, 1 ),
          LE_compression( this, "LE_compression", _("clip point"), 0, 0, 100, 1, 5, 100 ),
          LE_slope( this, "LE_slope", _("slope"), 0, 1, 10, 0.05, 0.2, 1 ),
          LE_lin_max( this, "LE_lin_max", _("linear range"), 0, 10, 100, 1, 5, 100 ),
          LE_knee_strength( this, "LE_knee_strength", _("knee strength"), 0, 1, 2, 0.05, 0.2, 1 ),
          LE_shoulder_slope( this, "LE_shoulder_slope", _("shoulder slope"), 0, 1, 100, 1, 5, 10 ),
          LE_shoulder_slope2( this, "LE_shoulder_slope2", _("shoulder slope 2"), 0, 0, 100, 1, 5, 100 ),
          LE_shoulder_max( this, "LE_shoulder_max", _("white level"), 0, 0, 100, 0.5, 5, 1 ),
          lumi_blend_frac_slider( this, "lumi_blend_frac", _("hue matching"), 1, 0, 100, 1, 5, 100 ),
          tc_frame( _("tone curve") ),
          saturation_scaling_slider( this, "saturation_scaling", _("sat scaling"), 1, 0, 100, 0.5, 5, 100 ),
          hl_desaturation_slider( this, "hl_desaturation", _("hl desaturation"), 1, 0, 100, 0.5, 5, 100 ),
          local_contrast_frame( _("local contrast") ),
          local_contrast_slider( this, "local_contrast_amount", _("amount"), 1, 0, 100, 0.5, 5, 100 ),
          local_contrast_radius_slider( this, "local_contrast_radius", _("radius"), 1, 0, 100, 0.5, 5, 10 ),
          local_contrast_threshold_slider( this, "local_contrast_threshold", _("threshold"), 1, 0.5, 100, 0.5, 5, 1000 )
{
  controlsBox.pack_start( curve_area_box, Gtk::PACK_SHRINK, 2 );

  LE_gain.set_conversion_functions(LE_gain_slider_to_prop, LE_gain_prop_to_slider);
  //shadows_box.pack_start( LE_gain, Gtk::PACK_SHRINK );

  tc_box.pack_start( LE_slope, Gtk::PACK_SHRINK );
  //tc_box.pack_start( LE_knee_strength, Gtk::PACK_SHRINK );
  //tc_box.pack_start( LE_compression, Gtk::PACK_SHRINK );
  tc_box.pack_start( LE_lin_max, Gtk::PACK_SHRINK );
  LE_shoulder_max.set_conversion_functions(LE_max_slider_to_prop, LE_max_prop_to_slider);
  tc_box.pack_start( LE_shoulder_max, Gtk::PACK_SHRINK );
  tc_box.pack_start( LE_shoulder_slope, Gtk::PACK_SHRINK );
  tc_frame.add( tc_box );
  controlsBox.pack_start( tc_frame, Gtk::PACK_SHRINK, 2 );

  local_contrast_box.pack_start( local_contrast_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_radius_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_threshold_slider, Gtk::PACK_SHRINK );
  local_contrast_frame.add(local_contrast_box);
  controlsBox.pack_start( local_contrast_frame, Gtk::PACK_SHRINK, 2 );

  controlsBox.pack_start( controlsBox2, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 2 );

  curve_area_box.pack_start( curve_area, Gtk::PACK_SHRINK, 10 );

  //controlsBox.pack_start( lumi_blend_frac_slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( saturation_scaling_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( hl_desaturation_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( hue_protection_checkbox, Gtk::PACK_SHRINK, 2 );


  globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( globalBox );
}




void PF::ToneMappingConfigGUI_V2::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::ToneMappingParV2* tmpar = dynamic_cast<PF::ToneMappingParV2*>(par);
    if( !tmpar ) return;

    //std::cout<<"PF::ToneMappingConfigGUI_V2::do_update() called."<<std::endl;

    LE_compression.init();
    curve_area.set_params( tmpar );
  }

  OperationConfigGUI::do_update();
}

