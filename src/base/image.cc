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
#include <fstream>

#include "fileutils.hh"
//#include "pf_mkstemp.hh"
#include "exif_data.hh"
#include "image.hh"
#include "imageprocessor.hh"
#include "pf_file_loader.hh"
#include "../operations/operations.hh"
//#include "../operations/convert2srgb.hh"
//#include "../operations/convertformat.hh"
#include "../operations/icc_transform.hh"
//#include "../operations/gmic/gmic_untiled_op.hh"

#define BENCHMARK
#include "../rt/rtengine/StopWatch.h"



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
          modified_flag( false ),
          rebuilding( false ),
          loaded( false ),
          disable_update( false ),
          force_synced_update( false )
{
  rebuild_mutex = vips_g_mutex_new();
  //g_mutex_lock( rebuild_mutex );
  rebuild_done = vips_g_cond_new();

  export_mutex = vips_g_mutex_new();
  //g_mutex_lock( export_mutex );
  export_done = vips_g_cond_new();

  sample_mutex = vips_g_mutex_new();
  //g_mutex_lock( sample_mutex );
  sample_done = vips_g_cond_new();

  remove_layer_mutex = vips_g_mutex_new();
  //g_mutex_lock( remove_layer_mutex );
  remove_layer_done = vips_g_cond_new();

  layer_manager.signal_modified.connect(sigc::mem_fun(this, &Image::update_all) );
  layer_manager.signal_modified.connect(sigc::mem_fun(this, &Image::modified) );
  //convert2srgb = new PF::Processor<PF::Convert2sRGBPar,PF::Convert2sRGBProc>();
  convert_format = new_convert_format();
  convert2outprof = new_icc_transform();

  //add_pipeline( VIPS_FORMAT_UCHAR, 0 );
  //add_pipeline( VIPS_FORMAT_UCHAR, 0 );
  //add_pipeline( VIPS_FORMAT_USHORT, 0 );
  //add_pipeline( VIPS_FORMAT_USHORT, 0 );

  //pipelines[0]->set_render_mode( PF_RENDER_PREVIEW );
  //pipelines[1]->set_render_mode( PF_RENDER_PREVIEW );
}

PF::Image::~Image()
{
  /*
  for( unsigned int vi = 0; vi < pipelines.size(); vi++ ) {
    if( pipelines[vi] != NULL )
      delete pipelines[vi];
  }
   */
}


void PF::Image::lock()
{
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  //std::cout<<"  LOCKING REBUILD MUTEX"<<std::endl;
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  g_mutex_lock( rebuild_mutex);
}

void PF::Image::unlock()
{
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  UNLOCKING REBUILD MUTEX"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
  g_mutex_unlock( rebuild_mutex);
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  REBUILD MUTEX UNLOCKED"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
}

void PF::Image::sample_lock()
{
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  //std::cout<<"  LOCKING SAMPLE MUTEX"<<std::endl;
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  //g_mutex_lock( sample_mutex);
  sample_cond.lock();
}

void PF::Image::sample_unlock()
{
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  UNLOCKING SAMPLE MUTEX"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
  //g_mutex_unlock( sample_mutex);
  sample_cond.unlock();
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  SAMPLE MUTEX UNLOCKED"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
}


void PF::Image::destroy_lock()
{
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  //std::cout<<"  LOCKING SAMPLE MUTEX"<<std::endl;
  //std::cout<<"+++++++++++++++++++++"<<std::endl;
  //g_mutex_lock( sample_mutex);
  destroy_cond.lock();
}

void PF::Image::destroy_unlock()
{
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  UNLOCKING SAMPLE MUTEX"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
  //g_mutex_unlock( sample_mutex);
  destroy_cond.unlock();
  //std::cout<<"---------------------"<<std::endl;
  //std::cout<<"  SAMPLE MUTEX UNLOCKED"<<std::endl;
  //std::cout<<"---------------------"<<std::endl;
}




// The area parameter represents the region of the image that was actually
// modified and that needs to be re-computed. This allows certain sinks
// to reduce the amount of computations in case only part of the image
// needs to be updated. If area is NULL, it means that the whole image 
// was changed.
void PF::Image::set_pipeline_level( PF::Pipeline* target_pipeline, int level )
{
#ifndef NDEBUG
  std::cout<<"Image::set_pipeline_level( "<<target_pipeline<<", "<<level<<" ) called."<<std::endl;
#endif

  if( !target_pipeline ) return;

  if( PF::PhotoFlow::Instance().is_batch() ) {
    target_pipeline->set_level( level );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.pipeline = target_pipeline;
    request.level = level;
    request.request = PF::IMAGE_PIPELINE_SET_LEVEL;
    PF::ImageProcessor::Instance().submit_request( request );
  }
}


void PF::Image::update( PF::Pipeline* target_pipeline, bool sync )
{
#ifndef NDEBUG
  std::cout<<"Image::update( "<<target_pipeline<<", "<<sync<<" ) called."<<std::endl;
#endif
  if( disable_update ) return;

  if( force_synced_update )
    sync = true;

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

    if( sync ) rebuild_done_reset(); //rebuild_cond.lock(); //g_mutex_lock( rebuild_mutex );
#ifndef NDEBUG
    std::cout<<"PF::Image::update(): submitting rebuild request..."<<std::endl;
#endif
    PF::ImageProcessor::Instance().submit_request( request );
#ifndef NDEBUG
    std::cout<<"PF::Image::update(): request submitted."<<std::endl;
#endif

    if( sync ) {
#ifndef NDEBUG
      std::cout<<"PF::Image::update(): waiting for rebuild_done...."<<std::endl;
#endif
      //unlock(); //g_mutex_unlock( rebuild_mutex );
      //g_cond_wait( rebuild_done, rebuild_mutex );
      //rebuild_cond.wait();
      //rebuild_cond.unlock();
      rebuild_done_wait( true );
#ifndef NDEBUG
      std::cout<<"PF::Image::update(): ... rebuild_done received."<<std::endl;
#endif
    }

    // In sync mode, the image is left in a locked state to allow further 
    // actions to be taken before any subsequent rebuild and reprocessing 
    // takes place
    //if( sync && target_pipeline ) {
    //  std::cout<<"PF::Image::update(): unlocking rebuild mutex after condition...."<<std::endl;
    //}
  }

  /*
  if( is_async() )
    update_async();
  else
    update_sync();
   */
}


void PF::Image::do_update( PF::Pipeline* target_pipeline, bool update_gui )
{
  //std::cout<<"PF::Image::do_update(): is_modified()="<<is_modified()<<std::endl;
  //if( !is_modified() ) return;

  // Set the rebuild condition to FALSE
  //rebuild_done_reset();

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

  //clear_modified();

  //vips_cache_drop_all();

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

    if( pipeline->get_auto_zoom() ) {
      // This pipeline requires to automatically set the zoom level so that
      // the width and height do not exceed a given size, so we have to
      // look into the previously processed pipelines to get the most
      // accurate estimate of the full-res image
      unsigned int level_min = 1000;
      PF::Pipeline* hires_pipeline = NULL;
      for( unsigned int j = 0; j < i; j++ ) {
        PF::Pipeline* pipeline2 = get_pipeline( j );
        if( !pipeline2 ) continue;
        //std::cout<<"pipeline("<<j<<")->get_level()="<<pipeline2->get_level()<<std::endl;
        if( pipeline2->get_level() < level_min ) {
          hires_pipeline = pipeline2;
          level_min = pipeline2->get_level();
        }
      }
      int level = -1;
      if( hires_pipeline ) {
        level = (int)hires_pipeline->get_level();
        //std::cout<<"hires_pipeline->get_level()="<<level<<std::endl;
        VipsImage* hires_image = NULL;
        if( pipeline->get_output_layer_id() >= 0 ) {
          PF::PipelineNode* node = hires_pipeline->get_node( pipeline->get_output_layer_id() );
          if( node ) hires_image = node->blended;
        } else {
          hires_image = hires_pipeline->get_output();
        }
        if( hires_image ) {
          int w = hires_image->Xsize;
          int h = hires_image->Ysize;
          //std::cout<<"hires_image dimensions: "<<w<<","<<h<<std::endl;
          while( (w > pipeline->get_auto_zoom_width()) ||
              (h > pipeline->get_auto_zoom_height()) ) {
            w /= 2; h /= 2; level += 1;
          }
          //std::cout<<"auto_zoom dimensions: "<<w<<","<<h<<"  level: "<<level<<std::endl;
        }
      }
      if( level >= 0 ) {
        pipeline->set_level( level );
      }
    }

//#ifndef NDEBUG
    std::cout<<"PF::Image::do_update(): updating pipeline #"<<i<<std::endl;
//#endif
    //get_layer_manager().rebuild( pipeline, PF::PF_COLORSPACE_RGB, 100, 100, area );
    get_layer_manager().rebuild( pipeline, PF::PF_COLORSPACE_RGB, 100, 100, NULL );
//#ifndef NDEBUG
    std::cout<<"PF::Image::do_update(): pipeline #"<<i<<" updated."<<std::endl;
//#endif
    //pipeline->update();
  }

  //std::cout<<"PF::Image::update(): waiting for rebuild_done...."<<std::endl;
  // Set the rebuild condition to TRUE and emit the signal
  rebuild_done_signal();
  //rebuild_cond.unlock();
  //std::cout<<"PF::Image::do_update(): signaling done condition."<<std::endl;
  signal_updated.emit();

#ifndef NDEBUG
  std::cout<<"PF::Image::do_update(): finalizing..."<<std::endl;
#endif
  bool _update_gui;
  if( target_pipeline ) _update_gui = false;
  else _update_gui = update_gui;
  get_layer_manager().rebuild_finalize( _update_gui );
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

  save_backup();
}



void PF::Image::sample( int layer_id, int x, int y, int size,
    VipsImage** image, std::vector<float>& values ) {
  int left = (int)x-size/2;
  int top = (int)y-size/2;
  int width = size;
  int height = size;
  VipsRect area = {left, top, width, height};
  std::vector<VipsRect> areas; areas.push_back(area);
  sample(layer_id, areas, false, image, values);
}



void PF::Image::sample( int layer_id, std::vector<VipsRect>& areas, bool weighted,
    VipsImage** image, std::vector<float>& values )
{

  if( true || PF::PhotoFlow::Instance().is_batch() ) {
    do_sample( layer_id, areas, weighted );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.layer_id = layer_id;
    request.request = PF::IMAGE_SAMPLE;
    request.areas = areas;
    request.weighted_average = weighted;

    sample_lock(); //g_mutex_lock( sample_mutex );
#ifndef NDEBUG
    std::cout<<"PF::Image::sample(): submitting sample request..."<<std::endl;
#endif
    PF::ImageProcessor::Instance().submit_request( request );
#ifndef NDEBUG
    std::cout<<"PF::Image::sample(): request submitted."<<std::endl;
#endif

    //g_cond_wait( sample_done, sample_mutex );
    //std::cout<<"Image::sample(): unlocking mutex."<<std::endl;
    //sample_unlock(); //g_mutex_unlock( sample_mutex );
#ifndef NDEBUG
    std::cout<<"Image::sample(): waiting for done."<<std::endl;
#endif
    sample_cond.wait();
    sample_unlock();
#ifndef NDEBUG
    std::cout<<"Image::sample(): done received."<<std::endl;
#endif
  }

  if(image) *image = sampler_image;
  values.clear();
  values = sampler_values;

  /*
  if( is_async() )
    update_async();
  else
    update_sync();
   */
}


void PF::Image::do_sample( int layer_id, std::vector<VipsRect>& areas, bool weighted )
{
  //std::cout<<"Image::do_sample(): waiting for rebuild_done..."<<std::endl;
  //rebuild_lock();
  //rebuild_done_wait( true );
  //std::cout<<"Image::do_sample(): rebuild_done received"<<std::endl;

#ifndef NDEBUG
  std::cout<<"Image::do_sample(): locking image..."<<std::endl;
#endif
  lock();
#ifndef NDEBUG
  std::cout<<"Image::do_sample(): image locked"<<std::endl;
#endif

  // Get the default pipeline of the image 
  // (it is supposed to be at 1:1 zoom level 
  // and floating point accuracy)
  PF::Pipeline* pipeline = get_pipeline( 0 );
  if( !pipeline ) {
    std::cout<<"Image::do_sample(): NULL pipeline"<<std::endl;
    std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
    unlock();
    return;
  }

  // Get the node associated to the layer
  PF::PipelineNode* node = pipeline->get_node( layer_id );
  if( !node ) {
    std::cout<<"Image::do_sample(): NULL pipeline node"<<std::endl;
    std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
    unlock();
    return;
  }

  // Finally, get the underlying VIPS image associated to the layer
  VipsImage* image = node->image;
  if( !image ) {
    std::cout<<"Image::do_sample(): NULL image"<<std::endl;
    std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
    unlock();
    return;
  }

  // Now we have to process a small portion of the image
  // to get the corresponding Lab values
  VipsImage* spot = image;
  VipsRect all = {0 ,0, image->Xsize, image->Ysize};

  /*
	if( vips_crop( image, &spot, 
								 clipped.left, clipped.top, 
								 clipped.width+1, clipped.height+1, 
								 NULL ) ) {
    std::cout<<"Image::do_sample(): vips_crop() failed"<<std::endl;
		return;
  }
	VipsRect rspot = {0 ,0, spot->Xsize, spot->Ysize};
   */

  //VipsImage* outimg = im_open( "spot_wb_img", "p" );
  //if (vips_sink_screen (spot, outimg, NULL,
  //											64, 64, 1,
  //												0, NULL, this))
  //	return;

  PF::ProcessorBase* convert_format = new_convert_format();
  std::vector<VipsImage*> in;
  in.push_back( spot );
  convert_format->get_par()->set_image_hints( spot );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  unsigned int level = 0;
  VipsImage* outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  if( outimg == NULL ) {
    std::cout<<"Image::do_sample(): NULL image after convert_format"<<std::endl;
    std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
    unlock();
    return;
  }
  //PF_UNREF( spot, "Image::do_sample() spot unref" )
  //if( vips_sink_memory( spot ) )
  //  return;

  float wtot = 0;
  std::vector< std::vector<float> > sumv;
  std::vector<float> weights;
  for(unsigned int ai = 0; ai < areas.size(); ai++) {
    VipsRect& area = areas[ai];
    VipsRect clipped;
    vips_rect_intersectrect( &area, &all, &clipped );
    VipsRect rspot = {clipped.left, clipped.top, clipped.width+1, clipped.height+1};
    //std::cout<<"Image::do_sample(): rspot="<<rspot.width<<","<<rspot.height<<"+"<<rspot.left<<"+"<<rspot.top<<std::endl;

    //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount before vips_region_new()" )
    VipsRegion* region = vips_region_new( outimg );
    if (vips_region_prepare (region, &rspot)) {
      std::cout<<"Image::do_sample(): vips_region_prepare() failed"<<std::endl;
      std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
      unlock();
      return;
    }
    //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount after vips_region_new()" )

    int row, col, b;
    int line_size = clipped.width*image->Bands;
    float* p;
    std::vector<float> sum;
    for( int i = 0; i < 16; i++ ) sum.push_back(0);
    for( row = 0; row < clipped.height; row++ ) {
      p = (float*)VIPS_REGION_ADDR( region, rspot.left, rspot.top+row );
      //std::cout<<"do_sample(): rspot.left="<<rspot.left<<"  rspot.top+row="<<rspot.top+row<<std::endl;
      for( col = 0; col < line_size; col += image->Bands ) {
        for( b = 0; b < image->Bands; b++ ) {
          sum[b] += p[col+b];
          //std::cout<<"do_sample(): p["<<row<<"]["<<col+b<<"]="<<p[col+b]<<std::endl;
        }
      }
    }
    sumv.push_back(sum);
    weights.push_back(clipped.width*clipped.height);
    wtot += clipped.width*clipped.height;

    PF_UNREF( region, "Image::do_sample(): region unref" );
  }

  sampler_values.clear();
  if( weighted ) {
    for( int b = 0; b < image->Bands; b++ ) {
      float bsum = 0;
      for(int ai = 0; ai < sumv.size(); ai++) {
        bsum += sumv[ai][b];
      }
#ifndef NDEBUG
      std::cout<<"sampler_values.push_back("<<sum/(sumv.size())<<")"<<std::endl;
#endif
      sampler_values.push_back( bsum/wtot );
    }
    sampler_image = image;
  } else {
    for( int b = 0; b < image->Bands; b++ ) {
      float bsum = 0;
      for(int ai = 0; ai < sumv.size(); ai++) {
        bsum += sumv[ai][b] / weights[ai];
      }
#ifndef NDEBUG
      std::cout<<"sampler_values.push_back("<<sum/(sumv.size())<<")"<<std::endl;
#endif
      sampler_values.push_back( bsum/(sumv.size()) );
    }
    sampler_image = image;
  }
#ifndef NDEBUG
  std::cout<<"Image::do_sample() finished."<<std::endl;
#endif

  //g_object_unref( spot );
  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount before region unref" );
  //PF_PRINT_REF( outimg, "Image::do_sample(): outimg refcount after region unref" );
  PF_UNREF( outimg, "Image::do_sample(): outimg unref" );

#ifndef NDEBUG
  std::cout<<"Image::do_sample(): unlocking image."<<std::endl;
#endif
  unlock();
}


void PF::Image::destroy()
{
  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_destroy();
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.request = PF::IMAGE_DESTROY;

    // Set the rebuild condition to FALSE
    rebuild_done_reset();
    //destroy_lock(); //g_mutex_lock( sample_mutex );
#ifndef NDEBUG
    std::cout<<"PF::Image::destroy(): submitting destroy request..."<<std::endl;
#endif
    PF::ImageProcessor::Instance().submit_request( request );
#ifndef NDEBUG
    std::cout<<"PF::Image::destroy(): request submitted."<<std::endl;
#endif

#ifndef NDEBUG
    std::cout<<"Image::destroy(): waiting for done."<<std::endl;
#endif
    //destroy_cond.wait();
    //destroy_unlock();
    rebuild_done_wait( true );
#ifndef NDEBUG
    std::cout<<"Image::destroy(): done received."<<std::endl;
#endif
  }
}


void PF::Image::do_destroy()
{
#ifndef NDEBUG
  std::cout<<"Image::do_destroy() called."<<std::endl;
#endif
  // Set the rebuild condition to FALSE
  //rebuild_done_reset();

  for( unsigned int vi = 0; vi < pipelines.size(); vi++ ) {
    if( pipelines[vi] != NULL ) {
#ifndef NDEBUG
      std::cout<<"Image::do_destroy(): deleting pipeline #"<<vi<<std::endl;
#endif
      delete pipelines[vi];
#ifndef NDEBUG
      std::cout<<"Image::do_destroy(): pipeline #"<<vi<<" delete"<<std::endl;
#endif
    }
  }
#ifndef NDEBUG
  //std::cout<<"Image::do_destroy(): deleting convert2srgb"<<std::endl;
#endif
  //delete convert2srgb;
#ifndef NDEBUG
  std::cout<<"Image::do_destroy(): convert2srgb deleted"<<std::endl;
  std::cout<<"Image::do_destroy(): deleting convert_format"<<std::endl;
#endif
  delete convert_format;
#ifndef NDEBUG
  std::cout<<"Image::do_destroy(): convert_format deleted"<<std::endl;
  std::cout<<"Image::do_destroy(): deleting convert2outprof"<<std::endl;
#endif
  delete convert2outprof;
#ifndef NDEBUG
  std::cout<<"Image::do_destroy(): convert2outprof deleted"<<std::endl;
#endif

  // Set the rebuild condition to TRUE and emit the signal
#ifndef NDEBUG
  std::cout<<"Image::do_destroy() finished."<<std::endl;
#endif
  rebuild_done_signal();
}


void PF::Image::remove_layer( PF::Layer* layer )
{
  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_remove_layer( layer );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.layer = layer;
    request.request = PF::IMAGE_REMOVE_LAYER;
    //g_mutex_lock( remove_layer_mutex );
    PF::ImageProcessor::Instance().submit_request( request );
    //g_cond_wait( remove_layer_done, remove_layer_mutex );
    //g_mutex_unlock( remove_layer_mutex );
  }
}


void PF::Image::do_remove_layer( PF::Layer* layer )
{
  // Set the rebuild condition to FALSE
  //rebuild_done_reset();

  std::list<Layer*> children;
  layer_manager.get_child_layers( layer, children );
  for( std::list<Layer*>::iterator i = children.begin(); i != children.end(); i++ ) {
    if( !(*i) ) continue;
    (*i)->set_dirty( true );
  }

  remove_from_inputs( layer );
  std::list<Layer*>* list = layer_manager.get_list( layer );
  if( list )
    remove_layer( layer, *list );

  // The rebuild condition will be cleared and signaled when updating the image
  //rebuild_done_signal();
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
#ifndef NDEBUG
  if( layer ) std::cout<<"Image::remove_layer(\""<<layer->get_name()<<"\") called."<<std::endl;
#endif
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


bool PF::Image::open( std::string filename, std::string bckname )
{
  std::string ext;
  if( !getFileExtensionLowcase( "/", filename, ext ) ) return false;
  disable_update = true;

#ifndef NDEBUG
  std::cout<<"ext: "<<ext<<std::endl;
#endif

  if( !bckname.empty() ) {

    std::cout<<"Opening image backup "<<bckname<<std::endl;

    PF::load_pf_image( bckname, this );
    file_name = filename;
    backup_file_name = bckname;

  } else if( ext == "pfi" ) {

    std::cout<<"Opening PFI image "<<filename<<std::endl;

    loaded = false;
    if( PF::load_pf_image( filename, this ) ) {
      //PF::PhotoFlow::Instance().set_image( pf_image );
      //layersWidget.set_image( pf_image );
      //add_pipeline( VIPS_FORMAT_UCHAR, 0 );
      file_name = filename;
    } else {
      return false;
    }

  } else if( ext=="tiff" || ext=="tif" || ext=="jpg" || ext=="jpeg" || ext=="png" || ext=="exr" || ext=="fits" || ext=="fts" || ext=="fit" ) {

    //PF::PhotoFlow::Instance().set_image( pf_image );
    //layersWidget.set_image( pf_image );

    std::cout<<"Opening raster image "<<filename<<std::endl;

    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "imageread", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( _("background") );
    layer_manager.get_layers().push_back( limg );
    file_name = filename;

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

    std::cout<<"Opening RAW image "<<filename<<std::endl;

    PF::Layer* limg = layer_manager.new_layer();
    PF::ProcessorBase* proc = PF::PhotoFlow::Instance().new_operation( "raw_loader", limg );
    if( proc->get_par() && proc->get_par()->get_property( "file_name" ) )
      proc->get_par()->get_property( "file_name" )->set_str( filename );
    limg->set_processor( proc );
    limg->set_name( "RAW loader" );
    layer_manager.get_layers().push_back( limg );

    if( !PF::PhotoFlow::Instance().is_batch() ) {
      Glib::ustring profile;
#ifdef __WIN32__
      profile = PF::PhotoFlow::Instance().get_presets_dir() + "\\default.pfp";
#else
      profile = PF::PhotoFlow::Instance().get_presets_dir() + "/default.pfp";
#endif
      struct stat buffer;
      int stat_result = stat(profile.c_str(), &buffer);
      if( stat_result != 0 ) profile = "";

      if( !profile.empty() &&
          PF::PhotoFlow::Instance().get_options().get_apply_default_preset() ) {
        PF::insert_pf_preset( profile.c_str(), this, NULL, &(layer_manager.get_layers()), false );
      } else {
        PF::Layer* limg2 = layer_manager.new_layer();
        PF::ProcessorBase* proc2 = PF::PhotoFlow::Instance().new_operation( "raw_developer_v2", limg2 );
        limg2->set_processor( proc2 );
        limg2->set_name( "RAW developer" );
        layer_manager.get_layers().push_back( limg2 );
      }
    }

    file_name = filename;

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

  return true;
}


bool PF::Image::save( std::string filename, bool do_clear )
{
  std::string ext;
  if( getFileExtension( "/", filename, ext ) &&
      ext == "pfi" ) {

    std::ofstream of;
    of.open( filename.c_str() );
    if( !of ) return false;
    of<<"<image version=\""<<PF_FILE_VERSION<<"\">"<<std::endl;
    layer_manager.save( of );
    of<<"</image>"<<std::endl;
    file_name = filename;
    if(do_clear) clear_modified();
    return true;
  } else {
    return false;
  }
}



bool PF::Image::save_backup()
{
  if( backup_file_name.empty() ) return false;

  std::ofstream of;
  of.open( backup_file_name.c_str() );
  if( !of ) return false;
  of<<"<image version=\""<<PF_FILE_VERSION<<"\">"<<std::endl;
  layer_manager.save( of );
  of<<"</image>"<<std::endl;
  return true;
}



void PF::Image::export_merged( std::string filename )
{
  if( PF::PhotoFlow::Instance().is_batch() ) {
    do_export_merged( filename );
  } else {
    ProcessRequestInfo request;
    request.image = this;
    request.request = PF::IMAGE_EXPORT;
    request.area.width = request.area.height = 0;
    request.filename = filename;

#ifndef NDEBUG
    std::cout<<"PF::Image::export_merged(): locking mutex..."<<std::endl;
#endif
    //g_mutex_lock( export_mutex );
#ifndef NDEBUG
    std::cout<<"PF::Image::export_merged(): submitting export request..."<<std::endl;
#endif
    PF::ImageProcessor::Instance().submit_request( request );
#ifndef NDEBUG
    std::cout<<"PF::Image::export_merged(): request submitted."<<std::endl;
#endif

    //std::cout<<"PF::Image::export_merged(): waiting for export_done...."<<std::endl;
    //g_cond_wait( export_done, export_mutex );
    //std::cout<<"PF::Image::export_merged(): ... export_done received."<<std::endl;

    //g_mutex_unlock( export_mutex );
  }
}


void PF::Image::do_export_merged( std::string filename )
{
  std::string ext;
  if( getFileExtension( "/", filename, ext ) &&
      ext != "pfi" ) {
    std::cout<<"Saving image to file "<<filename<<"..."<<std::endl;
    //Glib::Threads::Mutex::Lock lock( rebuild_mutex );
    unsigned int level = 0;
    PF::Pipeline* pipeline = add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_NORMAL );
    if( pipeline ) pipeline->set_op_caching_enabled( true );
    //PF::Pipeline* pipeline = add_pipeline( VIPS_FORMAT_USHORT, 0, PF_RENDER_NORMAL );
    do_update();
    /*
    while( true ) {
      PF::CacheBuffer* buf = layer_manager.get_cache_buffer( PF_RENDER_NORMAL );
      //std::cout<<"ImageProcessor::run(): buf="<<buf<<std::endl;
      if( !buf ) break;
      buf->write();
#warning "TODO: check if one can update only the export pipeline"
      do_update();
    }
     */

    std::string msg;

    VipsImage* image = pipeline->get_output();
    VipsImage* outimg = NULL;

    /*
    int tw = 128;
    int th = tw;
    int nt = (image->Xsize/tw + 1);
    VipsAccess acc = VIPS_ACCESS_RANDOM;
    int threaded = 1, persistent = 0;
    VipsImage* cached;
    if( !vips_tilecache(image, &cached,
        "tile_width", tw, "tile_height", th, "max_tiles", nt,
        "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
      //PF_UNREF( image, "Image::do_export_merged(): image unref" );
      image = cached;
    } else {
      std::cout<<"Image::do_export_merged(): vips_tilecache() failed."<<std::endl;
      PF_REF( image, "Image::do_export_merged(): image ref" );
    }
    */
    PF_REF( image, "Image::do_export_merged(): image ref" );

    bool saved = false;

    std::vector<VipsImage*> in;
    if( ext == "jpg" || ext == "jpeg" ) {
      /*
      in.clear();
      in.push_back( image );
      convert_format->get_par()->set_image_hints( image );
      convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
      outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
      PF_UNREF( image, "Image::do_export_merged(): image unref" );
      */
      //outimg = image;
      if( vips_linear1(image, &outimg, 255, 0, NULL) ) {
        std::cout<<"WARNING!!! Image::do_export_merged(): vips_linear1() failed"<<std::endl;
        outimg = image;
        PF_REF( image, "Image::do_export_merged(): image ref after vips_linear1() failed" );
      }
      PF_UNREF( image, "Image::do_export_merged(): image unref" );
      if( outimg ) {
        BENCHFUN
        Glib::Timer timer;
        timer.start();
        vips_jpegsave( outimg, filename.c_str(), "Q", 75, NULL );
        timer.stop();
        std::cout<<"Jpeg image saved in "<<timer.elapsed()<<" s"<<std::endl;
        if( PF::PhotoFlow::Instance().get_options().get_save_sidecar_files() != 0 &&
            !(PF::PhotoFlow::Instance().is_plugin()) ) {
          save(filename+".pfi", false);
        }
        saved = true;
      }
    }

    if( ext == "tif" || ext == "tiff" ) {
      in.clear();
      in.push_back( image );
      convert_format->get_par()->set_image_hints( image );
      //convert_format->get_par()->set_format( VIPS_FORMAT_USHORT );
      convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
      outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
      PF_UNREF( image, "Image::do_export_merged(): image unref" );
      std::cout<<"Image::do_export_merged(): saving TIFF file "<<filename<<"   outimg="<<outimg<<std::endl;
      if( outimg ) {
        int predictor = 2;
#ifndef NDEBUG
        std::cout<<"Image::do_export_merged(): calling vips_tiffsave()..."<<std::endl;
#endif

        vips_tiffsave( outimg, filename.c_str(), "compression",
            //VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
            VIPS_FOREIGN_TIFF_COMPRESSION_NONE,
            //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
            "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_HORIZONTAL, NULL );
#ifndef NDEBUG
        std::cout<<"Image::do_export_merged(): vips_tiffsave() finished..."<<std::endl;
#endif
        //vips_image_write_to_file( outimg, filename.c_str(), NULL );
        if( PF::PhotoFlow::Instance().get_options().get_save_sidecar_files() != 0 &&
            !(PF::PhotoFlow::Instance().is_plugin()) ) {
          save(filename+".pfi", false);
        }
        saved = true;
      }
    }
    /**/

    if( saved ) {

      try {
        PF::exiv2_data_t* exiv2_buf;
        size_t exiv2_buf_length;
        if( vips_image_get_blob( outimg, "exiv2-data",
            (void**)(&exiv2_buf), &exiv2_buf_length ) )
          exiv2_buf = NULL;
        if( exiv2_buf && (exiv2_buf_length==sizeof(PF::exiv2_data_t)) && exiv2_buf->image.get() != NULL ) {
          Exiv2::BasicIo::AutoPtr file (new Exiv2::FileIo (filename));
          Exiv2::Image::AutoPtr exiv2_image = Exiv2::ImageFactory::open(file);
          if(exiv2_image.get() != 0) {
            //exiv2_image->readMetadata();
            exiv2_image->setMetadata( *(exiv2_buf->image.get()) );

            void *iccdata;
            size_t iccdata_length;

            if( !vips_image_get_blob( outimg, VIPS_META_ICC_NAME,
                                     &iccdata, &iccdata_length ) ) {
              Exiv2::byte *iccdata2 = (Exiv2::byte *)iccdata;
              Exiv2::DataBuf iccbuf(iccdata2, iccdata_length);
              exiv2_image->setIccProfile( iccbuf, true );

              /*
              Exiv2::ExifKey            key("Exif.Image.InterColorProfile");
              Exiv2::ExifData::iterator pos   = exiv2_image->exifData().findKey(key);
              bool                      found = pos != exiv2_image->exifData().end();
              if ( iccdata ) {
                  Exiv2::DataValue value(iccdata2,iccdata_length);
                  if ( found ) pos->setValue(&value);
                  else     exiv2_image->exifData().add(key,&value);
              } else {
                  if ( found ) exiv2_image->exifData().erase(pos);
              }
              */
            }
            exiv2_image->writeMetadata();
          }
        }
      } catch(Exiv2::AnyError &e) {
        std::string s(e.what());
        std::cerr << "[exiv2] " << filename << ": " << s << std::endl;
        //return 1;
      }

      /*
      void* gexiv2_buf;
      size_t gexiv2_buf_length;
      if( vips_image_get_blob( outimg, "gexiv2-data",
                               &gexiv2_buf, &gexiv2_buf_length ) )
        gexiv2_buf = NULL;
      if( gexiv2_buf && (gexiv2_buf_length==sizeof(GExiv2Metadata)) ) {
        gexiv2_metadata_save_file( (GExiv2Metadata*)gexiv2_buf, filename.c_str(), NULL );
      }
       */
    }

    if( outimg ) {
      msg = std::string("PF::Image::export_merged(") + filename + "): outimg unref";
      PF_UNREF( outimg, msg.c_str() );
    }
    remove_pipeline( pipeline );
    delete pipeline;
    //layer_manager.reset_cache_buffers( PF_RENDER_NORMAL, true );
    std::cout<<"Image saved to file "<<filename<<std::endl;
  }
}





static void* memsave_start( struct _VipsImage *out, void *a, void *b )
{
  PF::ImageBuffer* imgbuf = (PF::ImageBuffer*)a;
  return imgbuf;
}


static int memsave_stop( void* seq, void *a, void *b )
{
  return(0);
}



/* Loop over region, accumulating a sum in *tmp.
 */
static int memsave_scan( VipsRegion *region,
    void *seq, void *a, void *b, gboolean *stop )
{
  VipsRect *r = &region->valid;
  int lsk = VIPS_REGION_LSKIP( region );
  int bands = vips_image_get_bands( region->im );
  int lsz = bands * r->width;

  int x, y;
  VipsPel* p;
  void* pout;

  PF::ImageBuffer* imgbuf = (PF::ImageBuffer*)seq;

  for( y = 0; y < r->height; y++ ) {
    p = VIPS_REGION_ADDR( region, r->left, r->top+y );
    pout = &( imgbuf->buf[ (r->top+y)*imgbuf->width*3 + r->left*3 ] );
    memcpy( pout, p, sizeof(float)*lsz );
  }

  return( 0 );
}



void PF::Image::export_merged_to_mem( PF::ImageBuffer* imgbuf, void* out_iccdata, size_t out_iccsize )
{
  imgbuf->iccdata = NULL;
  imgbuf->iccsize = 0;
  imgbuf->buf = NULL;

//#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_mem(): waiting for caching completion..."<<std::endl;
//#endif
  PF::ImageProcessor::Instance().wait_for_caching();
//#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_mem(): ... caching completed"<<std::endl;
//#endif

  unsigned int level = 0;
  PF::Pipeline* pipeline = add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_NORMAL );
  if( pipeline ) pipeline->set_op_caching_enabled( true );
  update( pipeline, true );
#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_mem(): image updated."<<std::endl;
#endif

  std::string msg;
  VipsImage* image = pipeline->get_output();
  VipsImage* outimg = NULL;

  std::vector<VipsImage*> in;
  in.clear();
  in.push_back( image );
  convert_format->get_par()->set_image_hints( image );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  VipsImage* floatimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );

  PF::ICCProfile* out_iccprofile = NULL;
  outimg = floatimg;
#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_mem(): out_iccdata="<<(void*)out_iccdata<<std::endl;
#endif
  if( floatimg && out_iccdata ) {
    out_iccprofile = PF::ICCStore::Instance().get_profile( out_iccdata, out_iccsize );
#ifndef NDEBUG
    std::cout<<"Image::export_merged_to_mem(): out_iccprofile="<<(void*)out_iccprofile<<std::endl;
#endif
    if( out_iccprofile ) {
      PF::ICCTransformPar* conv_par =
          dynamic_cast<PF::ICCTransformPar*>( convert2outprof->get_par() );
#ifndef NDEBUG
      std::cout<<"Image::export_merged_to_mem(): conv_par="<<(void*)conv_par<<std::endl;
#endif
      if( conv_par ) {
        in.clear();
        in.push_back( floatimg );
        conv_par->set_image_hints( floatimg );
        conv_par->set_format( VIPS_FORMAT_FLOAT );
        conv_par->set_out_profile( out_iccprofile );
        outimg = convert2outprof->get_par()->build( in, 0, NULL, NULL, level );
        PF_UNREF( floatimg, "" );
      }
    }
  }

#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_mem(): outimg="<<outimg<<std::endl;
#endif
  if( outimg ) {
    imgbuf->buf = (float*)malloc( sizeof(float)*3*outimg->Xsize*outimg->Ysize );
    imgbuf->width = outimg->Xsize;
    imgbuf->height = outimg->Ysize;

    vips_sink( outimg, memsave_start, memsave_scan, memsave_stop, imgbuf, NULL );
#ifndef NDEBUG
    std::cout<<"Image::export_merged_to_mem(): vips_sink() finished."<<std::endl;
#endif

    //if( out_iccprofile ) cmsCloseProfile( out_iccprofile );
    //std::cout<<"Image::export_merged_to_mem(): output profile closed."<<std::endl;

    void *iccdata;
    size_t iccsize;
    if( !vips_image_get_blob( outimg, VIPS_META_ICC_NAME,
        &iccdata, &iccsize ) ) {
      imgbuf->iccdata = malloc(iccsize);
      if( imgbuf->iccdata ) {
        imgbuf->iccsize = iccsize;
        memcpy( imgbuf->iccdata, iccdata, iccsize );
      }
    } else {
      imgbuf->iccdata = NULL;
      imgbuf->iccsize = 0;
    }

    imgbuf->trc_type = PF_TRC_STANDARD;
    ICCProfile* iccinfo  = get_icc_profile( outimg );
    if( iccinfo ) imgbuf->trc_type = iccinfo->get_trc_type();

    /*
    void* gexiv2_buf;
    size_t gexiv2_buf_length;
    if( !vips_image_get_blob( outimg, "gexiv2-data",
                             &gexiv2_buf, &gexiv2_buf_length ) &&
        gexiv2_buf && (gexiv2_buf_length==sizeof(GExiv2Metadata)) ) {
      imgbuf->exif_buf = (GExiv2Metadata*)gexiv2_buf;
      //imgbuf->exif_buf = (GExiv2Metadata*)malloc( sizeof(GExiv2Metadata) );
      //if( imgbuf->exif_buf ) {
        //memcpy( imgbuf->exif_buf, gexiv2_buf, sizeof(GExiv2Metadata) );
      //}
    } else {
      imgbuf->exif_buf = NULL;
    }
     */

    msg = std::string("PF::Image::export_merged_to_mem(): outimg unref");
    PF_UNREF( outimg, msg.c_str() );
  }

  remove_pipeline( pipeline );
  delete pipeline;
  //layer_manager.reset_cache_buffers( PF_RENDER_NORMAL, true );
  std::cout<<"Image saved to memory "<<std::endl;
}



void PF::Image::export_merged_to_tiff( const std::string filename )
{
  std::cout<<"Image::export_merged_to_tiff(): waiting for caching completion..."<<std::endl;
  PF::ImageProcessor::Instance().wait_for_caching();
  std::cout<<"Image::export_merged_to_tiff(): ... caching completed"<<std::endl;

  unsigned int level = 0;
  PF::Pipeline* pipeline = add_pipeline( VIPS_FORMAT_FLOAT, 0, PF_RENDER_NORMAL );
  if( pipeline ) pipeline->set_op_caching_enabled( true );
  update( pipeline, true );
#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_tiff(): image updated."<<std::endl;
#endif

  void* out_iccdata;
  size_t out_iccsize;

  PF::Layer* layer = get_layer_manager().get_layers().front();
  if( !layer ) return;

  PipelineNode* node = pipeline->get_node( layer->get_id() );
  if( !node ) return;

  VipsImage* bgdimg = node->image;
  cmsHPROFILE in_profile = NULL;
  PF::ICCProfile* iccprof_in = PF::get_icc_profile( bgdimg );
  if( iccprof_in )  {
    in_profile = iccprof_in->get_profile();
  }


  std::string msg;
  VipsImage* image = pipeline->get_output();
  VipsImage* outimg = NULL;

  std::vector<VipsImage*> in;
  in.clear();
  in.push_back( image );
  convert_format->get_par()->set_image_hints( image );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  VipsImage* floatimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );

  outimg = floatimg;
  PF::ICCTransformPar* conv_par =
      dynamic_cast<PF::ICCTransformPar*>( convert2outprof->get_par() );
//#ifndef NDEBUG
  std::cout<<"Image::export_merged_to_tiff(): conv_par="<<(void*)conv_par<<std::endl;
//#endif
  if( conv_par ) {
    in.clear();
    in.push_back( floatimg );
    conv_par->set_image_hints( floatimg );
    conv_par->set_format( VIPS_FORMAT_FLOAT );
    conv_par->set_out_profile( iccprof_in );
    outimg = convert2outprof->get_par()->build( in, 0, NULL, NULL, level );
    PF_UNREF( floatimg, "" );
  }

  std::cout<<"Image::export_merged_to_tiff(): outimg="<<outimg<<std::endl;
  if( outimg ) {
    std::cout<<"Image::do_export_merged(): calling vips_tiffsave()..."<<std::endl;
    vips_tiffsave( outimg, filename.c_str(), "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
        "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
    //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_HORIZONTAL, NULL );
    std::cout<<"Image::do_export_merged(): vips_tiffsave() finished..."<<std::endl;

    msg = std::string("PF::Image::export_merged_to_tiff(): outimg unref");
    PF_UNREF( outimg, msg.c_str() );
  }

  remove_pipeline( pipeline );
  delete pipeline;
  //layer_manager.reset_cache_buffers( PF_RENDER_NORMAL, true );
  std::cout<<"Image saved to TIFF: \""<<filename<<"\""<<std::endl;
}
