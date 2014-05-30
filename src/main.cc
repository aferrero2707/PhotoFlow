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

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include <gtkmm/main.h>
#include <vips/vips>
#include <vips/vips.h>
#include "gui/mainwindow.hh"

#include "base/new_operation.hh"


/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  extern GType vips_layer_get_type( void ); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);

  if (argc != 2) {
    printf ("usage: %s <filename>", argv[0]);
    exit(1);
  }

  if (vips_init (argv[0]))
    vips::verror ();

  vips_layer_get_type();

#ifndef NDEBUG
  im_concurrency_set( 1 );
  vips_cache_set_trace( true );
#endif

  //im_package* result = im_load_plugin("src/pfvips.plg");
  //if(!result) verror ();
  //std::cout<<result->name<<" loaded."<<std::endl;

  PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation_with_gui );
  PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );

  // Create the cache directory if possible
  char fname[500];
  if( getenv("HOME") ) {
    sprintf( fname,"%s/.photoflow", getenv("HOME") );
    int result = mkdir(fname, 0755);
    if( (result == 0) || (errno == EEXIST) ) {
      sprintf( fname,"%s/.photoflow/cache", getenv("HOME") );
      result = mkdir(fname, 0755);
      if( (result != 0) && (errno != EEXIST) ) {
	perror("mkdir");
	std::cout<<"Cannot create "<<fname<<"    exiting."<<std::endl;
	exit( 1 );
      }
    } else {
      perror("mkdir");
      std::cout<<"Cannot create "<<fname<<" (result="<<result<<")   exiting."<<std::endl;
      exit( 1 );
    }
  }

  PF::MainWindow mainWindow;
  char* fullpath = realpath( argv[1], NULL );
  if(!fullpath)
    return 1;
  mainWindow.open_image( fullpath );
  free(fullpath);
  //Shows the window and returns when it is closed.
  Gtk::Main::run(mainWindow);

  im_close_plugins();

  return 0;
}

