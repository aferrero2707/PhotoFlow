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
#include <algorithm>

#include <gtk/gtk.h>

#include "image.hh"
#include "imageprocessor.hh"
#include "pf_file_loader.hh"
#include "../operations/convert2srgb.hh"
#include "../operations/convertformat.hh"


static bool getFileExtension(const char * dir_separator, const std::string & file, std::string & ext)
{
    std::size_t ext_pos = file.rfind(".");
    std::size_t dir_pos = file.rfind(dir_separator);

    if(ext_pos>dir_pos+1)
    {
        ext.append(file.begin()+ext_pos+1,file.end());
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return true;
    }

    return false;
}



gint PF::image_rebuild_callback( gpointer data )
{
  PF::Image* image = (PF::Image*)data;
#ifndef NDEBUG
  //std::cout<<"PF::image_rebuild_callback(): called, image->is_modified()="<<image->is_modified()<<std::endl;
#endif
  if( image->is_modified() ) {

    bool result = image->get_layer_manager().rebuild_prepare();
    /*
#ifndef NDEBUG
    std::cout<<"PF::image_rebuild_callback(): rebuild prepare "<<(result?"OK":"failed")<<std::endl;
#endif
    if( !result ) {
      // something wrong here, we release all the locks and stop the update
      // in fact, this should never happen...
      for( unsigned int i = 0; i < image->get_npipelines(); i++ ) {
	PF::Pipeline* pipeline = image->get_pipeline( i );
	if( !pipeline ) continue;
	pipeline->unlock_processing();
      }
      return false;
    }
    */
    
    image->clear_modified();

    // Loop on pipelines, re-build and update
    for( unsigned int i = 0; i < image->get_npipelines(); i++ ) {
      PF::Pipeline* pipeline = image->get_pipeline( i );
      if( !pipeline ) continue;

#ifndef NDEBUG
      std::cout<<"PF::image_rebuild_callback(): updating pipeline #"<<i<<std::endl;
#endif
      //vips_cache_drop_all();
      image->get_layer_manager().rebuild( pipeline, PF::PF_COLORSPACE_RGB, 100, 100, NULL );
      //pipeline->update();
    }

    image->get_layer_manager().rebuild_finalize();
  }
  return false;
}


PF::Image::Image(): 
  layer_manager( this ), 
  async( false ), 
  modified( false ), 
  rebuilding( false ), 
  disable_update( false )
{
  rebuild_mutex = vips_g_mutex_new();
  //g_mutex_lock( rebuild_mutex );
  rebuild_done = vips_g_cond_new();

  sample_mutex = vips_g_mutex_new();
  //g_mutex_lock( sample_mutex );
  sample_done = vips_g_cond_new();

  remove_layer_mutex = vips_g_mutex_new();
  //g_mutex_lock( remove_layer_mutex );
  remove_layer_done = vips_g_cond_new();

  layer_manager.signal_modified.connect(sigc::mem_fun(this, &Image::update_all) );
  convert2srgb = new PF::Processor<PF::Convert2sRGBPar,PF::Convert2sRGBProc>();
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();

  add_pipeline( VIPS_FORMAT_USHORT, 0 );
  add_pipeline( VIPS_FORMAT_USHORT, 0 );

  pipelines[0]->set_render_mode( PF_RENDER_PREVIEW );
  pipelines[1]->set_render_mode( PF_RENDER_PREVIEW );
}

PF::Image::~Image()
{
  for( unsigned int vi = 0; vi < pipelines.size(); vi++ ) {
    if( pipelines[vi] != NULL )
      delete pipelines[vi];
  }
}


// The area parameter represents the region of the image that was actually
// modified and that needs to be re-computed. This allows certain sinks
// to reduce the amount of computations in case only part of the image
// needs to be updated. If area is NULL, it means that the whole image 
// was changed.
void PF::Image::update( PF::Pipeline* target_pipeline, bool sync )
{
  std::cout<<"Image::update( "<<target_pipeline<<", "<<sync<<" ) called."<<std::endl;
  if( disable_update ) return;

  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_update( target_pipeline );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.pipeline = target_pipeline;
    request.request = PF::IMAGE_REBUILD;
		/*
		if(area) {
			request.area.left = area->left;
			request.area.top = area->top;
			request.area.width = area->width;
			request.area.height = area->height;
		} else {
		*/
		request.area.width = request.area.height = 0;
		//}
    
    if( sync ) g_mutex_lock( rebuild_mutex );
#ifndef NDEBUG
    std::cout<<"PF::Image::update(): submitting rebuild request..."<<std::endl;
#endif
    PF::ImageProcessor::Instance().submit_request( request );
#ifndef NDEBUG
    std::cout<<"PF::Image::update(): request submitted."<<std::endl;
#endif

    if( sync ) {
      std::cout<<"PF::Image::update(): waiting for rebuild_done...."<<std::endl;
      g_cond_wait( rebuild_done, rebuild_mutex );
      std::cout<<"PF::Image::update(): ... rebuild_done received."<<std::endl;
    }

    // In sync mode, the image is left in a locked state to allow further 
    // actions to be taken before any subsequent rebuild and reprocessing 
    // takes place
    //if( sync ) g_mutex_unlock( rebuild_mutex );
  }

  /*
  if( is_async() )
    update_async();
  else
    update_sync();
  */
}


void PF::Image::do_update( PF::Pipeline* target_pipeline )
{
  //std::cout<<"PF::Image::do_update(): is_modified()="<<is_modified()<<std::endl;
  //if( !is_modified() ) return;

#ifndef NDEBUG
  std::cout<<std::endl<<"============================================"<<std::endl;
#endif
  //std::cout<<"PF::Image::do_update(): is_modified()="<<is_modified()<<std::endl;
  bool result = get_layer_manager().rebuild_prepare();
  /*
    #ifndef NDEBUG
    std::cout<<"PF::image_rebuild_callback(): rebuild prepare "<<(result?"OK":"failed")<<std::endl;
    #endif
    if( !result ) {
    // something wrong here, we stop the update
    // in fact, this should never happen...
    return false;
    }
  */
    
  clear_modified();

  // Loop on pipelines, re-build and update
  for( unsigned int i = 0; i < get_npipelines(); i++ ) {
    PF::Pipeline* pipeline = get_pipeline( i );
    if( !pipeline ) continue;

		if( !target_pipeline) {
			// We do not target a specific pipeline
			// For the sake of performance, we only rebuild pipelines that have
			// sinks attached to them, as otherwise the pipeline can be considered inactive
			//if( !(pipeline->has_sinks()) ) continue;
		} else {
			// We only rebuild the target pipeline
			if( pipeline != target_pipeline ) continue;
		}

#ifndef NDEBUG
    std::cout<<"PF::Image::do_update(): updating pipeline #"<<i<<std::endl;
#endif
    //get_layer_manager().rebuild( pipeline, PF::PF_COLORSPACE_RGB, 100, 100, area );
    get_layer_manager().rebuild( pipeline, PF::PF_COLORSPACE_RGB, 100, 100, NULL );
#ifndef NDEBUG
    std::cout<<"PF::Image::do_update(): pipeline #"<<i<<" updated."<<std::endl;
#endif
    //pipeline->update();
  }

#ifndef NDEBUG
	std::cout<<"PF::Image::do_update(): finalizing..."<<std::endl;
#endif
  get_layer_manager().rebuild_finalize();
#ifndef NDEBUG
	std::cout<<"PF::Image::do_update(): finalizing done."<<std::endl;
#endif

#ifndef NDEBUG
  for( unsigned int i = 0; i < get_npipelines(); i++ ) {
    PF::Pipeline* pipeline = get_pipeline( i );
    if( !pipeline ) continue;
    std::cout<<"PF::Image::do_update(): ref_counts of pipeline #"<<i<<std::endl;
    for(int ni = 0; ni < pipeline->get_nodes().size(); ni++ ) {
      PF::PipelineNode* node = pipeline->get_nodes()[ni];
      if( !node ) continue;
      if( !(node->image) ) {
      std::cout<<"  node #"<<ni<<" ("<<(void*)node->image<<")"<<std::endl;
				continue;
			}
      std::cout<<"  node #"<<ni<<" ("<<(void*)node->image<<") = "<<G_OBJECT(node->image)->ref_count<<std::endl;
    }
  }
  std::cout<<std::endl<<"============================================"<<std::endl<<std::endl<<std::endl;
#endif
}



void PF::Image::sample( int layer_id, int x, int y, int size, 
												VipsImage** image, std::vector<float>& values )
{
	int left = (int)x-size/2;
	int top = (int)y-size/2;
	int width = size;
	int height = size;
	VipsRect area = {left, top, width, height};

  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_sample( layer_id, area );
  } else {
    ProcessRequestInfo request;
    request.image = this;
		request.layer_id = layer_id;
    request.request = PF::IMAGE_SAMPLE;
		request.area.left = area.left;
		request.area.top = area.top;
		request.area.width = area.width;
		request.area.height = area.height;
    
    g_mutex_lock( sample_mutex );
    //#ifndef NDEBUG
    std::cout<<"PF::Image::sample(): submitting sample request..."<<std::endl;
    //#endif
    PF::ImageProcessor::Instance().submit_request( request );
    //#ifndef NDEBUG
    std::cout<<"PF::Image::sample(): request submitted."<<std::endl;
    //#endif
    
    g_cond_wait( sample_done, sample_mutex );
    g_mutex_unlock( sample_mutex );

		if(image)
			*image = sampler_image;
    values.clear();
		values = sampler_values;
  }

  /*
  if( is_async() )
    update_async();
  else
    update_sync();
  */
}


void PF::Image::do_sample( int layer_id, VipsRect& area )
{
  // Get the default pipeline of the image 
  // (it is supposed to be at 1:1 zoom level 
  // and floating point accuracy)
  PF::Pipeline* pipeline = get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"Image::do_sample(): NULL pipeline"<<std::endl;
    return;
  }

  // Get the node associated to the layer
  PF::PipelineNode* node = pipeline->get_node( layer_id );
  if( !node ) {
    std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
    return;
  }

  // Finally, get the underlying VIPS image associated to the layer
  VipsImage* image = node->image;
  if( !image ) {
    std::cout<<"Image::do_sample(): NULL image"<<std::endl;
    return;
  }

	// Now we have to process a small portion of the image 
	// to get the corresponding Lab values
	VipsImage* spot;
	VipsRect all = {0 ,0, image->Xsize, image->Ysize};
	VipsRect clipped;
	vips_rect_intersectrect( &area, &all, &clipped );
  
	if( vips_crop( image, &spot, 
								 clipped.left, clipped.top, 
								 clipped.width+1, clipped.height+1, 
								 NULL ) ) {
    std::cout<<"Image::do_sample(): vips_crop() failed"<<std::endl;
		return;
  }

	VipsRect rspot = {0 ,0, spot->Xsize, spot->Ysize};

	//VipsImage* outimg = im_open( "spot_wb_img", "p" );
	//if (vips_sink_screen (spot, outimg, NULL,
	//											64, 64, 1, 
  //												0, NULL, this))
	//	return;
  
  PF::ProcessorBase* convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  std::vector<VipsImage*> in;
  in.push_back( spot );
  convert_format->get_par()->set_image_hints( spot );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  unsigned int level = 0;
  VipsImage* outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  if( outimg == NULL ) {
    std::cout<<"Image::do_sample(): NULL image after convert_format"<<std::endl;
    return;
  }
  PF_UNREF( spot, "Image::do_sample() spot unref" )
	//if( vips_sink_memory( spot ) )
	//  return;

  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount before vips_region_new()" )
	VipsRegion* region = vips_region_new( outimg );
	if (vips_region_prepare (region, &rspot)) {
    std::cout<<"Image::do_sample(): vips_region_prepare() failed"<<std::endl;
		return;
  }
  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount after vips_region_new()" )

	int row, col, b;
	int line_size = clipped.width*image->Bands;
	float* p;
	float avg[16];
  for( int i = 0; i < 16; i++ ) avg[i] = 0;
	for( row = 0; row < clipped.height; row++ ) {
		p = (float*)VIPS_REGION_ADDR( region, rspot.left, rspot.top );
		for( col = 0; col < line_size; col += image->Bands ) {
			for( b = 0; b < image->Bands; b++ ) {
				avg[b] += p[col+b];
        std::cout<<"do_sample(): p["<<row<<"]["<<col+b<<"]="<<p[col+b]<<std::endl;
			}
		}
	}

	sampler_values.clear();
	for( b = 0; b < image->Bands; b++ ) {
		avg[b] /= clipped.width*clipped.height;
		sampler_values.push_back( avg[b] );
	}
	sampler_image = image;

	//g_object_unref( spot );
  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount before region unref" );
	PF_UNREF( region, "Image::do_sample(): region unref" );
  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount after region unref" );
  PF_UNREF( outimg, "Image::do_sample(): outimg unref" );
}



// The area parameter represents the region of the image that was actually
// modified and that needs to be re-computed. This allows certain sinks
// to reduce the amount of computations in case only part of the image
// needs to be updated. If area is NULL, it means that the whole image 
// was changed.
void PF::Image::remove_layer( PF::Layer* layer )
{
  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_remove_layer( layer );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.layer = layer;
    request.request = PF::IMAGE_REMOVE_LAYER;
    g_mutex_lock( remove_layer_mutex );
    PF::ImageProcessor::Instance().submit_request( request );
    g_cond_wait( remove_layer_done, remove_layer_mutex );
    g_mutex_unlock( remove_layer_mutex );
  }
}


void PF::Image::do_remove_layer( PF::Layer* layer )
{
  remove_from_inputs( layer );
  std::list<Layer*>* list = layer_manager.get_list( layer );
  if( list )
    remove_layer( layer, *list );
}


void PF::Image::remove_from_inputs( PF::Layer* layer )
{
  remove_from_inputs( layer, layer_manager.get_layers() );
}


void PF::Image::remove_from_inputs( PF::Layer* layer, std::list<Layer*>& list )
{
  std::list<PF::Layer*>::iterator li;
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    l->remove_input( layer->get_id() );
    remove_from_inputs( layer, l->get_sublayers() );
    remove_from_inputs( layer, l->get_imap_layers() );
    remove_from_inputs( layer, l->get_omap_layers() );
  }  
}


void PF::Image::remove_layer( PF::Layer* layer, std::list<Layer*>& list )
{
  std::vector<Pipeline*>::iterator vi;
  for( vi = pipelines.begin(); vi != pipelines.end(); vi++ ) {
    (*vi)->remove_node( layer->get_id() );
  }  

  std::list<Layer*>::iterator it;
  std::list<Layer*> sublayers = layer->get_sublayers();
  for( it = sublayers.begin(); it != sublayers.end(); it++ ) {
    remove_layer( *it, layer->get_sublayers() );
  }

  sublayers = layer->get_imap_layers();
  for( it = sublayers.begin(); it != sublayers.end(); it++ ) {
    remove_layer( *it, layer->get_imap_layers() );
  }

  sublayers = layer->get_omap_layers();
  for( it = sublayers.begin(); it != sublayers.end(); it++ ) {
    remove_layer( *it, layer->get_omap_layers() );
  }

  std::list<PF::Layer*>::iterator li;
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    if( l->get_id() == layer->get_id() ) {
      list.erase( li );
      break;
    }
  }

  layer_manager.delete_layer( layer );
}


bool PF::Image::open( std::string filename )
{
  std::string ext;
  if( !getFileExtension( "/", filename, ext ) ) return false;
  disable_update = true;
  if( ext == "pfi" ) {

    PF::load_pf_image( filename, this );
    //PF::PhotoFlow::Instance().set_image( pf_image );
    //layersWidget.set_image( pf_image );
    //add_pipeline( VIPS_FORMAT_UCHAR, 0 );
		file_name = filename;

  } else if( ext=="tiff" || ext=="tif" || ext=="jpg" || ext=="jpeg" ) {

    //PF::PhotoFlow::Instance().set_image( pf_image );
    //layersWidget.set_image( pf_image );

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
  } else {
    
    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "raw_loader", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( "RAW loader" );
    layer_manager.get_layers().push_back( limg );

		/*
    limg = layer_manager.new_layer();
    proc = PF::PhotoFlow::Instance().new_operation( "raw_developer", limg );
    limg->set_processor( proc );
    limg->set_name( "RAW developer" );
    layer_manager.get_layers().push_back( limg );
		*/

    /*
    limg = layer_manager.new_layer();
    proc = PF::PhotoFlow::Instance().new_operation( "raw_output", limg );
    limg->set_processor( proc );
    limg->set_name( "RAW output" );
    layer_manager.get_layers().push_back( limg );
    */
  }
  disable_update = false;

  //imageArea.set_pipeline( pf_image->get_pipeline(0) );
  //pf_image->signal_modified.connect( sigc::mem_fun(&imageArea, &ImageArea::update_image) );
  //sleep(5);
  //update();
}


bool PF::Image::save( std::string filename )
{
  std::string ext;
  if( getFileExtension( "/", filename, ext ) &&
      ext == "pfi" ) {

    std::ofstream of;
    of.open( filename.c_str() );
    if( !of ) return false;
    of<<"<image version=\"2\">"<<std::endl;
    layer_manager.save( of );
    of<<"</image>"<<std::endl;
		file_name = filename;
    return true;
  } else {
    return false;
  }
}



bool PF::Image::export_merged( std::string filename )
{
  std::string ext;
  if( getFileExtension( "/", filename, ext ) &&
      ext != "pfi" ) {
    //Glib::Threads::Mutex::Lock lock( rebuild_mutex );
    unsigned int level = 0;
    //PF::Pipeline* pipeline = new PF::Pipeline( this, VIPS_FORMAT_USHORT, 0, PF_RENDER_NORMAL );
    PF::Pipeline* pipeline = new PF::Pipeline( this, VIPS_FORMAT_FLOAT, 0, PF_RENDER_NORMAL );
    layer_manager.rebuild_all( pipeline, PF::PF_COLORSPACE_RGB, 100, 100 );
    VipsImage* image = pipeline->get_output();
    VipsImage* outimg = image;

    std::vector<VipsImage*> in;
    /*
    convert2srgb->get_par()->set_image_hints( image );
    convert2srgb->get_par()->set_format( pipeline->get_format() );
    in.clear(); in.push_back( image );
    VipsImage* srgbimg = convert2srgb->get_par()->build(in, 0, NULL, NULL, level );
    //g_object_unref( image );
    std::string msg = std::string("PF::Image::export_merged(") + filename + "), image";
    PF_UNREF( image, msg.c_str() );
    */
    VipsImage* srgbimg = image;

		outimg = srgbimg;
    std::string msg;
		if( ext == "jpg" || ext == "jpeg" ) {
      in.clear();
      in.push_back( srgbimg );
      convert_format->get_par()->set_image_hints( srgbimg );
      convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
      outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
      //g_object_unref( srgbimg );
      // msg = std::string("PF::Image::export_merged(") + filename + "), srgbimg";
      //PF_UNREF( srgbimg, msg.c_str() );
		}
    
		if( ext == "tif" || ext == "tiff" ) {
      in.clear();
      in.push_back( srgbimg );
      convert_format->get_par()->set_image_hints( srgbimg );
      convert_format->get_par()->set_format( VIPS_FORMAT_USHORT );
      outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
      //g_object_unref( srgbimg );
      //msg = std::string("PF::Image::export_merged(") + filename + "), srgbimg";
      //PF_UNREF( srgbimg, msg.c_str() );
		}
    
#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
    vips_image_write_to_file( outimg, filename.c_str() );
#else
    vips_image_write_to_file( outimg, filename.c_str(), NULL );
#endif
    //g_object_unref( outimg );
    msg = std::string("PF::Image::export_merged(") + filename + "), outimg";
    PF_UNREF( outimg, msg.c_str() );
    delete pipeline;
    return true;
  } else {
    return false;
  }
}
