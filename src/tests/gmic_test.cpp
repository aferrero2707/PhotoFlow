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

#define _(S) (S)

#include <vips/vips.h>
#include <vips/dispatch.h>

#include <limits.h>

#include <iostream>
#include <fstream>

//#include "CImg.h"
#include "../vips/gmic/gmic/src/gmic.h"
//#include "gmic.h"

#include "../base/photoflow.hh"

static char* custom_gmic_commands = 0;
static GMutex* gmic_mutex = 0;

static gmic* gmic_instances[2];


using namespace cimg_library;



int main(int argc, char** argv)
{
  gmic_list<float> images;
  gmic_list<char> images_names;

  gmic_instances[0] = new gmic( 0, "gmic_def.gmic", false, 0, 0 );
  gmic_instances[1] = new gmic( 0, "gmic_def.gmic", false, 0, 0 );

  std::string command = "";

  try {
    images.assign( (guint) 1 );

    gmic_image<float> &img = images._data[0];
    img.assign( 128, 128, 1, 3 );

    gmic_instances[0]->run( command.c_str(), images, images_names );

    /*
    g_mutex_lock( gmic_mutex );
    std::cout<<"Running G'MIC command: \""<<vipsgmic->command<<"\"  seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<std::endl;
    std::cout<<"  gmic_instance->verbosity="<<seq->gmic_instance->verbosity<<std::endl;
    char* command = strdup( vipsgmic->command );
    seq->gmic_instance->run( command, images, images_names );
    free( command );
    std::cout<<"G'MIC command: \""<<vipsgmic->command<<"\"  seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<" finished"<<std::endl;
    g_mutex_unlock( gmic_mutex );
    std::cout<<"images._width="<<images._width<<"    &images._data[0]="<<&images._data[0]<<std::endl;
    */
  }
  catch( gmic_exception e ) {
    images.assign( (guint) 0 );
    return( -1 );
  }
  images.assign( (guint) 0 );

  return 0;
}
