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
#include <libgen.h>
#include <dirent.h>

#include <gdk/gdk.h>

#include "../base/pf_mkstemp.hh"
#include "../base/imageprocessor.hh"
#include "imageeditor.hh"


#define PIPELINE_ID 1


PF::ImageEditor::ImageEditor( std::string fname ):
  filename( fname ),
  image( new PF::Image() ),
  image_opened( false ),
  active_layer( NULL ),
  //imageArea( image->get_pipeline(PIPELINE_ID) ),
  layersWidget( image, this ),
  buttonZoomIn( "Zoom +" ),
  buttonZoomOut( "Zoom -" ),
  buttonZoom100( "1:1" ),
  buttonZoomFit( "Fit" ),
  buttonShowMerged( "show merged layers" ),
  buttonShowActive( "show active layer" ),
  tab_label_widget( NULL )
{
  image->add_pipeline( VIPS_FORMAT_USHORT, 0, PF_RENDER_PREVIEW );
  image->add_pipeline( VIPS_FORMAT_USHORT, 0, PF_RENDER_PREVIEW );

  imageArea = new PF::ImageArea( image->get_pipeline(PIPELINE_ID) );

  imageArea->set_adjustments( imageArea_scrolledWindow.get_hadjustment(),
			     imageArea_scrolledWindow.get_vadjustment() );

  imageArea_scrolledWindow.add( *imageArea );
  imageArea_eventBox.add( imageArea_scrolledWindow );

  radioBox.pack_start( buttonShowMerged );
  radioBox.pack_start( buttonShowActive );

  Gtk::RadioButton::Group group = buttonShowMerged.get_group();
  buttonShowActive.set_group(group);

  controlsBox.pack_end( radioBox, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoom100, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomFit, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomOut, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomIn, Gtk::PACK_SHRINK );

  imageBox.pack_start( imageArea_eventBox );
  imageBox.pack_start( controlsBox, Gtk::PACK_SHRINK );

  pack1( imageBox, true, false );

  pack2( layersWidget, false, false );

  buttonZoomIn.signal_clicked().connect( sigc::mem_fun(*this,
						       &PF::ImageEditor::zoom_in) );
  buttonZoomOut.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_out) );
  buttonZoom100.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_actual_size) );
  buttonZoomFit.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_fit) );

  buttonShowMerged.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged),
							 true) );
  buttonShowActive.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged),
							 false) );
  //set_position( get_allocation().get_width()-200 );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(imageArea,
								  &PF::ImageArea::set_active_layer) );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(this,
								  &PF::ImageEditor::set_active_layer) );

  /*
  imageArea_eventBox.signal_button_press_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_button_press_event) ); 
  imageArea_eventBox.signal_button_release_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_button_release_event) ); 
  imageArea_eventBox.signal_motion_notify_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_motion_notify_event) ); 
  */

  imageArea->add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  | Gdk::POINTER_MOTION_HINT_MASK | Gdk::STRUCTURE_MASK );
  imageArea_scrolledWindow.add_events( Gdk::STRUCTURE_MASK );
	imageArea_scrolledWindow.signal_configure_event().
		connect( sigc::mem_fun(*this, &PF::ImageEditor::on_configure_event) ); 
  //add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
	//add_events( Gdk::STRUCTURE_MASK );

  //open_image();

  show_all_children();
}


PF::ImageEditor::~ImageEditor()
{
	/*
  if( image )
    delete image;
	*/
  ProcessRequestInfo request;
  request.image = image;
  request.request = PF::IMAGE_DESTROY;
  PF::ImageProcessor::Instance().submit_request( request );	
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
  //#ifndef NDEBUG
  std::cout<<"Collecting children of layer \""<<layer->get_name()<<"\"("<<layer->get_id()<<")"<<std::endl;
  //#endif
  std::list<PF::Layer*> tmplist;
  std::list<PF::Layer*>::reverse_iterator li;
  // Loop over layers in reverse order and fill a temporary list,
  // until either the target layer is found or the end of the
  // container list is reached
  for(li = container.rbegin(); li != container.rend(); ++li) {
    PF::Layer* l = *li;
    if( !l ) continue;
    //#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
    //#endif
    if( (layer != NULL) && (layer->get_id() == l->get_id()) ) break;
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
      expand_layer( l, tmplist );
    } else {
      // Add layer to the temporary list
      tmplist.push_front( l );
      //#ifndef NDEBUG
      std::cout<<"    added."<<std::endl;
      //#endif
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
  std::cout<<"ImageEditor::open_image(): opening image..."<<std::endl;

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
    Glib::ustring msg = "Crash recovery found for file\n\"";
    msg += filename;
    msg += "\"\nDo you want to restore it?";
    Gtk::MessageDialog dialog(msg,
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

  image->open( filename, bckname );
  std::cout<<"ImageEditor::open_image(): ... done."<<std::endl;
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
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
}


void PF::ImageEditor::on_image_modified()
{
  std::cout<<"ImageEditor::on_image_modified() called."<<std::endl;
  if( !tab_label_widget ) return;
  char* fullpath = strdup( image->get_filename().c_str() );
  char* fname = basename( fullpath );
  Glib::ustring label = "*";
  label = label + fname;
  tab_label_widget->set_label( label );
}


void PF::ImageEditor::on_map()
{
  std::cout<<"ImageEditor::on_map() called."<<std::endl;
  //open_image();
  Gtk::Paned::on_map();
}

void PF::ImageEditor::on_realize()
{
  std::cout<<"ImageEditor::on_realize() called."<<std::endl;
  open_image();
  Gtk::Paned::on_realize();
}

void PF::ImageEditor::zoom_out()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  pipeline->set_level( level + 1 );
	imageArea->set_shrink_factor( 1 );
  image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_out(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
}


void PF::ImageEditor::zoom_in()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  if( level > 0 ) {
    pipeline->set_level( level - 1 );
		imageArea->set_shrink_factor( 1 );
    image->update();
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


void PF::ImageEditor::zoom_fit()
{
  if( !image ) return;
  image->lock();
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  PF::Pipeline* pipeline2 = image->get_pipeline( PIPELINE_ID );
  if( !pipeline || !pipeline2) {
    image->unlock();
    return;
  }
  VipsImage* out = pipeline->get_output();
  if( !out ) {
    image->unlock();
    return;
  }

	float shrink_h = ((float)imageArea_scrolledWindow.get_hadjustment()->get_page_size())/out->Xsize;
	float shrink_v = ((float)imageArea_scrolledWindow.get_vadjustment()->get_page_size())/out->Ysize;
	float shrink_min = (shrink_h<shrink_v) ? shrink_h : shrink_v;
	int target_level = 0;
	while( shrink_min < 0.5 ) {
		target_level++;
		shrink_min *= 2;
	}
  /*
  if( shrink_min < 0.75 ) {
    target_level++;
    shrink_min *= 2;
  }
  */

  std::cout<<"ImageEditor::zoom_fit(): image area size="
           <<imageArea_scrolledWindow.get_hadjustment()->get_page_size()<<","
           <<imageArea_scrolledWindow.get_vadjustment()->get_page_size()
           <<"  image size="<<out->Xsize<<","<<out->Ysize
           <<"  level="<<target_level<<"  shrink="<<shrink_min<<std::endl;

	imageArea->set_shrink_factor( shrink_min );
	pipeline2->set_level( target_level );
	image->update();
  image->unlock();

  /*
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
	pipeline->set_level( 0 );
	imageArea->set_shrink_factor( 1 );
	image->update(pipeline,true);
	if( !imageArea->get_display_image() ) return;
	float shrink_h = ((float)imageArea_scrolledWindow.get_hadjustment()->get_page_size())/imageArea->get_display_image()->Xsize;
	float shrink_v = ((float)imageArea_scrolledWindow.get_vadjustment()->get_page_size())/imageArea->get_display_image()->Ysize;
	float shrink_min = (shrink_h<shrink_v) ? shrink_h : shrink_v;
	int target_level = 0;
	while( shrink_min < 0.5 ) {
		target_level++;
		shrink_min *= 2;
	}

	imageArea->set_shrink_factor( shrink_min );
	pipeline->set_level( target_level );
	image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
  */
}


void PF::ImageEditor::zoom_actual_size()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
	pipeline->set_level( 0 );
	imageArea->set_shrink_factor( 1 );
	image->update();

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
  //std::cout<<"ImageEditor::set_active_layer("<<id<<"): old_active="<<old_active<<"  active_layer="<<active_layer<<std::endl;
  if( old_active != active_layer ) {
    if( old_active &&
        old_active->get_processor() &&
        old_active->get_processor()->get_par() &&
        old_active->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigUI* ui = old_active->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
#if defined(_WIN32) || defined(WIN32)
      if( dialog && dialog->is_visible() ) {
#else
      if( dialog && dialog->get_visible() ) {
#endif
        dialog->disable_editing();
      }
    }
    
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() &&
        active_layer->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if( dialog ) {
        dialog->set_editor( this );
#if defined(_WIN32) || defined(WIN32)
        if( dialog && dialog->is_visible() ) {
#else
        if( dialog && dialog->get_visible() ) {
#endif
          dialog->enable_editing();
        }
      }
      active_layer_children.clear();
      //image->get_layer_manager().get_child_layers( active_layer, active_layer_children );
      get_child_layers();
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
  if( (x<0) || (y<0) ) return;
  if( imageArea->get_display_image() ) {
    if( x >= imageArea->get_display_image()->Xsize ) 
      return;
    if( y >= imageArea->get_display_image()->Ysize ) 
      return;
  }

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
  if( !imageArea->get_display_merged() ) return;
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::image2layer(): before layer corrections: x'="<<x<<"  y'="<<y<<std::endl;
#endif

  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"ImageEditor::image2layer(): NULL pipeline"<<std::endl;
    return;
  }

  std::list<PF::Layer*>::reverse_iterator li;
  for(li = active_layer_children.rbegin(); li != active_layer_children.rend(); ++li) {
    PF::Layer* l = *li;
    if( l && l->is_visible() ) {
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
  if( !imageArea->get_display_merged() ) return;
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::layer2image(): before layer corrections: x'="<<x<<"  y'="<<y<<std::endl;
#endif

  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"ImageEditor::layer2image(): NULL pipeline"<<std::endl;
    return;
  }

  std::list<PF::Layer*>::iterator li;
  for(li = active_layer_children.begin(); li != active_layer_children.end(); ++li) {
    PF::Layer* l = *li;
    if( l && l->is_visible() ) {
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
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::layer2image(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  //return true;
}


bool PF::ImageEditor::on_button_press_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_press_event(): button "<<button->button<<" pressed."<<std::endl;
#endif
  gdouble x = button->x;
  gdouble y = button->y;

#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
  std::cout<<"  active_layer: "<<active_layer<<std::endl;
#endif
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
#if defined(_WIN32) || defined(WIN32)
    if( dialog && dialog->is_visible() ) {
#else
    if( dialog && dialog->get_visible() ) {
#endif
#ifndef NDEBUG
      std::cout<<"  sending button press event to dialog"<<std::endl;
#endif
      int mod_key = PF::MOD_KEY_NONE;
      if( button->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
      if( button->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
      if( dialog->pointer_press_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
}


bool PF::ImageEditor::on_button_release_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_release_event(): button "<<button->button<<" released."<<std::endl;
#endif
  gdouble x = button->x;
  gdouble y = button->y;

#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
#endif
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
#if defined(_WIN32) || defined(WIN32)
    if( dialog && dialog->is_visible() ) {
#else
    if( dialog && dialog->get_visible() ) {
#endif
#ifndef NDEBUG
      std::cout<<"  sending button release event to dialog"<<std::endl;
#endif
      int mod_key = PF::MOD_KEY_NONE;
      if( button->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
      if( button->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
      if( dialog->pointer_release_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
}


bool PF::ImageEditor::on_motion_notify_event( GdkEventMotion* event )
{
  /*
  GDK_SHIFT_MASK    = 1 << 0,
  GDK_LOCK_MASK     = 1 << 1,
  GDK_CONTROL_MASK  = 1 << 2,
  GDK_MOD1_MASK     = 1 << 3,
  GDK_MOD2_MASK     = 1 << 4,
  GDK_MOD3_MASK     = 1 << 5,
  GDK_MOD4_MASK     = 1 << 6,
  GDK_MOD5_MASK     = 1 << 7,
  GDK_BUTTON1_MASK  = 1 << 8,
  GDK_BUTTON2_MASK  = 1 << 9,
  GDK_BUTTON3_MASK  = 1 << 10,
  GDK_BUTTON4_MASK  = 1 << 11,
  GDK_BUTTON5_MASK  = 1 << 12,
  */

	int ix, iy;
	gdouble x, y;
	guint state;
	if (event->is_hint) {
		//event->window->get_pointer(&ix, &iy, &state);
		/*
      x = ix;
      y = iy;
      return true;
		*/
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
  if( true || (state & GDK_BUTTON1_MASK) ) {

#ifndef NDEBUG
    std::cout<<"PF::ImageEditor::on_motion_notify_event(): pointer @ "<<x<<","<<y
             <<"  hint: "<<event->is_hint<<"  state: "<<event->state
             <<std::endl;
#endif
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() ) {
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
#if defined(_WIN32) || defined(WIN32)
      if( dialog && dialog->is_visible() ) {
#else
      if( dialog && dialog->get_visible() ) {
#endif
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


bool PF::ImageEditor::on_configure_event( GdkEventConfigure* event )
{
	std::cout<<"ImageEditor::on_configure_event() called"<<std::endl;
	return false;
}
