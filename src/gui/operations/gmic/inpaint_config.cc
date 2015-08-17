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

#include "../../../base/imageprocessor.hh"
#include "../../../operations/gmic/inpaint.hh"

#include "inpaint_config.hh"


PF::GmicInpaintConfigGUI::GmicInpaintConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Inpaint (G'MIC)"  ),
  updateButton( "Update" ),
  patch_size( this, "patch_size", "Patch size: ", 7, 1, 64, 1, 5, 1 ),
  lookup_size( this, "lookup_size", "Lookup size: ", 16, 1, 32, 1, 5, 1 ),
  lookup_factor( this, "lookup_factor", "Lookup factor: ", 0.1, 0, 1, 0.01, 0.1, 1 ),
  blend_size( this, "blend_size", "Blend size: ", 1.2, 0, 5, 0.5, 1, 1 ),
  blend_threshold( this, "blend_threshold", "Blend threshold: ", 0, 0, 1, 0.1, 0.5, 1 ),
  blend_decay( this, "blend_decay", "Blend decay: ", 0.05, 0, 0.5, 0.01, 0.1, 1 ),
  blend_scales( this, "blend_scales", "Blend scales: ", 10, 1, 20, 1, 5, 1 ),
  allow_outer_blending( this, "allow_outer_blending", "Allow outer blending: ", 1, 0, 1, 1, 1, 1 ),
  pen_size( this, "pen_size", "Pen size: ", 5, 0, 1000000, 1, 10, 1),
  display_mode_selector( this, "display_mode", "Display mode", 0)
{
  controlsBox.pack_start( display_mode_selector );
  controlsBox.pack_start( updateButton );
  controlsBox.pack_start( pen_size );
  controlsBox.pack_start( patch_size );
  controlsBox.pack_start( lookup_size );
  controlsBox.pack_start( lookup_factor );
  controlsBox.pack_start( blend_size );
  controlsBox.pack_start( blend_threshold );
  controlsBox.pack_start( blend_decay );
  controlsBox.pack_start( blend_scales );
  controlsBox.pack_start( allow_outer_blending );
  
  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicInpaintConfigGUI::on_update) );
  
  add_widget( controlsBox );
}



void PF::GmicInpaintConfigGUI::on_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicInpaintPar* par = dynamic_cast<GmicInpaintPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}


void PF::GmicInpaintConfigGUI::open()
{
  OperationConfigGUI::open();
}


void PF::GmicInpaintConfigGUI::start_stroke()
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::GmicInpaintPar* par = dynamic_cast<PF::GmicInpaintPar*>( processor->get_par() );
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

    par = dynamic_cast<PF::GmicInpaintPar*>( processor->get_par() );
    if( !par ) continue;

    //par->start_stroke( get_pen_size(), get_pen_opacity() );
    par->start_stroke();
  }
}


void PF::GmicInpaintConfigGUI::draw_point( double x, double y )
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::GmicInpaintPar* par = dynamic_cast<PF::GmicInpaintPar*>( processor->get_par() );
  if( !par ) return;
  
  VipsRect update = {0,0,0,0};
  //std::cout<<"GmicInpaintConfigGUI::draw_point( "<<x<<", "<<y<<" )"<<std::endl;
  par->draw_point( x, y, update );

  if( (update.width < 1) || (update.height < 1) )  
    return;

  // Then we loop over all the operations associated to the 
  // layer in the different pipelines and we let them record the stroke as well
  PF::Image* image = layer->get_image();
  if( !image ) return;

#ifndef NDEBUG
	std::cout<<"PF::GmicInpaintConfigGUI::draw_point(): npipelines="<<image->get_npipelines()<<std::endl;
#endif
  for( unsigned int vi = 0; vi < image->get_npipelines(); vi++ ) {
    PF::Pipeline* pipeline = image->get_pipeline( vi );
    if( !pipeline ) continue;

    PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
    if( !node ) continue;

    processor = node->processor;
    if( !processor || !(processor->get_par()) ) continue;

    par = dynamic_cast<PF::GmicInpaintPar*>( processor->get_par() );
    if( !par ) continue;

    par->draw_point( x, y, update );
		//continue;
    layer2image( update );

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
				std::cout<<"PF::GmicInpaintConfigGUI::draw_point(): submitting rebuild request with area."<<std::endl;
#endif
				PF::ImageProcessor::Instance().submit_request( request );
      }
    }
		/**/
  }
	//exit(1);

	/*
	std::cout<<"GmicInpaintConfigGUI::draw_point("<<x<<","<<y<<"): area = "
					 <<update.width<<","<<update.height<<"+"<<update.left<<","<<update.top<<std::endl;
	image->update( &update );
	//image->update_all();
	*/
	return;
}


bool PF::GmicInpaintConfigGUI::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  double lx = x, ly = y, lw = 1, lh = 1;
  screen2layer( lx, ly, lw, lh );
  start_stroke();
  draw_point( lx, ly );
  return false;
}


bool PF::GmicInpaintConfigGUI::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  //draw_point( x, y );
  return false;
}


bool PF::GmicInpaintConfigGUI::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
#ifndef NDEBUG
  std::cout<<"PF::GmicInpaintConfigGUI::pointer_motion_event() called."<<std::endl;
#endif
  double lx = x, ly = y, lw = 1, lh = 1;
  screen2layer( lx, ly, lw, lh );
  draw_point( lx, ly );
  return false;
}


