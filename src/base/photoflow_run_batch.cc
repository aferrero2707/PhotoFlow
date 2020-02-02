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
  extern void phf_tile_pool_set_size( int );
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


static void decode_export_options(char* arg, PF::image_export_opt_t& export_opt)
{
  int arglen = strlen(arg);
  int optidx = strlen("--export-opt=");
  while(optidx >= 0) {
    int nextidx = -1;
    for(int idx = optidx+1; idx < arglen-1; idx++) {
      //std::cout<<"arg["<<idx<<"]="<<arg[idx]<<std::endl;
      if(arg[idx] == ',') {
        nextidx = idx+1;
        arg[idx] = 0;
        //std::cout<<"  comma found"<<std::endl;
        break;
      }
    }
    std::cout<<"optidx="<<optidx<<"    nextidx="<<nextidx<<std::endl;
    char* arg2 = arg+optidx;
    std::cout<<"arg@"<<optidx<<" = "<<arg2<<std::endl;
    if( !strncmp(arg2,"jpeg_quality=",strlen("jpeg_quality=")) ) {
      std::istringstream str( arg+optidx+strlen("jpeg_quality=") );
      int val;
      str >> val;
      std::cout<<"jpeg_quality: "<<val<<std::endl;
      export_opt.jpeg_quality = val;
    } else if( !strncmp(arg2,"jpeg_chroma_subsampling=",strlen("jpeg_chroma_subsampling=")) ) {
      std::istringstream str( arg+optidx+strlen("jpeg_chroma_subsampling=") );
      int val;
      str >> val;
      std::cout<<"jpeg_chroma_subsampling: "<<val<<std::endl;
      export_opt.jpeg_chroma_subsampling = (val == 1);
    } else if( !strncmp(arg2,"jpeg_quant_table=",strlen("jpeg_quant_table=")) ) {
      std::string str( arg+optidx+strlen("jpeg_quant_table=") );
      std::cout<<"jpeg_quant_table: "<<str<<std::endl;
      if( str == "default") export_opt.jpeg_quant_table = PF::JPEG_QUANT_TABLE_DEFAULT;
      if( str == "medium") export_opt.jpeg_quant_table = PF::JPEG_QUANT_TABLE_MEDIUM;
      if( str == "best") export_opt.jpeg_quant_table = PF::JPEG_QUANT_TABLE_BEST;
    } else if( !strncmp(arg2,"tiff_compress=",strlen("tiff_compress=")) ) {
      std::istringstream str( arg+optidx+strlen("tiff_compress=") );
      int val;
      str >> val;
      std::cout<<"tiff_compress: "<<val<<std::endl;
      export_opt.tiff_compress = (val == 1);
    } else if( !strncmp(arg2,"tiff_depth=",strlen("tiff_depth=")) ) {
      std::istringstream sdepth( arg+optidx+strlen("tiff_depth=") );
      int depth;
      sdepth >> depth;
      std::cout<<"tiff_depth: "<<depth<<std::endl;
      switch(depth) {
      case  8: export_opt.tiff_format = PF::EXPORT_FORMAT_TIFF_8; break;
      case 16: export_opt.tiff_format = PF::EXPORT_FORMAT_TIFF_16; break;
      case 32: export_opt.tiff_format = PF::EXPORT_FORMAT_TIFF_32f; break;
      default: break;
      }
    } else if( !strncmp(arg2,"width=",strlen("width=")) ) {
      std::istringstream str( arg+optidx+strlen("width=") );
      int val; str >> val;
      std::cout<<"width: "<<val<<std::endl;
      export_opt.size = PF::SIZE_CUSTOM;
      export_opt.width = val;
    } else if( !strncmp(arg2,"height=",strlen("height=")) ) {
      std::istringstream str( arg+optidx+strlen("height=") );
      int val; str >> val;
      std::cout<<"height: "<<val<<std::endl;
      export_opt.size = PF::SIZE_CUSTOM;
      export_opt.height = val;
    } else if( !strncmp(arg2,"interpolator=",strlen("interpolator=")) ) {
      std::string str( arg+optidx+strlen("interpolator=") );
      std::cout<<"interpolator: "<<str<<std::endl;
      if( str == "nearest") export_opt.interpolator = PF::SCALE_INTERP_NEAREST;
      if( str == "bilinear") export_opt.interpolator = PF::SCALE_INTERP_BILINEAR;
      if( str == "bicubic") export_opt.interpolator = PF::SCALE_INTERP_BICUBIC;
      if( str == "lanczos2") export_opt.interpolator = PF::SCALE_INTERP_LANCZOS2;
      if( str == "lanczos3") export_opt.interpolator = PF::SCALE_INTERP_LANCZOS3;
    } else if( !strncmp(arg2,"sharpen_enabled=",strlen("sharpen_enabled=")) ) {
      std::istringstream str( arg+optidx+strlen("sharpen_enabled=") );
      int val;
      str >> val;
      std::cout<<"sharpen_enabled: "<<val<<std::endl;
      export_opt.sharpen_enabled = (val == 1);
    } else if( !strncmp(arg2,"sharpen_radius=",strlen("sharpen_radius=")) ) {
      std::istringstream str( arg+optidx+strlen("sharpen_radius=") );
      float val; str >> val;
      std::cout<<"sharpen_radius: "<<val<<std::endl;
      export_opt.sharpen_radius = val;
    } else if( !strncmp(arg2,"profile_type=",strlen("profile_type=")) ) {
      std::string str( arg+optidx+strlen("profile_type=") );
      std::cout<<"profile_type: "<<str<<std::endl;
      if( str == "no_change") export_opt.profile_type = PF::PROF_TYPE_EMBEDDED;
      if( str == "sRGB") export_opt.profile_type = PF::PROF_TYPE_sRGB;
      if( str == "Rec2020") export_opt.profile_type = PF::PROF_TYPE_REC2020;
      if( str == "AdobeRGB") export_opt.profile_type = PF::PROF_TYPE_ADOBE;
      if( str == "ProPhoto") export_opt.profile_type = PF::PROF_TYPE_PROPHOTO;
      if( str == "ACEScg") export_opt.profile_type = PF::PROF_TYPE_ACEScg;
      if( str == "ACES") export_opt.profile_type = PF::PROF_TYPE_ACES;
      if( str == "from_disk") export_opt.profile_type = PF::PROF_TYPE_FROM_DISK;
    } else if( !strncmp(arg2,"trc_type=",strlen("trc_type=")) ) {
      std::string str( arg+optidx+strlen("trc_type=") );
      std::cout<<"trc_type: "<<str<<std::endl;
      if( str == "standard") export_opt.trc_type = PF::PF_TRC_STANDARD;
      if( str == "linear") export_opt.trc_type = PF::PF_TRC_LINEAR;
      if( str == "perceptual") export_opt.trc_type = PF::PF_TRC_PERCEPTUAL;
      if( str == "sRGB") export_opt.trc_type = PF::PF_TRC_sRGB;
    } else if( !strncmp(arg2,"intent=",strlen("intent=")) ) {
      std::string str( arg+optidx+strlen("intent=") );
      std::cout<<"intent: "<<str<<std::endl;
      if( str == "relative_colorimetric") export_opt.intent = INTENT_RELATIVE_COLORIMETRIC;
      if( str == "perceptual") export_opt.intent = INTENT_PERCEPTUAL;
      if( str == "saturation") export_opt.intent = INTENT_SATURATION;
      if( str == "absolute_colorimetric") export_opt.intent = INTENT_ABSOLUTE_COLORIMETRIC;
    } else if( !strncmp(arg2,"bpc=",strlen("bpc=")) ) {
      std::istringstream str( arg+optidx+strlen("bpc=") );
      int val;
      str >> val;
      std::cout<<"bpc: "<<val<<std::endl;
      export_opt.bpc = (val == 1);
    } else if( !strncmp(arg2,"profile_name=",strlen("profile_name=")) ) {
      std::string str( arg+optidx+strlen("profile_name=") );
      std::cout<<"profile_name: "<<str<<std::endl;
      export_opt.custom_profile_name = str;
    }
    optidx = nextidx;
  }
}


static void print_help()
{
  std::cout<<"Usage: photoflow --batch [--config=config_file.pfp] --export-opt=[export options] in_file out_file"<<std::endl;
  std::cout<<"Export options (comma-separated list):"<<std::endl
      <<"  jpeg_quality=[0..100] (default: 80)"<<std::endl
      <<"  jpeg_chroma_subsampling=0/1 (default: 0)"<<std::endl
      <<"  jpeg_quant_table=default:medium:best (default: best)"<<std::endl
      <<"  tiff_depth=8:16:32 (default: 16) tiff_depth=32 generates an image in floating-point format"<<std::endl
      <<"  tiff_compress=0/1 (default: 0)"<<std::endl
      <<"  width=X (width of the exported image in pixels)"<<std::endl
      <<"  height=X (height of the exported image in pixels)"<<std::endl
      <<"  interpolator=X (kernel used to interpolate the scaled output)"<<std::endl
      <<"    allowed values are:"<<std::endl
      <<"      nearest, bilinear, bicubic, lanczos2, lanczos3 (default: lanczos3)"<<std::endl
      <<"  sharpen_enabled=0/1 (default: 0) enable/disable post-resize sharpening step"<<std::endl
      <<"  sharpen_radius=X (radius for the post-resize sharpening step)"<<std::endl
      <<"  profile_type=X (default: sRGB) ICC profile for the exported image"<<std::endl
      <<"    allowed values are:"<<std::endl
      <<"      no_change: keep the image in the same colorspace used for processing (by default linear Rec2020)"<<std::endl
      <<"      sRGB, Rec2020, AdobeRGB, ProPhoto, ACEScg, ACES: convert the image to the specified colorspace"<<std::endl
      <<"      from_disk: use a custom ICC profile from disk"<<std::endl
      <<"  profile_name=\"X.icc\" path to profile from disk when \"profile_type\" is set to \"from_disk\""<<std::endl
      <<"  trc_type=standard:linear:perceptual:sRGB (default: standard) TRC of the output ICC profile"<<std::endl
      <<"    the meaning of the values is:"<<std::endl
      <<"      standard: use the standard TRC for the selected colorspace"<<std::endl
      <<"                for example: sRGB TRC for the sRGB colorspace, gamma=2.2 for AdobeRGB, gamma=1.8 for ProPhoto, etc..."<<std::endl
      <<"      linear: linear (gamma=1.0) TRC"<<std::endl
      <<"      perceptual: same TRC as defined in the CIELab L channel specifications"<<std::endl
      <<"      sRGB: same TRC as defined in the sRGB specifications"<<std::endl
      <<"    Notice that this option has no effect when using a custom profile from disk"<<std::endl
      <<"  intent=relative_colorimetric:perceptual:saturation:absolute_colorimetric (default: relative_colorimetric)"<<std::endl
      <<"    Rendering intent for the output ICC conversion."<<std::endl
      <<"    Note that only relative and absolute colorimetric intents are implemented for the built-in colorspaces."<<std::endl
      <<"    Perceptual and saturation intents might be available when using LUT profiles from disk."<<std::endl
      <<"  bpc=0/1 (default: 1) enable/disable black point compensation in the output ICC conversion"<<std::endl
      <<"Example:"<<std::endl
      <<"  --export-opt=tiff_depth=16,tiff_compress=1,width=800,height=600,interpolator=bicubic,sharpen_enabled=1,sharpen_radius=0.5,profile_type=Rec2020,trc_type=linear"<<std::endl;
}


gchar* PF::resolve_filename(gchar* filename)
{
  if( !filename ) return NULL;
  const gchar* appimage_path = g_getenv("APPIMAGE_OWD");
  gchar* filename_full = filename;
  if(appimage_path && !g_path_is_absolute(filename)) {
    filename_full = g_build_filename( appimage_path, filename, NULL );
  }
  gchar* fullpath = realpath( filename_full, NULL );
  if(filename) std::cout<<"filename: \""<<filename<<"\"  ";
  if(appimage_path) std::cout<<"APPIMAGE_OWD=\""<<appimage_path<<"\"  ";
  if(fullpath) std::cout<<"fullpath=\""<<fullpath<<"\"";
  std::cout<<std::endl;
  if( filename_full != filename ) g_free(filename_full);

  return fullpath;
}


int PF::PhotoFlow::run_batch(int argc, char *argv[])
{
/*
  if (argc != 2) {
  printf ("usage: %s <filename>", argv[0]);
  exit(1);
  }
*/

  std::cout<<"PhotoFlow::run_batch(): argc="<<argc<<std::endl;
  for(int i = 0; i < argc; i++)
    std::cout<<"  argv["<<i<<"]: \""<<argv[i]<<"\""<<std::endl;



  if( argc > 1 && !strncmp(argv[1], "--help", strlen("--help")) ) {
    print_help();
    return 0;
  }


  if( argc < 3 ) {
    std::cout<<"PhotoFlow::run_batch(): too few arguments: argc="<<argc<<std::endl;
    return 1;
  }

  //if (vips_init (argv[0]))
    //vips::verror ();
  //  return 1;

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

  phf_tile_pool_set_size( PF::PhotoFlow::Instance().get_options().get_tile_cache_size() );


  PF::Image* image = new PF::Image();
  if( !image ) {
    std::cout<<"Cannot create image object. Exiting."<<std::endl;
    return 1;
  }
  PF::Pipeline* pipeline = image->add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_PREVIEW/*PF_RENDER_NORMAL*/ );
  if( pipeline ) pipeline->set_op_caching_enabled( false );

  //argv++;

  if( argc >= 2 ) {
    std::string img_in, img_out;
    std::vector< std::string > presets;
    image_export_opt_t export_opt;
    export_opt.jpeg_quality = 90;
    export_opt.jpeg_chroma_subsampling = false;
    export_opt.jpeg_quant_table = PF::JPEG_QUANT_TABLE_BEST;
    export_opt.tiff_format = PF::EXPORT_FORMAT_TIFF_16;
    export_opt.tiff_compress = false;
    export_opt.size = PF::SIZE_ORIGINAL;
    export_opt.width = 0;
    export_opt.height = 0;
    export_opt.interpolator = PF::SCALE_INTERP_LANCZOS3;
    export_opt.sharpen_enabled = false;
    export_opt.sharpen_radius = 1;
    export_opt.sharpen_amount = 100;
    export_opt.profile_type = PF::PROF_TYPE_sRGB;
    export_opt.trc_type = PF::PF_TRC_STANDARD;
    export_opt.intent = INTENT_RELATIVE_COLORIMETRIC;
    export_opt.bpc = true;

    fullpath = resolve_filename( argv[argc-2] );
    if(!fullpath) {
      std::cout<<"PhotoFlow::run_batch(): input file not found: \""<<argv[argc-2]<<"\""<<std::endl;
      return 1;
    }
    std::cout<<"PhotoFlow::run_batch(): input image=\""<<fullpath<<"\""<<std::endl;
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
      std::cout<<"PhotoFlow::run_batch(): Cannot detemine input file extension. Exiting."<<std::endl;
      return 1;
    }
    std::string iname;
    if( !PF::getFileName( "", bname, iname ) ) {
      std::cout<<"PhotoFlow::run_batch(): Cannot detemine input file name. Exiting."<<std::endl;
      return 1;
    }
    std::cout<<"iname="<<iname<<std::endl;
    std::cout<<"ext=  "<<ext<<std::endl;

    img_out = argv[argc-1];
    std::cout<<"img_out(1)= "<<img_out<<std::endl;
    std::string patt = "%name%";
    replace_string( img_out, patt, iname );
    std::cout<<"img_out(2)= "<<img_out<<std::endl;
    fullpath = resolve_filename((gchar*)(img_out.c_str()));
    if( fullpath ) {
      img_out = fullpath;
      free(fullpath);
    }
    std::cout<<"img_out(3)= "<<img_out<<std::endl;


    image->open( img_in );

    for( int i = 1; i < argc-2; i++ ) {
      char* par = argv[i];
      std::cout<<"par: "<<argv[i]<<std::endl;
      if( !strncmp(par,"--config=",strlen("--config=")) ) {
        fullpath = resolve_filename( argv[i]+strlen("--config=") );
        if( !fullpath ) continue;
        presets.push_back( std::string(fullpath) );
        PF::insert_pf_preset( fullpath, image, NULL, &(image->get_layer_manager().get_layers()), false );
        free(fullpath);
      } else if( !strncmp(par,"--export-opt=",strlen("--export-opt=")) ) {
        decode_export_options(argv[i], export_opt);
        std::cout<<"export_opt.tiff_format="<<export_opt.tiff_format<<std::endl;
      }
    }

    std::string oext;
    if( !PF::getFileExtension( "", img_out, oext ) ) {
      std::cout<<"Cannot detemine output file extension. Exiting."<<std::endl;
      return 1;
    }
    if( oext == "pfi") {
      image->save( img_out );
    } else {
      void* data = malloc(sizeof(image_export_opt_t));
      memcpy(data, &export_opt, sizeof(image_export_opt_t));
      image->export_merged( img_out, (image_export_opt_t*)data );
    }
  }
  //Shows the window and returns when it is closed.

	//PF::ImageProcessor::Instance().join();

  image->destroy();
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

