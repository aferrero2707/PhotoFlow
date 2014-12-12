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
#include "../../operations/clone_stamp.hh"

#include "clone_stamp_config.hh"


PF::CloneStampConfigDialog::CloneStampConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "CloneStamp" ),
  stamp_size( this, "stamp_size", "Stamp size: ", 5, 0, 1000000, 1, 10, 1),
  stamp_opacity( this, "stamp_opacity", "Stamp opacity: ", 100, 0, 100, 0.1, 1, 100),
  stamp_smoothness( this, "stamp_smoothness", "Stamp smoothness: ", 100, 0, 100, 0.1, 1, 100),
  srcpt_row( 0 ), srcpt_col( 0 ), srcpt_ready( false ), srcpt_changed( false )
{
  controlsBox.pack_start( stamp_size, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( stamp_opacity, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( stamp_smoothness, Gtk::PACK_SHRINK );

  add_widget( controlsBox );
}



void PF::CloneStampConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
  }
  OperationConfigDialog::open();
}


void PF::CloneStampConfigDialog::start_stroke( double x, double y )
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
  
  PF::CloneStampPar* par = dynamic_cast<PF::CloneStampPar*>( processor->get_par() );
  if( !par ) return;

  // The source point needs to be set before we can do anything...
  if( !srcpt_ready ) return;

  std::cout<<"CloneStampConfigDialog::start_stroke(): srcpt_changed="<<srcpt_changed<<std::endl;
  if( srcpt_changed ) {
    // A new source point was defined, so we need to start a new strokes group
    par->new_group( (int)(y-srcpt_row), (int)(x-srcpt_col) );
  }

  //par->start_stroke( get_pen_size(), get_pen_opacity() );
  par->start_stroke();

  for( unsigned int vi = 0; vi < image->get_npipelines(); vi++ ) {
    PF::Pipeline* pipeline = image->get_pipeline( vi );
    if( !pipeline ) continue;

    PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
    if( !node ) continue;

    processor = node->processor;
    if( !processor || !(processor->get_par()) ) continue;

    par = dynamic_cast<PF::CloneStampPar*>( processor->get_par() );
    if( !par ) continue;

    if( srcpt_changed ) {
      // A new source point was defined, so we need to start a new strokes group
      par->new_group( (int)(y-srcpt_row), (int)(x-srcpt_col) );
    }

    //par->start_stroke( get_pen_size(), get_pen_opacity() );
    par->start_stroke();
  }
  srcpt_changed = false;
}


void PF::CloneStampConfigDialog::draw_point( double x, double y )
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;
  
  PF::CloneStampPar* par = dynamic_cast<PF::CloneStampPar*>( processor->get_par() );
  if( !par ) return;
  
  // The source point needs to be set before we can do anything...
  if( !srcpt_ready ) return;

  VipsRect update;
  par->draw_point( x, y, update );

  if( (update.width < 1) || (update.height < 1) )  
    return;

  std::cout<<"PF::CloneStampConfigDialog::draw_point(): area="<<update.width<<","<<update.height
           <<"+"<<update.left<<"+"<<update.top<<std::endl;

  // Then we loop over all the operations associated to the 
  // layer in the different pipelines and we let them record the stroke as well
  PF::Image* image = layer->get_image();
  if( !image ) return;

#ifndef NDEBUG
	std::cout<<"PF::CloneStampConfigDialog::draw_point(): npipelines="<<image->get_npipelines()<<std::endl;
#endif
  for( unsigned int vi = 0; vi < image->get_npipelines(); vi++ ) {
    PF::Pipeline* pipeline = image->get_pipeline( vi );
    if( !pipeline ) continue;

    PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
    if( !node ) continue;

    processor = node->processor;
    if( !processor || !(processor->get_par()) ) continue;

    par = dynamic_cast<PF::CloneStampPar*>( processor->get_par() );
    if( !par ) continue;

    par->draw_point( x, y, update );
		//continue;

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
				std::cout<<"PF::CloneStampConfigDialog::draw_point(): submitting image update request with area."<<std::endl;
#endif
				PF::ImageProcessor::Instance().submit_request( request );
      }
    }
		/**/
  }
	//exit(1);

	/*
	std::cout<<"CloneStampConfigDialog::draw_point("<<x<<","<<y<<"): area = "
					 <<update.width<<","<<update.height<<"+"<<update.left<<","<<update.top<<std::endl;
	image->update( &update );
	//image->update_all();
	*/
	return;
}


bool PF::CloneStampConfigDialog::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  if( (mod_key & PF::MOD_KEY_CTRL) != 0 ) return false;
  start_stroke( x, y );
  draw_point( x, y );
  return false;
}


bool PF::CloneStampConfigDialog::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  if( (mod_key & PF::MOD_KEY_CTRL) != 0 ) {
    srcpt_row = y;
    srcpt_col = x;
    srcpt_ready = true;
    srcpt_changed = true;
  } else {
    //draw_point( x, y );
  }
  return false;
}


bool PF::CloneStampConfigDialog::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  if( (mod_key & PF::MOD_KEY_CTRL) != 0 ) return false;
#ifndef NDEBUG
  std::cout<<"PF::CloneStampConfigDialog::pointer_motion_event() called."<<std::endl;
#endif
  draw_point( x, y );
  return false;
}


