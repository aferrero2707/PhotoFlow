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
#include <signal.h>
#include <unistd.h>

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

#include<libintl.h>
#include<locale.h>

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
#include "base/imageprocessor.hh"
#include "gui/mainwindow.hh"
#include "plugin/pluginwindow.hh"

#include "base/new_operation.hh"
#include "base/pf_file_loader.hh"
#include "rt/rtengine/color.h"

//#include "external/DeathHandler/death_handler.h"

extern int vips__leak;

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern GType vips_layer_get_type( void ); 
extern GType vips_gmic_get_type( void ); 
extern GType vips_cimg_blur_anisotropic_get_type( void );
extern GType vips_cimg_blur_bilateral_get_type( void );
extern void vips_cimg_operation_init( void );
extern GType vips_clone_stamp_get_type( void ); 
extern GType vips_lensfun_get_type( void );
extern GType vips_perspective_get_type( void );
extern GType phf_tile_cache_get_type( void );
extern void phf_tile_pool_set_size( int );
#ifdef __cplusplus
}
#endif /*__cplusplus*/


#ifndef WIN32
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif


int main (int argc, char *argv[])
{
  if( argc > 1 && std::string(argv[1]) == "--version" ) {
    std::cout<<"this is photoflow 0.2.8"<<std::endl;
    return 0;
  }


//#ifndef WIN32
//  signal(SIGSEGV, handler);   // install our handler
//#endif
  //Debug::DeathHandler dh;


#if defined(WIN32)
  std::string mimePath = (std::string)(PF::PhotoFlow::Instance().get_base_dir()) + "\\..\\share";
  std::string mimeVar = "XDG_DATA_HOME";
  std::cout<<"Setting XDG_DATA_HOME to "<<mimePath<<std::endl;
  Glib::setenv( mimeVar, mimePath, true );
  const gchar * const * system_data_dirs = g_get_system_data_dirs();
  std::cout<<"System data dirs: "<<std::endl;
  int di = 0;
  while( system_data_dirs[di] != NULL ) {
    std::cout<<"  "<<system_data_dirs[di]<<std::endl;
    di++;
  }
  std::cout<<"User data dir: "<<g_get_user_data_dir()<<std::endl;
#endif
  //return 0;

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
  //vips_cimg_blur_anisotropic_get_type();
  //vips_cimg_blur_bilateral_get_type();
  //vips_cimg_operation_init();
  vips_clone_stamp_get_type();
  vips_lensfun_get_type();
  vips_perspective_get_type();
  phf_tile_cache_get_type();

  //vips_profile_set( TRUE );
  //vips_profile_set(true);
  //vips_cache_set_trace(TRUE);
  //vips_concurrency_set( 1 );
#ifndef NDEBUG
  vips_cache_set_trace( true );
#endif

  rtengine::Color::init();

  vips__leak = 1;

  bool is_plugin = false;

  PF::ICCStore::Instance();


  std::cout<<"PhotoFlow::main(): argc="<<argc<<std::endl;
  for(int i = 0; i < argc; i++)
    std::cout<<"  argv["<<i<<"]: \""<<argv[i]<<"\""<<std::endl;

  if( argc >= 2 && std::string(argv[1]) == "--batch" ) {
    argc--;
    argv++;
    return PF::PhotoFlow::Instance().run_batch(argc, argv);
  }

  if( argc >= 5 && argc <= 6 && std::string(argv[1]) == "--plugin" ) {
    argc--;
    argv++;
    is_plugin = true;
    printf("PhF plugin: argc=%d\n", argc);
    PF::PhotoFlow::Instance().set_plugin( true );
  }

  if(!Glib::thread_supported()) 
    Glib::thread_init();

  //return 0;

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
  Glib::ustring configPath = PF::PhotoFlow::Instance().get_config_dir();
#if defined(WIN32)
  Glib::ustring themesPath = dataPath + "\\themes";
  Glib::ustring themesPath2 = configPath + "\\themes";
#else
  Glib::ustring themesPath = dataPath + "/themes";
  Glib::ustring themesPath2 = configPath + "/themes";
#endif

  PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation_with_gui );
  PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );
  PF::PhotoFlow::Instance().set_batch( false );

  PF::PhotoFlow::Instance().get_options().load();

  Glib::ustring idir = "icons";
  if( PF::PhotoFlow::Instance().get_options().get_ui_use_inverted_icons() ||
      !(PF::PhotoFlow::Instance().get_options().get_ui_use_system_theme()) ) {
    idir = "icons-inverted";
  }
  PF::PhotoFlow::Instance().set_icons_dir(
      Glib::build_filename( Glib::ustring(PF::PhotoFlow::Instance().get_data_dir()), idir ) );
  std::cout<<"icons dir: "<<PF::PhotoFlow::Instance().get_icons_dir()<<std::endl;




  std::cout<<"Starting image processor..."<<std::endl;
  PF::ImageProcessor::Instance().start();
  std::cout<<"Image processor started."<<std::endl;

  if( PF::PhotoFlow::Instance().get_cache_dir().empty() ) {
    std::cout<<"FATAL: Cannot create cache dir."<<std::endl;
    return 1;
  }

  if(argc>1) std::cout<<"argv[1]"<<argv[1]<<")\n";
#ifndef _WIN32
  std::cout<<"locale dir: "<<PF::PhotoFlow::Instance().get_locale_dir()<<std::endl;
  bindtextdomain("photoflow",PF::PhotoFlow::Instance().get_locale_dir().c_str());
  bind_textdomain_codeset("photoflow", "UTF-8");
  setlocale(LC_ALL,"");
  textdomain("photoflow");
#else
  gtk_disable_setlocale();
#endif
  if(argc>1) std::cout<<"argv[1]"<<argv[1]<<")\n";

  vips__leak = 1;

  phf_tile_pool_set_size( PF::PhotoFlow::Instance().get_options().get_tile_cache_size() );

//return 0;
#ifdef _WIN32
  Gtk::Main* app = new Gtk::Main(argc, argv, true);
#else
  Gtk::Main* app = new Gtk::Main(argc, argv);
#endif

  g_setenv("LC_NUMERIC", "C", TRUE);

  struct stat buffer;   
  //#ifndef WIN32
#ifdef GTKMM_2
#if defined(WIN32)
  Glib::ustring themerc = themesPath + "\\photoflow-dark.gtkrc";
  Glib::ustring themerc2 = themesPath2 + "\\photoflow-dark.gtkrc";
#else
  Glib::ustring themerc = themesPath + "/photoflow-dark.gtkrc";
  Glib::ustring themerc2 = themesPath2 + "/photoflow-dark.gtkrc";
#endif
  int stat_result = stat(themerc.c_str(), &buffer);
  int stat_result2 = stat(themerc2.c_str(), &buffer);
#ifdef WIN32
  //stat_result = 1;
#endif
  std::cout<<"stat_result="<<stat_result<<std::endl;
  std::cout<<"stat_result2="<<stat_result2<<std::endl;
  if( PF::PhotoFlow::Instance().get_options().get_ui_use_system_theme() == false ) {
  if( stat_result2 == 0 ) {
    Gtk::Settings::get_default()->property_gtk_theme_name() = "";
    std::vector<Glib::ustring> files;
    files.push_back (themerc2);
    Gtk::RC::set_default_files (files);
    Gtk::RC::reparse_all (Gtk::Settings::get_default());
    //GdkEventClient event = { GDK_CLIENT_EVENT, NULL, TRUE, gdk_atom_intern("_GTK_READ_RCFILES", FALSE), 8 };
    //gdk_event_send_clientmessage_toall ((GdkEvent*)&event);
  } else if( stat_result == 0 ) {
    Gtk::Settings::get_default()->property_gtk_theme_name() = "";
    std::vector<Glib::ustring> files;
    files.push_back (themerc);
    Gtk::RC::set_default_files (files);
    Gtk::RC::reparse_all (Gtk::Settings::get_default());
    //GdkEventClient event = { GDK_CLIENT_EVENT, NULL, TRUE, gdk_atom_intern("_GTK_READ_RCFILES", FALSE), 8 };
    //gdk_event_send_clientmessage_toall ((GdkEvent*)&event);
  }}
#endif
  //#endif

  if(argc>1) std::cout<<"argv[1]"<<argv[1]<<")\n";

  PF::MainWindow* mainWindow = NULL;
  PF::PluginWindow* pluginwin = NULL;
  if( is_plugin )
    pluginwin = new PF::PluginWindow();
  else
    mainWindow = new PF::MainWindow();
#ifdef GTKMM_3
  Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme().set_value(true);
#if defined(WIN32)
  Glib::ustring themerc = themesPath + "\\photoflow-dark.css";
  Glib::ustring themerc2 = themesPath2 + "\\photoflow-dark.css";
#else
  Glib::ustring themerc = themesPath + "/photoflow-dark.css_";
  //Glib::ustring themerc2 = themesPath2 + "/photoflow-dark.css";
  Glib::ustring themerc2 = themesPath2 + "/gtk-3.0/gtk.css";
#endif
  std::cout<<"themerc2: "<<themerc2<<std::endl;

  int stat_result = stat(themerc.c_str(), &buffer);
  int stat_result2 = stat(themerc2.c_str(), &buffer);
  //int stat_result = stat((themesPath + "/RawTherapee.css").c_str(), &buffer);
  //int stat_result = stat((themesPath + "/gtk-3.0/gtk.css").c_str(), &buffer);
  //stat_result = 1;
  Glib::ustring themerc_real;
  if( stat_result2 == 0 ) themerc_real = themerc2;
  else if( stat_result == 0 ) themerc_real = themerc;

  try {
  if( PF::PhotoFlow::Instance().get_options().get_ui_use_system_theme() == false  &&
      !themerc_real.empty() ) {
    Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
    //Glib::RefPtr<Gtk::StyleContext> cntx = mainWindow->get_style_context();
    Glib::RefPtr<Gtk::StyleContext> cntx = Gtk::StyleContext::create();
    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    //cntx->set_screen( screen );
    //cntx->set_path( mainWindow->get_path() );
    //cntx->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    cntx->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    //cntx->invalidate();
    std::cout<<"Loading theme file "<<themerc_real<<std::endl;
    css->load_from_path(themerc_real.c_str());
    //std::cout<<"Loading theme file "<<themesPath + "/RawTherapee.css"<<std::endl;
    //css->load_from_path(themesPath + "/RawTherapee.css");
    //css->load_from_path(themesPath + "/gtk-3.0/gtk.css");
    //css->load_from_path("themes/photoflow-dark/gtk.css");
  }
  } catch( Gtk::CssProviderError& error ) {

  }
#endif

  std::cout<<"PhotoFlow: is_plugin="<<is_plugin<<std::endl;
  if(argc>1) std::cout<<"argv[1]"<<argv[1]<<")\n";

  if( is_plugin ) {
    fullpath = realpath( argv[1], NULL );
    std::cout<<"PhotoFlow plug-in: argv[1]=\""<<argv[1]<<"\"  fullpath="<<(void*)fullpath<<std::endl;
    if(!fullpath)
      return 1;
    std::cout<<"filename: "<<fullpath<<std::endl;
    if( argc == 5 ) {
      std::cout<<"pfiname: "<<argv[2]<<std::endl;
      std::cout<<"filename_out: "<<argv[3]<<std::endl;
      std::cout<<"pfiname_out: "<<argv[4]<<std::endl;
      std::string pfiname = argv[2];
      pluginwin->open_image( fullpath, true );
      if( pfiname != "none" ) {
        g_assert( pluginwin->get_image_editor() != NULL );
        PF::ImageEditor* pfeditor = pluginwin->get_image_editor();
        g_assert( pfeditor != NULL );
        PF::Image* pfimage = pfeditor->get_image();
        g_assert( pfimage != NULL );
        PF::insert_pf_preset( pfiname, pfimage, NULL, &(pfimage->get_layer_manager().get_layers()), false );
      }
      pluginwin->set_filename_out( argv[3] );
      pluginwin->set_pfiname_out( argv[4] );
    } else {
      pluginwin->open_image( fullpath, false );
      pluginwin->set_filename_out( argv[2] );
      pluginwin->set_pfiname_out( argv[3] );
    }
    pluginwin->show_all();
    free(fullpath);
    app->run(*pluginwin);
  } else {
    //Shows the window and returns when it is closed.
    mainWindow->show_all();
    if( argc > 1 ) {
      /*fullpath = PF::resolve_filename( argv[argc-1] );
      if(!fullpath)
        return 1;
      mainWindow->open_image( fullpath );
      free(fullpath);*/
      std::cout<<"argv[1]"<<argv[1]<<")\n";
      std::cout<<"argv[argc-1]"<<argv[argc-1]<<")\n";
      std::string filename = argv[argc-1];
      std::cout<<"mainWindow->open_image("<<filename<<")\n";
      mainWindow->open_image( argv[argc-1] );
    } else {
      mainWindow->on_button_open_clicked();
    }
    if(mainWindow->get_number_of_images() > 0) app->run(*mainWindow);
  }

  if( mainWindow ) delete mainWindow;
  if( pluginwin ) delete pluginwin;

  PF::ProcessRequestInfo request;
  request.request = PF::PROCESSOR_END;
  PF::ImageProcessor::Instance().submit_request( request );
  PF::ImageProcessor::Instance().join();

  std::cout<<"Image processing thread finished"<<std::endl;

  delete app;

  PF::PhotoFlow::Instance().close();

  return 0;
}

