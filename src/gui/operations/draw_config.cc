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

#include "../../base/imageprocessor.hh"
#include "../../operations/draw.hh"

#include "draw_config.hh"


PF::DrawConfigGUI::DrawConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Draw" ),
  pen_color_label(_("Pen color:              ")),
  bgd_color_label(_("Background color: ")),
#ifdef GTKMM_2
  pen_color_button( Gdk::Color("white") ),
  bgd_color_button( Gdk::Color("black") ),
#endif
#ifdef GTKMM_3
  pen_color_button( Gdk::RGBA("white") ),
  bgd_color_button( Gdk::RGBA("black") ),
#endif
  bgd_transparent_checkbox( this, "bgd_transparent", _("transparent"), false ),
  pen_size( this, "pen_size", _("Pen size: "), 5, 0, 1000000, 1, 10, 1),
  pen_opacity( this, "pen_opacity", _("Pen opacity: "), 100, 0, 100, 0.1, 1, 100),
  pen_smoothness( this, "pen_smoothness", _("pen smoothness: "), 0, 0, 100, 1, 10, 100),
  undoButton(_("Undo")), inhibit( false ), stroke_started( false ), stroke_active( false )
{
  colorButtonsBox1.pack_start( bgd_color_label, Gtk::PACK_SHRINK );
  colorButtonsBox1.pack_start( bgd_color_button, Gtk::PACK_SHRINK );
  colorButtonsBox1.pack_start( bgd_transparent_checkbox, Gtk::PACK_SHRINK );
  colorButtonsBox2.pack_start( pen_color_label, Gtk::PACK_SHRINK );
  colorButtonsBox2.pack_start( pen_color_button, Gtk::PACK_SHRINK );
  //colorButtonsBox2.pack_start( pen_size, Gtk::PACK_SHRINK );
  controlsBox.pack_start( colorButtonsBox1 );
  controlsBox.pack_start( colorButtonsBox2 );
  controlsBox.pack_start( pen_size, Gtk::PACK_SHRINK );
  controlsBox.pack_start( pen_smoothness );
  penBox.pack_start( undoButton );
  controlsBox.pack_start( penBox );

  /*
  controlsBox.pack_start( pen_grey );
  controlsBox.pack_start( pen_R );
  controlsBox.pack_start( pen_G );
  controlsBox.pack_start( pen_B );
  
  controlsBox.pack_start( bgd_grey );
  controlsBox.pack_start( bgd_R );
  controlsBox.pack_start( bgd_G );
  controlsBox.pack_start( bgd_B );
  */  

  pen_color_button.signal_color_set().
    connect( sigc::mem_fun(this, &PF::DrawConfigGUI::on_pen_color_changed) );
  bgd_color_button.signal_color_set().
    connect( sigc::mem_fun(this, &PF::DrawConfigGUI::on_bgd_color_changed) );

  undoButton.signal_clicked().connect( sigc::mem_fun(this, &DrawConfigGUI::on_undo) );

  add_widget( controlsBox );
}



void PF::DrawConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;

    inhibit = true;

  #ifdef GTKMM_2
    Gdk::Color rgb;
    rgb.set_rgb_p( par->get_pen_color().get().r*1.f,
        par->get_pen_color().get().g*1.f,
        par->get_pen_color().get().b*1.f );
    pen_color_button.set_color( rgb );
    rgb.set_rgb_p( par->get_bgd_color().get().r*1.f,
        par->get_bgd_color().get().g*1.f,
        par->get_bgd_color().get().b*1.f );
    bgd_color_button.set_color( rgb );
  #endif

  #ifdef GTKMM_3
    Gdk::RGBA rgba;
    rgba.set_rgba( par->get_pen_color().get().r, par->get_pen_color().get().g,
        par->get_pen_color().get().b, 1.f );
    pen_color_button.set_rgba( rgba );
    rgba.set_rgba( par->get_bgd_color().get().r, par->get_bgd_color().get().g,
        par->get_bgd_color().get().b, 1.f );
    bgd_color_button.set_rgba( rgba );
  #endif

    pen_size.init();
    pen_opacity.init();

    inhibit = false;
  }
  OperationConfigGUI::open();
}


void PF::DrawConfigGUI::on_pen_color_changed()
{
  if( inhibit ) return;

  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;
  
#ifdef GTKMM_2
  float value = pen_color_button.get_color().get_red();
  par->get_pen_color().get().r = value/65535;
  value = pen_color_button.get_color().get_green();
  par->get_pen_color().get().g = value/65535;
  value = pen_color_button.get_color().get_blue();
  par->get_pen_color().get().b = value/65535;
#endif

#ifdef GTKMM_3
  par->get_pen_color().get().r = pen_color_button.get_rgba().get_red();
  par->get_pen_color().get().g = pen_color_button.get_rgba().get_green();
  par->get_pen_color().get().b = pen_color_button.get_rgba().get_blue();
#endif

  if( layer->get_image() )
    layer->get_image()->update();
}


void PF::DrawConfigGUI::on_bgd_color_changed()
{
  if( inhibit ) return;

  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;
  
#ifdef GTKMM_2
  float value = bgd_color_button.get_color().get_red();
  par->get_bgd_color().get().r = value/65535;
  value = bgd_color_button.get_color().get_green();
  par->get_bgd_color().get().g = value/65535;
  value = bgd_color_button.get_color().get_blue();
  par->get_bgd_color().get().b = value/65535;
#endif

#ifdef GTKMM_3
  par->get_bgd_color().get().r = bgd_color_button.get_rgba().get_red();
  par->get_bgd_color().get().g = bgd_color_button.get_rgba().get_green();
  par->get_bgd_color().get().b = bgd_color_button.get_rgba().get_blue();
#endif

  if( layer->get_image() )
    layer->get_image()->update();
	std::cout<<"DrawConfigGUI::on_bgd_color_changed(): image updated"<<std::endl;
}


void PF::DrawConfigGUI::on_undo()
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // Then we loop over all the operations associated to the
  // layer in the different pipelines and we let them record the stroke as well
  PF::Image* image = layer->get_image();
  if( !image ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;

  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;

  image->lock();
  Property< std::list< Stroke<Pencil> > >& pstrokes = par->get_strokes();
  std::list< Stroke<Pencil> >& strokes = pstrokes.get();
  if( !strokes.empty() ) {
    strokes.pop_back();
    image->update();
  }
  image->unlock();
}


void PF::DrawConfigGUI::start_stroke()
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;
  
  //par->start_stroke( get_pen_size(), get_pen_opacity() );
  par->start_stroke();


  // Then we loop over all the operations associated to the 
  // layer in the different pipelines and we let them record the stroke as well
  PF::Image* image = layer->get_image();
  if( !image ) return;

  for( unsigned int vi = 0; vi < image->get_npipelines(); vi++ ) {
    PF::Pipeline* pipeline = image->get_pipeline( vi );
    if( !pipeline ) continue;

    PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
    if( !node ) continue;

    processor = node->processor;
    if( !processor || !(processor->get_par()) ) continue;

    par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
    if( !par ) continue;

    //par->start_stroke( get_pen_size(), get_pen_opacity() );
    par->start_stroke();
  }
}


void PF::DrawConfigGUI::end_stroke()
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;

  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;

  //par->start_stroke( get_pen_size(), get_pen_opacity() );
  par->end_stroke();

  PF::Image* image = layer->get_image();
  if( !image ) return;

  image->update();
}


void PF::DrawConfigGUI::draw_point( double x, double y )
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::DrawPar* par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
  if( !par ) return;
  
  VipsRect update = {0,0,0,0};
  double lx = x, ly = y, lw = 1, lh = 1;
  screen2layer( lx, ly, lw, lh );
  //std::cout<<"DrawConfigGUI::draw_point( "<<lx<<", "<<ly<<" )  x="<<x<<" y="<<y<<std::endl;
  par->draw_point( lx, ly, update );
  //double ix = lx, iy = ly, iw = 1, ih = 1;
  //layer2image( ix, iy, iw, ih );
  //std::cout<<"DrawConfigGUI::draw_point(): ix="<<ix<<"  iy="<<iy<<std::endl;

  return;

  if( (update.width < 1) || (update.height < 1) )  
    return;

  // Then we loop over all the operations associated to the 
  // layer in the different pipelines and we let them record the stroke as well
  PF::Image* image = layer->get_image();
  if( !image ) return;

#ifndef NDEBUG
	std::cout<<"PF::DrawConfigGUI::draw_point(): npipelines="<<image->get_npipelines()<<std::endl;
#endif
  for( unsigned int vi = 0; vi < image->get_npipelines(); vi++ ) {
    PF::Pipeline* pipeline = image->get_pipeline( vi );
    if( !pipeline ) continue;

    PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
    if( !node ) continue;

    processor = node->processor;
    if( !processor || !(processor->get_par()) ) continue;

    par = dynamic_cast<PF::DrawPar*>( processor->get_par() );
    if( !par ) continue;

    par->draw_point( lx, ly, update );

    if( (int)(vi) != PF::PhotoFlow::Instance().get_preview_pipeline_id() )
      continue;

		//continue;
    //update.left -= dx;
    //update.top -= dy;
    //std::cout<<"lx="<<lx<<"  ly="<<ly<<std::endl;
    //std::cout<<"update(1): "<<update.width<<","<<update.height<<"+"<<update.left<<"+"<<update.top<<std::endl;
    layer2image( update );
    //std::cout<<"update(2): "<<update.width<<","<<update.height<<"+"<<update.left<<"+"<<update.top<<std::endl;

		/**/
    if( (update.width > 0) &&
				(update.height > 0) ) {
      if( PF::PhotoFlow::Instance().is_batch() ) {
				pipeline->sink( update );	
      } else {
				ProcessRequestInfo request;
				request.pipeline = pipeline;
				request.request = PF::IMAGE_UPDATE;
				request.area.left = update.left;
				request.area.top = update.top;
				request.area.width = update.width;
				request.area.height = update.height;
#ifndef NDEBUG
				std::cout<<"PF::DrawConfigGUI::draw_point(): submitting rebuild request with area."<<std::endl;
#endif
				PF::ImageProcessor::Instance().submit_request( request );
      }
    }
		/**/
  }
	//exit(1);

	/*
	std::cout<<"DrawConfigGUI::draw_point("<<x<<","<<y<<"): area = "
					 <<update.width<<","<<update.height<<"+"<<update.left<<","<<update.top<<std::endl;
	image->update( &update );
	//image->update_all();
	*/
	return;
}


bool PF::DrawConfigGUI::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( !get_editing_flag() ) return false;

  //std::cout<<"DrawConfigGUI::pointer_press_event() called"<<std::endl;

  if( button != 1 ) return false;
  if( mod_key != (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) ) {
    stroke_started = true;
    stroke_active = true;
    start_stroke();
  }
  draw_point( x, y );
  return true;
}


bool PF::DrawConfigGUI::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( !get_editing_flag() ) return false;

  if( button != 1 ) return false;
  if( mod_key != (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) ) {
    end_stroke();
    stroke_started = false;
    stroke_active = false;
  }
  //draw_point( x, y );
  return false;
}


bool PF::DrawConfigGUI::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( !get_editing_flag() ) return false;

  mouse_x = x; mouse_y = y;
  //std::cout<<"DrawConfigGUI::pointer_motion_event(): mouse_x="<<mouse_x<<" mouse_y="<<mouse_y<<std::endl;
  if( button != 1 ) return true;
#ifndef NDEBUG
  std::cout<<"PF::DrawConfigGUI::pointer_motion_event() called."<<std::endl;
#endif
  draw_point( x, y );
  return true;
}




bool PF::DrawConfigGUI::modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out,
                                           float scale, int xoffset, int yoffset )
{
  /*
#if defined(_WIN32) || defined(WIN32)
  if( !is_mapped() )
    return false;
#else
  if( !get_mapped() )
    return false;
#endif
*/
  if( !get_layer() ) return false;
  if( !get_layer()->get_image() ) return false;
  if( !get_layer()->get_processor() ) return false;
  if( !get_layer()->get_processor()->get_par() ) return false;

  PF::OpParBase* par = get_layer()->get_processor()->get_par();

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  //std::cout<<"DrawConfigGUI::modify_preview(): stroke_started="<<stroke_started<<" stroke_active="<<stroke_active<<std::endl;

  if( stroke_started ) {
    // Resize the temp buffer to match the input one
    buf_temp.resize( buf_in.get_rect() );
    // Copy pixel data from input to temp
    buf_temp.copy( buf_in );
    stroke_started = false;
  } else if( !stroke_active ){
    // Copy pixel data from input to output
    buf_out.copy( buf_in );
    buf_temp.resize( 0,0,1,1 );
  }

  int pensize = pen_size.get_adjustment()->get_value();//*scale;
  double tx = 0, ty = 0, tw = pensize, th = pensize;
  layer2screen( tx, ty, tw, th );
  pensize = tw;
  int pen_size2 = pensize*pensize;

  int x0 = mouse_x;//*scale + xoffset;
  int y0 = mouse_y;//*scale + yoffset;

  if( stroke_active ) {
    //buf_temp.draw_circle( x0, y0, pensize, buf_in );
    buf_temp.draw_circle( x0, y0, pensize, 255, 0, 0, true );
    buf_out.copy( buf_temp );
  } else {
    buf_out.draw_circle( x0, y0, pensize, buf_in );
  }
  return true;
}
