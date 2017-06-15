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
#include <stdio.h>
#ifndef WIN32
#include <execinfo.h>
#endif
#include <libgen.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <stdio.h>  /* defines FILENAME_MAX */
//#ifdef WINDOWS
#if defined(__MINGW32__) || defined(__MINGW64__)
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#else
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

#if defined(__APPLE__) && defined (__MACH__)
#include <mach-o/dyld.h>
#endif

#include <gtkmm/main.h>
#ifdef GTKMM_3
#include <gtkmm/cssprovider.h>
#endif
//#include <vips/vips>
#include <vips/vips.h>

#include "base/pf_mkstemp.hh"
#include "base/fileutils.hh"
#include "base/pf_file_loader.hh"

#include "base/photoflow.hh"
#include "base/image.hh"

#include "base/new_operation.hh"

extern int vips__leak;

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  extern GType vips_layer_get_type( void ); 
  extern GType vips_gmic_get_type( void ); 
  extern GType vips_clone_stamp_get_type( void );
  extern GType vips_lensfun_get_type( void );
#ifdef __cplusplus
}
#endif /*__cplusplus*/


bool replace_string(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int PF::PhotoFlow::run_batch(int argc, char *argv[])
{
/*
  if (argc != 2) {
  printf ("usage: %s <filename>", argv[0]);
  exit(1);
  }
*/

  if (vips_init (argv[0]))
    //vips::verror ();
    return 1;

  vips_layer_get_type();
  vips_gmic_get_type();
  vips_clone_stamp_get_type();
  vips_lensfun_get_type();

  //im_concurrency_set( 1 );
#ifndef NDEBUG
  vips_cache_set_trace( true );
#endif

  //vips__leak = 1;

  //im_package* result = im_load_plugin("src/pfvips.plg");
  //if(!result) verror ();
  //std::cout<<result->name<<" loaded."<<std::endl;

  char cCurrentPath[FILENAME_MAX];
  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
    return errno;
  }
  cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
  char* fullpath = realpath( cCurrentPath, NULL );
  if(!fullpath) return 1;
  //PF::PhotoFlow::Instance().set_base_dir( fullpath );
  free( fullpath );

  Glib::ustring dataPath = PF::PhotoFlow::Instance().get_data_dir();
  Glib::ustring themesPath = dataPath + "/themes";

  PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation );
  PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );
  PF::PhotoFlow::Instance().set_batch( true );

  //PF::ImageProcessor::Instance();

  if( PF::PhotoFlow::Instance().get_cache_dir().empty() ) {
    std::cout<<"FATAL: Cannot create cache dir."<<std::endl;
    return 1;
  }

  vips__leak = 1;
  struct stat buffer;   
  //#ifndef WIN32

  PF::Image* image = new PF::Image();
  if( !image ) {
    std::cout<<"Cannot create image object. Exiting."<<std::endl;
    return 1;
  }

  //argv++;
  if( argc >= 2 ) {
    std::string img_in, img_out;
    std::vector< std::string > presets;

    fullpath = realpath( argv[1], NULL );
    if(!fullpath) return 1;
    img_in = fullpath;
    char* str1 = strdup(fullpath);
    char* str2 = strdup(fullpath);
    free(fullpath);

    std::string bname = basename(str1);
    std::string dname = dirname(str2);
    free(str1);
    free(str2);
    std::cout<<"dname="<<dname<<std::endl;
    std::cout<<"bname="<<bname<<std::endl;
    std::string ext;
    if( !PF::getFileExtension( "", bname, ext ) ) {
      std::cout<<"Cannot detemine input file extension. Exiting."<<std::endl;
      return 1;
    }
    std::string iname;
    if( !PF::getFileName( "", bname, iname ) ) {
      std::cout<<"Cannot detemine input file name. Exiting."<<std::endl;
      return 1;
    }
    std::cout<<"iname="<<iname<<std::endl;
    std::cout<<"ext=  "<<ext<<std::endl;

    img_out = argv[argc-1];
    std::cout<<"img_out(1)= "<<img_out<<std::endl;
    std::string patt = "%name%";
    replace_string( img_out, patt, iname );
    std::cout<<"img_out(2)= "<<img_out<<std::endl;


    image->open( img_in );

    for( int i = 2; i < argc-1; i++ ) {
      presets.push_back( std::string(argv[i]) );
      PF::insert_pf_preset( argv[i], image, NULL, &(image->get_layer_manager().get_layers()), false );
    }

    std::string oext;
    if( !PF::getFileExtension( "", img_out, oext ) ) {
      std::cout<<"Cannot detemine input file extension. Exiting."<<std::endl;
      return 1;
    }
    if( oext == "pfi") {
      image->save( img_out );
    } else {
      image->export_merged( img_out );
    }
  }
  //Shows the window and returns when it is closed.

	//PF::ImageProcessor::Instance().join();

  delete image;

  //im_close_plugins();
  vips_shutdown();

  return 0;
	
  /*
#if defined(__MINGW32__) || defined(__MINGW64__)
	for (int i = 0; i < _getmaxstdio(); ++i) ::close (i);
#else
	rlimit rlim;
	getrlimit(RLIMIT_NOFILE, &rlim);
	for (int i = 0; i < rlim.rlim_max; ++i) ::close (i);
#endif
	std::list<std::string>::iterator fi;
	for(fi = cache_files.begin(); fi != cache_files.end(); fi++)
		unlink( fi->c_str() );
*/
  return 0;
}

