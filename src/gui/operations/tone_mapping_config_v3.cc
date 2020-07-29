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

#include "tone_mapping_config_v3.hh"



#define CURVE_SIZE 300


PF::ToneMappingCurveAreaV3::ToneMappingCurveAreaV3(): border_size( 0 ), is_linear( false ), exponent(1/*2.45*/), mem_array(NULL)
{
  hist = new unsigned long int[65536*3];
  for( int i = 0; i < 65536*3; i++ ) {
    hist[i] = 0;
  }
  set_size_request(CURVE_SIZE*1,CURVE_SIZE*1);
}


void PF::ToneMappingCurveAreaV3::set_op(PF::ToneMappingParV3* p)
{
  bool redraw = true;
  //    (true || tmpar->get_LE_gain() != LE_gain ) ? true : false;

  opar = p;

  //if( redraw )
  queue_draw();
}


void PF::ToneMappingCurveAreaV3::update_histogram()
{
  unsigned int npx = array_sz/sizeof(float);

  for( int j = 0; j < 65536*3; j++ ) {
    hist[j] = 0;
  }

  float* p = mem_array;
  hist_min = 0; hist_max = pow((opar ? opar->get_tm().whitePoint : 2), 1.0f / exponent);

  //std::cout<<"[Histogram::update_histogram] min="<<hist_min<<" max="<<hist_max<<std::endl;

  ulong_p h1 = hist;
  ulong_p h2 = &(hist[65536]);
  ulong_p h3 = &(hist[65536*2]);
  p = mem_array;
  float val;
  unsigned short int idx;
  for( unsigned int i = 0; i < npx; i+=image->Bands ) {
    val = p[0];
    if( (val > hist_min) && (val < hist_max) ) {
      idx = static_cast<unsigned short int>( (val-hist_min) * 65535 / (hist_max-hist_min) );
      h1[idx] += 1;
    }
    if(image->Bands > 1) {
      val = p[1];
      if( (val > hist_min) && (val < hist_max) ) {
        idx = static_cast<unsigned short int>( (val-hist_min) * 65535 / (hist_max-hist_min) );
        h2[idx] += 1;
      }
    }
    if(image->Bands > 2) {
      val = p[2];
      if( (val > hist_min) && (val < hist_max) ) {
        idx = static_cast<unsigned short int>( (val-hist_min) * 65535 / (hist_max-hist_min) );
        h3[idx] += 1;
      }
    }

    p += image->Bands;
  }

  //std::cout<<"[Histogram::update_histogram] emitting signal_queue_draw"<<std::endl;
  queue_draw();
}



void PF::ToneMappingCurveAreaV3::set_image(VipsImage* i)
{
  image = i;
  return;

  // write image to memory buffer
  std::cout<<"[ToneMappingCurveAreaV3::set_image] filling memory array\n";
  if(mem_array) free(mem_array);
  mem_array = (float*)vips_image_write_to_memory( image, &array_sz );
  PF_UNREF( image, "Histogram::update(): image unref after vips_sink()" );
  std::cout<<"[ToneMappingCurveAreaV3::set_image] filling finished, updating histogram\n";

  update_histogram();
}



float PF::ToneMappingCurveAreaV3::get_curve_value( float val )
{
  if( !opar ) return 0;
  float result = opar->get_tm().get(val, false);
  return result;
}




#ifdef GTKMM_2
bool PF::ToneMappingCurveAreaV3::on_expose_event(GdkEventExpose* event)
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
bool PF::ToneMappingCurveAreaV3::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
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

  opar->get_tm().get(0.1845 / opar->get_tm().exposure, false);

  // Draw curve
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector< std::pair< std::pair<float,int>, float > > vec;
  //std::cout<<"PF::CurveArea::on_expose_event(): width="<<width<<"  height="<<height<<std::endl;
  //std::cout<<"PF::CurveArea::on_expose_event(): get_curve_value(1)="<<get_curve_value(1)<<std::endl;
  //std::cout<<"SH_HL_mapping("<<1<<"): "<<SH_HL_mapping(1, sh_compr, hl_compr, pow(0.5,2.45), true )<<std::endl;
  float xmax = (opar ? opar->get_tm().whitePoint : 2), ymax = 1.;
  //xmax = 0.1845;
  for( int i = width-1; i >= 0; i-- ) {
    float fi = i;
    fi /= (width-1);
    //fi *= xmax;
    //float fil = labprof->perceptual2linear(fi*xmax);
    //float fil = fi*xmax; //
    float fil = pow(fi*xmax, exponent);
    int index = 0;
    //index = (fil < LE_params.LE_Kmax) ? 1 : ((fil < LE_params.LE_linmax) ? 2 : 3);

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

  double xmg = (pow(0.1845 / opar->get_tm().exposure, 1.0f / exponent) / xmax) * width;
  double ymg = (1.0f - pow(opar->get_tm().get(0.1845 / opar->get_tm().exposure, true), 1.0f / exponent) / ymax) * height;
  // special case for the grey node
  cr->set_source_rgb(0.75, 0.5, 0.0);
  cr->arc(x0+xmg, y0+ymg, 4, 0, 2. * M_PI);
  cr->fill();
  cr->stroke();

  double xw = (pow(1, 1.0f / exponent) / xmax) * width;
  double yw = (1.0f - pow(opar->get_tm().get(1), 1.0f / exponent) / ymax) * height;
  // special case for the grey node
  cr->set_source_rgb(0.75, 0.5, 0.0);
  cr->arc(0.5+x0+xw, 0.5+y0+yw, 6, 0, 2. * M_PI);
  cr->fill();
  cr->stroke();

  return true;
}



void PF::ToneMappingCurveAreaV3::draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;

  float xmax = pow((opar ? opar->get_tm().whitePoint : 2), 1.0f / exponent), ymax = 1;
  double xmg = (pow(0.1845, 1.0f / exponent) / xmax) * width;
  double ymg = (1.0f - pow(0.1845, 1.0f / exponent)) * height;
  double xwp = (pow(1, 1.0f / exponent) / xmax) * width;



  unsigned long int* h[3];
  h[0] = hist;
  h[1] = &(hist[65536]);
  h[2] = &(hist[65536*2]);

  unsigned long int* hh[3];
  hh[0] = new unsigned long int[width];
  hh[1] = new unsigned long int[width];
  hh[2] = new unsigned long int[width];

  unsigned long int max = 0;
  for( int i = 0; i < 3; i++ ) {
    for( int j = 0; j < width; j++ ) {
      hh[i][j] = 0;
    }
    for( int j = 0; j < 65536; j++ ) {
      float nj = j; nj /= 65536; nj *= width;
      int bin = (int)nj;
      //if(j==65535) std::cout<<"j="<<j<<"  bin="<<bin<<"  width="<<width<<std::endl;
      hh[i][bin] += h[i][j];
    }
    for( int j = 0; j < width; j++ ) {
      if( hh[i][j] > max) max = hh[i][j];
    }
  }

  for( int i = 0; i < 3; i++ ) {
    if( i == 0 ) cr->set_source_rgb( 0.9, 0., 0. );
    if( i == 1 ) cr->set_source_rgb( 0., 0.9, 0. );
    if( i == 2 ) cr->set_source_rgb( 0.2, 0.6, 1. );

    float ny = 0;
    if( max > 0 ) {
      ny = hh[i][0]; ny *= height; ny /= max;
    }
    float y = height; y -= ny; y -= 1;
    cr->move_to( x0, y+y0 );
    for( int j = 1; j < width; j++ ) {
      ny = 0;
      if( max > 0 ) {
        ny = hh[i][j]; ny *= height; ny /= max;
      }
      y = height; y -= ny; y -= 1;
      //y = (max>0) ? height - hh[i][j]*height/max - 1 : height-1;
      //std::cout<<"bin #"<<j<<": "<<hh[i][j]<<" ("<<max<<") -> "<<y<<std::endl;
      cr->line_to( j+x0, y+y0 );
    }
    cr->stroke();
  }


  // Draw grid
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  std::vector<double> ds (2);
  ds[0] = 4;
  ds[1] = 4;
  cr->unset_dash ();
  //cr->move_to( double(0.5+x0+width/2), double(y0) );
  //cr->move_to( double(0.5+x0+xmg), double(y0) );
  //cr->rel_line_to (double(0), double(height) );
  //cr->stroke ();
  //cr->move_to( double(0.5+x0+xwp), double(y0) );
  //cr->rel_line_to (double(0), double(height) );
  //cr->stroke ();


  /*
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
  */
}


static double contrast_slider_to_prop(double& val, PF::OperationConfigGUI*, void*)
{
  return pow(10,val);
}

static double cotrast_prop_to_slider(double& val, PF::OperationConfigGUI*, void*)
{
  //std::cout<<"[cotrast_prop_to_slider] val="<<val<<"  out="<<log(val)/log(10)<<std::endl;
  return log(val)/log(10);
}


static double white_point_slider_to_prop(double& val, PF::OperationConfigGUI*, void*)
{
  return pow(2,val);
}

static double white_point_prop_to_slider(double& val, PF::OperationConfigGUI*, void*)
{
  return log(val)/log(2);
}


PF::ToneMappingConfigGUI_V3::ToneMappingConfigGUI_V3( PF::Layer* layer ):
          OperationConfigGUI( layer, "Tone Mapping" ),
          hue_protection_checkbox( this, "hue_protection", _("protect hues"), true ),
          LE_frame( _("contrast curve") ),
          exposure_slider( this, "exposure", _("exposure (EV)"), 0, 0, 10, 0.1, 0.5, 1 ),
          latitude_slider( this, "latitude", _("latitude"), 0, 0, 100, 1, 5, 100 ),
          contrast_slider( this, "slope", _("mid-tones slope"), 0, -100, 100, 5, 20, 100 ),
          slope2_slider( this, "slope2", _("highlights slope"), 0, -100, 100, 5, 20, 100 ),
          slope3_slider( this, "slope3", _("shadows slope"), 0, -100, 100, 5, 20, 100 ),
          white_point_slider( this, "white_point", _("white level (EV)"), 0, 0, 10, 0.1, 0.5, 1 ),
          lumi_blend_frac_slider( this, "lumi_blend_frac", _("lumi blend"), 1, 0, 100, 1, 5, 100 ),
          tc_frame( _("tone curve") ),
          contrast_frame( _("contrast") ),
          color_frame( _("color") ),
          saturation_scaling_slider( this, "saturation_scaling", _("sat scaling"), 1, 0, 100, 0.5, 5, 100 ),
          sh_desaturation_slider( this, "sh_desaturation", _("shadows desaturation"), 1, 0, 100, 0.5, 5, 100, 250, 3 ),
          hl_desaturation_slider( this, "hl_desaturation", _("highlights desaturation"), 1, 0, 100, 0.5, 5, 100, 250, 3 ),
          local_contrast_frame( _("local contrast") ),
          lc_enable_checkbox( this, "local_contrast_enable", _("local contrast"), true ),
          local_contrast_slider( this, "local_contrast_amount", _("amount"), 1, 0, 100, 0.5, 5, 100 ),
          local_contrast_radius_slider( this, "local_contrast_radius", _("radius"), 1, 1, 256, 0.5, 5, 1 ),
          local_contrast_threshold_slider( this, "local_contrast_threshold", _("threshold"), 1, 0.5, 100, 0.5, 5, 200 ),
          lcCurveEditor( this, "lc_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE/2 )
{
  controlsBox.set_spacing(4);
  controlsBox.pack_start( notebook, Gtk::PACK_SHRINK, 0 );
  //controlsBox.pack_start( curve_area_box, Gtk::PACK_SHRINK, 0 );
  //controlsBox.pack_start( lcCurveEditor, Gtk::PACK_SHRINK, 0 );


  lcCurveEditor.set_points_move_vertically(true);

  //LE_gain.set_conversion_functions(LE_gain_slider_to_prop, LE_gain_prop_to_slider);
  //shadows_box.pack_start( LE_gain, Gtk::PACK_SHRINK );

  tc_box.set_spacing(4);
  tc_box.pack_start( curve_area_box, Gtk::PACK_SHRINK, 0 );
  white_point_slider.set_conversion_functions(white_point_slider_to_prop, white_point_prop_to_slider);
  tc_box.pack_start( white_point_slider, Gtk::PACK_SHRINK );
  exposure_slider.set_conversion_functions(white_point_slider_to_prop, white_point_prop_to_slider);
  tc_box.pack_start( exposure_slider, Gtk::PACK_SHRINK );
  tc_box.pack_start( latitude_slider, Gtk::PACK_SHRINK );

  slope2_slider.set_conversion_functions(contrast_slider_to_prop, cotrast_prop_to_slider);
  tc_box.pack_start( slope2_slider, Gtk::PACK_SHRINK );
  contrast_slider.set_conversion_functions(contrast_slider_to_prop, cotrast_prop_to_slider);
  tc_box.pack_start( contrast_slider, Gtk::PACK_SHRINK );
  slope3_slider.set_conversion_functions(contrast_slider_to_prop, cotrast_prop_to_slider);
  tc_box.pack_start( slope3_slider, Gtk::PACK_SHRINK );

  contrast_frame.add(contrast_box);
  //tc_box.pack_start( contrast_frame, Gtk::PACK_SHRINK );
  //tc_frame.add( tc_box );
  //controlsBox.pack_start( tc_box, Gtk::PACK_SHRINK, 0 );
  notebook.append_page(tc_box, _("Tone mapping"));

  //controlsBox.pack_start( color_frame, Gtk::PACK_SHRINK, 2 );
  color_box.set_spacing(4);
  color_box.pack_start( hl_desaturation_slider, Gtk::PACK_SHRINK, 0 );
  color_box.pack_start( hue_protection_checkbox, Gtk::PACK_SHRINK, 0 );
  notebook.append_page(color_box, _("Color"));
  //color_frame.add(color_box);

  local_contrast_box.pack_start( lcCurveEditor, Gtk::PACK_SHRINK, 0 );
  local_contrast_box.pack_start( local_contrast_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_radius_slider, Gtk::PACK_SHRINK );
  local_contrast_box.pack_start( local_contrast_threshold_slider, Gtk::PACK_SHRINK );
  //notebook.append_page(local_contrast_box, _("Local contrast"));

  //controlsBox.pack_start( local_contrast_frame, Gtk::PACK_SHRINK, 2 );

  controlsBox.pack_start( controlsBox2, Gtk::PACK_SHRINK, 0 );
  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 0 );

  curve_area_box.pack_start( curve_area, Gtk::PACK_EXPAND_WIDGET, 0 );

  //controlsBox.pack_start( lumi_blend_frac_slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( saturation_scaling_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( sh_desaturation_slider, Gtk::PACK_SHRINK, 0 );
  //controlsBox.pack_start( hue_protection_checkbox, Gtk::PACK_SHRINK, 0 );


  //globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( controlsBox );
}



void PF::ToneMappingConfigGUI_V3::switch_to_custom()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::ToneMappingParV3* tmpar = dynamic_cast<PF::ToneMappingParV3*>(par);
    if( !tmpar ) return;

    //std::cout<<"PF::ToneMappingConfigGUI_V3::do_update() called."<<std::endl;
  }
}




void PF::ToneMappingConfigGUI_V3::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::ToneMappingParV3* tmpar = dynamic_cast<PF::ToneMappingParV3*>(par);
    if( !tmpar ) return;

    std::cout<<"PF::ToneMappingConfigGUI_V3::do_update() called."<<std::endl;
    curve_area.set_op( tmpar );

    // Get the layer associated to this operation
    PF::Layer* l = get_layer();
    // Get the image the layer belongs to
    PF::Image* img = l->get_image();
    if( img ) {

      // Get the default pipeline of the image
      // (it is supposed to be at 1:1 zoom level
      // and floating point accuracy)
      PF::Pipeline* pipeline = img->get_pipeline(HISTOGRAM_PIPELINE_ID);
      if( pipeline ) {

        // Get the node associated to the layer
        PF::PipelineNode* node = pipeline->get_node( l->get_id() );
        if( node ) {

          // Finally, get the underlying VIPS image associated to the layer
          VipsImage* image = node->image;
          if( image ) {
            curve_area.set_image(image);
          }
        }
      }
    }
  }

  OperationConfigGUI::do_update();
}

