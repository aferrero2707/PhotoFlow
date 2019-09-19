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


#include "pipeline.hh"
#include "processor.hh"
#include "image.hh"
#include "layer.hh"
#include "photoflow.hh"


PF::PipelineNode::~PipelineNode()
{
  char tstr[500];
  if( image != NULL ) {
    if( G_OBJECT( image )->ref_count <= 0 ) {
      std::cout<<"~PipelineNode(): image refcount <= 0"<<std::endl;
    }
    g_assert( G_OBJECT( image )->ref_count > 0 );
  }

  if( blended != NULL ) {
    if( G_OBJECT( blended )->ref_count <= 0 ) {
      std::cout<<"~PipelineNode(): blended refcount <= 0"<<std::endl;
    }
    g_assert( G_OBJECT( blended )->ref_count > 0 );
    //PF_PRINT_REF(blended, "~PipelineNode() blended before unref: ");
    PF_UNREF( blended, "~PipelineNode() unref blended image" );
  }

  for( size_t j = 0; j < images.size(); j++ ) {
    snprintf( tstr, 499, "~PipelineNode() unref images[%d]", (int)j );
    //PF_PRINT_REF(images[j], tstr);
    PF_UNREF( images[j], tstr );
#ifndef NDEBUG
    std::cout<<"~PipelineNode(): unref of images["<<j<<"]"<<std::endl;
#endif
  }

  if( processor != NULL ) {
#ifndef NDEBUG
    std::cout<<"~PipelineNode(): deleting processor"<<std::endl;
#endif
    delete( processor );
#ifndef NDEBUG
    std::cout<<"~PipelineNode(): processor deleted"<<std::endl;
#endif
  }

  if( blender != NULL ) {
#ifndef NDEBUG
    std::cout<<"~PipelineNode(): deleting blender"<<std::endl;
#endif
    delete( blender );
#ifndef NDEBUG
    std::cout<<"~PipelineNode(): blender deleted"<<std::endl;
#endif
  }
}


PF::Pipeline::~Pipeline()
{
#ifndef NDEBUG
  std::cout<<"Pipeline::~Pipeline() called."<<std::endl;
#endif
  char tstr[500];
  for( unsigned int i = 0; i < nodes.size(); i++ ) {
    if( nodes[i] != NULL ) {
      PF::Layer* l = image->get_layer_manager().get_layer( i );
#ifndef NDEBUG
      std::cout<<"Pipeline::~Pipeline(): deleting node of layer \""
          <<(l ? l->get_name() : "")<<"\""<<std::endl;
#endif
      delete nodes[i];
    }
  }
#ifndef NDEBUG
  std::cout<<"Pipeline::~Pipeline(): deleting sinks"<<std::endl;
#endif
  for( unsigned int i = 0; i < sinks.size(); i++ ) {
    if( sinks[i] == NULL ) continue;
#ifndef NDEBUG
    std::cout<<"Pipeline::~Pipeline(): cleaning sink #"<<i<<std::endl;
#endif
    //delete( sinks[i] );
    sinks[i]->dispose();
#ifndef NDEBUG
    std::cout<<"Pipeline::~Pipeline(): sink #"<<i<<" cleaned"<<std::endl;
#endif
  }
#ifndef NDEBUG
  std::cout<<"Pipeline::~Pipeline(): all sinks cleaned"<<std::endl;
#endif
}


PF::PipelineNode* PF::Pipeline::set_node( Layer* layer, Layer* input_layer )
{
  if( !layer )
    return NULL;

  uint32_t id = layer->get_id();
  if( id >= nodes.size() ) {
    while( nodes.size() <= (id+1) ) nodes.push_back(NULL);
    nodes[id] = new PF::PipelineNode;
  }
  if( nodes[id] == NULL )
    nodes[id] = new PF::PipelineNode;

  PF::PipelineNode* node = nodes[id];
  if( node == NULL )
    return NULL;

  node->input_id = ( input_layer != NULL ) ? input_layer->get_id() : -1;

  PF::OpParBase* srcpar = NULL;
  if( layer->get_processor() != NULL )
    srcpar = layer->get_processor()->get_par();

  if( (srcpar != NULL ) ) {
    if( (node->processor == NULL) ||
        (node->processor->get_par() == NULL) ||
        (node->processor->get_par()->get_type() != srcpar->get_type()) ) {
      if( node->processor != NULL )
        delete( node->processor );
      node->processor = PF::PhotoFlow::Instance().
        new_operation_nogui( srcpar->get_type(), NULL );
    }

    if( (node->processor != NULL) &&
        (node->processor->get_par() != NULL) ) {
      bool result = node->processor->get_par()->import_settings( srcpar );
      g_assert( result != false );
    }
  }


  PF::OpParBase* srcblender = NULL;
  if( layer->get_blender() != NULL )
    srcblender = layer->get_blender()->get_par();

  if( srcblender != NULL ) {
    if( node->blender == NULL )
      node->blender = PF::PhotoFlow::Instance().
        new_operation_nogui( "blender", NULL );
    if( (node->blender != NULL) &&
        (node->blender->get_par() != NULL) ) {
      bool result = node->blender->get_par()->import_settings( srcblender );
      if( !result ) std::cout<<"Pipeline::set_node(): failed to import settings for layer \""<<layer->get_name()<<"\""<<std::endl;
      g_assert( result != false );
    }
  }

  return node;
}


void PF::Pipeline::set_image( VipsImage* img, unsigned int id )
{
  if( id >= nodes.size() ) 
    return;
  
  PF::Layer* l = image->get_layer_manager().get_layer( id );
#ifndef NDEBUG
  if( l ) {
    std::cout<<"Pipeline::set_image(): layer \""<<l->get_name()<<"\"";
    if( nodes[id] != NULL ) {
      std::cout<<"  old image="<<nodes[id]->image;
    }
    std::cout<<"  new image="<<img<<std::endl;
  }
#endif

  char tstr[500];
  if( nodes[id] != NULL ) {
    if( !(nodes[id]->images.empty()) ) {
      //if( G_OBJECT( nodes[id]->image )->ref_count < 1 )
      //  std::cout<<"!!! Pipeline::set_image(): wrong ref_count for node #"<<id<<", image="<<nodes[id]->image<<std::endl;
      //g_assert( G_OBJECT( nodes[id]->image )->ref_count > 0 );
      //g_object_unref( nodes[id]->image );
      //PF::Layer* l = image->get_layer_manager().get_layer( id );
      if( l ) {
        snprintf( tstr, 499, "PF::Pipeline::set_image() unref image of layer %s",
                  l->get_name().c_str() );
      } else {
        snprintf( tstr, 499, "PF::Pipeline::set_image() unref image (NULL layer)" );
      }
      for( size_t i = 0; i < nodes[id]->images.size(); i++ )
        PF_UNREF( nodes[id]->images[i], tstr );
      nodes[id]->images.clear();
      nodes[id]->image = NULL;
    }
    nodes[id]->image = img;
    if( img ) nodes[id]->images.push_back( img );
  }
}


void PF::Pipeline::set_images( std::vector<VipsImage*> imgvec, unsigned int id )
{
  if( id >= nodes.size() )
    return;

  PF::Layer* l = image->get_layer_manager().get_layer( id );
#ifndef NDEBUG
  if( l ) {
    std::cout<<"Pipeline::set_images(): layer \""<<l->get_name()<<"\"";
    if( nodes[id] != NULL ) {
      std::cout<<"  old image="<<nodes[id]->image;
    }
    if(imgvec.size() > 1) std::cout<<"  new image="<<imgvec[0];
    std::cout<<std::endl;
  }
#endif

  char tstr[500];
  if( nodes[id] != NULL ) {
    if( !(nodes[id]->images.empty()) ) {
      //if( G_OBJECT( nodes[id]->image )->ref_count < 1 )
      //  std::cout<<"!!! Pipeline::set_image(): wrong ref_count for node #"<<id<<", image="<<nodes[id]->image<<std::endl;
      //g_assert( G_OBJECT( nodes[id]->image )->ref_count > 0 );
      //g_object_unref( nodes[id]->image );
      //PF::Layer* l = image->get_layer_manager().get_layer( id );
      if( l ) {
        snprintf( tstr, 499, "PF::Pipeline::set_images() unref image of layer %s",
                  l->get_name().c_str() );
      } else {
        snprintf( tstr, 499, "PF::Pipeline::set_images() unref image (NULL layer)" );
      }
      for( size_t i = 0; i < nodes[id]->images.size(); i++ )
        PF_UNREF( nodes[id]->images[i], tstr );
      nodes[id]->images.clear();
      nodes[id]->image = NULL;
    }

    nodes[id]->images = imgvec;
    if( l ) {
      snprintf( tstr, 499, "PF::Pipeline::set_images() ref image of layer %s",
                l->get_name().c_str() );
    } else {
      snprintf( tstr, 499, "PF::Pipeline::set_images() ref image (NULL layer)" );
    }
    //for( size_t i = 0; i < nodes[id]->images.size(); i++ )
    //  PF_REF( nodes[id]->images[i], tstr );
    if( !(imgvec.empty()) ) nodes[id]->image = imgvec[0];
    else nodes[id]->image = NULL;
  }
}


void PF::Pipeline::set_blended( VipsImage* img, unsigned int id )
{
  if( id >= nodes.size() ) 
    return;
  
  char tstr[500];
  if( nodes[id] != NULL ) {
    if( nodes[id]->blended != NULL ) {
      if( G_OBJECT( nodes[id]->blended )->ref_count < 1 )
      	std::cout<<"!!! Pipeline::set_image(): wrong ref_count for node #"<<id<<", image="<<nodes[id]->blended<<std::endl;
      g_assert( G_OBJECT( nodes[id]->blended )->ref_count > 0 );
      //g_object_unref( nodes[id]->blended );
      PF::Layer* l = image->get_layer_manager().get_layer( id );
      if( l )
        snprintf( tstr, 499, "PF::Pipeline::set_image() unref image of layer %s",
                  l->get_name().c_str() );
      else
        snprintf( tstr, 499, "PF::Pipeline::set_image() unref image (NULL layer)" );
      PF_UNREF( nodes[id]->blended, tstr );
    }
    nodes[id]->blended = img;
  }
}


void PF::Pipeline::remove_node( unsigned int id )
{
  if( id >= nodes.size() ) return;

  char tstr[500];
  if( nodes[id] != NULL ) {
    /*
    PF::Layer* l = image->get_layer_manager().get_layer( id );
    if( nodes[id]->blended != NULL ) {
      //g_object_unref( nodes[id]->image );
      if( l )
        snprintf( tstr, 499, "PF::Pipeline::remove_node() unref blended image of layer %s",
            l->get_name().c_str() );
      else
        snprintf( tstr, 499, "PF::Pipeline::remove_node() unref blended image (NULL layer)" );
      PF_UNREF( nodes[id]->blended, tstr );
    }
    if( nodes[id]->blender != NULL )
      delete( nodes[id]->blender );

    if( nodes[id]->image != NULL ) {
      //g_object_unref( nodes[id]->image );
      if( l )
        snprintf( tstr, 499, "PF::Pipeline::remove_node() unref image of layer %s",
            l->get_name().c_str() );
      else
        snprintf( tstr, 499, "PF::Pipeline::remove_node() unref image (NULL layer)" );
      PF_UNREF( nodes[id]->image, tstr );
    }
    if( nodes[id]->processor != NULL )
      delete( nodes[id]->processor );
    */
    delete nodes[id];
    nodes[id] = NULL;
  }
}


bool PF::Pipeline::processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::Pipeline::update(): sink #"<<i<<" -> processing="<<sinks[i]->is_processing()<<std::endl;
#endif
    if( sinks[i]->is_processing() ) return true;
  }
  return false;
}


void PF::Pipeline::lock_processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    //std::cout<<"PF::Pipeline::update(): locking sink #"<<i<<std::endl;
#endif
    //sinks[i]->get_processing_mutex().lock();
#ifndef NDEBUG
    //std::cout<<"PF::Pipeline::update(): sink #"<<i<<" locked"<<std::endl;
#endif
  }
}


void PF::Pipeline::unlock_processing()
{
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    //std::cout<<"PF::Pipeline::update(): unlocking sink #"<<i<<std::endl;
#endif
    //sinks[i]->get_processing_mutex().unlock();
#ifndef NDEBUG
    //std::cout<<"PF::Pipeline::update(): sink #"<<i<<" unlocked"<<std::endl;
#endif
  }
}


void PF::Pipeline::update( VipsRect* area )
{
#ifndef NDEBUG
  std::cout<<"PF::Pipeline::update(): called"<<std::endl;
#endif
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::Pipeline::update(): updating sink #"<<i<<std::endl;
#endif
    sinks[i]->update( area );
#ifndef NDEBUG
    std::cout<<"PF::Pipeline::update(): sink #"<<i<<" updated"<<std::endl;
#endif
  }
}


/**/
void PF::Pipeline::sink( const VipsRect& area )
{
#ifndef NDEBUG
  std::cout<<"PF::Pipeline::update(const VipsRect& area): called"<<std::endl;
#endif
  for( unsigned int i = 0; i < sinks.size(); i++) {
#ifndef NDEBUG
    std::cout<<"PF::Pipeline::update(const VipsRect& area): updating sink #"<<i<<std::endl;
#endif
    sinks[i]->sink( area );
#ifndef NDEBUG
    std::cout<<"PF::Pipeline::update(const VipsRect& area): sink #"<<i<<" updated"<<std::endl;
#endif
  }
}
/**/
