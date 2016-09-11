
#include "pluginwindow.hh"
#include "../base/new_operation.hh"
#include "../base/imageprocessor.hh"
#if HAVE_GIMP_2_9
#include <gegl.h>
#endif
#include <libgimpbase/gimpbase.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <glib/gi18n.h>
#include <string.h>

#include "file-formats.h"

#define LOAD_THUMB_PROC "file-raw-load-thumb"

#define VERSION "0.2.6"

//#define HAVE_GIMP_2_9 1


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
#ifdef __cplusplus
}
#endif /*__cplusplus*/

GimpPDBStatusType status = GIMP_PDB_CALLING_ERROR;

static const char raw_ext[] = "3fr,ari,arw,cap,cine,cr2,crw,cs1,dc2,dcr,dng,erf,fff,"
  "hdr,ia,iiq,k25,kc2,kdc,mdc,mef,mos,mrw,nef,"
  "nrw,orf,ori,pef,pxn,qtk,r3d,raf,raw,rdc,rw2,rwl,sr2,"
  "srf,srw,sti,pfi,x3f";



static PF::PluginWindow* pluginwin;

static void     query                (void);
static void     run                  (const gchar      *name,
                                      gint              nparams,
                                      const GimpParam  *param,
                                      gint             *nreturn_vals,
                                      GimpParam       **return_vals);
static gint32   load_image           (const gchar      *filename,
                                      GimpRunMode       run_mode,
                                      GError          **error);

static gint32   load_thumbnail_image (const gchar      *filename,
                                      gint             thumb_size,
                                      gint             *width,
                                      gint             *height,
                                      GError          **error);


//long pf_save_gimp_image(ufraw_data *uf, GtkWidget *widget);

GimpPlugInInfo PLUG_IN_INFO = {
  NULL,  /* init_procedure */
  NULL,  /* quit_procedure */
  query, /* query_procedure */
  run,   /* run_procedure */
};

MAIN()

void query()
{
  static const GimpParamDef load_args[] = {
    { GIMP_PDB_INT32,  "run_mode", "Interactive, non-interactive" },
    { GIMP_PDB_STRING, "filename", "The name of the file to load" },
    { GIMP_PDB_STRING, "raw_filename", "The name of the file to load" },
  };
  static const GimpParamDef load_return_vals[] = {
    { GIMP_PDB_IMAGE, "image", "Output image" },
  };
  static const GimpParamDef thumb_args[] = {
    { GIMP_PDB_STRING, "filename",     "The name of the file to load" },
    { GIMP_PDB_INT32,  "thumb_size",   "Preferred thumbnail size" }
  };
  static const GimpParamDef thumb_return_vals[] = {
    { GIMP_PDB_IMAGE,  "image",        "Thumbnail image" },
    { GIMP_PDB_INT32,  "image_width",  "Width of full-sized image" },
    { GIMP_PDB_INT32,  "image_height", "Height of full-sized image" }
  };


  for (int i = 0; i < G_N_ELEMENTS (file_formats); i++)
    {
      const FileFormat *format = &file_formats[i];

      gimp_install_procedure (format->load_proc,
                              format->load_blurb,
                              format->load_help,
                              "Tobias Ellinghaus",
                              "Tobias Ellinghaus",
                              "2016",
                              format->file_type,
                              NULL,
                              GIMP_PLUGIN,
                              G_N_ELEMENTS (load_args),
                              G_N_ELEMENTS (load_return_vals),
                              load_args, load_return_vals);

      gimp_register_file_handler_mime (format->load_proc,
                                       format->mime_type);
      gimp_register_magic_load_handler (format->load_proc,
                                        format->extensions,
                                        "",
                                        format->magic);
    }

  /*
    gimp_install_procedure("file_pf_load",
    "Loads digital camera raw files",
    "Loads digital camera raw files.",
    "Udi Fuchs",
    "Copyright 2003 by Dave Coffin\n"
    "Copyright 2004 by Pawel Jochym\n"
    "Copyright 2004-2015 by Udi Fuchs",
    "pf-" VERSION,
    "raw image",
    NULL,
    GIMP_PLUGIN,
    G_N_ELEMENTS(load_args),
    G_N_ELEMENTS(load_return_vals),
    load_args,
    load_return_vals);

    #if HAVE_GIMP_2_9
    gimp_register_magic_load_handler("file_pf_load",
    (char *)raw_ext,
    "",
    "0,string,II*\\0,"
    "0,string,MM\\0*,"
    "0,string,<?xml");
    #else
    gimp_register_load_handler("file_pf_load", (char *)raw_ext, "");
    #endif
  */
  gimp_install_procedure("file_pf_load_thumb",
			 "Loads thumbnails from digital camera raw files.",
			 "Loads thumbnails from digital camera raw files.",
			 "Udi Fuchs",
			 "Copyright 2004-2015 by Udi Fuchs",
			 "pf-" VERSION,
			 NULL,
			 NULL,
			 GIMP_PLUGIN,
			 G_N_ELEMENTS(thumb_args),
			 G_N_ELEMENTS(thumb_return_vals),
			 thumb_args, thumb_return_vals);

  gimp_register_thumbnail_loader("file_pf_load",
				 "file_pf_load_thumb");

}

char *pf_binary;
gboolean sendToGimpMode;


static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[6];
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
  GimpRunMode        run_mode;
  gint               image_ID;
  GError            *error = NULL;
  gint               i;

  //INIT_I18N ();

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

  /* check if the format passed is actually supported & load */
  for (i = 0; i < G_N_ELEMENTS (file_formats); i++)
    {
      const FileFormat *format = &file_formats[i];

      if (format->load_proc && ! strcmp (name, format->load_proc))
        {
          image_ID = load_image (param[1].data.d_string, run_mode, &error);

          if (image_ID != -1)
            {
              *nreturn_vals = 2;
              values[1].type         = GIMP_PDB_IMAGE;
              values[1].data.d_image = image_ID;
            }
          else
            {
              status = GIMP_PDB_EXECUTION_ERROR;
            }

          break;
        }
      else if (! strcmp (name, LOAD_THUMB_PROC))
        {
          gint width  = 0;
          gint height = 0;

          image_ID = load_thumbnail_image (param[0].data.d_string,
                                           param[1].data.d_int32,
                                           &width,
                                           &height,
                                           &error);

          if (image_ID != -1)
            {
              *nreturn_vals = 6;
              values[1].type         = GIMP_PDB_IMAGE;
              values[1].data.d_image = image_ID;
              values[2].type         = GIMP_PDB_INT32;
              values[2].data.d_int32 = width;
              values[3].type         = GIMP_PDB_INT32;
              values[3].data.d_int32 = height;
              values[4].type         = GIMP_PDB_INT32;
              values[4].data.d_int32 = GIMP_RGB_IMAGE;
              values[5].type         = GIMP_PDB_INT32;
              values[5].data.d_int32 = 1; /* num_layers */
            }
          else
            {
              status = GIMP_PDB_EXECUTION_ERROR;
            }

          break;
        }
    }

  if (i == G_N_ELEMENTS (file_formats))
    status = GIMP_PDB_CALLING_ERROR;

  if (status != GIMP_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type           = GIMP_PDB_STRING;
      values[1].data.d_string  = error->message;
    }

  values[0].data.d_status = status;
}



gint32 load_image (const gchar      *filename,
		   GimpRunMode       run_mode,
		   GError          **error)
{
  // TODO: Check if the static variable here is really needed.
  // In any case this should cause no issues with threads.
  int size;

#if !GLIB_CHECK_VERSION(2,31,0)
  g_thread_init(NULL);
#endif
#ifndef _WIN32
  gdk_threads_init();
  gdk_threads_enter();
#endif
  pf_binary = g_path_get_basename(gimp_get_progname());
#if HAVE_GIMP_2_9
  gegl_init(NULL, NULL);
#endif

#if HAVE_GIMP_2_9
  GeglBuffer *buffer;
#else
  GimpDrawable *drawable;
  GimpPixelRgn pixel_region;
  int tile_height, row, nrows;
#endif
  gint32 layer;
  gint32 gimpImage = -1;

  //std::cout<<"pfgimp::run(): name="<<name<<std::endl;

  gimp_ui_init("photoflow-gimp", TRUE);

  int loadThumbnail = 0;
  int sendToGimpMode = 0;
  std::cout<<"  filename="<<filename<<std::endl;
  /* BUG - what should be done with GIMP_RUN_WITH_LAST_VALS */
  if (run_mode == GIMP_RUN_INTERACTIVE &&
      !loadThumbnail && !sendToGimpMode) {
    /* Show the preview in interactive mode, unless if we are
     * in thumbnail mode or 'send to gimp' mode. */
    std::cout<<"  before creating window"<<std::endl;
    //vips_init (NULL);
    vips_layer_get_type();
    vips_gmic_get_type();
    //vips_cimg_blur_anisotropic_get_type();
    //vips_cimg_blur_bilateral_get_type();
    //vips_cimg_operation_init();
    vips_clone_stamp_get_type();
    vips_lensfun_get_type();
    vips_perspective_get_type();
    if(!Glib::thread_supported())
      Glib::thread_init();
    //gimp_ui_init("photoflow-gimp", TRUE);
    PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation_with_gui );
    PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );
    PF::PhotoFlow::Instance().set_batch( false );

    PF::PhotoFlow::Instance().get_options().load();

    std::cout<<"Starting image processor..."<<std::endl;
    PF::ImageProcessor::Instance().start();
    std::cout<<"Image processor started."<<std::endl;
    Gtk::Main* app = new Gtk::Main(0, NULL);

    struct stat statbuf;
    Glib::ustring dataPath = PF::PhotoFlow::Instance().get_data_dir();
#if defined(WIN32)
    Glib::ustring themesPath = dataPath + "\\themes";
#else
    Glib::ustring themesPath = dataPath + "/themes";
#endif

#if defined(WIN32)
    Glib::ustring themerc = themesPath + "\\photoflow-dark.gtkrc";
#else
    Glib::ustring themerc = themesPath + "/photoflow-dark.gtkrc";
#endif
    int stat_result = stat(themerc.c_str(), &statbuf);
#ifdef WIN32
    //stat_result = 1;
#endif
    std::cout<<"stat_result="<<stat_result<<std::endl;
    if( stat_result == 0 ) {
      std::vector<Glib::ustring> files;
      files.push_back (themerc);
      Gtk::RC::set_default_files (files);
      Gtk::RC::reparse_all (Gtk::Settings::get_default());
      GdkEventClient event = { GDK_CLIENT_EVENT, NULL, TRUE, gdk_atom_intern("_GTK_READ_RCFILES", FALSE), 8 };
      //gdk_event_send_clientmessage_toall ((GdkEvent*)&event);
    }

    pluginwin = new PF::PluginWindow();
    std::cout<<"  window created"<<std::endl;
    std::cout<<"  data_dir="<<PF::PhotoFlow::Instance().get_data_dir()<<std::endl;
    pluginwin->open_image( filename );
    std::cout<<"  file opened"<<std::endl;
    pluginwin->show_all();
    app->run(*pluginwin);
    //gtk_main();
    std::cout<<"Plug-in window closed."<<std::endl;
    std::cout<<"pluginwin->get_image_buffer().buf="<<pluginwin->get_image_buffer().buf<<std::endl;


    int width = 100, height = 100;
    if( pluginwin->get_image_buffer().buf ) {
      width = pluginwin->get_image_buffer().width;
      height = pluginwin->get_image_buffer().height;

#if HAVE_GIMP_2_9
      gimpImage =
        gimp_image_new_with_precision(width, height, GIMP_RGB,
				      GIMP_PRECISION_FLOAT_GAMMA);
#else
      gimpImage = gimp_image_new(width, height, GIMP_RGB);
#endif
      gimp_image_set_filename(gimpImage, filename);

      /* Create the "background" layer to hold the image... */
      layer = gimp_layer_new(gimpImage, _("Background"), width,
			     height, GIMP_RGB_IMAGE, 100.0,
			     GIMP_NORMAL_MODE);
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,7,3)
      gimp_image_insert_layer(gimpImage, layer, 0, 0);
#else
      gimp_image_add_layer(gimpImage, layer, 0);
#endif

      /* Get the drawable and set the pixel region for our load... */
#if HAVE_GIMP_2_9
      buffer = gimp_drawable_get_buffer(layer);
#else
      drawable = gimp_drawable_get(layer);
      gimp_pixel_rgn_init(&pixel_region, drawable, 0, 0, drawable->width,
			  drawable->height, TRUE, FALSE);
      tile_height = gimp_tile_height();
#endif

      if( pluginwin->get_image_buffer().buf ) {
#if HAVE_GIMP_2_9
	GeglRectangle gegl_rect;
	gegl_rect.x = 0;
	gegl_rect.y = 0;
	gegl_rect.width = width;
	gegl_rect.height = height;
	gegl_buffer_set(buffer, &gegl_rect,
			//GEGL_RECTANGLE(0, 0, width, height),
			0, NULL, pluginwin->get_image_buffer().buf,
			GEGL_AUTO_ROWSTRIDE);
#else
	for (row = 0; row < Crop.height; row += tile_height) {
	  nrows = MIN(Crop.height - row, tile_height);
	  gimp_pixel_rgn_set_rect(&pixel_region,
				  uf->thumb.buffer + 3 * row * Crop.width, 0, row, Crop.width, nrows);
	}
#endif
	status = GIMP_PDB_SUCCESS;
      }

      std::cout<<"PhF plug-in: buffer copied"<<std::endl;

      PF::ImageEditor* pfeditor = pluginwin->get_image_editor();
      g_assert( pfeditor != NULL );
      PF::Image* pfimage = pfeditor->get_image();
      g_assert( pfimage != NULL );
      std::string pfiname = PF::PhotoFlow::Instance().get_cache_dir() + "/gimp_layer.pfi";
      if( pfimage->save(pfiname) ) {
	// Load PFI file into memory
	std::ifstream t;
	std::stringstream strstr;
	t.open( pfiname );
	strstr << t.rdbuf();
	char* buffer = strdup( strstr.str().c_str() );
	/*
	  int length;
	  t.seekg(0,std::ios::end);
	  length = t.tellg();
	  t.seekg(0,std::ios::beg);
	  char* buffer = new char[length+1];
	  t.read( buffer, length );
	  buffer[length] = 0;
	*/
	t.close();

	GimpParasite *cfg_parasite;
	cfg_parasite = gimp_parasite_new("phf-config",
					 GIMP_PARASITE_PERSISTENT, strlen(buffer), buffer);
	gimp_item_attach_parasite(layer, cfg_parasite);
	gimp_parasite_free(cfg_parasite);
      }

#if HAVE_GIMP_2_9
      gegl_buffer_flush(buffer);
#else
      gimp_drawable_flush(drawable);
      gimp_drawable_detach(drawable);
#endif

      //printf("pluginwin->get_image_buffer().exif_buf=%X\n",pluginwin->get_image_buffer().exif_buf);

      if( true ) {
	GimpParasite *exif_parasite;

	std::cout<<"pluginwin->get_image_buffer().exif_buf="<<pluginwin->get_image_buffer().exif_buf<<std::endl;
	if( pluginwin->get_image_buffer().exif_buf ) {
	  gimp_image_set_metadata( gimpImage, pluginwin->get_image_buffer().exif_buf );
	  /*
	    gchar* meta_string = gimp_metadata_serialize( pluginwin->get_image_buffer().exif_buf );
	    if( meta_string ) {
	    exif_parasite = gimp_parasite_new("gimp-image-metadata",
	    GIMP_PARASITE_PERSISTENT, strlen( meta_string ) + 1, meta_string);
	    //#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
	    //      gimp_image_attach_parasite(gimpImage, exif_parasite);
	    //#else
	    gimp_image_parasite_attach(gimpImage, exif_parasite);
	    //#endif
	    //gimp_parasite_free(exif_parasite);
	    g_free( meta_string );
	    }
	  */
	}
	/*
	  #if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
	  {
	  GimpParam    *return_vals;
	  gint          nreturn_vals;
	  return_vals = gimp_run_procedure("plug-in-metadata-decode-exif",
	  &nreturn_vals,
	  GIMP_PDB_IMAGE, gimpImage,
	  GIMP_PDB_INT32, 7,
	  GIMP_PDB_INT8ARRAY, "unused",
	  GIMP_PDB_END);
	  if (return_vals[0].data.d_status != GIMP_PDB_SUCCESS) {
          g_warning("UFRaw Exif -> XMP Merge failed");
	  }
	  }
	  #endif
	*/
      }

      /* Create "icc-profile" parasite from output profile
       * if it is not the internal sRGB.*/
      if( pluginwin->get_image_buffer().iccdata ) {
	printf("Saving ICC profile parasite\n");
	GimpParasite *icc_parasite;
	icc_parasite = gimp_parasite_new("icc-profile",
					 GIMP_PARASITE_PERSISTENT | GIMP_PARASITE_UNDOABLE,
					 pluginwin->get_image_buffer().iccsize,
					 pluginwin->get_image_buffer().iccdata);
	std::cout<<"ICC parasite created"<<std::endl;
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
	gimp_image_attach_parasite(gimpImage, icc_parasite);
#else
	gimp_image_parasite_attach(gimpImage, icc_parasite);
#endif
	gimp_parasite_free(icc_parasite);

	std::cout<<"ICC profile attached"<<std::endl;
      }
    }
    std::cout<<"+++++++++++++++++++++++++++++++++++"<<std::endl;
    std::cout<<"Plug-in: deleting main window"<<std::endl;
    delete pluginwin;
    std::cout<<"Plug-in: main window deleted"<<std::endl;
    std::cout<<"Plug-in: deleting application"<<std::endl;
    std::cout<<"Plug-in: stopping image processor"<<std::endl;
    PF::ProcessRequestInfo request;
    request.request = PF::PROCESSOR_END;
    PF::ImageProcessor::Instance().submit_request( request );
    PF::ImageProcessor::Instance().join();
    std::cout<<"Plug-in: image processor stopped"<<std::endl;
    delete app;
    std::cout<<"Plug-in: application deleted"<<std::endl;
    std::cout<<"Plug-in: closing photoflow"<<std::endl;
    PF::PhotoFlow::Instance().close();
    std::cout<<"Plug-in: photoflow closed"<<std::endl;
  }
  std::cout<<"Plug-in: calling gdk_threads_leave()"<<std::endl;
#ifndef _WIN32
  gdk_threads_leave();
#endif
  std::cout<<"Plug-in: gdk_threads_leave() done"<<std::endl;
  return gimpImage;
}


static gint32
load_thumbnail_image (const gchar   *filename,
                      gint           thumb_size,
                      gint          *width,
                      gint          *height,
                      GError       **error)
{
  gint32  image_ID         = -1;

  return image_ID;
}
