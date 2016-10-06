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

#ifndef PF_IMAGE_PROCESSOR_HH
#define PF_IMAGE_PROCESSOR_HH

#include <list>
#include <queue>

#include "image.hh"


namespace PF
{
  
  enum process_request_t {
    IMAGE_REBUILD,
    IMAGE_PIPELINE_SET_LEVEL,
    IMAGE_UPDATE,
    IMAGE_EXPORT,
    IMAGE_SAMPLE,
    IMAGE_REDRAW_START,
    IMAGE_REDRAW_END,
    IMAGE_REDRAW,
    IMAGE_MOVE_LAYER,
    IMAGE_REMOVE_LAYER,
    IMAGE_DESTROY,
    OBJECT_UNREF,
    PROCESSOR_END
  };
  
  struct ProcessRequestInfo
  {
    GObject* obj;
    Image* image;
    Pipeline* pipeline;
    PipelineSink* sink;
    std::string filename;
    Layer* layer;
    int layer_id;
    int dnd_dest_layer_id;
    std::list<Layer*>* dnd_dest_layer_list;
		int level;
    VipsRect area;
    unsigned char* buf;
    process_request_t request;
    GCond* done;
    GMutex* mutex;
  };


  class ImageProcessor: public sigc::trackable
  {
#if defined(__APPLE__)
    pthread_t _thread;
#else
    GThread* thread;
#endif
    std::list<Image*> images;

    static ImageProcessor* instance;

    GMutex* processing_mutex;

    // Handling of requests queue
    std::deque<ProcessRequestInfo> optimized_requests;

    bool caching_completed;
    GCond* caching_completed_cond;
    GMutex* caching_completed_mutex;

    GAsyncQueue* requests;

    void optimize_requests();

  public:
    ImageProcessor();

    static ImageProcessor& Instance();

    sigc::signal<void> signal_status_ready, signal_status_caching;
    sigc::signal<void> signal_status_processing, signal_status_exporting;
    sigc::signal<void> signal_status_updating;

    void start();
    void run();

    void wait_for_caching();

    void submit_request( ProcessRequestInfo request );

		void join()
		{
#if defined(__APPLE__)
		  pthread_join(_thread, NULL);
#else
      if( thread )
        g_thread_join( thread );
			thread = NULL;
#endif
		}

    //void add_image( Image* img );
    //void remove_image( Image* img );
  };

}


#endif
