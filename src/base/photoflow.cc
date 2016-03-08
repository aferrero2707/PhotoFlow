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
#include <glibmm.h>

#include <stdio.h>  /* defines FILENAME_MAX */
//#ifdef WINDOWS
#if defined(__MINGW32__) || defined(__MINGW64__)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
  #include<windows.h>
#endif

#if defined(__APPLE__) && defined (__MACH__)
#include <mach-o/dyld.h>
#endif

#include "pf_mkstemp.hh"
#include "imageprocessor.hh"
#include "photoflow.hh"

PF::PhotoFlow::PhotoFlow(): 
  active_image( NULL ),
  batch(true),
  single_win_mode(true)
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

#ifdef WIN32
    WCHAR pathW[MAX_PATH] = {0};
    char pathA[MAX_PATH];

    if (SHGetSpecialFolderPathW(NULL, pathW, CSIDL_LOCAL_APPDATA, false)) {
      WideCharToMultiByte(CP_UTF8, 0, pathW, -1, pathA, MAX_PATH, 0, 0);
      config_dir = Glib::build_filename(Glib::ustring(pathA), "photoflow");
      int result = mkdir(config_dir.c_str(), 0755);
      if( (result == 0) || (errno == EEXIST) ) {
        config_dir = Glib::build_filename(config_dir, "config");
        result = mkdir(config_dir.c_str(), 0755);
        if( (result != 0) && (errno != EEXIST) ) {
          perror("mkdir");
          std::cout<<"Cannot create "<<config_dir<<"    exiting."<<std::endl;
          exit( 1 );
        }
    }
#else
    if( getenv("HOME") ) {
      sprintf( fname,"%s/.photoflow", getenv("HOME") );
      int result = mkdir(fname, 0755);
      if( (result == 0) || (errno == EEXIST) ) {
        sprintf( fname,"%s/.photoflow/config/", getenv("HOME") );
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
      config_dir = fname;
    }
#endif

  char exname[512] = {0};
  Glib::ustring exePath;
  // get the path where the executable is stored
#ifdef WIN32
  WCHAR exnameU[512] = {0};
  GetModuleFileNameW (NULL, exnameU, 512);
  WideCharToMultiByte(CP_UTF8,0,exnameU,-1,exname,512,0,0 );
#elif defined(__APPLE__) && defined (__MACH__)
  char path[1024];
  uint32_t size = sizeof(exname);
  if (_NSGetExecutablePath(exname, &size) == 0)
    printf("executable path is %s\n", exname);
  else
    printf("buffer too small; need size %u\n", size);
#else
  if (readlink("/proc/self/exe", exname, 512) < 0) {
    //strncpy(exname, argv[0], 512);
    std::cout<<"Cannot determine full executable name."<<std::endl;
    exname[0] = '\0';
  }
#endif
  exePath = Glib::path_get_dirname(exname);

  Glib::ustring dataPath;
#if defined(__APPLE__) && defined (__MACH__)
  char* dataPath_env = getenv("PF_DATA_DIR");
  if( dataPath_env ) {
    dataPath = Glib::ustring(dataPath_env) + "/photoflow";
  } else {
    dataPath = exePath + "/../share/photoflow";
  }
#elif defined(WIN32)
  dataPath = exePath + "\\..\\share\\photoflow";
#else
  dataPath = Glib::ustring(INSTALL_PREFIX) + "/share/photoflow";
#endif
  std::cout<<"exePath: "<<exePath<<std::endl;

  Glib::ustring localePath;
#if defined(__APPLE__) && defined (__MACH__)
  if( dataPath_env ) {
    localePath = Glib::ustring(dataPath_env) + "/locale";
  } else {
    localePath = exePath + "/../share/locale";
  }
#else
  localePath = Glib::ustring(INSTALL_PREFIX) + "/share/locale";
#endif
  std::cout<<"exePath: "<<exePath<<std::endl;

  set_base_dir( exePath );
  set_data_dir( dataPath );
  set_locale_dir( localePath );
}


PF::PhotoFlow* PF::PhotoFlow::instance = NULL;

PF::PhotoFlow& PF::PhotoFlow::Instance()
{
  if(!PF::PhotoFlow::instance) 
    PF::PhotoFlow::instance = new PF::PhotoFlow();
  return( *instance );
};


void PF::PhotoFlow::close()
{
  //PF::ImageProcessor::Instance().join();

  //im_close_plugins();
  std::cout<<"PhotoFlow::close(): calling vips shutdown"<<std::endl;
  vips_shutdown();
  std::cout<<"PhotoFlow::close(): vips shutdown done"<<std::endl;

  options.save();
/*
#if defined(__MINGW32__) || defined(__MINGW64__)
  for (int i = 0; i < _getmaxstdio(); ++i) ::close (i);
#elif defined(__APPLE__) && defined(__MACH__)
#else
  rlimit rlim;
  //getrlimit(RLIMIT_NOFILE, &rlim);
  if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
    std::cout<<"rlim.rlim_max="<<rlim.rlim_max<<std::endl;
    for (int i = 3; i < rlim.rlim_max; ++i) {
      //std::cout<<"i="<<i<<std::endl;
      //::close (i);
    }
  }
#endif
*/
  std::cout<<"PhotoFlow::close(): deleting cache files"<<std::endl;
  std::list<std::string>::iterator fi;
  for(fi = cache_files.begin(); fi != cache_files.end(); fi++) {
    std::cout<<"  deleting "<<fi->c_str()<<std::endl;
    unlink( fi->c_str() );
    std::cout<<"  "<<fi->c_str()<<" deleted"<<std::endl;
  }
  std::cout<<"PhotoFlow::close(): cache files deleted"<<std::endl;
}



void PF::PhotoFlow::obj_unref( GObject* obj, char* msg )
{
	if( PF::PhotoFlow::Instance().is_batch() ){
		PF_UNREF( obj, msg );
	} else {
		ProcessRequestInfo request;
		request.obj = obj;
		request.request = PF::OBJECT_UNREF;
		PF::ImageProcessor::Instance().submit_request( request );
	}
}
    

void PF::pf_object_ref(GObject* object, const char* msg)
{
#ifdef PF_VERBOSE_UNREF
  std::cout<<"pf_object_ref()";
	if(msg) std::cout<<": "<<msg;
	std::cout<<std::endl;
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
  g_object_ref( object );
#ifdef PF_VERBOSE_UNREF
  std::cout<<"                   ref_count after:  "<<object->ref_count<<std::endl;
#endif
}




void PF::pf_object_unref(GObject* object, const char* msg)
{
#ifdef PF_VERBOSE_UNREF
  std::cout<<"pf_object_unref()";
	if(msg) std::cout<<": "<<msg;
	std::cout<<std::endl;
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
