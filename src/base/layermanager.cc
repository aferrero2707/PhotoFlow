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

#include <string.h>

#include "layermanager.hh"
#include "image.hh"


PF::LayerManager::LayerManager( PF::Image* img ): image( img )
{
}


PF::LayerManager::~LayerManager()
{
  for(unsigned int i = 0; i < layers_pool.size(); i++) {
    if(layers_pool[i] == NULL) 
      continue;
    delete layers_pool[i];
  }
}


PF::Layer* PF::LayerManager::new_layer()
{
  uint32_t i;
  for(i = 0; i < layers_pool.size(); i++) {
    if(layers_pool[i] == NULL) {
      PF::Layer* l = new PF::Layer(i);
      l->set_image( image );
      layers_pool[i] = l;
      return l;
    }
  }
  PF::Layer* l = new PF::Layer(layers_pool.size());
  l->set_image( image );
  layers_pool.push_back(l);
  return l;
}


void PF::LayerManager::delete_layer( PF::Layer* layer )
{
  if( layer->get_id() < 0 ) {
    std::cout<<"ERROR: LayerManager::delete_layer(): layer->get_id() < 0"<<std::endl;
    return;
  }
  if( layer->get_id() >= (int)layers_pool.size() ) {
    std::cout<<"ERROR: LayerManager::delete_layer(): layer->get_id() >= layers_pool.size()"<<std::endl;
    return;
  }
  layers_pool[layer->get_id()] = NULL;
  delete layer;
}


PF::Layer* PF::LayerManager::get_layer(int id)
{
  if(id < 0 || id >= (int)layers_pool.size()) return NULL;
  return ( layers_pool[id] );
}



// Fills a list with the current layer and all its children
void PF::LayerManager::expand_layer( PF::Layer* layer, std::list<PF::Layer*>& list )
{
  if( !layer ) return;
  list.push_back( layer );
  // Sublayers
  for( std::list<PF::Layer*>::iterator li = layer->get_sublayers().begin();
       li != layer->get_sublayers().end(); li++ ) {
    expand_layer( *li, list);
  }
  // Intensity map layers
  for( std::list<PF::Layer*>::iterator li = layer->get_imap_layers().begin();
       li != layer->get_imap_layers().end(); li++ ) {
    expand_layer( *li, list);
  }
  // Opacity map layers
  for( std::list<PF::Layer*>::iterator li = layer->get_omap_layers().begin();
       li != layer->get_omap_layers().end(); li++ ) {
    expand_layer( *li, list);
  }
}


void PF::LayerManager::get_input_layers( Layer* layer, std::list<PF::Layer*>& container,
                                         std::list<Layer*>& inputs )
{
  //#ifndef NDEBUG
  std::cout<<"Collecting inputs of layer \""<<layer->get_name()<<"\"("<<layer->get_id()<<")"<<std::endl;
  //#endif
  std::list<PF::Layer*>::iterator li;
  for(li = container.begin(); li != container.end(); ++li) {
    PF::Layer* l = *li;
    //#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
    //#endif
    if( l->get_id() == layer->get_id() ) break;
    // Add layer and all its children to the inputs list
    expand_layer( l, inputs );  
    //#ifndef NDEBUG
    std::cout<<"    added."<<std::endl;
    //#endif
  }

  PF::Layer* container_layer = get_container_layer( layer );
  if( !container_layer ) return;
  std::list<PF::Layer*>* clist = get_list( container_layer );
  if( !clist ) return;
  get_input_layers( container_layer, *clist, inputs );
}


void PF::LayerManager::get_input_layers( Layer* layer, std::list<Layer*>& inputs )
{
  if( !layer ) return;
  std::list<PF::Layer*>* clist = get_list( layer );
  if( !clist ) return;
  get_input_layers( layer, *clist, inputs );
}


void PF::LayerManager::get_child_layers( Layer* layer, std::list<PF::Layer*>& container,
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
#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
#endif
    if( l->get_id() == layer->get_id() ) break;
    // Add layer and all its children to the inputs list
    //expand_layer( l, inputs );
    // Add layer to the temporary list
    tmplist.push_front( l );
#ifndef NDEBUG
    std::cout<<"    added."<<std::endl;
#endif
  }

  // Append the temporary list to the childrens one
  children.insert( children.end(), tmplist.begin(), tmplist.end() );

  PF::Layer* container_layer = get_container_layer( layer );
  if( !container_layer ) return;

  // Add the container layer to the list of children
  children.push_back( container_layer );

  std::list<PF::Layer*>* clist = get_list( container_layer );
  if( !clist ) return;

  // Add all the children of the container layer to the children list
  get_child_layers( container_layer, *clist, children );
}


void PF::LayerManager::get_child_layers( Layer* layer, std::list<Layer*>& children )
{
  if( !layer ) return;
  std::list<PF::Layer*>* clist = get_list( layer );
  if( !clist ) return;
  get_child_layers( layer, *clist, children );
}

/*
 * Fill the list of layers that are parent of a target layer, i.e. the list of layers
 * that contribute to the input of the target layer (excluding the mask associated
 * to the target layer itself)
 */
bool PF::LayerManager::get_parent_layers(Layer* layer, 
                                         std::list< std::pair<std::string,Layer*> >& plist,
                                         std::string parent_name, std::list<Layer*>& list)
{  
#ifndef NDEBUG
  std::cout<<"Collecting parents of layer \""<<layer->get_name()<<"\"("<<layer->get_id()<<")"<<std::endl;
#endif
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    std::string name;
    if( !parent_name.empty() ) name = parent_name + "/";
    name = name + l->get_name();
#ifndef NDEBUG
    std::cout<<"  checking layer \""<<l->get_name()<<"\"("<<l->get_id()<<")"<<std::endl;
#endif

    if( get_parent_layers( layer, plist, name, l->sublayers ) )
      return true;

    if( get_parent_layers( layer, plist, name+"/IMap/", l->imap_layers ) )
      return true;

    if( get_parent_layers( layer, plist, name+"/OMap/", l->omap_layers ) )
      return true;

    if( l->get_id() != layer->get_id() ) {
      plist.push_back( make_pair( name, l ) );
#ifndef NDEBUG
      std::cout<<"    added."<<std::endl;
#endif
    }

    if( l->get_id() == layer->get_id() ) 
      return true;
  }
  return false;
}


void PF::LayerManager::get_parent_layers(PF::Layer* layer, 
                                         std::list< std::pair<std::string,PF::Layer*> >& plist)
{
  get_parent_layers( layer, plist, std::string(""), layers );
}



PF::Layer* PF::LayerManager::get_container_layer( Layer* layer, std::list<Layer*>& list )
{
  if( !layer )
    return NULL;

  bool is_map = false;
  if( layer && layer->get_processor() &&
      layer->get_processor()->get_par() )
    is_map = layer->get_processor()->get_par()->is_map();

  // Walk through the list and, for each layer in the list, search for the target layer in the 
  // lists (imaps, omaps and sublayers)
  std::list<PF::Layer*>::iterator li;
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    // We first look in the sublayers list
    std::list<PF::Layer*>::iterator lj;
    for( lj = l->get_sublayers().begin(); 
         lj != l->get_sublayers().end(); ++lj ) {
      int id1 = layer->get_id();
      int id2 = ( (*lj)!=NULL ) ? (*lj)->get_id() : -1;
      if( (*lj) && ((*lj)->get_id() == layer->get_id()) ) {
        // We found it, no need to continue...
        return( l );
      }
    }
    if( is_map ) {
      // If the target layer is a layer map, then we also look into the 
      // intensity and opacity maps
      std::list<PF::Layer*>::iterator lj;
      for( lj = l->get_imap_layers().begin(); 
           lj != l->get_imap_layers().end(); ++lj ) {
        int id1 = layer->get_id();
        int id2 = ( (*lj)!=NULL ) ? (*lj)->get_id() : -1;
        if( (*lj) && ((*lj)->get_id() == layer->get_id()) ) {
          // We found it, no need to continue...
          return( l );
        }
      }
      for( lj = l->get_omap_layers().begin(); 
           lj != l->get_omap_layers().end(); ++lj ) {
        int id1 = layer->get_id();
        int id2 = ( (*lj)!=NULL ) ? (*lj)->get_id() : -1;
        if( (*lj) && ((*lj)->get_id() == layer->get_id()) ) {
          // We found it, no need to continue...
          return( l );
        }
      }
    }
  }

  // If we got here it means that the layer was not found yet, so we
  // recursively search it in all the sub-layers in the list
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    PF::Layer* result;    
    result = get_container_layer( layer, l->get_imap_layers() );
    if( result ) 
      return( result );
    result = get_container_layer( layer, l->get_omap_layers() );
    if( result ) 
      return( result );
    result = get_container_layer( layer, l->get_sublayers() );
    if( result ) 
      return( result );
  }

  // If we reach this point, it means that the layer could not be found...
  return( NULL );
}

PF::Layer* PF::LayerManager::get_container_layer( Layer* layer )
{
  if( !layer ) 
    return( NULL );
  return( get_container_layer( layer, layers ) );
}

PF::Layer* PF::LayerManager::get_container_layer( int id )
{
  PF::Layer* layer = get_layer( id );
  if( !layer ) 
    return( NULL );
  return( get_container_layer( layer ) );
}



std::list<PF::Layer*>* PF::LayerManager::get_list( PF::Layer* layer, std::list<PF::Layer*>& list)
{
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    if( l->get_id() == layer->get_id() ) {
      return( &list );
    }
  }

  // The layer is not contained in the current list, then we look into sub-lists
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;
    std::list<PF::Layer*>* result;
    result = get_list( layer, l->sublayers );
    if( result ) 
      return result;
    result = get_list( layer, l->imap_layers );
    if( result ) 
      return result;
    result = get_list( layer, l->omap_layers );
    if( result ) 
      return result;
  }

  return NULL;
}

std::list<PF::Layer*>* PF::LayerManager::get_list(PF::Layer* layer)
{
  return get_list( layer, layers );
}




PF::CacheBuffer* PF::LayerManager::get_cache_buffer( rendermode_t mode )
{  
  return( get_cache_buffer(mode, layers) );
}


PF::CacheBuffer* PF::LayerManager::get_cache_buffer( rendermode_t mode, std::list<Layer*>& list )
{  
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    if( !l ) continue;
    if( !l->is_enabled() ) continue;
    //std::cout<<"LayerManager::get_cache_buffer(): checking layer "<<l->get_name()<<std::endl;

    PF::CacheBuffer* buf = NULL;
    /*
    for( unsigned int i = 0; i < l->extra_inputs.size(); i++ ) {
      Layer* lextra = get_layer( l->extra_inputs[i].first );
      if( lextra && lextra->is_enabled() && lextra->is_cached() && lextra->get_cache_buffer(mode) &&
          !lextra->get_cache_buffer(mode)->is_completed() ) {
        buf = lextra->get_cache_buffer( mode );
#ifndef NDEBUG
        std::cout<<"Extra layer #"<<i<<"(\""<<lextra->get_name()<<"\"): pending cache buffer "<<buf<<std::endl;
#endif
        break;
      }
    }
    if( buf ) {
      std::cout<<"Found pending cache buffer for layer "<<l->get_name<<std::endl;
      return buf;
    }
    */

    // Now we walk through the intensity and opacity maps to see if they contain a pending buffer
    buf = get_cache_buffer( mode, l->imap_layers );
    if( buf ) return buf;

    buf = get_cache_buffer( mode, l->omap_layers );
    if( buf ) return buf;

    // Finally we walk through the sub-layers; again, if re-building is needed 
    // we mark this layer "dirty" as well
    buf = get_cache_buffer( mode, l->sublayers );
    if( buf ) return buf;

    // If the current layer is cached and the cache buffer is not completed, we return it.
    if( l->get_image() && l->is_cached() && l->get_cache_buffer(mode) &&
        !l->get_cache_buffer(mode)->is_completed() ) {
      buf = l->get_cache_buffer( mode );
#ifndef NDEBUG
      std::cout<<"Layer \""<<l->get_name()<<"\": pending cache buffer "<<buf<<std::endl;
      std::cout<<"  l->get_image()->get_npipelines()="<<l->get_image()->get_npipelines()<<std::endl;
#endif
      for( unsigned int pi = 0; pi < l->get_image()->get_npipelines(); pi++ ) {
        PF::Pipeline* pipeline = l->get_image()->get_pipeline(pi);
        //std::cout<<"    l->get_image()->get_pipeline("<<pi<<")->get_render_mode()="
        //    <<pipeline->get_render_mode()<<std::endl;
        if( pipeline && pipeline->get_render_mode() == mode &&
            pipeline->get_node(l->get_id()) ) {
          PF::PipelineNode* node = pipeline->get_node(l->get_id());
          buf->set_image( node->image );
#ifndef NDEBUG
          std::cout<<"Caching layer \""<<l->get_name()<<"\"  image="<<node->image<<std::endl;
#endif
          return( buf );
        }
      }
    }
  }
  return NULL;
}




void PF::LayerManager::reset_cache_buffers( rendermode_t mode, bool reinit )
{
  for(unsigned int i = 0; i < layers_pool.size(); i++) {
    if(layers_pool[i] == NULL)
      continue;
    PF::CacheBuffer* buf = layers_pool[i]->get_cache_buffer( mode );
    if( buf ) buf->reset( reinit );
  }
}




void PF::LayerManager::update_visible( std::list<Layer*>& list, bool parent_visible )
{
  bool visible;
  std::list<PF::Layer*>::reverse_iterator li;
  for(li = list.rbegin(); li != list.rend(); ++li) {
    PF::Layer* l = *li;
    if(!l) continue;

    // Set visible flag to parent value first
    visible = parent_visible;
    // Check if current layer is enabled
    // If not, this and all subsequent and/or child layers
    // will be marked as invisible
    if( !l->is_enabled() ) {
      //std::cout<<"Layer \""<<l->get_name()<<"\" disabled, setting visible flag to false"<<std::endl;
      visible = false;
    }
    l->set_visible( visible );
    //std::cout<<"Layer \""<<l->get_name()<<"\" visible flag set to "<<visible<<std::endl;

    // Now we walk through the intensity and opacity maps.
    update_visible( l->imap_layers, visible );

    update_visible( l->omap_layers, visible );

    // Finally we walk through the sub-layers.
    update_visible( l->sublayers, visible );
  }
}



void PF::LayerManager::update_dirty( std::list<Layer*>& list, bool& dirty )
{
  //if( !list.empty() ) std::cout<<"LayerManager::update_dirty("<<dirty<<")"<<std::endl;
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    bool input_dirty = dirty;
    bool filter_dirty = false;
    bool blender_dirty = false;

    //std::cout<<"  Layer \""<<l->get_name()<<"\": dirty="<<dirty
    //         <<" l->is_dirty()="<<l->is_dirty()<<std::endl;
    // if the current layer is dirty the dirty flag is set to true
    // this will also qualify as "dirty" all the subsequent layers in the list
    // It probably means that the visibility of the layer has been toggled
    if( l->is_dirty() )
      dirty = true;

    // If the operation associated to the current layer has been modified,
    // the dirty flag is set to true.
    // This will also qualify as "dirty" all the subsequent layers in the list
#ifndef NDEBUG
    if( l->get_processor() &&
        l->get_processor()->get_par() ) 
      std::cout<<"  Layer \""<<l->get_name()<<"\": par->is_modified()="
               <<l->get_processor()->get_par()->is_modified()<<std::endl;
    if( l->get_blender() &&
        l->get_blender()->get_par() ) 
      std::cout<<"  Layer \""<<l->get_name()<<"\": blender->is_modified()="
               <<l->get_blender()->get_par()->is_modified()<<std::endl;
#endif
    if( l->get_processor() &&
        l->get_processor()->get_par() &&
        l->get_processor()->get_par()->is_modified() )
      filter_dirty = true;

    if( l->get_blender() &&
        l->get_blender()->get_par() &&
        l->get_blender()->get_par()->is_modified() )
      blender_dirty = true;

    // if the current layer is not qualified as "dirty", but one of the extra input layers is,
    // then we set the dirty flag to true as well
    for( unsigned int i = 0; i < l->extra_inputs.size(); i++ ) {
      Layer* lextra = get_layer( l->extra_inputs[i].first.first );
      if( lextra && lextra->is_dirty() ) {
        input_dirty = true;
        break;
      }
    }

    // Now we walk through the intensity and opacity maps to see what needs to be re-built.
    // If either one or the other has to be re-built, then we mark this layer "dirty" as well
    bool imap_dirty = false;
    update_dirty( l->imap_layers, imap_dirty );

    bool omap_dirty = false;
    update_dirty( l->omap_layers, omap_dirty );

    //std::cout<<"  Layer \""<<l->get_name()<<"\": filter_dirty="<<filter_dirty<<" blender_dirty="<<blender_dirty<<" input_dirty="<<input_dirty<<" imap_dirty="<<imap_dirty<<" omap_dirty="<<omap_dirty<<std::endl;

    if( imap_dirty )
      input_dirty = true;

    if( omap_dirty )
      blender_dirty = true;

    // Finally we walk through the sub-layers; again, if re-building is needed 
    // we mark this layer "dirty" as well
    bool sub_dirty = false;
    update_dirty( l->sublayers, sub_dirty );

    //std::cout<<"  Layer \""<<l->get_name()<<"\": sub_dirty="<<sub_dirty<<std::endl;

    if( sub_dirty )
      input_dirty = true;

    dirty = dirty || input_dirty || blender_dirty || filter_dirty;
    
    // Now we have finished walking through all the subchains and extra inputs,
    // and we can set the dirty flag for the current layer
    //if( dirty ) {
    //  std::cout<<"  Layer \""<<l->get_name()<<"\"->is_dirty()="<<l->is_dirty()<<std::endl;
    //  std::cout<<"  Layer \""<<l->get_name()<<"\"->set_dirty("<<dirty<<")"<<std::endl;
    //}
    l->set_dirty( dirty );
    // If the current layer is cached, we reset the corresponding cache buffer
    // so that the computation will restart from scratch at the next idle loop
    if( input_dirty || filter_dirty ) {
      if( l->is_cached() )
        l->reset_cache_buffers();
      //if( l->is_cached() && l->get_cache_buffer() ) l->get_cache_buffer()->reset();
    }
  }
}




void PF::LayerManager::reset_dirty( std::list<Layer*>& list )
{  
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    if( l->get_processor() &&
        l->get_processor()->get_par() ) 
      l->get_processor()->get_par()->clear_modified();
    if( l->get_blender() &&
        l->get_blender()->get_par() ) 
      l->get_blender()->get_par()->clear_modified();

    l->clear_dirty();

    reset_dirty( l->imap_layers );
    reset_dirty( l->omap_layers );
    reset_dirty( l->sublayers );
  }
}




void PF::LayerManager::update_ui( std::list<Layer*>& list )
{
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    if( l->get_processor() &&
        l->get_processor()->get_par() &&
        l->get_processor()->get_par()->get_config_ui() ) {
      l->get_processor()->get_par()->get_config_ui()->update();
    }
    update_ui( l->imap_layers );
    update_ui( l->omap_layers );
    update_ui( l->sublayers );
  }
}




bool PF::insert_layer( std::list<Layer*>& layers, Layer* layer, int32_t lid )
{  
  if( lid < 0 ) {
    layers.push_back( layer );
    return true;
  }

  std::list<Layer*>::iterator it;
  for( it = layers.begin(); it != layers.end(); ++it )
    if( (*it)->get_id() == lid ) break;

  if( it == layers.end() ) return false;
  it++;
  layers.insert( it, layer );

  return true;
}



bool PF::LayerManager::insert_layer( Layer* layer, int32_t lid )
{  
  return PF::insert_layer( layers, layer, lid );
}


bool PF::LayerManager::remove_layer( PF::Layer* layer )
{  
  if( layer == NULL ) return false;

  // Get the container list
  std::list<Layer*>* list = get_list( layer );
  if( list == NULL ) return false;

  // Iterate over the list to find the good one
  std::list<Layer*>::iterator it;
  for( it = list->begin(); it != list->end(); ++it ) {
    if( (*it) == layer ) {
      list->erase( it );
      return true;
    }
  }

  return false;
}




VipsImage* PF::LayerManager::rebuild_chain( PF::Pipeline* pipeline, colorspace_t cs, 
																						int width, int height, 
																						std::list<PF::Layer*>& list, 
																						PF::Layer* previous_layer )
{ 
  PipelineNode* previous_node = NULL;
  VipsImage* previous = NULL;

  //std::cout<<"LayerManager::rebuild_chain() called"<<std::endl;

  VipsImage* out = NULL;
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    // Detect "pathological" conditions
    g_assert( l->get_processor() != NULL );
    g_assert( l->get_processor()->get_par() != NULL );
    g_assert( l->get_blender() != NULL );
    g_assert( l->get_blender()->get_par() != NULL );

    char* name = (char*)l->get_name().c_str();
    if( !l->is_enabled() ) continue;

#ifndef NDEBUG
    std::cout<<"PF::LayerManager::rebuild_chain(): processing layer \""<<name<<"\""<<std::endl;
#endif
    if( previous_layer ) {
      previous_node = pipeline->get_node( previous_layer->get_id() );
      //if( previous_node ) previous = previous_node->image;
      if( previous_node ) previous = previous_node->blended;
#ifndef NDEBUG
      std::cout<<"  Previous layer: \""<<previous_layer->get_name()<<"\""<<std::endl;
#endif
    }

    // Create the node if it does not yet exist, and copy the parameters
    // from the operation associated with the layer to the
    // operation associated with the node
    PF::PipelineNode* node = pipeline->set_node( l, previous_layer );
    g_assert( node != NULL );
    g_assert( node->processor != NULL );
    g_assert( node->processor->get_par() != NULL );
    g_assert( node->blender != NULL );
    g_assert( node->blender->get_par() != NULL );
    if( node->image ) vips_image_invalidate_all( node->image );
    if( node->blended ) vips_image_invalidate_all( node->blended );

    PF::OpParBase* par = l->get_processor()->get_par();
    l->set_cached( par->needs_caching() );

    PF::OpParBase* pipelinepar = node->processor->get_par();
#ifndef NDEBUG
    std::cout<<"PF::LayerManager::rebuild_chain(): setting format for layer "<<l->get_name()
                   <<" to "<<pipeline->get_format()<<std::endl;
    std::cout<<"  pipelinepar->set_render_mode( "<<pipeline->get_render_mode()<<" );"<<std::endl;
#endif
    pipelinepar->set_format( pipeline->get_format() );
    pipelinepar->set_render_mode( pipeline->get_render_mode() );

    // Run pre-build phase
    par->pre_build( pipeline->get_render_mode() );

    PF::OpParBase* blender = l->get_blender()->get_par();
    PF::PropertyBase* p_rgb_target_ch =  blender->get_property( "rgb_target_channel" );
    if( p_rgb_target_ch )
      p_rgb_target_ch->set_enum_value( par->get_rgb_target_channel() );
    PF::PropertyBase* p_lab_target_ch =  blender->get_property( "lab_target_channel" );
    if( p_lab_target_ch )
      p_lab_target_ch->set_enum_value( par->get_lab_target_channel() );
    PF::PropertyBase* p_cmyk_target_ch = blender->get_property( "cmyk_target_channel" );
    if( p_cmyk_target_ch )
      p_cmyk_target_ch->set_enum_value( par->get_cmyk_target_channel() );

    PF::OpParBase* pipelineblender = node->blender->get_par();
#ifndef NDEBUG
    std::cout<<"PF::LayerManager::rebuild_chain(): setting format for layer "<<l->get_name()
                   <<" to "<<pipeline->get_format()<<std::endl;
#endif
    pipelineblender->set_format( pipeline->get_format() );
    pipelineblender->set_render_mode( pipeline->get_render_mode() );

    // If the layer is at the beginning of the chain, we set hints about the desired
    // colorspace and pixel format using default values. 
    // The size and colorspace hints might be ignored by
    // the operation (for example in the case of an image from a file, where the
    // size and colorspace are dictated by the file content).
    // On the other hand, the pixel format hint should be strictly respected by all 
    // operators, as it defined the accuracy at which the final image is rendered.
    // If a previous image has been created already, hints are copied from it.
    //if( li == list.begin() || !previous )
    if( !previous ) {
      pipelinepar->set_image_hints( width, height, cs );
      pipelineblender->set_image_hints( width, height, cs );
    } else {
      pipelinepar->set_image_hints( previous );
      pipelineblender->set_image_hints( previous );
    }
    //PF::colorspace_t cs = PF::convert_colorspace( pipelinepar->get_interpretation() );
    //std::cout<<"  par: "<<pipelinepar<<std::endl;
    //std::cout<<"  cs after set_image_hints: "<<cs<<std::endl;
    
    if( l->is_cached() &&
        (l->get_cache_buffer(pipeline->get_render_mode()) != NULL) &&
        l->get_cache_buffer(pipeline->get_render_mode())->is_completed() ) {
      // The layer is cached, no need to process the underlying layers
      // We only need to associate the cached image with the blender
      unsigned int level = pipeline->get_level();
      PF::PyramidLevel* pl = l->get_cache_buffer(pipeline->get_render_mode())->get_pyramid().get_level( level );
      if( pl && pl->image ) {
        pipeline->set_level( level );
        VipsImage* newimg = pl->image;
        VipsImage* blendedimg;
        pipeline->set_image( newimg, l->get_id() );
        if( par->has_opacity() && blender && pipelineblender) {
          pipelineblender->import_settings( blender );
          VipsImage* omap = NULL;
          if( previous && !l->omap_layers.empty() ) {
            omap = rebuild_chain( pipeline, PF_COLORSPACE_GRAYSCALE, 
                                  previous->Xsize, previous->Ysize, 
                                  l->omap_layers, NULL );
          }
          std::vector<VipsImage*> in;
          // we add the previous image to the list of inputs, even if it is NULL
          in.push_back( previous );
          in.push_back( newimg );
          blendedimg = pipelineblender->build( in, 0, NULL, omap, level );
        } else {
          blendedimg = newimg;
          PF_REF(blendedimg,"LayerManager::rebuild_chain(): blendedimg ref");
        }
        pipeline->set_blended( blendedimg, l->get_id() );
        out = blendedimg;
        //previous = newimg;
        previous_layer = l;
        continue;
      }
    }


    /* At this point there are two possibilities:
       1. the layer has no sub-layers, in which case it is combined with the output
       of the previous layer plus any extra inputs it might have
       2. the layer has sub-layers, in which case we first build the sub-layers chain
       and then we combine it with the output of the previous layer
    */
    VipsImage* newimg = NULL;
    std::vector<VipsImage*> newimgvec;
    VipsImage* imap = NULL;
    VipsImage* omap = NULL;
    if( l->sublayers.empty() ) {
      std::vector<VipsImage*> in;
      if( par->needs_input() && !previous ) {
        // Here we have a problem: the operation we are trying to insert in the chain requires
        // a primary input image, but there is no previous image available... we give up
        std::cout<<"LayerManager::rebuild_chain(): missing input data for layer \""<<l->get_name()<<"\""<<std::endl;
        return NULL;
      }

      // We build the chains for the intensity and opacity maps
#ifndef NDEBUG
      std::cout<<"Layer \""<<l->get_name()<<"\""
               <<"  imap_layers.size()="<<l->imap_layers.size()
               <<"  omap_layers.size()="<<l->omap_layers.size()
               <<std::endl;
#endif
      if( previous && !l->imap_layers.empty() && pipelinepar->get_mask_enabled()==true ) {
        imap = rebuild_chain( pipeline, PF_COLORSPACE_GRAYSCALE, 
                              previous->Xsize, previous->Ysize, 
                              l->imap_layers, NULL );
      }

      if( previous && !l->omap_layers.empty() && pipelineblender->get_mask_enabled()==true ) {
        omap = rebuild_chain( pipeline, PF_COLORSPACE_GRAYSCALE, 
                              previous->Xsize, previous->Ysize, 
                              l->omap_layers, NULL );
      }
      
      // we add the previous image to the list of inputs, even if it is NULL
      in.push_back(previous);
#ifndef NDEBUG
      std::cout<<"Layer \""<<l->get_name()<<"\": added "<<previous<<" to the input vector"<<std::endl;
      std::cout<<"Layer \""<<l->get_name()<<"\": extra inputs size: "<<l->extra_inputs.size()<<std::endl;
#endif

      // Now we loop on the vector of extra inputs, and we include the corresponding
      // images in the input vector
      for(uint32_t iextra = 0; iextra < l->extra_inputs.size(); iextra++) {
#ifndef NDEBUG
        std::cout<<"Layer \""<<l->get_name()<<"\": adding extra input layer id="<<l->extra_inputs[iextra].first.first
                 <<" (blended="<<l->extra_inputs[iextra].second<<")..."<<std::endl;
#endif
        PF::Layer* lextra = get_layer( l->extra_inputs[iextra].first.first );
        int imgid = l->extra_inputs[iextra].first.second;
        // If the extra input layer is not found we have a problem, better to give up
        // with an error.
        //g_assert( lextra != NULL );
        if( !lextra ) {
          std::cout<<"Layer \""<<l->get_name()<<"\": extra input layer id="<<l->extra_inputs[iextra].first.first
                 <<" (blended="<<l->extra_inputs[iextra].second<<") not found (NULL layer pointer)"<<std::endl;
          in.push_back( NULL );
          continue;
        }
        // If the layer is not visible (either bcause it is disabled,
        // or one of its parents is disabled) we ignore the associated image
        if( !(lextra->is_visible()) ) {
          std::cout<<"Layer \""<<l->get_name()<<"\": extra input layer id="<<l->extra_inputs[iextra].first.first
                 <<" (blended="<<l->extra_inputs[iextra].second<<") not visible"<<std::endl;
          in.push_back( NULL );
          continue;
        }
        PF::PipelineNode* extra_node = pipeline->get_node( lextra->get_id() );
        //g_assert( extra_node != NULL );
        if( extra_node == NULL ) {
          std::cout<<"Layer \""<<l->get_name()<<"\": extra input layer id="<<l->extra_inputs[iextra].first.first
                 <<" (blended="<<l->extra_inputs[iextra].second<<") node not found"<<std::endl;
          in.push_back( NULL );
          continue;
        }
        VipsImage* extra_img = NULL;
        // std::cout<<"  imgid="<<imgid<<"  extra_node->images.size()="<<extra_node->images.size()<<std::endl;
        if( l->extra_inputs[iextra].second == true ) {
          extra_img = extra_node->blended;
        } else {
          if( (imgid>=0) && (imgid<(int)extra_node->images.size()) ) {
            extra_img = extra_node->images[imgid];
            //std::cout<<"  extra_node->images[imgid]="<<extra_node->images[imgid]<<std::endl;
          }
        }
        // Similarly, if the extra input layer has no valid image associated to it
        // we have a problem and we gve up
        //g_assert( extra_img != NULL );
        if( extra_img == NULL ) {
          std::cout<<"Layer \""<<l->get_name()<<"\": extra input layer id="<<l->extra_inputs[iextra].first.first
                 <<" (blended="<<l->extra_inputs[iextra].second<<") NULL image"<<std::endl;
          in.push_back( NULL );
          continue;
        }
        in.push_back( extra_img );

        // We inherit the image properties (size, colorspace, ...) from the last extra input
        pipelinepar->set_image_hints( extra_img );
        pipelineblender->set_image_hints( extra_img );
        if( par->is_map() ) {
          // If the layer is a mask, we force the output image colorspace to grayscale
          pipelinepar->grayscale_image( pipelinepar->get_xsize(), pipelinepar->get_ysize() );
          pipelineblender->grayscale_image( pipelinepar->get_xsize(), pipelinepar->get_ysize() );
        }
#ifndef NDEBUG
        std::cout<<" ...added."<<std::endl;
#endif
      }

      //if( par->get_config_ui() ) {
      //  par->get_config_ui()->update_properties();
      //}
#ifndef NDEBUG
      std::cout<<"Building layer \""<<l->get_name()<<"\"..."<<std::endl;
#endif

      // We import the parameters from the "master" operation associated to the layer,
      // and which is directly connected with the GUI controls
      pipelinepar->import_settings( par );

      unsigned int level = pipeline->get_level();
      newimgvec = pipelinepar->build_many( in, 0, imap, omap, level );
      newimg = (newimgvec.empty()) ? NULL : newimgvec[0];
      //cs = PF::convert_colorspace( pipelinepar->get_interpretation() );
      //std::cout<<"  par: "<<pipelinepar<<std::endl;
      //std::cout<<"  cs after build: "<<cs<<std::endl;
      //cs = PF::convert_colorspace( newimg->Type );
      //std::cout<<"  cs from newimg: "<<cs<<std::endl;

      if( (newimg != NULL) && (newimgvec.size() == 1) && l->is_cached() &&
          (l->get_cache_buffer(pipeline->get_render_mode()) != NULL) &&
          (l->get_cache_buffer(pipeline->get_render_mode())->is_initialized() == false) ) {
        // The image is being loaded, and the current layer needs to be cached
        // In this case we cache the data immediately and we use the cached
        // image instead of the newly built one
        PF::CacheBuffer* buf = l->get_cache_buffer(pipeline->get_render_mode());
        buf->set_image( newimg );
        std::cout<<"Writing cache buffer for layer "<<l->get_name()<<std::endl;
        double time1 = g_get_real_time();
        buf->write();
        buf->set_initialized( true );
        double time2 = g_get_real_time();
        std::cout<<"Buffer saved in "<<(time2-time1)/1000000<<" seconds."<<std::endl;
        PF_UNREF( newimg, "rebuild_chain(): newimg unref after cache buffer filling" );

        PF::PyramidLevel* pl = buf->get_pyramid().get_level( level );
        if( pl && pl->image ) {
          pipeline->set_level( level );
          newimg = pl->image;
          newimgvec[0] = pl->image;
        }
      }

      pipelinepar->clear_modified();
      pipeline->set_level( level );
#ifndef NDEBUG
      if( newimg ) {
        std::cout<<"rebuild_chain(): Layer \""<<l->get_name()<<"\" level="<<level
                 <<"  image size: "<<newimg->Xsize<<","<<newimg->Ysize<<std::endl
                 <<"  inputs:";
        for(int i_in = 0; i_in < in.size(); i_in++)
          std::cout<<" 0x"<<in[i_in];
        std::cout<<std::endl<<"  output: 0x"<<newimg<<std::endl;
      }
      //#endif
      //#ifndef NDEBUG
      if( !newimg ) {
        std::cout<<"WARNING: NULL image from layer \""<<name<<"\""<<std::endl;
      } else {
        void *data;
        size_t data_length;
        if( vips_image_get_blob( newimg, VIPS_META_ICC_NAME, 
                                 &data, &data_length ) ) {
          std::cout<<"WARNING: missing ICC profile from layer \""<<name<<"\""<<std::endl;
        } else {
          cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
          if( profile_in ) {
            char tstr[1024];
            cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
            std::cout<<"  Embedded profile found in layer \""<<name<<"\": "<<tstr<<std::endl;
          }
        }
      }
      std::cout<<"... done."<<std::endl;
#endif
      //if( par->get_config_ui() ) par->get_config_ui()->update();
    } else {// if( l->sublayers.empty() )
      std::vector<VipsImage*> in;

      // we add the previous image to the list of inputs, even if it is NULL
      //in.push_back( previous );

      // Then we build the chain for the sub-layers, passing the previous image as the
      // initial input
      VipsImage* isub = NULL;
      if( previous ) 
        isub = rebuild_chain( pipeline, cs, 
                              previous->Xsize, previous->Ysize, 
                              l->sublayers, previous_layer );
      else
        isub = rebuild_chain( pipeline, cs, 
                              width, height, 
                              l->sublayers, NULL );

      // we add the output of the sub-layers chain to the list of inputs, even if it is NULL
      in.push_back( isub );
      
      // Then we build the chains for the intensity and opacity maps
#ifndef NDEBUG
      std::cout<<"Layer \""<<l->get_name()<<"\""
               <<"  imap_layers.size()="<<l->imap_layers.size()
               <<"  omap_layers.size()="<<l->omap_layers.size()
               <<std::endl;
#endif
      if( previous && !l->imap_layers.empty() && pipelinepar->get_mask_enabled()==true ) {
        imap = rebuild_chain( pipeline, PF_COLORSPACE_GRAYSCALE, 
                              previous->Xsize, previous->Ysize, 
                              l->imap_layers, NULL );
        //if( !imap ) return false;
        //std::list<PF::Layer*>::reverse_iterator map_i = l->imap_layers.rbegin();
        //if(map_i != l->imap_layers.rend()) 
        //imap = (*map_i)->get_processor()->get_par()->get_image();
      }
      if( previous && !l->omap_layers.empty() && pipelineblender->get_mask_enabled()==true ) {
        omap = rebuild_chain( pipeline, PF_COLORSPACE_GRAYSCALE, 
                              previous->Xsize, previous->Ysize, 
                              l->omap_layers, NULL );
        //if( !omap ) return false;
        //std::list<PF::Layer*>::reverse_iterator map_i = l->omap_layers.rbegin();
        //if(map_i != l->omap_layers.rend()) 
        //omap = (*map_i)->get_processor()->get_par()->get_image();
      }
      
      //if( par->get_config_ui() ) par->get_config_ui()->update_properties();
#ifndef NDEBUG
      std::cout<<"Building layer \""<<l->get_name()<<"\"..."<<std::endl;
#endif
      unsigned int level = pipeline->get_level();
      pipelinepar->import_settings( par );
      newimgvec = pipelinepar->build_many( in, 0, imap, omap, level );
      newimg = (newimgvec.empty()) ? NULL : newimgvec[0];

      if( (newimg != NULL) && (newimgvec.size() == 1) && !image->is_loaded() && l->is_cached() &&
          (l->get_cache_buffer(pipeline->get_render_mode()) != NULL) &&
          (l->get_cache_buffer(pipeline->get_render_mode())->is_completed() == false) ) {
        // The image is being loaded, and the current layer needs to be cached
        // In this case we cache the data immediately and we use the cached
        // image instead of the newly built one
        PF::CacheBuffer* buf = l->get_cache_buffer(pipeline->get_render_mode());
        buf->set_image( newimg );
        std::cout<<"Writing cache buffer for layer "<<l->get_name()<<std::endl;
        gint64 time1 = g_get_real_time();
        buf->write();
        gint64 time2 = g_get_real_time();
        std::cout<<"Buffer saved in "<<(time2-time1)/1000000<<" seconds."<<std::endl;
        PF_UNREF( newimg, "rebuild_chain(): newimg unref after cache buffer filling" );

        PF::PyramidLevel* pl = buf->get_pyramid().get_level( level );
        if( pl && pl->image ) {
          pipeline->set_level( level );
          newimg = pl->image;
          newimgvec[0] = pl->image;
        }
      }

      pipelinepar->clear_modified();
      pipeline->set_level( level );
#ifndef NDEBUG
      if( newimg ) {
        std::cout<<"rebuild_chain(): Layer \""<<l->get_name()<<"\" level="<<level
                 <<"  image size: "<<newimg->Xsize<<","<<newimg->Ysize<<std::endl
                 <<"  inputs:";
        for(int i_in = 0; i_in < in.size(); i_in++)
          std::cout<<" 0x"<<in[i_in];
        std::cout<<std::endl<<"  output: 0x"<<newimg<<std::endl;
      }
#endif
#ifndef NDEBUG
      if( !newimg ) {
        std::cout<<"WARNING: NULL image from layer \""<<name<<"\""<<std::endl;
      } else {
        void *data;
        size_t data_length;
        if( vips_image_get_blob( newimg, VIPS_META_ICC_NAME, 
                                 &data, &data_length ) ) {
          std::cout<<"WARNING: missing ICC profile from layer \""<<name<<"\""<<std::endl;
        } else {
          cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
          if( profile_in ) {
            char tstr[1024];
            cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
            std::cout<<"  Embedded profile found in layer \""<<name<<"\": "<<tstr<<std::endl;
          }
        }
      }
      std::cout<<"... done."<<std::endl;
#endif
      //if( par->get_config_ui() ) par->get_config_ui()->update();
    }// if( l->sublayers.empty() )

    if( newimg ) {
      VipsImage* blendedimg;
      pipeline->set_images( newimgvec, l->get_id() );
      if( par->has_opacity() && blender && pipelineblender) {
        unsigned int level = pipeline->get_level();
        pipelineblender->import_settings( blender );

        std::vector<VipsImage*> in;
        // we add the previous image to the list of inputs, even if it is NULL
        in.push_back( previous );
        in.push_back( newimg );
        blendedimg = pipelineblender->build( in, 0, NULL, omap, level );
#ifndef NDEBUG
        if(blendedimg)
          std::cout<<"rebuild_chain(): Layer \""<<l->get_name()<<"\" level="<<level
                   <<"  blended size: "<<blendedimg->Xsize<<","<<blendedimg->Ysize<<std::endl;
#endif
      } else {
        blendedimg = newimg;
        PF_REF(blendedimg,"LayerManager::rebuild_chain(): blendedimg ref");
      }
#ifndef NDEBUG
      std::cout<<"rebuild_chain(): Layer \""<<l->get_name()<<"\"  blended: "<<blendedimg<<std::endl;
#endif
      pipeline->set_blended( blendedimg, l->get_id() );
      out = blendedimg;
      //previous = newimg;
      previous_layer = l;
    }
    if(pipelineblender) pipelineblender->clear_modified();
  }
  return out;
}


bool PF::LayerManager::rebuild_prepare()
{
#ifndef NDEBUG
  std::cout<<"PF::LayerManager::rebuild_prepare(): layers.size()="<<layers.size()<<std::endl;
#endif
  bool visible = true;
  update_visible( layers, visible );

  bool dirty = false;
  update_dirty( layers, dirty );

#ifndef NDEBUG
  std::cout<<"PF::LayerManager::rebuild_prepare(): finished"<<std::endl;
#endif
  if( !dirty ) {
    return false;
  }
  return true;
}


bool PF::LayerManager::rebuild( Pipeline* pipeline, colorspace_t cs, int width, int height, VipsRect* area )
{
  if( pipeline && pipeline->get_output() ) {
    //vips_image_invalidate_all( pipeline->get_output() );
  }
  VipsImage* output = rebuild_chain( pipeline, cs, width, height, layers, NULL );
	//std::cout<<"LayerManager::rebuild(): chain rebuild finished."<<std::endl;
  pipeline->set_output( output );
  pipeline->update( area );
	//std::cout<<"LayerManager::rebuild(): pipeline updated."<<std::endl;
  return true;
}


bool PF::LayerManager::rebuild_finalize( bool ui_update )
{
  reset_dirty( layers );
  if( ui_update ) update_ui( layers );
  return true;
}




bool PF::LayerManager::rebuild_all(Pipeline* pipeline, colorspace_t cs, int width, int height)
{
  if( layers.empty() )
    return true;
  PF::Layer* l = *(layers.begin());
  l->set_dirty( true );
  
  if( !l->imap_layers.empty() ) {
    PF::Layer* ll = *(l->imap_layers.begin());
    ll->set_dirty( true );
  }
  if( !l->omap_layers.empty() ) {
    PF::Layer* ll = *(l->omap_layers.begin());
    ll->set_dirty( true );
  }
  if( !l->sublayers.empty() ) {
    PF::Layer* ll = *(l->sublayers.begin());
    ll->set_dirty( true );
  }

  bool dirty = false;
  update_dirty( layers, dirty );

  if( !dirty ) {
    return false;
  }

  VipsImage* output  = rebuild_chain( pipeline, cs, width, height, layers, NULL );
  pipeline->set_output( output );

  reset_dirty( layers );



  return true;
}



bool PF::LayerManager::save( std::ostream& ostr )
{
  int level = 1;
  std::list<PF::Layer*>::iterator li;
  for(li = layers.begin(); li != layers.end(); ++li) {
    PF::Layer* l = *li;
    if( !l->save( ostr, level ) ) return false;
  }
  return true;
}
