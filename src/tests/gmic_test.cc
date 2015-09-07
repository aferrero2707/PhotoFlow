/* Pass VIPS images through gmic
 *
 * AF, 6/10/14
 */

/*

    This file is part of VIPS.

    VIPS is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA

 */

/*

    These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

#include <limits.h>

#include <iostream>
#include <fstream>

#include <glibmm.h>

#include "../vips/gmic/gmic/src/gmic.h"

static char* custom_gmic_commands = 0;
static GMutex* gmic_mutex = 0;

static gmic* gmic_instances[2];


using namespace cimg_library;


static gpointer run_processor( gpointer data );
static void run_gmic();



enum process_request_t {
  RUN_GMIC,
  PROCESSOR_END
};

struct ProcessRequestInfo
{
  process_request_t request;
};


class Processor: public sigc::trackable
{
  GThread* thread;

  GAsyncQueue* requests;

public:
  Processor()
  {
    requests = g_async_queue_new();
  }

  void start()
  {
    thread = g_thread_new( "processor", run_processor, this );
  }
  void run()
  {
    //std::cout<<"Processor started."<<std::endl;
    bool running = true;
    while( running ) {

      // Add any further request in the queue
      while( g_async_queue_length( requests ) > 0 ) {
        ProcessRequestInfo* req = (ProcessRequestInfo*)g_async_queue_pop( requests );

        // Process the request
        switch( req->request ) {
        case RUN_GMIC:
          std::cout<<"Processor::run(): running gmic."<<std::endl;
          run_gmic();
          break;
        case PROCESSOR_END:
          running = false;
          std::cout<<"Processor::run(): processing ended."<<std::endl;
          break;
        default:
          break;
        }

        delete( req );
      }
    }
  }

  void submit_request( ProcessRequestInfo request )
  {
    ProcessRequestInfo* req_copy = new ProcessRequestInfo( request );
    g_async_queue_push( requests, req_copy );
  }

  void join()
  {
    if( thread )
      g_thread_join( thread );
    thread = NULL;
  }
};




static gpointer run_processor( gpointer data )
{
  //std::cout<<"Calling ImageProcessor::instance().run()"<<std::endl;
  Processor* processor = (Processor*)data;
  processor->run();
  //return data;
}


static void run_gmic()
{
  if( !custom_gmic_commands ) {
    std::ifstream t;
    int length;
    t.open("gmic_def.gmic");      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    custom_gmic_commands = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(custom_gmic_commands, length);       // read the whole file into the buffer
    t.close();                    // close file handle
    std::cout<<"G'MIC custom commands loaded"<<std::endl;
  }

  gmic_list<float> images;
  gmic_list<char> images_names;

  images.assign( (guint) 1 );

  gmic_image<float> &img = images._data[0];
  img.assign( 128, 128, 1, 3 );
  gmic* gmic_instance = new gmic( 0, custom_gmic_commands, false, 0, 0 );
  //gmic_instance->run( "-verbose - -tv_flow  10,30", images, images_names );
  gmic_instance->run( "-verbose + -_gimp_freaky_details 2,10,1 ", images, images_names );
  images.assign( (guint) 0 );
  delete gmic_instance;
}



int main(int argc, char** argv)
{
  Processor* processor = new Processor();
  processor->start();

  for(int i = 0; i < 10; i++) {
    std::cout<<"running gmic"<<std::endl;
    run_gmic();
  }

  std::cout<<std::endl<<"==========================="<<std::endl<<std::endl;
  //return(0);

  ProcessRequestInfo request;
  request.request = RUN_GMIC;

  for(int i = 0; i < 10; i++)
    processor->submit_request( request );


  request.request = PROCESSOR_END;
  processor->submit_request( request );


  processor->join();

  return( 0 );
}
