
#include "pluginwindow.hh"
#include "../base/new_operation.hh"
#include "../base/imageprocessor.hh"
#include "../base/pf_file_loader.hh"
#if HAVE_GIMP_2_9
#include <gegl.h>
#endif
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <libgimpbase/gimpbase.h>
#include <libgimpwidgets/gimpwidgets.h>
#include <glib/gi18n.h>
#include <string.h>

#include <fstream>

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



// Manage different versions of the GIMP API.
#define _gimp_item_is_valid gimp_item_is_valid
#define _gimp_image_get_item_position gimp_image_get_item_position
#if GIMP_MINOR_VERSION<=8
#define _gimp_item_get_visible gimp_drawable_get_visible
#else
#define _gimp_item_get_visible gimp_item_get_visible
#endif

//static GimpPDBStatusType status = GIMP_PDB_SUCCESS;   // The plug-in return status.

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
  static const GimpParamDef args[] = {
    {GIMP_PDB_INT32,    (gchar*)"run_mode", (gchar*)"Interactive, non-interactive"},
    {GIMP_PDB_IMAGE,    (gchar*)"image",    (gchar*)"Input image"},
    {GIMP_PDB_DRAWABLE, (gchar*)"drawable", (gchar*)"Input drawable (unused)"},
  };

  gimp_install_procedure("plug-in-photoflow",             // name
                         "PhotoFlow",                    // blurb
                         "PhotoFlow",                    // help
                         "Andrea Ferrero", // author
                         "Andrea Ferrero", // copyright
                         "2016",                     // date
                         "_PhotoFlow...",                // menu_path
                         "RGB*",              // image_types
                         GIMP_PLUGIN,                // type
                         G_N_ELEMENTS(args),         // nparams
                         0,                          // nreturn_vals
                         args,                       // params
                         0);                         // return_vals

  gimp_plugin_menu_register("plug-in-photoflow", "<Image>/Filters");
}

char *pf_binary;
gboolean sendToGimpMode;


static int
edit_current_layer_dialog()
{
  GtkWidget        *dialog;
  GtkWidget        *hbox;
  GtkWidget        *image;
  GtkWidget        *main_vbox;
  GtkWidget        *label;
  gchar            *text;
  int  retval;

  GtkDialogFlags flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
  dialog = gimp_dialog_new (_("Edit Current Layer"), "pfgimp-edit-current-layer-confirm",
                            NULL, flags,
                            gimp_standard_help_func,
                            "pfgimp-edit-current-layer-confirm-dialog",

                            _("Create new"), GTK_RESPONSE_CANCEL,
                            _("Edit current"),     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gimp_window_set_transient (GTK_WINDOW (dialog));

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      hbox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_widget_show (hbox);

  image = gtk_image_new_from_icon_name ("dialog-warning",
                                        GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.0);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  main_vbox = gtk_vbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (hbox), main_vbox, FALSE, FALSE, 0);
  gtk_widget_show (main_vbox);

  text = g_strdup ("This is a PhotoFlow layer.\nDo you want to continue\nediting this layer\nor create a new one?");
  label = gtk_label_new (text);
  g_free (text);

  gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (main_vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_widget_show (dialog);

  switch (gimp_dialog_run (GIMP_DIALOG (dialog)))
    {
    case GTK_RESPONSE_OK:
      retval = Gtk::RESPONSE_YES;
      break;

    default:
      retval = Gtk::RESPONSE_NO;
      break;
    }

  gtk_widget_destroy (dialog);

  return retval;
}


void run(const gchar *name,
    gint nparams,
    const GimpParam *param,
    gint *nreturn_vals,
    GimpParam **return_vals)
{
  GimpRunMode run_mode = (GimpRunMode)param[0].data.d_int32;

  int size;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;

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
  //gint32 layer;

  static GimpParam return_values[1];
  *return_vals  = return_values;
  *nreturn_vals = 1;
  return_values[0].type = GIMP_PDB_STATUS;

  gimp_ui_init("pfgimp", FALSE);

  int image_id = param[1].data.d_drawable;
#if GIMP_MINOR_VERSION<=8
  gimp_tile_cache_ntiles(2*(gimp_image_width(image_id)/gimp_tile_width() + 1));
#endif

  Gtk::Main* app = new Gtk::Main(0, NULL);

  // Retrieve the list of desired layers.
  int
  nb_layers = 0,
  *layers = gimp_image_get_layers(image_id,&nb_layers),
  active_layer_id = gimp_image_get_active_layer(image_id);
  int source_layer_id = active_layer_id;

  GimpParasite *phf_parasite = gimp_item_get_parasite( active_layer_id, "phf-config" );
  std::cout<<"PhF plug-in: phf_parasite="<<phf_parasite<<std::endl;
  bool replace_layer = false;
  std::string pfiname;
  if( phf_parasite && gimp_parasite_data_size( phf_parasite ) > 0 &&
      gimp_parasite_data( phf_parasite ) != NULL ) {

    /*
    char tstr[501];
    snprintf( tstr, 500, _("PhF editing config detected.\nDo you want to continue editing the current layer?") );
    Gtk::MessageDialog dialog(tstr,
        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
    //dialog.set_transient_for(*this);
    dialog.set_default_response( Gtk::RESPONSE_YES );

    //Show the dialog and wait for a user response:
    int result = dialog.run();
    */
    int result = edit_current_layer_dialog();

    //Handle the response:
    switch(result) {
    case Gtk::RESPONSE_YES: {
      glong size = gimp_parasite_data_size( phf_parasite );
      pfiname = PF::PhotoFlow::Instance().get_cache_dir() + "/gimp_layer.pfi";
      std::ofstream t;
      t.open( pfiname );
      t.write( (const char*)(gimp_parasite_data( phf_parasite )), size );
      t.close();
      replace_layer = true;
      break;
    }
    case Gtk::RESPONSE_NO:
      break;
    }
  }

  if( replace_layer ) {
    if (active_layer_id>=0) {
      int i = 0; for (i = 0; i<nb_layers; ++i) if (layers[i]==active_layer_id) break;
      if (i<nb_layers - 1) source_layer_id = layers[i + 1];
      else source_layer_id = -1;
    }
  }
  std::cout<<"PhF plug-in: pfiname="<<pfiname<<"  replace_layer="<<replace_layer<<"  source_layer_id="<<source_layer_id<<std::endl;

  gimp_ui_init("photoflow-gimp", TRUE);

  //GimpParasite *exif_parasite = gimp_image_parasite_find( image_id, "gimp-image-metadata" );
  GimpMetadata* exif_metadata = gimp_image_get_metadata( image_id );
  GimpParasite *icc_parasite  = gimp_image_parasite_find( image_id, "icc-profile" );
  glong iccsize = 0;
  void* iccdata = NULL;

  std::cout<<std::endl<<std::endl
      <<"image_id: "<<image_id
      <<"  ICC parasite: "<<icc_parasite
      <<"  EXIF metadata: "<<exif_metadata
      <<std::endl<<std::endl;

  if( icc_parasite && gimp_parasite_data_size( icc_parasite ) > 0 &&
      gimp_parasite_data( icc_parasite ) != NULL ) {
    iccsize = gimp_parasite_data_size( icc_parasite );
    iccdata = malloc( iccsize );
    memcpy( iccdata, gimp_parasite_data( icc_parasite ), iccsize );
  }

  std::string filename;
  cmsBool is_lin_gamma = false;
  std::string format = "R'G'B' float";

  if( source_layer_id >= 0 ) {
    // Get input buffer
    gint rgn_x, rgn_y, rgn_width, rgn_height;
    if (!_gimp_item_is_valid(source_layer_id)) return;
    if (!gimp_drawable_mask_intersect(source_layer_id,&rgn_x,&rgn_y,&rgn_width,&rgn_height)) return;
    const int spectrum = (gimp_drawable_is_rgb(source_layer_id)?3:1) +
        (gimp_drawable_has_alpha(source_layer_id)?1:0);
    float* inbuf = (float*)malloc( sizeof(float)*3*rgn_width*rgn_height );
#if GIMP_MINOR_VERSION<=8
    GimpDrawable *drawable = gimp_drawable_get(source_layer_id);
    GimpPixelRgn region;
    gimp_pixel_rgn_init(&region,drawable,rgn_x,rgn_y,rgn_width,rgn_height,false,false);
    guchar* row = g_new(guchar,rgn_width*spectrum), *ptrs = 0;
    switch (spectrum) {
    case 1 : {
      float* ptr_r = inbuf;
      for( int y = 0; y < rgn_height; y++ ) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,rgn_x,rgn_y + y,rgn_width);
        for( int x = 0; x < rgn_width; x++ ) {
          float val = ((float)*(ptrs++))/255;
          *(ptr_r++) = val;
          *(ptr_r++) = val;
          *(ptr_r++) = val;
        }
      }
    } break;
    case 2 : {
      float* ptr_r = inbuf;
      for( int y = 0; y < rgn_height; y++ ) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,rgn_x,rgn_y + y,rgn_width);
        for( int x = 0; x < rgn_width; x++ ) {
          float val = ((float)*(ptrs++))/255; ptrs++;
          *(ptr_r++) = val;
          *(ptr_r++) = val;
          *(ptr_r++) = val;
        }
      }
    } break;
    case 3 : {
      float* ptr_r = inbuf;
      for( int y = 0; y < rgn_height; y++ ) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,rgn_x,rgn_y + y,rgn_width);
        for( int x = 0; x < rgn_width; x++ ) {
          *(ptr_r++) = ((float)*(ptrs++))/255;
          *(ptr_r++) = ((float)*(ptrs++))/255;
          *(ptr_r++) = ((float)*(ptrs++))/255;
        }
      }
    } break;
    case 4 : {
      float* ptr_r = inbuf;
      for( int y = 0; y < rgn_height; y++ ) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,rgn_x,rgn_y + y,rgn_width);
        for( int x = 0; x < rgn_width; x++ ) {
          *(ptr_r++) = ((float)*(ptrs++))/255;
          *(ptr_r++) = ((float)*(ptrs++))/255;
          *(ptr_r++) = ((float)*(ptrs++))/255; ptrs++;
        }
      }
    } break;
    }
    g_free(row);
    gimp_drawable_detach(drawable);
#else
    if( iccdata ) {
      cmsHPROFILE iccprofile = cmsOpenProfileFromMem( iccdata, iccsize );
      if( iccprofile ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(iccprofile, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<std::endl<<std::endl<<"embedded profile: "<<tstr<<std::endl<<std::endl;
        cmsToneCurve *red_trc   = (cmsToneCurve*)cmsReadTag(iccprofile, cmsSigRedTRCTag);
        is_lin_gamma = cmsIsToneCurveLinear(red_trc);
        cmsCloseProfile( iccprofile );
      }
    }

    GeglRectangle rect;
    gegl_rectangle_set(&rect,rgn_x,rgn_y,rgn_width,rgn_height);
    buffer = gimp_drawable_get_buffer(source_layer_id);
    format = is_lin_gamma ? "RGB float" : "R'G'B' float";
    gegl_buffer_get(buffer,&rect,1,babl_format(format.c_str()),inbuf,0,GEGL_ABYSS_NONE);
    g_object_unref(buffer);
#endif

    VipsImage* input_img =
        vips_image_new_from_memory( inbuf, sizeof(float)*3*rgn_width*rgn_height,
            rgn_width, rgn_height, 3, VIPS_FORMAT_FLOAT );

    if( icc_parasite && gimp_parasite_data_size( icc_parasite ) > 0 &&
        gimp_parasite_data( icc_parasite ) != NULL ) {
      glong iccsize = gimp_parasite_data_size( icc_parasite );
      void* iccdata = malloc( iccsize );
      memcpy( iccdata, gimp_parasite_data( icc_parasite ), iccsize );

      vips_image_set_blob( input_img, VIPS_META_ICC_NAME,
          (VipsCallbackFn) g_free, iccdata, iccsize );
    }

    filename = PF::PhotoFlow::Instance().get_cache_dir() + "/gimp_layer.tif";
    vips_tiffsave( input_img, filename.c_str(), "compression", VIPS_FOREIGN_TIFF_COMPRESSION_NONE,
        "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_HORIZONTAL, NULL );

    g_object_unref( input_img );

    //if( exif_parasite ) {
    //  std::cout<<"Metadata:"<<std::endl<<(gchar*)gimp_parasite_data(exif_parasite)<<std::endl;
    //  GimpMetadata* exif_metadata = gimp_metadata_deserialize( (gchar*)gimp_parasite_data(exif_parasite) );
      if( exif_metadata ) {
        gexiv2_metadata_save_file( /*(GExiv2Metadata*)*/exif_metadata, filename.c_str(), NULL );
        g_object_unref( exif_metadata );
      }
    //}
  }

  //gimp_parasite_free(exif_parasite);
  //gimp_parasite_free(icc_parasite);

  std::cout<<"plug-in: run_mode="<<run_mode<<"  GIMP_RUN_INTERACTIVE="<<GIMP_RUN_INTERACTIVE<<std::endl;

  /* BUG - what should be done with GIMP_RUN_WITH_LAST_VALS */
  if (run_mode == GIMP_RUN_INTERACTIVE) {
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
    std::cout<<"  filename="<<filename<<"  pfiname="<<pfiname<<std::endl;
    if( !filename.empty() ) {
      pluginwin->open_image( filename, true );
      if( !pfiname.empty() ) {
        g_assert( pluginwin->get_image_editor() != NULL );
        PF::ImageEditor* pfeditor = pluginwin->get_image_editor();
        g_assert( pfeditor != NULL );
        PF::Image* pfimage = pfeditor->get_image();
        g_assert( pfimage != NULL );
        PF::insert_pf_preset( pfiname, pfimage, NULL, &(pfimage->get_layer_manager().get_layers()), false );
      }
    } else if( !pfiname.empty() ) {
      pluginwin->open_image( pfiname, false );
    }
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

    // Transfer the output layers back into GIMP.
    GimpLayerModeEffects layer_blendmode = GIMP_NORMAL_MODE;
    gint layer_posx = 0, layer_posy = 0;
    double layer_opacity = 100;

    gint32 dest_layer_id = active_layer_id;
    if( !replace_layer ) {
      /* Create the "background" layer to hold the image... */
      gint32 layer = gimp_layer_new(image_id, _("PhF output"), width,
          height, GIMP_RGB_IMAGE, 100.0,
          GIMP_NORMAL_MODE);
      std::cout<<"PhF plug-in: new layer created"<<std::endl;
#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,7,3)
      gimp_image_insert_layer(image_id, layer, 0, -1);
#else
      gimp_image_add_layer(image_id, layer, -1);
#endif
      std::cout<<"PhF plug-in: new layer added"<<std::endl;
      dest_layer_id = layer;
    }
    /* Get the drawable and set the pixel region for our load... */
#if HAVE_GIMP_2_9
    buffer = gimp_drawable_get_buffer(dest_layer_id);
#else
    drawable = gimp_drawable_get(dest_layer_id);
    gimp_pixel_rgn_init(&pixel_region, drawable, 0, 0, drawable->width,
        drawable->height, TRUE, FALSE);
    tile_height = gimp_tile_height();
#endif

    if( pluginwin->get_image_buffer().buf ) {
      std::cout<<"PhF plug-in: copying buffer..."<<std::endl;
#if HAVE_GIMP_2_9
      format = is_lin_gamma ? "RGB float" : "R'G'B' float";
      GeglRectangle gegl_rect;
      gegl_rect.x = 0;
      gegl_rect.y = 0;
      gegl_rect.width = width;
      gegl_rect.height = height;
      gegl_buffer_set(buffer, &gegl_rect,
          //GEGL_RECTANGLE(0, 0, width, height),
          0, babl_format(format.c_str()), pluginwin->get_image_buffer().buf,
          GEGL_AUTO_ROWSTRIDE);
      g_object_unref(buffer);
      //gimp_drawable_merge_shadow(layer_id,true);
      gimp_drawable_update(dest_layer_id,0,0,width,height);
      gimp_layer_resize(dest_layer_id,width,height,0,0);
#else
      for (row = 0; row < Crop.height; row += tile_height) {
        nrows = MIN(Crop.height - row, tile_height);
        gimp_pixel_rgn_set_rect(&pixel_region,
            uf->thumb.buffer + 3 * row * Crop.width, 0, row, Crop.width, nrows);
      }
#endif
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
        gimp_item_attach_parasite(dest_layer_id, cfg_parasite);
        gimp_parasite_free(cfg_parasite);
      }
    }

#if HAVE_GIMP_2_9
    gegl_buffer_flush(buffer);
#else
    gimp_drawable_flush(drawable);
    gimp_drawable_detach(drawable);
#endif

    //printf("pluginwin->get_image_buffer().exif_buf=%X\n",pluginwin->get_image_buffer().exif_buf);

    if( false ) {
      GimpParasite *exif_parasite;

      exif_parasite = gimp_parasite_new("exif-data",
          GIMP_PARASITE_PERSISTENT, sizeof( GExiv2Metadata ),
          pluginwin->get_image_buffer().exif_buf);
//#if defined(GIMP_CHECK_VERSION) && GIMP_CHECK_VERSION(2,8,0)
//      gimp_image_attach_parasite(gimpImage, exif_parasite);
//#else
      gimp_image_parasite_attach(image_id, exif_parasite);
//#endif
      gimp_parasite_free(exif_parasite);
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

    if( false ) {
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
        gimp_image_attach_parasite(image_id, icc_parasite);
#else
        gimp_image_parasite_attach(image_id, icc_parasite);
#endif
        gimp_parasite_free(icc_parasite);

        std::cout<<"ICC profile attached"<<std::endl;
      }
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
    std::cout<<"plug-in: execution skipped"<<std::endl;
  }

  gimp_displays_flush();

  std::cout<<"Closing PhotoFlow plug-in"<<std::endl;

  std::cout<<"Plug-in: setting return values"<<std::endl;
  return_values[0].data.d_status = status;
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
