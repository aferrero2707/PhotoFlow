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

#include "base/new_operation.hh"

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
  //return 0;

  /*
  if (argc != 2) {
    printf ("usage: %s <filename>", argv[0]);
    exit(1);
  }
   */

#ifndef WIN32
  signal(SIGSEGV, handler);   // install our handler
#endif

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

#ifndef NDEBUG
  im_concurrency_set( 1 );
  vips_cache_set_trace( true );
#endif

  //vips__leak = 1;

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
  Glib::ustring themesPath = dataPath + "/themes";

  PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation_with_gui );
  PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );
  PF::PhotoFlow::Instance().set_batch( false );

  std::cout<<"Starting image processor..."<<std::endl;
  PF::ImageProcessor::Instance().start();
  std::cout<<"Image processor started."<<std::endl;

  if( PF::PhotoFlow::Instance().get_cache_dir().empty() ) {
    std::cout<<"FATAL: Cannot create cache dir."<<std::endl;
    return 1;
  }

  vips__leak = 1;

//return 0;
  Gtk::Main* app = new Gtk::Main(argc, argv);

  struct stat buffer;   
  //#ifndef WIN32
#ifdef GTKMM_2
  int stat_result = stat((themesPath + "/photoflow-dark.gtkrc").c_str(), &buffer);
  std::cout<<"stat_result="<<stat_result<<std::endl;
  if( stat_result == 0 ) {
    std::vector<Glib::ustring> files;
    files.push_back (themesPath + "/photoflow-dark.gtkrc");
    Gtk::RC::set_default_files (files);
    Gtk::RC::reparse_all (Gtk::Settings::get_default());
    GdkEventClient event = { GDK_CLIENT_EVENT, NULL, TRUE, gdk_atom_intern("_GTK_READ_RCFILES", FALSE), 8 };
    gdk_event_send_clientmessage_toall ((GdkEvent*)&event);
  }
#endif
  //#endif

  PF::MainWindow* mainWindow = new PF::MainWindow();
#ifdef GTKMM_3
  Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme().set_value(true);

  int stat_result = stat((themesPath + "/photoflow-dark.css").c_str(), &buffer);
  if( stat_result == 0 ) {
    Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
    //Glib::RefPtr<Gtk::StyleContext> cntx = mainWindow->get_style_context();
    Glib::RefPtr<Gtk::StyleContext> cntx = Gtk::StyleContext::create();
    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    //cntx->set_screen( screen );
    //cntx->set_path( mainWindow->get_path() );
    //cntx->add_provider(css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    cntx->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    //cntx->invalidate();
    css->load_from_path(themesPath + "/photoflow-dark.css");
    //css->load_from_path("themes/photoflow-dark/gtk.css");
  }
#endif

  if( argc > 1 ) {
    fullpath = realpath( argv[argc-1], NULL );
    if(!fullpath)
      return 1;
    mainWindow->open_image( fullpath );
    free(fullpath);
  }
  //Shows the window and returns when it is closed.
  mainWindow->show_all();
  app->run(*mainWindow);

  delete mainWindow;
  delete app;

  PF::ImageProcessor::Instance().join();

  //im_close_plugins();
  vips_shutdown();

#if defined(__MINGW32__) || defined(__MINGW64__)
  for (int i = 0; i < _getmaxstdio(); ++i) close (i);
#elif defined(__APPLE__) && defined(__MACH__)
#else
  rlimit rlim;
  //getrlimit(RLIMIT_NOFILE, &rlim);
  if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
    std::cout<<"rlim.rlim_max="<<rlim.rlim_max<<std::endl;
    for (int i = 3; i < rlim.rlim_max; ++i) {
      //std::cout<<"i="<<i<<std::endl;
      close (i);
    }
  }
#endif
  std::list<std::string>::iterator fi;
  for(fi = cache_files.begin(); fi != cache_files.end(); fi++)
    unlink( fi->c_str() );

  return 0;
}

