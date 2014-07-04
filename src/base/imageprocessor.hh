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
    IMAGE_UPDATE,
    IMAGE_REDRAW_START,
    IMAGE_REDRAW_END,
    IMAGE_REDRAW
  };
  
  struct ProcessRequestInfo
  {
    Image* image;
    View* view;
    ViewSink* sink;
    VipsRect area;
    unsigned char* buf;
    process_request_t request;
    GCond* done;
    GMutex* mutex;
  };


  class ImageProcessor: public sigc::trackable
  {
    GThread* thread;
    std::list<Image*> images;

    static ImageProcessor* instance;

    GMutex* processing_mutex;

    // Handling of requests queue
    std::queue<ProcessRequestInfo> requests;
    GCond* requests_pending;
    GMutex* requests_mutex;

  public:
    ImageProcessor();

    static ImageProcessor& Instance();

    void run();

    void submit_request( ProcessRequestInfo request );

    //void add_image( Image* img );
    //void remove_image( Image* img );
  };

}


#endif
