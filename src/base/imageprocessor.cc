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

#include "imageprocessor.hh"


static gpointer run_image_processor( gpointer data )
{
  PF::ImageProcessor::Instance().run();
}


PF::ImageProcessor::ImageProcessor()
{
  processing_mutex = vips_g_mutex_new();
  requests_mutex = vips_g_mutex_new();
  requests_pending = vips_g_cond_new();
  thread = vips_g_thread_new( "image_processor", run_image_processor, NULL );
}


void PF::ImageProcessor::run()
{
  while( true ) {
    g_mutex_lock( requests_mutex );
    if( requests.empty() ) {
      //std::cout<<"PF::ImageProcessor::run(): waiting for new requests..."<<std::endl;
      g_cond_wait( requests_pending, requests_mutex );
      //std::cout<<"PF::ImageProcessor::run(): resuming."<<std::endl;
      if( requests.empty() ) {
	std::cout<<"PF::ImageProcessor::run(): WARNING: empty requests queue after resuming!!!"<<std::endl;
	g_mutex_unlock( requests_mutex );
	continue;
      }
    }
    PF::ProcessRequestInfo request = requests.front();
    requests.pop();
    /*
    std::cout<<"PF::ImageProcessor::run(): processing new request: ";
    switch( request.request ) {
    case IMAGE_REBUILD: std::cout<<"IMAGE_REBUILD"; break;
    case IMAGE_REDRAW_START: std::cout<<"IMAGE_REDRAW_START"; break;
    case IMAGE_REDRAW_END: std::cout<<"IMAGE_REDRAW_END"; break;
    case IMAGE_REDRAW: std::cout<<"IMAGE_REDRAW"; break;
    default: break;
    }
    std::cout<<std::endl;
    */
    g_mutex_unlock( requests_mutex );


    // Process the request
    switch( request.request ) {
    case IMAGE_REBUILD:
      if( !request.image ) continue;
      //std::cout<<"PF::ImageProcessor::run(): updating image."<<std::endl;
      request.image->lock();
      request.image->do_update();
      request.image->unlock();
      request.image->rebuild_done_signal();
      //std::cout<<"PF::ImageProcessor::run(): updating image done."<<std::endl;
      break;
    case IMAGE_UPDATE:
      if( !request.view ) continue;
      std::cout<<"PF::ImageProcessor::run(): updating area."<<std::endl;
      request.view->update( request.area );
      std::cout<<"PF::ImageProcessor::run(): updating area done."<<std::endl;
      break;
    case IMAGE_REDRAW_START:
      request.sink->process_start( request.area );
      break;
    case IMAGE_REDRAW_END:
      request.sink->process_end( request.area );
      break;
    case IMAGE_REDRAW:
      // Get exclusive access to the request data structure
      //g_mutex_lock( request.mutex );
      if( !request.sink ) continue;

      //std::cout<<"PF::ImageProcessor::run(): processing area "
      //	       <<request.area.width<<","<<request.area.height
      //       <<"+"<<request.area.left<<"+"<<request.area.top
      //       <<std::endl;
      // Process the requested image portion
      request.sink->process_area( request.area );
      //std::cout<<"PF::ImageProcessor::run(): processing area done."<<std::endl;

      // Notify that the processing is finished
      //g_cond_signal( request.done );
      //g_mutex_unlock( request.mutex );
      break;
    default:
      break;
    }
  }
}


void  PF::ImageProcessor::submit_request( PF::ProcessRequestInfo request )
{
  //std::cout<<"PF::ImageProcessor::submit_request(): locking mutex."<<std::endl;
  g_mutex_lock( requests_mutex );
  //std::cout<<"PF::ImageProcessor::submit_request(): pushing request."<<std::endl;
  requests.push( request );
  //std::cout<<"PF::ImageProcessor::submit_request(): unlocking mutex."<<std::endl;
  g_mutex_unlock( requests_mutex );
  //std::cout<<"PF::ImageProcessor::submit_request(): signaling condition."<<std::endl;
  g_cond_signal( requests_pending );
}


PF::ImageProcessor* PF::ImageProcessor::instance = NULL;

PF::ImageProcessor& PF::ImageProcessor::Instance() { 
  if(!PF::ImageProcessor::instance) 
    PF::ImageProcessor::instance = new PF::ImageProcessor();
  return( *instance );
};

