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
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
#include "print_display_profile.hh"

#include "../rt/rtengine/rtlensfun.h"

#include "../vips/gmic/gmic/src/gmic.h"


#if defined(__MINGW32__) || defined(__MINGW64__)
const char* _lf_get_database_dir()
{
  std::string dbdir;
  //if( getenv("LOCALAPPDATA") ) {
  //  dbdir = getenv("LOCALAPPDATA");
  if( getenv("PROGRAMDATA") ) {
    dbdir = getenv("PROGRAMDATA");
    dbdir += "\\lensfun\\version_1";
  }
  std::cout<<"LensFun database dir: "<<dbdir<<std::endl;
  return dbdir.c_str();
}
#endif

static void write_escaped(std::string const& s, std::string& s2) {
  //s2 += '"';
  for (std::string::const_iterator i = s.begin(), end = s.end(); i != end; ++i) {
    unsigned char c = *i;
    if (' ' <= c and c <= '~' and c != '\\' and c != '"') {
      s2 += c;
    }
    else {
      s2 += '\\';
      switch(c) {
      case '"':  s2 += '"';  break;
      case '\\': s2 += '\\'; break;
      case '\t': s2 += 't';  break;
      case '\r': s2 += 'r';  break;
      case '\n': s2 += 'n';  break;
      default:
        char const* const hexdig = "0123456789ABCDEF";
        s2 += 'x';
        s2 += hexdig[c >> 4];
        s2 += hexdig[c & 0xF];
      }
    }
  }
  //s2 += '"';
}

PF::PhotoFlow::PhotoFlow(): 
  active_image( NULL ),
  batch(true), plugin(false),
  single_win_mode(true)
{
  // Create the cache directory if possible
  char fname[500];

  PF::print_display_profile();

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

#if defined(__MINGW32__) || defined(__MINGW64__)
    WCHAR pathW[MAX_PATH] = {0};
    char pathA[MAX_PATH];

    if (SHGetSpecialFolderPathW(NULL, pathW, CSIDL_LOCAL_APPDATA, false)) {
      WideCharToMultiByte(CP_UTF8, 0, pathW, -1, pathA, MAX_PATH, 0, 0);
      presets_dir = Glib::build_filename(Glib::ustring(pathA), "photoflow");
      int result = mkdir(presets_dir.c_str());
      if( (result == 0) || (errno == EEXIST) ) {
        presets_dir = Glib::build_filename(presets_dir, "presets");
        result = mkdir(presets_dir.c_str());
        if( (result != 0) && (errno != EEXIST) ) {
          perror("mkdir");
          std::cout<<"Cannot create "<<presets_dir<<"    exiting."<<std::endl;
          exit( 1 );
        }
      }
    }
#else
    if( getenv("HOME") ) {
      sprintf( fname,"%s/.photoflow", getenv("HOME") );
      int result = mkdir(fname, 0755);
      if( (result == 0) || (errno == EEXIST) ) {
        sprintf( fname,"%s/.photoflow/presets/", getenv("HOME") );
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
      presets_dir = fname;
    }
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
    if (SHGetSpecialFolderPathW(NULL, pathW, CSIDL_LOCAL_APPDATA, false)) {
      WideCharToMultiByte(CP_UTF8, 0, pathW, -1, pathA, MAX_PATH, 0, 0);
      config_dir = Glib::build_filename(Glib::ustring(pathA), "photoflow");
      int result = mkdir(config_dir.c_str());
      if( (result == 0) || (errno == EEXIST) ) {
        config_dir = Glib::build_filename(config_dir, "config");
        result = mkdir(config_dir.c_str());
        if( (result != 0) && (errno != EEXIST) ) {
          perror("mkdir");
          std::cout<<"Cannot create "<<config_dir<<"    exiting."<<std::endl;
          exit( 1 );
        }
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
  std::cout<<"exePath: "<<exePath<<std::endl;

  Glib::ustring dataPath;
#if defined(__APPLE__) && defined (__MACH__)
  char* dataPath_env = getenv("PF_DATA_DIR");
  if( dataPath_env ) {
    dataPath = Glib::ustring(dataPath_env) + "/photoflow/";
  } else {
    dataPath = exePath + "/../share/photoflow/";
  }
#elif defined(WIN32)
  //if( getenv("LOCALAPPDATA") ) {
  //  dataPath = getenv("LOCALAPPDATA");
  if( false && getenv("PROGRAMDATA") ) {
    dataPath = getenv("PROGRAMDATA");
    dataPath += "\\photoflow\\";
    Glib::ustring testPath = dataPath + "\\gmic_def.gmic";
    struct stat stat_buf;
    if( stat(testPath.c_str(), &stat_buf) ) {
      dataPath = exePath + "\\..\\share\\photoflow\\";
    }
  } else {
    dataPath = exePath + "\\..\\share\\photoflow\\";
  }
#else
  char* dataPath_env = getenv("PF_DATA_DIR");
  if( dataPath_env ) {
    dataPath = Glib::ustring(dataPath_env) + "/photoflow/";
  } else {
    dataPath = Glib::ustring(INSTALL_PREFIX) + "/share/photoflow/";
  }
#endif
  std::cout<<"dataPath: "<<dataPath<<std::endl;

  std::string dataPathEscaped;
  write_escaped(dataPath, dataPathEscaped);
  Glib::setenv("GMIC_SYSTEM_PATH", dataPathEscaped.c_str(), 1);

  lensfun_db_dir = dataPath;
#if defined(WIN32)
  lensfun_db_dir += "\\lensfun\\version_1\\";
#else
  lensfun_db_dir += "/lensfun/version_1/";
#endif

  Glib::ustring localePath;
#if defined(__APPLE__) && defined (__MACH__)
  if( dataPath_env ) {
    localePath = Glib::ustring(dataPath_env) + "/locale";
  } else {
    localePath = exePath + "/../share/locale";
  }
#elif defined(WIN32)
  localePath = exePath + "\\..\\share\\locale\\";
#else
  if( dataPath_env ) {
    localePath = Glib::ustring(dataPath_env) + "/locale";
  } else {
    localePath = Glib::ustring(INSTALL_PREFIX) + "/share/locale";
  }
#endif
  std::cout<<"localePath: "<<localePath<<std::endl;

  gmic::init_rc();

  set_base_dir( exePath );
  set_data_dir( dataPath );
  set_locale_dir( localePath );

  Glib::ustring lfdb;
#if (BUNDLED_LENSFUN_DB == 1)
  lfdb = get_lensfun_db_dir();
#endif
  std::cout<<"Calling rtengine::LFDatabase::init(\""<<lfdb<<"\")"<<std::endl;
  rtengine::LFDatabase::init( lfdb );
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
  //sleep(10);
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


//#define PF_VERBOSE_UNREF 1

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
