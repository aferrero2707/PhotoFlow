
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

#define VERSION "0.2.5"

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



static const char raw_ext[] = "3fr,ari,arw,cap,cine,cr2,crw,cs1,dc2,dcr,dng,erf,fff,"
    "hdr,ia,iiq,k25,kc2,kdc,mdc,mef,mos,mrw,nef,"
    "nrw,orf,ori,pef,pxn,qtk,r3d,raf,raw,rdc,rw2,rwl,sr2,"
    "srf,srw,sti,pfi,x3f";



static PF::PluginWindow* pluginwin;

void query();
void run(const gchar *name,
    gint nparams,
    const GimpParam *param,
    gint *nreturn_vals,
    GimpParam **return_vals);

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

void run(const gchar *name,
    gint nparams,
    const GimpParam *param,
    gint *nreturn_vals,
    GimpParam **return_vals)
{
  // TODO: Check if the static variable here is really needed.
  // In any case this should cause no issues with threads.
  static GimpParam values[4];
  GimpRunMode run_mode;
  char *filename;
  int size;
  GimpPDBStatusType status;

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
  int gimpImage;

  *nreturn_vals = 1;
  *return_vals = values;

  std::cout<<"pfgimp::run(): name="<<name<<std::endl;

  if (!strcmp(name, "file_pf_load_thumb")) {
    run_mode = (GimpRunMode)0;
    filename = param[0].data.d_string;
    size = param[1].data.d_int32;
  } else if (!strcmp(name, "file_pf_load")) {
    run_mode = (GimpRunMode)param[0].data.d_int32;
    filename = param[1].data.d_string;
    size = 0;
  } else {
    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
#ifndef _WIN32
    gdk_threads_leave();
#endif
    return;
  }
  gboolean loadThumbnail = size > 0;
  /*
    char *gtkrcfile = g_build_filename(uf_get_home_dir(),
                                       ".ufraw-gtkrc", NULL);
    gtk_rc_add_default_file(gtkrcfile);
    g_free(gtkrcfile);
   */
  gimp_ui_init("photoflow-gimp", TRUE);

  //uf = ufraw_open(filename);
  /* if UFRaw fails on jpg/jpeg or tif/tiff then open with GIMP */
  std::cout<<"  filename="<<filename<<std::endl;
  if (true) {
    if (!strcasecmp(filename + strlen(filename) - 4, ".jpg") ||
        !strcasecmp(filename + strlen(filename) - 5, ".jpeg")) {
      if (loadThumbnail)
        *return_vals = gimp_run_procedure2("file_jpeg_load_thumb",
            nreturn_vals, nparams, param);
      else
        *return_vals = gimp_run_procedure2("file_jpeg_load",
            nreturn_vals, nparams, param);
#ifndef _WIN32
      gdk_threads_leave();
#endif
      return;
    } else if (!strcasecmp(filename + strlen(filename) - 4, ".tif") ||
        !strcasecmp(filename + strlen(filename) - 5, ".tiff")) {
      if (!loadThumbnail)
        *return_vals = gimp_run_procedure2("file_tiff_load",
            nreturn_vals, nparams, param);
      else {
        /* There is no "file_tiff_load_thumb".
         * The call to "file_ufraw_load" will handle the thumbnail */
        /* Following is another solution for tiff thumbnails
                GimpParam tiffParam[3];
                tiffParam[0].type = GIMP_PDB_INT32;
                tiffParam[0].data.d_int32 = GIMP_RUN_NONINTERACTIVE;
                tiffParam[1].type = GIMP_PDB_STRING;
                tiffParam[1].data.d_string = filename;
                tiffParam[2].type = GIMP_PDB_STRING;
                tiffParam[2].data.d_string = filename;
         *return_vals = gimp_run_procedure2 ("file_tiff_load",
                    	nreturn_vals, 3, tiffParam);
         */
      }
#ifndef _WIN32
      gdk_threads_leave();
#endif
      return;
/*
    } else {
      // Don't issue a message on thumbnail failure, since ufraw-gimp
      // will be called again with "file_ufraw_load"
      if (loadThumbnail) {
#ifndef _WIN32
        gdk_threads_leave();
#endif
        return;
      }
#ifndef _WIN32
      gdk_threads_leave();
#endif
      return;
*/
    }
  }
  /* Load $HOME/.ufrawrc */
  //conf_load(&rc, NULL);
  /*
    ufraw_config(uf, &rc, NULL, NULL);
    sendToGimpMode = (uf->conf->createID == send_id);
#if !HAVE_GIMP_2_9
    if (loadThumbnail) {
        uf->conf->size = size;
        uf->conf->embeddedImage = TRUE;
    }
#else
    if (run_mode == GIMP_RUN_NONINTERACTIVE) uf->conf->shrink = 8;
#endif
   */
  /* UFRaw already issues warnings.
   * With GIMP_PDB_CANCEL, Gimp won't issue another one. */
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_CANCEL;
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
    }

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

    std::cout<<"+++++++++++++++++++++++++++++++++++"<<std::endl;
    std::cout<<"Plug-in: stopping image processor"<<std::endl;
    PF::ProcessRequestInfo request;
    request.request = PF::PROCESSOR_END;
    PF::ImageProcessor::Instance().submit_request( request );
    PF::ImageProcessor::Instance().join();
    std::cout<<"Plug-in: image processor stopped"<<std::endl;
    std::cout<<"Plug-in: deleting main window"<<std::endl;
    delete pluginwin;
    std::cout<<"Plug-in: main window deleted"<<std::endl;
    std::cout<<"Plug-in: deleting application"<<std::endl;
    delete app;
    std::cout<<"Plug-in: application deleted"<<std::endl;
    std::cout<<"Plug-in: closing photoflow"<<std::endl;
    PF::PhotoFlow::Instance().close();
    std::cout<<"Plug-in: photoflow closed"<<std::endl;

  } else {
    if (sendToGimpMode) {
      char *text = g_strdup_printf(_("Loading raw file '%s'"),
          filename);
      gimp_progress_init(text);
      g_free(text);
    }
    /*
        if (sendToGimpMode) gimp_progress_update(0.1);
        status = ufraw_load_raw(uf);
        if (status != UFRAW_SUCCESS) {
            values[0].type = GIMP_PDB_STATUS;
            values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
#ifndef _WIN32
            gdk_threads_leave();
#endif
            return;
        }
        if (sendToGimpMode) gimp_progress_update(0.3);
        ufraw_save_gimp_image(uf, NULL);
        if (sendToGimpMode) gimp_progress_update(1.0);
        ufraw_close_darkframe(uf->conf);
        ufraw_close(uf);
     */
    /* To make sure we don't delete the raw file by mistake we check
     * that the file is really an ID file. */
    /*
    if (sendToGimpMode &&
        strcasecmp(filename + strlen(filename) - 6, ".ufraw") == 0)
      g_unlink(filename);
    */
  }
  /*
    if (status != UFRAW_SUCCESS || uf->gimpImage == -1) {
        values[0].type = GIMP_PDB_STATUS;
        if (status == UFRAW_CANCEL)
            values[0].data.d_status = GIMP_PDB_CANCEL;
        else
            values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
#ifndef _WIN32
        gdk_threads_leave();
#endif
        return;
    }
   */
  std::cout<<"Plug-in: setting return values"<<std::endl;
  *nreturn_vals = 2;
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_SUCCESS;
  values[1].type = GIMP_PDB_IMAGE;
  values[1].data.d_image = gimpImage;
  if (loadThumbnail) {
    *nreturn_vals = 4;
    values[2].type = GIMP_PDB_INT32;
    //values[2].data.d_int32 = uf->initialWidth;
    values[3].type = GIMP_PDB_INT32;
    //values[3].data.d_int32 = uf->initialHeight;
  }
  std::cout<<"Plug-in: return values done"<<std::endl;
  std::cout<<"Plug-in: calling gdk_threads_leave()"<<std::endl;
#ifndef _WIN32
  gdk_threads_leave();
#endif
  std::cout<<"Plug-in: gdk_threads_leave() done"<<std::endl;
  return;
}


#ifdef UFRAW_SOURCE

int gimp_row_writer(ufraw_data *uf, void *volatile out, void *pixbuf,
    int row, int width, int height, int grayscale, int bitDepth)
{
  (void)uf;
  (void)grayscale;
  (void)bitDepth;

#if HAVE_GIMP_2_9
  gegl_buffer_set(out, GEGL_RECTANGLE(0, row, width, height),
      0, NULL, pixbuf,
      GEGL_AUTO_ROWSTRIDE);
#else
  gimp_pixel_rgn_set_rect(out, pixbuf, 0, row, width, height);
#endif

  return UFRAW_SUCCESS;
}

long ufraw_save_gimp_image(ufraw_data *uf, GtkWidget *widget)
{
#if HAVE_GIMP_2_9
  GeglBuffer *buffer;
#else
  GimpDrawable *drawable;
  GimpPixelRgn pixel_region;
  int tile_height, row, nrows;
#endif
  gint32 layer;
  UFRectangle Crop;
  int depth;
  (void)widget;

  uf->gimpImage = -1;

  if (uf->conf->embeddedImage) {
    if (ufraw_convert_embedded(uf) != UFRAW_SUCCESS)
      return UFRAW_ERROR;
    Crop.height = uf->thumb.height;
    Crop.width = uf->thumb.width;
    Crop.y = 0;
    Crop.x = 0;
    depth = 3;
  } else {
    if (ufraw_convert_image(uf) != UFRAW_SUCCESS)
      return UFRAW_ERROR;
    ufraw_get_scaled_crop(uf, &Crop);
#if HAVE_GIMP_2_9
    if (uf->conf->profile[out_profile]
                          [uf->conf->profileIndex[out_profile]].BitDepth == 16)
      depth = 6;
    else
      depth = 3;
#else
    depth = 3;
#endif
  }
#if HAVE_GIMP_2_9
  uf->gimpImage =
      gimp_image_new_with_precision(Crop.width, Crop.height, GIMP_RGB,
          depth == 3 ? GIMP_PRECISION_U8_GAMMA :
              GIMP_PRECISION_U16_GAMMA);
#else
  uf->gimpImage = gimp_image_new(Crop.width, Crop.height, GIMP_RGB);
#endif
  if (uf->gimpImage == -1) {
    ufraw_message(UFRAW_ERROR, _("Can't allocate new image."));
    return UFRAW_ERROR;
  }
  gimp_image_set_filename(uf->gimpImage, uf->filename);

  /* Create the "background" layer to hold the image... */
  layer = gimp_layer_new(uf->gimpImage, _("Background"), Crop.width,
      Crop.height, GIMP_RGB_IMAGE, 100.0,
      GIMP_NORMAL_MODE);
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,7,3)
  gimp_image_insert_layer(uf->gimpImage, layer, 0, 0);
#else
  gimp_image_add_layer(uf->gimpImage, layer, 0);
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

  if (uf->conf->embeddedImage) {
#if HAVE_GIMP_2_9
    gegl_buffer_set(buffer,
        GEGL_RECTANGLE(0, 0, Crop.width, Crop.height),
        0, NULL, uf->thumb.buffer,
        GEGL_AUTO_ROWSTRIDE);
#else
    for (row = 0; row < Crop.height; row += tile_height) {
      nrows = MIN(Crop.height - row, tile_height);
      gimp_pixel_rgn_set_rect(&pixel_region,
          uf->thumb.buffer + 3 * row * Crop.width, 0, row, Crop.width, nrows);
    }
#endif
  } else {
#if HAVE_GIMP_2_9
    ufraw_write_image_data(uf, buffer, &Crop, depth == 3 ? 8 : 16, 0,
        gimp_row_writer);
#else
    ufraw_write_image_data(uf, &pixel_region, &Crop, depth == 3 ? 8 : 16, 0,
        gimp_row_writer);
#endif
  }
#if HAVE_GIMP_2_9
  gegl_buffer_flush(buffer);
#else
  gimp_drawable_flush(drawable);
  gimp_drawable_detach(drawable);
#endif

  if (uf->conf->embeddedImage) return UFRAW_SUCCESS;

  ufraw_exif_prepare_output(uf);
  if (uf->outputExifBuf != NULL) {
    if (uf->outputExifBufLen > 65533) {
      ufraw_message(UFRAW_SET_WARNING,
          _("EXIF buffer length %d, too long, ignored."),
          uf->outputExifBufLen);
    } else {
      GimpParasite *exif_parasite;

      exif_parasite = gimp_parasite_new("exif-data",
          GIMP_PARASITE_PERSISTENT, uf->outputExifBufLen, uf->outputExifBuf);
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
      gimp_image_attach_parasite(uf->gimpImage, exif_parasite);
#else
      gimp_image_parasite_attach(uf->gimpImage, exif_parasite);
#endif
      gimp_parasite_free(exif_parasite);

#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
      {
        GimpParam    *return_vals;
        gint          nreturn_vals;
        return_vals = gimp_run_procedure("plug-in-metadata-decode-exif",
            &nreturn_vals,
            GIMP_PDB_IMAGE, uf->gimpImage,
            GIMP_PDB_INT32, 7,
            GIMP_PDB_INT8ARRAY, "unused",
            GIMP_PDB_END);
        if (return_vals[0].data.d_status != GIMP_PDB_SUCCESS) {
          g_warning("UFRaw Exif -> XMP Merge failed");
        }
      }
#endif
    }
  }
  /* Create "icc-profile" parasite from output profile
   * if it is not the internal sRGB.*/
  if (strcmp(uf->developer->profileFile[out_profile], "")) {
    char *buf;
    gsize len;
    if (g_file_get_contents(uf->developer->profileFile[out_profile],
        &buf, &len, NULL)) {
      GimpParasite *icc_parasite;
      icc_parasite = gimp_parasite_new("icc-profile",
          GIMP_PARASITE_PERSISTENT, len, buf);
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
      gimp_image_attach_parasite(uf->gimpImage, icc_parasite);
#else
      gimp_image_parasite_attach(uf->gimpImage, icc_parasite);
#endif
      gimp_parasite_free(icc_parasite);
      g_free(buf);
    } else {
      ufraw_message(UFRAW_WARNING,
          _("Failed to embed output profile '%s' in image."),
          uf->developer->profileFile[out_profile]);
    }
  }
  return UFRAW_SUCCESS;
}


#endif
