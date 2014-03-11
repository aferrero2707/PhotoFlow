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

#include <fstream>

#include <gtk/gtk.h>

#include "image.hh"
#include "pf_file_loader.hh"


static bool getFileExtension(const char * dir_separator, const std::string & file, std::string & ext)
{
    std::size_t ext_pos = file.rfind(".");
    std::size_t dir_pos = file.rfind(dir_separator);

    if(ext_pos>dir_pos+1)
    {
        ext.append(file.begin()+ext_pos+1,file.end());
        return true;
    }

    return false;
}



gint PF::image_rebuild_callback( gpointer data )
{
  PF::Image* image = (PF::Image*)data;
#ifndef NDEBUG
  std::cout<<"PF::image_rebuild_callback(): called, image->is_modified()="<<image->is_modified()<<std::endl;
#endif
  if( image->is_modified() ) {

    // Try to acquire the processing locks.
    // If fails, it means that there is a processing still active on this view,
    // and the update has to be postponed.
    // The peding views will take care of re-installing the idle callback
    // if needed when the processing will be finished
    bool pending = false;
    for( unsigned int i = 0; i < image->get_nviews(); i++ ) {
      PF::View* view = image->get_view( i );
      if( !view ) continue;
      if( view->processing() ) { pending = true; break; }
    }

#ifndef NDEBUG
    std::cout<<"PF::image_rebuild_callback(): pending="<<pending<<std::endl;
#endif
    if( pending ) {
      return false;
    }

    bool result = image->get_layer_manager().rebuild_prepare();
#ifndef NDEBUG
    std::cout<<"PF::image_rebuild_callback(): rebuild prepare "<<(result?"OK":"failed")<<std::endl;
#endif
    if( !result ) return false;
    
    image->set_modified( false );

    // Loop on views, re-build and update
    for( unsigned int i = 0; i < image->get_nviews(); i++ ) {
      PF::View* view = image->get_view( i );
      if( !view ) continue;

#ifndef NDEBUG
      std::cout<<"PF::image_rebuild_callback(): updating view #"<<i<<std::endl;
#endif
      image->get_layer_manager().rebuild( view, PF::PF_COLORSPACE_RGB, 100, 100 );
      view->update();
    }

    image->get_layer_manager().rebuild_finalize();
    image->set_rebuilding( false );
  }

  return false;
}


PF::Image::Image(): 
  layer_manager( this ), 
  async( false ), 
  modified( false ), 
  rebuilding( false ) 
{
  layer_manager.signal_modified.connect(sigc::mem_fun(this, &Image::update) );
}

PF::Image::~Image()
{
  for( unsigned int vi = 0; vi < views.size(); vi++ ) {
    if( views[vi] != NULL )
      delete views[vi];
  }
}


void PF::Image::update()
{
  if( is_async() )
    update_async();
  else
    update_sync();
}


void PF::Image::update_sync()
{
  layer_manager.rebuild_prepare();
  std::vector<View*>::iterator vi;
  for( vi = views.begin(); vi != views.end(); vi++ ) {
    layer_manager.rebuild( (*vi), PF::PF_COLORSPACE_RGB, 100, 100 );
  }
  layer_manager.rebuild_finalize();

  //signal_modified.emit();
  //std::cout<<"PF::Image::update(): signal_modified() emitted."<<std::endl;
}


void PF::Image::update_async()
{
#ifndef NDEBUG
  std::cout<<"PF::Image::update_async(): called, rebuilding="<<rebuilding<<std::endl;
#endif
  Glib::Threads::Mutex::Lock lock( rebuild_mutex );
  modified = true;

#ifndef NDEBUG
  std::cout<<"PF::Image::update_async(): installing idle callback"<<std::endl;
#endif
  // Install idle callback to handle the re-building of the associated views
  gtk_idle_add( PF::image_rebuild_callback, (gpointer)this );  
}


bool PF::Image::open( std::string filename )
{
  std::string ext;
  if( getFileExtension( "/", filename, ext ) &&
      ext == "pfi" ) {

    PF::load_pf_image( filename, this );
    //PF::PhotoFlow::Instance().set_image( pf_image );
    //layersWidget.set_image( pf_image );
    add_view( VIPS_FORMAT_UCHAR, 0 );

  } else {

    //PF::PhotoFlow::Instance().set_image( pf_image );
    //layersWidget.set_image( pf_image );

    add_view( VIPS_FORMAT_UCHAR, 0 );

    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "imageread", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( "background" );
    layer_manager.get_layers().push_back( limg );

    /*
    PF::Processor<PF::ImageReaderPar,PF::ImageReader>* imgread = 
      new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
    imgread->get_par()->set_file_name( filename );

    PF::Layer* limg = layer_manager.new_layer();
    limg->set_processor( imgread );
    limg->set_name( "background" );

    PF::ImageReadConfigDialog* img_config = 
      new PF::ImageReadConfigDialog( limg );
    imgread->get_par()->set_config_ui( img_config );
    //img_config->set_layer( limg );
    //img_config->set_image( pf_image );

    //layer_manager.get_layers().push_back( limg );
    layersWidget.add_layer( limg );
    */
  }

  //imageArea.set_view( pf_image->get_view(0) );
  //pf_image->signal_modified.connect( sigc::mem_fun(&imageArea, &ImageArea::update_image) );
  update();
}


bool PF::Image::save( std::string filename )
{
  std::ofstream of;
  of.open( filename.c_str() );
  if( !of ) return false;
  of<<"<image>"<<std::endl;
  layer_manager.save( of );
  of<<"</image>"<<std::endl;
}
