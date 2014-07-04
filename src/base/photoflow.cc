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

#if defined(__MINGW32__) || defined(__MINGW64__)
  #include<windows.h>
#endif

#include "photoflow.hh"

PF::PhotoFlow::PhotoFlow(): render_mode(PF_RENDER_PREVIEW), batch(true)
{
  // Create the cache directory if possible
  char fname[500];

#if defined(__MINGW32__) || defined(__MINGW64__)
  char fname2[500];
  DWORD check = GetTempPath(499, fname);
  if (0 != check) {
    sprintf( fname2,"%s\\photoflow", fname );
    int result = mkdir(fname2);
    if( (result != 0) && (errno != EEXIST) ) {
      perror("mkdir");
      std::cout<<"Cannot create "<<fname2<<"    exiting."<<std::endl;
      exit( 1 );
    }
    sprintf( fname2,"%s\\photoflow\\cache\\", fname );
    result = mkdir(fname2);
    if( (result != 0) && (errno != EEXIST) ) {
      perror("mkdir");
      std::cout<<"Cannot create "<<fname2<<"    exiting."<<std::endl;
      exit( 1 );
    }
    cache_dir = fname2;
  }
#else
  if( getenv("HOME") ) {
    sprintf( fname,"%s/.photoflow", getenv("HOME") );
    int result = mkdir(fname, 0755);
    if( (result == 0) || (errno == EEXIST) ) {
      sprintf( fname,"%s/.photoflow/cache/", getenv("HOME") );
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
    cache_dir = fname;
  }
#endif
}



PF::PhotoFlow* PF::PhotoFlow::instance = NULL;

PF::PhotoFlow& PF::PhotoFlow::Instance() { 
  if(!PF::PhotoFlow::instance) 
    PF::PhotoFlow::instance = new PF::PhotoFlow();
  return( *instance );
};




void PF::pf_object_unref(GObject* object, const char* msg)
{
#ifdef PF_VERBOSE_UNREF
  std::cout<<"pf_object_unref(): "<<msg<<std::endl;
  std::cout<<"                   object="<<object<<std::endl;
#endif
  if( !object ) {
#ifdef PF_VERBOSE_UNREF
    std::cout<<"                   NULL object!!!"<<std::endl;
#endif
    return;
  }
#ifdef PF_VERBOSE_UNREF
  std::cout<<"                   ref_count before: "<<object->ref_count<<std::endl;
#endif
  g_assert( object->ref_count > 0 );
  g_object_unref( object );
#ifdef PF_VERBOSE_UNREF
  std::cout<<"                   ref_count after:  "<<object->ref_count<<std::endl;
#endif
}
