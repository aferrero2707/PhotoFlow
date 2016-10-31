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
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>

#include <gdk/gdk.h>

#include "../base/pf_mkstemp.hh"
#include "../base/imageprocessor.hh"
#include "imageeditor.hh"



void PF::PreviewScrolledWindow::on_map()
{
  std::cout<<"PreviewScrolledWindow::on_map() called."<<std::endl;
  Gtk::ScrolledWindow::on_map();
}


bool PF::PreviewScrolledWindow::on_configure_event(GdkEventConfigure*event)
{
  std::cout<<"PreviewScrolledWindow::on_configure_event() called."<<std::endl;
  Gtk::ScrolledWindow::on_configure_event(event);
  return false;
}


PF::ImageSizeUpdater::ImageSizeUpdater( Pipeline* v ):
  PipelineSink( v ),
  displayed_layer_id( -1 ),
  image( NULL ),
  image_width( 0 ),
  image_height( 0 )
{
}


void PF::ImageSizeUpdater::update( VipsRect* area )
{
  if( !get_pipeline() ) {
    std::cout<<"ImageSizeUpdater::update(): error: NULL pipeline"<<std::endl;
    return;
  }
  if( !get_pipeline()->get_output() ) {
    std::cout<<"ImageSizeUpdater::update(): error: NULL image"<<std::endl;
    return;
  }

  image = NULL;
  bool do_merged = (displayed_layer_id<0) ? true : false;
  if( !do_merged ) {
    PF::PipelineNode* node = get_pipeline()->get_node( displayed_layer_id );
    if( !node ) do_merged = true;
    //std::cout<<"ImageArea::update(): node="<<node<<std::endl;
    if( get_pipeline()->get_image() ) {
      PF::Layer* temp_layer = get_pipeline()->get_image()->get_layer_manager().get_layer( displayed_layer_id );
      if( !temp_layer ) do_merged = true;
      if( !(temp_layer->is_visible()) ) do_merged = true;
    }
  }
  if( do_merged ) {
    image = get_pipeline()->get_output();
  } else {
    PF::PipelineNode* node = get_pipeline()->get_node( displayed_layer_id );
    if( !node ) return;
    if( !(node->blended) ) return;
    image = node->blended;
  }

  if( image ) {
    #ifdef DEBUG_DISPLAY
    std::cout<<"ImageSizeUpdater::update(): image->Bands="<<image->Bands<<std::endl;
    std::cout<<"ImageSizeUpdater::update(): image->BandFmt="<<image->BandFmt<<std::endl;
    std::cout<<"ImageSizeUpdater::update(): image size: "<<image->Xsize<<"x"<<image->Ysize<<std::endl;
    #endif
    image_width = image->Xsize;
    image_height = image->Ysize;
  }
}



typedef struct {
  PF::ImageEditor* editor;
} EditorCBData;


static gboolean editor_on_image_modified_cb ( EditorCBData* data)
{
  if( data ) {
    data->editor->on_image_modified();
    g_free( data );
  }
  return false;
}


static gboolean editor_on_image_updated_cb ( EditorCBData* data)
{
  if( data ) {
    data->editor->on_image_updated();
    g_free( data );
  }
  return false;
}




class Layout2: public Gtk::HBox
{
  Gtk::Widget* histogram_widget;
  Gtk::Widget* buttons_widget;
  Gtk::Widget* layers_widget;
  Gtk::Widget* controls_widget;
  Gtk::Widget* preview_widget;

  Gtk::HBox hbox;
  Gtk::VBox vbox;
  Gtk::VPaned paned;

public:
  Layout2(Gtk::Widget* h, Gtk::Widget* b, Gtk::Widget* l, Gtk::Widget* c, Gtk::Widget* p ): Gtk::HBox(),
  histogram_widget(h), buttons_widget(b), layers_widget(l),
  controls_widget(c), preview_widget(p)
  //paned(Gtk::ORIENTATION_VERTICAL)
  {
    paned.add1( *layers_widget );
    paned.add2( *controls_widget );

    hbox.pack_start( *buttons_widget, Gtk::PACK_SHRINK );
    hbox.pack_start( paned, Gtk::PACK_EXPAND_WIDGET );

    vbox.pack_start( *histogram_widget, Gtk::PACK_SHRINK );
    vbox.pack_start( hbox, Gtk::PACK_EXPAND_WIDGET );

    pack_start( vbox, Gtk::PACK_SHRINK );
    pack_start( *preview_widget, Gtk::PACK_EXPAND_WIDGET );

    paned.set_position(150);
}
};



PF::ImageEditor::ImageEditor( std::string fname ):
  filename( fname ),
  image( new PF::Image() ),
  image_opened( false ),
  displayed_layer( NULL ),
  active_layer( NULL ),
  //imageArea( image->get_pipeline(PREVIEW_PIPELINE_ID) ),
  layersWidget( image, this ),
  aux_controls( NULL ),
  img_zoom_in(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-zoom-in.png"),
  img_zoom_out(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-zoom-out.png"),
  img_zoom_fit(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-zoom-fit.png"),
  //buttonZoomIn( "Zoom +" ),
  //buttonZoomOut( "Zoom -" ),
  buttonZoom100( "1:1" ),
  //buttonZoomFit( "Fit" ),
  img_highlights_warning(PF::PhotoFlow::Instance().get_data_dir()+"/icons/highlights_clip_warning.png"),
  img_shadows_warning(PF::PhotoFlow::Instance().get_data_dir()+"/icons/shadows_clip_warning.png"),
  buttonShowMerged( _("show merged layers") ),
  buttonShowActive( _("show active layer") ),
  tab_label_widget( NULL ),
  fit_image( true ),
  fit_image_needed( true ),
  hide_background_layer( false )
{
  std::cout<<"img_zoom_in: "<<PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-zoom-in.png"<<std::endl;
  // First pipeline is for full-res rendering, second one is for on-screen preview, third one is
  // for calculating the histogram
  image->add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_NORMAL );
  image->add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_PREVIEW );
  PF::Pipeline* p = image->add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_PREVIEW );
  if( p ) {
    p->set_auto_zoom( true, 256, 256 );
  }

  image_size_updater = new PF::ImageSizeUpdater( image->get_pipeline(0) );

  PF::PhotoFlow::Instance().set_preview_pipeline_id(PREVIEW_PIPELINE_ID);
  imageArea = new PF::ImageArea( image->get_pipeline(PREVIEW_PIPELINE_ID) );

  imageArea->set_adjustments( imageArea_scrolledWindow.get_hadjustment(),
			     imageArea_scrolledWindow.get_vadjustment() );

  imageArea_eventBox.add( *imageArea );

  imageArea_hbox.pack_start( imageArea_eventBox, Gtk::PACK_EXPAND_PADDING );
  imageArea_vbox.pack_start( imageArea_hbox, Gtk::PACK_EXPAND_PADDING );

#ifdef GTKMM_2
  Gdk::Color bg; bg.set_rgb_p(0.1,0.1,0.1);
  imageArea_eventBox2.modify_bg(Gtk::STATE_NORMAL,bg);
#endif
  imageArea_eventBox2.add(imageArea_vbox);

  imageArea_scrolledWindow.add( imageArea_eventBox2 );
  imageArea_scrolledWindow.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
  imageArea_scrolledWindow_box.pack_start( imageArea_scrolledWindow );

  histogram = new PF::Histogram( image->get_pipeline(HISTOGRAM_PIPELINE_ID) );


  radioBox.pack_start( buttonShowMerged );
  radioBox.pack_start( buttonShowActive );

  Gtk::RadioButton::Group group = buttonShowMerged.get_group();
  buttonShowActive.set_group(group);

  button_highlights_warning.set_image( img_highlights_warning );
  button_highlights_warning.set_tooltip_text( _("Toggle highlights clipping warning on/off") );
  button_highlights_warning.set_size_request(35,26);
  controlsBox.pack_end( button_highlights_warning, Gtk::PACK_SHRINK );
  button_shadows_warning.set_image( img_shadows_warning );
  button_shadows_warning.set_tooltip_text( _("Toggle shadows clipping warning on/off") );
  button_shadows_warning.set_size_request(35,26);
  controlsBox.pack_end( button_shadows_warning, Gtk::PACK_SHRINK );

  controlsBox.set_spacing(2);
  controlsBox.set_border_width(2);
  buttonZoom100.set_tooltip_text( _("Zoom to 100%") );
  buttonZoom100.set_size_request(26,0);
  controlsBox.pack_end( buttonZoom100, Gtk::PACK_SHRINK );
  buttonZoomFit.set_image( img_zoom_fit );
  buttonZoomFit.set_tooltip_text( _("Fit image to preview area") );
  buttonZoomFit.set_size_request(26,0);
  controlsBox.pack_end( buttonZoomFit, Gtk::PACK_SHRINK );
  buttonZoomOut.set_image( img_zoom_out );
  buttonZoomOut.set_tooltip_text( _("Zoom out") );
  buttonZoomOut.set_size_request(26,0);
  controlsBox.pack_end( buttonZoomOut, Gtk::PACK_SHRINK );
  buttonZoomIn.set_image( img_zoom_in );
  buttonZoomIn.set_tooltip_text( _("Zoom in") );
  buttonZoomIn.set_size_request(26,0);
  controlsBox.pack_end( buttonZoomIn, Gtk::PACK_SHRINK );
  controlsBox.pack_end( status_indicator, Gtk::PACK_SHRINK );

  //imageBox.pack_start( imageArea_eventBox );
  imageBox.pack_start( imageArea_scrolledWindow_box );
  imageBox.pack_start( controlsBox, Gtk::PACK_SHRINK );

  hist_expander.set_label( _("histogram") );
  hist_expander.set_expanded(true);
  hist_expander.add(*histogram);

  //layersWidget_box.pack_start( hist_expander, Gtk::PACK_SHRINK );
  aux_controlsBox.set_size_request(-1,70);
  //layersWidget_box.pack_start( aux_controlsBox, Gtk::PACK_SHRINK );
  layersWidget_box.pack_start( layersWidget, Gtk::PACK_EXPAND_WIDGET );

  controls_group_scrolled_window.add( layersWidget.get_controls_group() );
  controls_group_scrolled_window.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  controls_group_scrolled_window.set_size_request( 280, 0 );

  //controls_group_vbox.pack_start( hist_expander, Gtk::PACK_SHRINK );
  //controls_group_vbox.pack_start( controls_group_scrolled_window, Gtk::PACK_EXPAND_WIDGET );

  main_panel = new Layout2( &hist_expander, &(layersWidget.get_tool_buttons_box()),
      &layersWidget_box, &controls_group_scrolled_window, &imageBox );

  pack_start( *main_panel );
  //main_panel.pack1( imageBox );
  //main_panel.pack2( layersWidget, false, false );
  //main_panel.pack_start( controls_group_scrolled_window, Gtk::PACK_SHRINK );
  //main_panel.pack_start( layersWidget.get_controls_group(), Gtk::PACK_SHRINK );

  //main_panel.pack_start( layersWidget_box, Gtk::PACK_SHRINK );
  //main_panel.pack_start( imageBox, Gtk::PACK_EXPAND_WIDGET );
  //main_panel.pack_start( controls_group_vbox, Gtk::PACK_SHRINK );

  button_highlights_warning.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::ImageEditor::toggle_highlights_warning) );
  button_shadows_warning.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::ImageEditor::toggle_shadows_warning) );

  buttonZoomIn.signal_clicked().connect( sigc::mem_fun(*this,
						       &PF::ImageEditor::zoom_in) );
  buttonZoomOut.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_out) );
  buttonZoom100.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_actual_size) );
  buttonZoomFit.signal_clicked().connect( sigc::hide_return( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_fit) ) );

  /*
  buttonShowMerged.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged), true) );
  buttonShowActive.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged), false) );
  //set_position( get_allocation().get_width()-200 );
  */
  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(imageArea,
								  &PF::ImageArea::set_edited_layer) );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(this,
								  &PF::ImageEditor::set_active_layer) );

  //imageArea_eventBox.add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  | Gdk::POINTER_MOTION_HINT_MASK | Gdk::STRUCTURE_MASK );
  //main_panel.set_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  | Gdk::POINTER_MOTION_HINT_MASK | Gdk::STRUCTURE_MASK );
  imageArea_eventBox.set_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  /*| Gdk::POINTER_MOTION_HINT_MASK*/ | Gdk::STRUCTURE_MASK );
  imageArea_eventBox.signal_button_press_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::my_button_press_event) );
  imageArea_eventBox.signal_button_release_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::my_button_release_event) );
  imageArea_eventBox.signal_motion_notify_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::my_motion_notify_event) );


  //imageArea->add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  | Gdk::POINTER_MOTION_HINT_MASK | Gdk::STRUCTURE_MASK );
  imageArea_scrolledWindow.add_events( Gdk::STRUCTURE_MASK );
  imageArea_scrolledWindow_box.add_events( Gdk::STRUCTURE_MASK );
	imageArea_scrolledWindow.signal_size_allocate().
		connect( sigc::mem_fun(*this, &PF::ImageEditor::on_my_size_allocate) );
  //add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
	//add_events( Gdk::STRUCTURE_MASK );
  //signal_configure_event().connect_notify( sigc::mem_fun(*this, &PF::ImageEditor::on_preview_configure_event) );

  //open_image();

  PF::ImageProcessor::Instance().signal_status_ready.
      connect( sigc::mem_fun(*this, &PF::ImageEditor::set_status_ready) );
  PF::ImageProcessor::Instance().signal_status_caching.
      connect( sigc::mem_fun(*this, &PF::ImageEditor::set_status_caching) );
  PF::ImageProcessor::Instance().signal_status_processing.
      connect( sigc::mem_fun(*this, &PF::ImageEditor::set_status_processing) );
  PF::ImageProcessor::Instance().signal_status_updating.
      connect( sigc::mem_fun(*this, &PF::ImageEditor::set_status_updating) );
  PF::ImageProcessor::Instance().signal_status_exporting.
      connect( sigc::mem_fun(*this, &PF::ImageEditor::set_status_exporting) );

  show_all_children();
  //controls_group_scrolled_window.hide();
}


PF::ImageEditor::~ImageEditor()
{
  /**/
  std::cout<<"~ImageEditor(): deleting image"<<std::endl;
  if( image ) {
    image->destroy();
    delete image;
  }
  std::cout<<"~ImageEditor(): image deleted"<<std::endl;
  /**/
  /*
  // Images need to be destroyed by the processing thread
  ProcessRequestInfo request;
  request.image = image;
  request.request = PF::IMAGE_DESTROY;
  PF::ImageProcessor::Instance().submit_request( request );
  // Ugly temporary solution to make sure that the image is destroyed before continuing
  sleep(1);	
  */
}



// Fills a list with all children of the current layer
void PF::ImageEditor::expand_layer( PF::Layer* layer, std::list<PF::Layer*>& list )
{
  if( !layer ) return;
  // Sublayers
  for( std::list<PF::Layer*>::reverse_iterator li = layer->get_sublayers().rbegin();
       li != layer->get_sublayers().rend(); li++ ) {
    PF::Layer* l = *li;
    if( !l ) continue;
    //#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
    //#endif
    if( !l->is_visible() ) continue;
    if( l->get_processor() == NULL ) continue;
    if( l->get_processor()->get_par() == NULL ) continue;
    PF::OpParBase* par = l->get_processor()->get_par();
    PF::BlenderPar* blender = NULL;
    if( l->get_blender() ) {
      blender = dynamic_cast<BlenderPar*>( l->get_blender() );
    }

    if( l->is_group() ) {
      // Group layers do not need to be added, as they do not directly modify
      // the image geometry. However, if the blend mode is "passthrough"
      // the geometry might be indirectly modified by one of the child layers,
      //therefore we add them to the list
      if( blender && (blender->get_blend_mode() != PF::PF_BLEND_PASSTHROUGH) )
        continue;
      expand_layer( l, list );
    } else {
      // Add layer to the temporary list
      list.push_front( l );
      //#ifndef NDEBUG
      std::cout<<"    added."<<std::endl;
      //#endif
    }
  }
}


void PF::ImageEditor::get_child_layers( Layer* layer, std::list<PF::Layer*>& container,
    std::list<Layer*>& children )
{
#ifndef NDEBUG
  std::cout<<"Collecting children of layer \""<<layer->get_name()<<"\"("<<layer->get_id()<<")"<<std::endl;
#endif
  std::list<PF::Layer*> tmplist;
  std::list<PF::Layer*>::reverse_iterator li;
  // Loop over layers in reverse order and fill a temporary list,
  // until either the target layer is found or the end of the
  // container list is reached
  for(li = container.rbegin(); li != container.rend(); ++li) {
    PF::Layer* l = *li;
    if( !l ) continue;
#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
#endif
    if( (layer != NULL) && (layer->get_id() == l->get_id()) ) break;
    if( !l->is_enabled() ) continue;
    if( l->get_processor() == NULL ) continue;
    if( l->get_processor()->get_par() == NULL ) continue;
    PF::OpParBase* par = l->get_processor()->get_par();
    PF::BlenderPar* blender = NULL;
    if( l->get_blender() ) {
      blender = dynamic_cast<BlenderPar*>( l->get_blender() );
    }

    if( l->is_group() ) {
      // Group layers do not need to be added, as they do not directly modify
      // the image geometry. However, if the blend mode is "passthrough"
      // the geometry might be indirectly modified by one of the child layers,
      //therefore we add them to the list
      if( blender && (blender->get_blend_mode() != PF::PF_BLEND_PASSTHROUGH) )
        continue;
      expand_layer( l, tmplist );
    } else {
      // Add layer to the temporary list
      tmplist.push_front( l );
#ifndef NDEBUG
      std::cout<<"    added."<<std::endl;
#endif
    }
  }

  // Append the temporary list to the childrens one
  children.insert( children.end(), tmplist.begin(), tmplist.end() );

  PF::Layer* container_layer = image->get_layer_manager().get_container_layer( layer );
  if( !container_layer ) return;

  // Add the container layer to the list of children
  children.push_back( container_layer );

  std::list<PF::Layer*>* clist = image->get_layer_manager().get_list( container_layer );
  if( !clist ) return;

  // Add all the children of the container layer to the children list
  get_child_layers( container_layer, *clist, children );
}


void PF::ImageEditor::get_child_layers()
{
  if( !active_layer ) return;
  std::list<PF::Layer*>* clist = image->get_layer_manager().get_list( active_layer );
  if( !clist ) return;
  get_child_layers( active_layer, *clist, active_layer_children );
}


void PF::ImageEditor::open_image()
{
  if( image_opened ) return;
  //std::cout<<"ImageEditor::open_image(): opening image..."<<std::endl;

  std::string bckname;
  char* fullpath = realpath( filename.c_str(), NULL );
  if(fullpath) {
    // Look into the cache dir for *.info files, and see if there is one
    // that corresponds to the image we are opening.
    DIR* dirp = opendir( PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    if (dirp != NULL) {
      struct dirent* dp;
      while ((dp = readdir(dirp)) != NULL) {
        std::cout<<"ImageEditor::open_image(): checking "<<dp->d_name<<std::endl;
        int len = strlen(dp->d_name);
        if (len != 12 || strncmp(dp->d_name, "pfbck-", 6) != 0)
          continue;
        std::string infofile = dp->d_name;
        infofile += ".info";
        std::ifstream ifile;
        ifile.open( (PF::PhotoFlow::Instance().get_cache_dir()+infofile).c_str() );
        std::cout<<"ImageEditor::open_image(): checking "
            <<(PF::PhotoFlow::Instance().get_cache_dir()+infofile).c_str()<<std::endl;
        if( ifile ) {
          std::string fname;
          std::getline( ifile, fname );
          std::cout<<"ImageEditor::open_image(): fname="<<fname<<std::endl;
          if( fname == fullpath ) {
            bckname = PF::PhotoFlow::Instance().get_cache_dir() + dp->d_name;
            std::cout<<"ImageEditor::open_image(): bckname="<<bckname<<std::endl;
            break;
          }
        }
      }
      (void)closedir(dirp);
    }
  }

  bool do_recovery = false;
  if( !bckname.empty() ) {
    //Glib::ustring msg = "Crash recovery found for file\n\"";
    //msg += filename;
    //msg += "\"\nDo you want to restore it?";
    char tstr[501];
    snprintf( tstr, 500, _("Crash recovery found for file \"%s\". Do you want to restore it?"),
        filename.c_str());
    Gtk::MessageDialog dialog(tstr,
        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
    //dialog.set_transient_for(*this);
    dialog.set_default_response( Gtk::RESPONSE_YES );

    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result) {
    case Gtk::RESPONSE_YES:
      do_recovery = true;
      break;
    case Gtk::RESPONSE_NO:
      unlink( bckname.c_str() );
      std::string infofile = bckname + ".info";
      unlink( infofile.c_str() );
      bckname = "";
      break;
    }
  }

  std::cout<<"ImageEditor::open_image(): opening image "<<filename<<" ..."<<std::endl;
  image->open( filename, bckname );
  std::list<Layer*>& layers = image->get_layer_manager().get_layers();
  std::cout<<"ImageEditor::open_image(): ... done. layers.size()="<<layers.size()<<std::endl;
  if( !layers.empty() ) {
    std::cout<<"ImageEditor::open_image(): calling \""<<layers.front()->get_name()<<"\"->set_hidden( "<<hide_background_layer<<" )"<<std::endl;
    layers.front()->set_hidden( hide_background_layer );
  }

  /*
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  int level = 0;
  pipeline->set_level( level );
	imageArea->set_shrink_factor( 1 );
  layersWidget.update();
  std::cout<<"ImageEditor::open_image(): updating image"<<std::endl;
  image->set_loaded( false );
  image->update();
  //getchar();
  //PF::ImageProcessor::Instance().wait_for_caching();
  image->set_loaded( true );
  image_opened = true;

  image->clear_modified();
  image->signal_modified.connect(sigc::mem_fun(this, &PF::ImageEditor::on_image_modified) );
  //Gtk::Paned::on_map();
  if( do_recovery ) image->modified();
  */

  //char* fullpath = realpath( filename.c_str(), NULL );
  if( fullpath && !do_recovery ) {
    char bckfname[500];
    sprintf( bckfname,"%spfbck-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int fd = pf_mkstemp( bckfname );
    if( fd >= 0 ) {
      close(fd);
      image->set_backup_filename( bckfname );
      std::string infoname = bckfname;
      infoname += ".info";
      std::ofstream of;
      of.open( infoname.c_str() );
      if( of ) {
        of<<fullpath;
      }
    }
    free( fullpath );
  }
  image_opened = true;
}


void PF::ImageEditor::build_image()
{
  open_image();
  //std::cout<<"ImageEditor::open_image(): opening image..."<<std::endl;

  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  int level = 0;
  pipeline->set_level( level );
  imageArea->set_shrink_factor( 1 );
  layersWidget.update(true);
  std::cout<<"ImageEditor::open_image(): updating image"<<std::endl;
  image->set_loaded( false );
  image->update();
  //getchar();
  //PF::ImageProcessor::Instance().wait_for_caching();
  image->set_loaded( true );

  image->clear_modified();
  image->signal_modified.connect(sigc::mem_fun(this, &PF::ImageEditor::on_image_modified_idle_cb) );
  image->signal_updated.connect(sigc::mem_fun(this, &PF::ImageEditor::on_image_updated_idle_cb) );
  //Gtk::Paned::on_map();
}


void PF::ImageEditor::on_image_modified_idle_cb()
{
  EditorCBData * update = g_new (EditorCBData, 1);
  update->editor = this;
  g_idle_add ((GSourceFunc) editor_on_image_modified_cb, update);
}


void PF::ImageEditor::on_image_modified()
{
  //std::cout<<"ImageEditor::on_image_modified() called."<<std::endl;
  if( !tab_label_widget ) return;
  char* fullpath = strdup( image->get_filename().c_str() );
  char* fname = basename( fullpath );
  Glib::ustring label = "*";
  label = label + fname;
  tab_label_widget->set_label( label );
}


void PF::ImageEditor::on_image_updated_idle_cb()
{
  std::cout<<"ImageEditor::on_image_updated_idle_cb() called"<<std::endl;
  EditorCBData * update = g_new (EditorCBData, 1);
  update->editor = this;
  g_idle_add ((GSourceFunc) editor_on_image_updated_cb, update);
}


void PF::ImageEditor::on_image_updated()
{
  std::cout<<"ImageEditor::on_image_updated() called."<<std::endl;
  layersWidget.update_controls();
}


void PF::ImageEditor::update_controls()
{
  //std::cout<<"ImageEditor::update_controls(): layersWidget.get_controls_group().size()="<<layersWidget.get_controls_group().size()<<std::endl;
  if( layersWidget.get_controls_group().size() > 0 ) {
    //if( controls_group_scrolled_window.get_parent() != &main_panel ) {
    //  main_panel.pack_start( controls_group_scrolled_window, Gtk::PACK_SHRINK );
    //  main_panel.show_all_children();
    //  if( fit_image ) fit_image_needed = true;
    //}
    //controls_group_scrolled_window.show();
  } else {
    //if( controls_group_scrolled_window.get_parent() == &main_panel ) {
    //  main_panel.remove( controls_group_scrolled_window );
    //  main_panel.show_all_children();
    //  if( fit_image ) fit_image_needed = true;
    //}
    //controls_group_scrolled_window.hide();
  }
}


void PF::ImageEditor::set_aux_controls( Gtk::Widget* aux )
{
  if( aux_controls ) {
    if( aux_controls->get_parent() == &aux_controlsBox ) {
      aux_controlsBox.remove( *aux_controls );
    }
  }
  aux_controls = aux;
  if( aux_controls ) {
    aux_controlsBox.pack_start( *aux_controls, Gtk::PACK_SHRINK );
  }
  aux_controlsBox.show_all_children();
}


void PF::ImageEditor::on_map()
{
  std::cout<<"ImageEditor::on_map() called."<<std::endl;
  Gtk::Container* toplevel = get_toplevel();
#ifdef GTKMM_3
  if( toplevel->get_is_toplevel() ) {
#else
  if( toplevel->is_toplevel() ) {
#endif
    toplevel->add_events( Gdk::STRUCTURE_MASK );
    //toplevel->signal_configure_event().connect_notify( sigc::mem_fun(*this, &PF::ImageEditor::on_preview_configure_event) );
    std::cout<<"ImageEditor::on_map(): toplevel window configured."<<std::endl;
  }
  Glib::RefPtr< Gdk::Window > win = get_window();
  if( win ) {
    Gdk::EventMask events = win->get_events ();
    win->set_events( events | Gdk::STRUCTURE_MASK );
    //win->signal_configure_event().connect_notify( sigc::mem_fun(*this, &PF::ImageEditor::on_preview_configure_event) );
    std::cout<<"ImageEditor::on_map(): parent window configured."<<std::endl;
  }
  //open_image();

  if( fit_image ) zoom_fit();

  Gtk::HBox::on_map();
}

void PF::ImageEditor::on_realize()
{
  std::cout<<"ImageEditor::on_realize() called."<<std::endl;
  build_image();
  controls_group_scrolled_window.show();
  Gtk::HBox::on_realize();
}


void PF::ImageEditor::toggle_highlights_warning()
{
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  imageArea->set_highlights_warning( button_highlights_warning.get_active() );
  image->update();
}


void PF::ImageEditor::toggle_shadows_warning()
{
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  imageArea->set_shadows_warning( button_shadows_warning.get_active() );
  image->update();
}


void PF::ImageEditor::zoom_out()
{
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  pipeline->set_level( level + 1 );
	imageArea->set_shrink_factor( 1 );
  image->update();

  fit_image = false;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_out(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
}


void PF::ImageEditor::zoom_in()
{
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  if( level > 0 ) {
    pipeline->set_level( level - 1 );
		imageArea->set_shrink_factor( 1 );
    image->update();
  }

  fit_image = false;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


bool PF::ImageEditor::zoom_fit()
{
  //std::cout<<"ImageEditor::zoom_fit(): image="<<image<<std::endl;
  if( !image ) return false;
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  //std::cout<<"ImageEditor::zoom_fit(): pipeline="<<pipeline<<std::endl;
  if( !pipeline ) return false;
  //std::cout<<"image_size_updater->get_image_width()="<<image_size_updater->get_image_width()
  //    <<" get_image_height()="<<image_size_updater->get_image_height()<<std::endl;
  if( image_size_updater->get_image_width() < 1 ||
      image_size_updater->get_image_height() < 1 ) return false;

  //float area_hsize = imageArea_scrolledWindow.get_hadjustment()->get_page_size();
  //float area_vsize = imageArea_scrolledWindow.get_vadjustment()->get_page_size();
#ifdef GTKMM_2
  float area_hsize = imageArea_scrolledWindow.get_width();
  float area_vsize = imageArea_scrolledWindow.get_height();
#endif
#ifdef GTKMM_3
  float area_hsize = imageArea_scrolledWindow.get_allocated_width();
  float area_vsize = imageArea_scrolledWindow.get_allocated_height();
#endif
  //std::cout<<"ImageEditor::zoom_fit(): area_hsize="<<area_hsize<<"  area_vsize="<<area_vsize<<std::endl;
  area_hsize -= 20;
  area_vsize -= 20;

	float shrink_h = area_hsize/image_size_updater->get_image_width();
	float shrink_v = area_vsize/image_size_updater->get_image_height();
	float shrink_min = (shrink_h<shrink_v) ? shrink_h : shrink_v;
	unsigned int target_level = 0;
	//std::cout<<"ImageEditor::zoom_fit(): target_level="<<target_level<<"  shrink_min="<<shrink_min<<std::endl;
	while( shrink_min < 0.5 ) {
		target_level++;
		shrink_min *= 2;
	  //std::cout<<"ImageEditor::zoom_fit(): target_level="<<target_level<<"  shrink_min="<<shrink_min<<std::endl;
	}

  //std::cout<<"ImageEditor::zoom_fit(): image area size="
  //         <<area_hsize<<","<<area_vsize
  //         <<"  image size="<<image_size_updater->get_image_width()
  //         <<","<<image_size_updater->get_image_height()
  //         <<"  level="<<target_level<<"  shrink="<<shrink_min<<std::endl;

	if( (imageArea->get_shrink_factor() != shrink_min) ||
	    (pipeline->get_level() != target_level) ) {
	  imageArea->set_shrink_factor( shrink_min );
	  image->set_pipeline_level( pipeline, target_level );
	  //pipeline2->set_level( target_level );
	  image->update();
	}

  fit_image = true;
  return true;
}


void PF::ImageEditor::zoom_actual_size()
{
  PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
  if( !pipeline ) return;
	pipeline->set_level( 0 );
	imageArea->set_shrink_factor( 1 );
	image->update();

  fit_image = false;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


void PF::ImageEditor::set_active_layer( int id )
{
  PF::Layer* old_active = active_layer;
  active_layer = NULL;
  if( image )
    active_layer = image->get_layer_manager().get_layer( id );
  std::cout<<"ImageEditor::set_active_layer("<<id<<"): old_active="<<old_active<<"  active_layer="<<active_layer<<std::endl;
  if( old_active != active_layer ) {
    /*
    if( old_active &&
        old_active->get_processor() &&
        old_active->get_processor()->get_par() &&
        old_active->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigUI* ui = old_active->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( dialog ) {
        dialog->disable_editing();
      }
    }
    */
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() &&
        active_layer->get_processor()->get_par()->get_config_ui() ) {
      /*
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( dialog ) {
        dialog->set_editor( this );
        if( dialog ) {
          dialog->enable_editing();
        }
      }
      */
      active_layer_children.clear();
      //image->get_layer_manager().get_child_layers( active_layer, active_layer_children );
      get_child_layers();
      imageArea->set_edited_layer( id );
    } else {
      imageArea->set_edited_layer( -1 );
    }
  }
}


void PF::ImageEditor::set_displayed_layer( int id )
{
  PF::Layer* old_displayed = displayed_layer;
  displayed_layer = NULL;
  if( image )
    displayed_layer = image->get_layer_manager().get_layer( id );
  std::cout<<"ImageEditor::set_displayed_layer("<<id<<"): old_displayed="<<old_displayed<<"  displayed_layer="<<displayed_layer<<std::endl;
  if( old_displayed != displayed_layer ) {
    if( displayed_layer ) {
      image_size_updater->set_displayed_layer( id );
      imageArea->set_displayed_layer( id );
      imageArea->set_display_merged( false );
    } else {
      image_size_updater->set_displayed_layer( -1 );
      imageArea->set_display_merged( true );
      imageArea->set_displayed_layer( -1 );
    }
  }
}


void PF::ImageEditor::screen2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
#ifndef NDEBUG
  /**/
  std::cout<<"PF::ImageEditor::screen2image(): x="<<x<<"  y="<<y<<"  adjustments:"
     <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_value()
     <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_value()<<std::endl;
  /**/
#endif
  //x += imageArea_scrolledWindow.get_hadjustment()->get_value();
  //y += imageArea_scrolledWindow.get_vadjustment()->get_value();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): offsets: "
           <<imageArea->get_xoffset()<<" "
           <<imageArea->get_yoffset()<<std::endl;
#endif
  x -= imageArea->get_xoffset();
  y -= imageArea->get_yoffset();
  /*
  if( (x<0) || (y<0) ) return;
  if( imageArea->get_display_image() ) {
    if( x >= imageArea->get_display_image()->Xsize ) 
      return;
    if( y >= imageArea->get_display_image()->Ysize ) 
      return;
  }
  */

  float zoom_fact = get_zoom_factor();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): zoom_factor="<<zoom_fact<<std::endl;
  std::cout<<"PF::ImageEditor::screen2image(): shrink_factor="<<imageArea->get_shrink_factor()<<std::endl;
#endif
  zoom_fact *= imageArea->get_shrink_factor();
  x /= zoom_fact;
  y /= zoom_fact;
  w /= zoom_fact;
  h /= zoom_fact;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  //return true;
}


void PF::ImageEditor::image2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if( !image ) return;
  if( !active_layer ) return;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2layer(): before layer corrections: x'="<<x<<"  y'="<<y<<std::endl;
  std::cout<<"  active_layer_children.size()="<<active_layer_children.size()<<std::endl;
#endif

  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"ImageEditor::image2layer(): NULL pipeline"<<std::endl;
    return;
  }

  if( imageArea->get_display_merged() ) {
    std::list<PF::Layer*>::reverse_iterator li;
    for(li = active_layer_children.rbegin(); li != active_layer_children.rend(); ++li) {
      PF::Layer* l = *li;
      if( l && l->is_enabled() ) {
        // Get the node associated to the layer
        PF::PipelineNode* node = pipeline->get_node( l->get_id() );
        if( !node ) {
          std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
          continue;
        }
        if( !node->processor ) {
          std::cout<<"Image::do_sample(): NULL node processor"<<std::endl;
          continue;
        }

        PF::OpParBase* par = node->processor->get_par();
        VipsRect rin, rout;
        rout.left = x;
        rout.top = y;
        rout.width = w;
        rout.height = h;
        par->transform_inv( &rout, &rin );

        x = rin.left;
        y = rin.top;
        w = rin.width;
        h = rin.height;
#ifndef NDEBUG
        std::cout<<"PF::ImageEditor::image2layer(): after \""<<l->get_name()
              <<"\"("<<par->get_type()<<"): x'="<<x<<"  y'="<<y<<std::endl;
#endif
      }
    }
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2layer(): active layer: "<<active_layer->get_name()<<std::endl;
#endif

  PF::PipelineNode* node = pipeline->get_node( active_layer->get_id() );
  if( !node ) {
    std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
    return;
  }
  if( !node->processor ) {
    std::cout<<"Image::do_sample(): NULL node processor"<<std::endl;
    return;
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2layer(): before active layer: x'="<<x<<"  y'="<<y<<std::endl;
#endif

  PF::OpParBase* par = node->processor->get_par();
  VipsRect rin, rout;
  rout.left = x;
  rout.top = y;
  rout.width = w;
  rout.height = h;
  par->transform_inv( &rout, &rin );

  x = rin.left;
  y = rin.top;
  w = rin.width;
  h = rin.height;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2layer(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  //return true;
}


void PF::ImageEditor::image2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  float zoom_fact = get_zoom_factor();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2screen(): zoom_factor="<<zoom_fact<<std::endl;
  std::cout<<"PF::ImageEditor::image2screen(): shrink_factor="<<imageArea->get_shrink_factor()<<std::endl;
#endif
  zoom_fact *= imageArea->get_shrink_factor();
  x *= zoom_fact;
  y *= zoom_fact;
  w *= zoom_fact;
  h *= zoom_fact;

#ifndef NDEBUG
  /**/
  std::cout<<"PF::ImageEditor::image2screen(): x="<<x<<"  y="<<y<<"  adjustments:"
     <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_value()
     <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_value()<<std::endl;
  /**/
#endif
  //x += imageArea_scrolledWindow.get_hadjustment()->get_value();
  //y += imageArea_scrolledWindow.get_vadjustment()->get_value();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2screen(): offsets: "
           <<imageArea->get_xoffset()<<" "
           <<imageArea->get_yoffset()<<std::endl;
#endif
  x += imageArea->get_xoffset();
  y += imageArea->get_yoffset();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2screen(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  //return true;
}


void PF::ImageEditor::layer2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
{
  if( !image ) return;
  if( !active_layer ) return;

  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"ImageEditor::layer2image(): NULL pipeline"<<std::endl;
    return;
  }

  PF::PipelineNode* node = pipeline->get_node( active_layer->get_id() );
  if( !node ) {
    std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
    return;
  }
  if( !node->processor ) {
    std::cout<<"Image::do_sample(): NULL node processor"<<std::endl;
    return;
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::layer2image(): before layer corrections: x'="<<x<<"  y'="<<y<<std::endl;
#endif

  PF::OpParBase* par = node->processor->get_par();
  VipsRect rin, rout;
  rin.left = x;
  rin.top = y;
  rin.width = w;
  rin.height = h;
  par->transform( &rin, &rout );

  x = rout.left;
  y = rout.top;
  w = rout.width;
  h = rout.height;

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::layer2image(): after active layer corrections: x'="<<x<<"  y'="<<y<<std::endl;
#endif

  if( imageArea->get_display_merged() ) {
    std::list<PF::Layer*>::iterator li;
    for(li = active_layer_children.begin(); li != active_layer_children.end(); ++li) {
      PF::Layer* l = *li;
      if( l && l->is_enabled() ) {
        // Get the node associated to the layer
        PF::PipelineNode* node = pipeline->get_node( l->get_id() );
        if( !node ) {
          std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
          continue;
        }
        if( !node->processor ) {
          std::cout<<"Image::do_sample(): NULL node processor"<<std::endl;
          continue;
        }

        PF::OpParBase* par = node->processor->get_par();
        VipsRect rin, rout;
        rin.left = x;
        rin.top = y;
        rin.width = w;
        rin.height = h;
        par->transform( &rin, &rout );

        x = rout.left;
        y = rout.top;
        w = rout.width;
        h = rout.height;
#ifndef NDEBUG
        std::cout<<"PF::ImageEditor::layer2image(): after \""<<l->get_name()
              <<"\"("<<par->get_type()<<"): x'="<<x<<"  y'="<<y<<std::endl;
#endif
      }
    }
  }
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::layer2image(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  //return true;
}


bool PF::ImageEditor::my_button_press_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_press_event(): button "<<button->button<<" pressed."<<std::endl;
  std::cout<<"PF::ImageEditor::on_button_press_event(): type "<<button->type<<std::endl;
  if( button->type == GDK_BUTTON_PRESS ) std::cout<<"  single-click"<<std::endl;
  if( button->type == GDK_2BUTTON_PRESS ) std::cout<<"  double-click"<<std::endl;
#endif
  gdouble x = button->x;
  gdouble y = button->y;

  int mod_key = PF::MOD_KEY_NONE;
  if( button->state & GDK_CONTROL_MASK ) mod_key |= PF::MOD_KEY_CTRL;
  if( button->state & GDK_MOD1_MASK ) mod_key |= PF::MOD_KEY_ALT;
  if( button->state & GDK_SHIFT_MASK ) mod_key |= PF::MOD_KEY_SHIFT;

#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
  std::cout<<"  active_layer: "<<active_layer<<std::endl;
#endif

  // Handle CTRL-double-click events separately
  //if( button->type != GDK_BUTTON_PRESS ) {
  if( mod_key == PF::MOD_KEY_CTRL ) {
    if( button->type == GDK_2BUTTON_PRESS && mod_key == PF::MOD_KEY_CTRL) {
      PF::Pipeline* pipeline = image->get_pipeline( PREVIEW_PIPELINE_ID );
      if( !pipeline ) return false;
      if( pipeline->get_level() == 0 && imageArea->get_shrink_factor() == 1 ) {
        // we are at 100% zoom, so we switch to fit mode
        zoom_fit();
        //std::cout<<"ImageEditor::my_button_release_event(): zoom_fit() called"<<std::endl<<std::endl;
      } else {
        int imgw, imgh; imageArea->get_size_request( imgw, imgh );
        imageArea->set_target_area_center(x/imgw,y/imgh);
        zoom_actual_size();
        //std::cout<<"ImageEditor::my_button_release_event(): zoom_actual_size() called"<<std::endl<<std::endl;
      }
    }
    if( button->type == GDK_BUTTON_PRESS && mod_key == PF::MOD_KEY_CTRL && button->button == 1) {
      preview_drag_start_x = x;
      preview_drag_start_y = y;
      adjustment_drag_start_x = imageArea_scrolledWindow.get_hadjustment()->get_value();
      adjustment_drag_start_y = imageArea_scrolledWindow.get_vadjustment()->get_value();
    }
    return false;
  }

  std::cout<<"ImageEditor::my_button_press_event(): active_layer="<<active_layer<<std::endl;
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
    //std::cout<<"ImageEditor::my_button_press_event(): dialog="<<dialog<<std::endl;
    //if( dialog ) std::cout<<"ImageEditor::my_button_press_event(): dialog->get_editing_flag()="<<dialog->get_editing_flag()<<std::endl;
    if( dialog /*&& dialog->get_editing_flag() == true*/ ) {
#ifndef NDEBUG
      std::cout<<"  sending button press event to dialog"<<std::endl;
#endif
      if( dialog->pointer_press_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
  return false;
}


bool PF::ImageEditor::my_button_release_event( GdkEventButton* button )
{
//#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_release_event(): button "<<button->button<<" released."<<std::endl;
//#endif
  gdouble x = button->x;
  gdouble y = button->y;

  int mod_key = PF::MOD_KEY_NONE;
  if( button->state & GDK_CONTROL_MASK ) mod_key |= PF::MOD_KEY_CTRL;
  if( button->state & GDK_MOD1_MASK ) mod_key |= PF::MOD_KEY_ALT;
  if( button->state & GDK_SHIFT_MASK ) mod_key |= PF::MOD_KEY_SHIFT;

  if( mod_key == PF::MOD_KEY_CTRL ) {
    return false;
  }

#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
  std::cout<<"ImageEditor::my_button_release_event(): active_layer="<<active_layer<<std::endl;
#endif
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
    if( dialog /*&& dialog->get_editing_flag() == true*/ ) {
//#ifndef NDEBUG
      std::cout<<"  sending button release event to dialog"<<std::endl;
//#endif
      //std::cout<<"dialog->pointer_release_event( "<<button->button<<", "<<x<<", "<<y<<", "<<mod_key<<" )"<<std::endl;
      if( dialog->pointer_release_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
  return false;
}


bool PF::ImageEditor::my_motion_notify_event( GdkEventMotion* event )
{
	int ix, iy;
	gdouble x, y;
	guint state;
	if (event->is_hint) {
std::cout<<"event->is_hint"<<std::endl;
return false;
		//event->window->get_pointer(&ix, &iy, &state);
      //x = ix;
      //y = iy;
      //return true;

		x = event->x;
		y = event->y;
		state = event->state;	
	} else {
		x = event->x;
		y = event->y;
		state = event->state;	
	}
	int button = -1;
  if(state & GDK_BUTTON1_MASK) button = 1;
  if(state & GDK_BUTTON2_MASK) button = 2;
  if(state & GDK_BUTTON3_MASK) button = 3;
  if(state & GDK_BUTTON4_MASK) button = 4;
  if(state & GDK_BUTTON5_MASK) button = 5;

  int mod_key = PF::MOD_KEY_NONE;
  if( event->state & GDK_CONTROL_MASK ) mod_key |= PF::MOD_KEY_CTRL;
  if( event->state & GDK_MOD1_MASK ) mod_key |= PF::MOD_KEY_ALT;
  if( event->state & GDK_SHIFT_MASK ) mod_key |= PF::MOD_KEY_SHIFT;

  if( mod_key == PF::MOD_KEY_CTRL ) {
    if( button == 1 ) {
      int dx = (int)(x - preview_drag_start_x);
      int dy = (int)(y - preview_drag_start_y);
      if( fabs(imageArea_scrolledWindow.get_hadjustment()->get_value() - (adjustment_drag_start_x-dx)) > 2 ||
          fabs(imageArea_scrolledWindow.get_vadjustment()->get_value() - (adjustment_drag_start_y-dy)) > 2 ) {

        double xmax = imageArea_scrolledWindow.get_hadjustment()->get_upper() -
            imageArea_scrolledWindow.get_hadjustment()->get_page_size();
        double ymax = imageArea_scrolledWindow.get_vadjustment()->get_upper() -
            imageArea_scrolledWindow.get_vadjustment()->get_page_size();
        if( (adjustment_drag_start_x-dx) <= xmax ) {
          imageArea_scrolledWindow.get_hadjustment()->set_value(adjustment_drag_start_x-dx);
          adjustment_drag_start_x -= dx;
        }
        if( (adjustment_drag_start_y-dy) <= ymax ) {
          imageArea_scrolledWindow.get_vadjustment()->set_value(adjustment_drag_start_y-dy);
          adjustment_drag_start_y -= dy;
        }
      }
    }
    return false;
  }


  if( true || (state & GDK_BUTTON1_MASK) ) {

#ifndef NDEBUG
    std::cout<<"PF::ImageEditor::on_motion_notify_event(): pointer @ "<<x<<","<<y
             <<"  hint: "<<event->is_hint<<"  state: "<<event->state
             <<std::endl;
#endif
    //std::cout<<"PF::ImageEditor::on_motion_notify_event(): active_layer="<<active_layer;
    //if(active_layer) std::cout<<" ("<<active_layer->get_name()<<")"<<std::endl;
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() ) {
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if( dialog /*&& dialog->get_editing_flag() == true*/ ) {
#ifndef NDEBUG
        std::cout<<"  sending motion event to dialog"<<std::endl;
#endif
        int mod_key = PF::MOD_KEY_NONE;
        if( event->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
        if( event->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
        if( dialog->pointer_motion_event( button, x, y, mod_key ) ) {
          // The dialog requires to draw on top of the preview image, so we call draw_area() 
          // to refresh the preview
          //imageArea->draw_area();
          imageArea->queue_draw();
        }
      }
    }
  }
	return true;
}


//bool PF::ImageEditor::on_preview_configure_event( GdkEventConfigure* event )
void PF::ImageEditor::on_my_size_allocate(Gtk::Allocation& allocation)
{
	//std::cout<<"ImageEditor::on_my_size_allocate() called: fit_image="<<fit_image<<" fit_image_needed="<<fit_image_needed<<std::endl;
	//std::cout<<"  allocation width="<<allocation.get_width()<<" height="<<allocation.get_height()<<std::endl;
	if( fit_image /*&& fit_image_needed*/ ) {
	  if( zoom_fit() )
	    fit_image_needed = false;
	}
	//return false;
}
