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
  if( layer->get_id() >= layers_pool.size() ) {
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
    if( l->get_id() != layer->get_id() ) {
      plist.push_back( make_pair( name, l ) );
#ifndef NDEBUG
      std::cout<<"    added."<<std::endl;
#endif
    }

    if( get_parent_layers( layer, plist, name, l->sublayers ) )
      return true;

    if( get_parent_layers( layer, plist, name+"/IMap/", l->imap_layers ) )
      return true;

    if( get_parent_layers( layer, plist, name+"/OMap/", l->omap_layers ) )
      return true;

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
    if( is_map ) {
      // If the target layer is a layer map, then we only look into the 
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
    } else {
      // If the target layer is a normal one, we only look in the sublayers list
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

PF::Layer* PF::LayerManager::get_container_layer( int id )
{
  PF::Layer* layer = get_layer( id );
  if( !layer ) 
    return( NULL );
  return( get_container_layer( layer, layers ) );
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




void PF::LayerManager::update_dirty( std::list<Layer*>& list, bool& dirty )
{  
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    // if the current layer is dirty the dirty flag is set to true
    // this will also qualify as "dirty" all the subsequent layers in the list
    if( l->is_dirty() )
      dirty = true;

    // if the current layer is not qualified as "dirty", but one of the extra input layers is,
    // then we set the dirty flag to true as well
    if( !dirty ) {
      for( unsigned int i = 0; i < l->extra_inputs.size(); i++ ) {
	Layer* lextra = get_layer( l->extra_inputs[i] );
	if( lextra && lextra->is_dirty() ) {
	  dirty = true;
	  break;
	}
      }
    }

    // Now we walk through the intensity and opacity maps to see what needs to be re-built.
    // If either one or the other has to be re-built, then we mark this layer "dirty" as well
    bool imap_dirty = false;
    update_dirty( l->imap_layers, imap_dirty );

    bool omap_dirty = false;
    update_dirty( l->omap_layers, omap_dirty );

    if( imap_dirty || omap_dirty )
      dirty = true;

    // Finally we walk through the sub-layers; again, if re-building is needed 
    // we mark this layer "dirty" as well
    bool sub_dirty = false;
    update_dirty( l->sublayers, sub_dirty );

    if( sub_dirty )
      dirty = true;
    
    // Now we have finished walking through all the subchains and extra inputs,
    // and we can set the dirty flag for the current layer
    l->set_dirty( dirty );
  }
}




void PF::LayerManager::reset_dirty( std::list<Layer*>& list )
{  
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    l->clear_dirty();

    reset_dirty( l->imap_layers );
    reset_dirty( l->omap_layers );
    reset_dirty( l->sublayers );
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




VipsImage* PF::LayerManager::rebuild_chain( PF::View* view, colorspace_t cs, 
																						int width, int height, 
																						std::list<PF::Layer*>& list, 
																						PF::Layer* previous_layer )
{ 
  ViewNode* previous_node = NULL;
  VipsImage* previous = NULL;

  VipsImage* out = NULL;
  std::list<PF::Layer*>::iterator li = list.begin();
  for(li = list.begin(); li != list.end(); ++li) {
    PF::Layer* l = *li;

    char* name = (char*)l->get_name().c_str();

    //if(!strcmp(name,"Red channel mask")) {
#ifndef NDEBUG
    if(!strcmp(name,"before-after")) {
      std::cout<<"Rebuilding layer "<<name<<std::endl;
    }
#endif

    if( !l->is_visible() ) 
      continue;

#ifndef NDEBUG
    std::cout<<"PF::LayerManager::rebuild_chain(): rebuilding layer \""<<name<<"\""<<std::endl;
#endif
    if( previous_layer ) {
      previous_node = view->get_node( previous_layer->get_id() );
      if( previous_node ) previous = previous_node->image;
#ifndef NDEBUG
      std::cout<<"  Previous layer: \""<<previous_layer->get_name()<<"\""<<std::endl;
      if( previous ) {
	void *data;
	size_t data_length;
	if( vips_image_get_blob( previous, VIPS_META_ICC_NAME, 
				 &data, &data_length ) ) {
	  std::cout<<"  WARNING: missing ICC profile from previous layer \""<<previous_layer->get_name()<<"\""<<std::endl;
	} else {
	  cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
	  if( profile_in ) {
	    char tstr[1024];
	    cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
	    std::cout<<"  Embedded profile found in previous layer \""<<previous_layer->get_name()<<"\": "<<tstr<<std::endl;
	  }
	}
      } else {
	std::cout<<"  WARNING: NULL image previous layer \""<<previous_layer->get_name()<<"\""<<std::endl;
      }
#endif
    }

    PF::ViewNode* node = view->set_node( l, previous_layer );
    PF::OpParBase* par = NULL;
    if( (l->get_processor() != NULL) &&
	(l->get_processor()->get_par() != NULL) )
      par = l->get_processor()->get_par();
    PF::OpParBase* viewpar = NULL;
    if( (node != NULL) &&
	(node->processor != NULL) &&
	(node->processor->get_par() != NULL) )
      viewpar = node->processor->get_par();

    g_assert( viewpar != NULL );

    if( par ) {
#ifndef NDEBUG
      std::cout<<"PF::LayerManager::rebuild_chain(): setting format for layer "<<l->get_name()
	       <<" to "<<view->get_format()<<std::endl;
#endif
      par->set_format( view->get_format() );
    }

    // If the layer is at the beginning of the chain, we set hints about the desired
    // colorspace and pixel format using default values. 
    // The size and colorspace hints might be ignored by
    // the operation (for example in the case of an image from a file, where the
    // size and colorspace are dictated by the file content).
    // On the other hand, the pixel format hint should be strictly respected by all 
    // operators, as it defined the accuracy at which the final image is rendered.
    // If a previous image has been created already, hints are copied from it.
    //if( li == list.begin() || !previous )
    if( !previous )
      par->set_image_hints( width, height, cs );
    else if( previous )
      par->set_image_hints( previous );
    
    // If a node exists already for this layer, we simply take the associated image,
    // otherwise it means that the layer has never been processed before
    if( node ) {
      if( node->image ) {
	bool ldirty = l->is_dirty();
	if( false && !l->is_dirty() &&
	    node->image->Xsize == par->get_xsize() &&
	    node->image->Ysize == par->get_ysize() &&
	    node->image->BandFmt == par->get_format() &&
	    node->image->Bands == par->get_nbands() ) {
	  out = node->image;
	  previous_layer = l;
	  continue;
	} else {
	  //vips_image_invalidate_all( node->image );
	}
      }
    }

    // first we build the chains for the intensity and opacity maps
    VipsImage* imap = NULL;
#ifndef NDEBUG
    std::cout<<"Layer \""<<l->get_name()<<"\""
	     <<"  imap_layers.size()="<<l->imap_layers.size()
	     <<"  omap_layers.size()="<<l->omap_layers.size()
	     <<std::endl;
#endif
    if( previous && !l->imap_layers.empty() ) {
      imap = rebuild_chain( view, PF_COLORSPACE_GRAYSCALE, 
			    previous->Xsize, previous->Ysize, 
			    l->imap_layers, NULL );
      if( !imap )
	return false;
      //std::list<PF::Layer*>::reverse_iterator map_i = l->imap_layers.rbegin();
      //if(map_i != l->imap_layers.rend()) 
      //imap = (*map_i)->get_processor()->get_par()->get_image();
    }
    VipsImage* omap = NULL;
    if( previous && !l->omap_layers.empty() ) {
      omap = rebuild_chain( view, PF_COLORSPACE_GRAYSCALE, 
			    previous->Xsize, previous->Ysize, 
			    l->omap_layers, NULL );
      if( !omap )
	return false;
      //std::list<PF::Layer*>::reverse_iterator map_i = l->omap_layers.rbegin();
      //if(map_i != l->omap_layers.rend()) 
      //omap = (*map_i)->get_processor()->get_par()->get_image();
    }

    /* At this point there are two possibilities:
       1. the layer has no sub-layers, in which case it is combined with the output
          of the previous layer plus any extra inputs it might have
       2. the layer has sub-layers, in which case we first build the sub-layers chain
          and then we combine it with the output of the previous layer
    */
    VipsImage* newimg = NULL;
    if( l->sublayers.empty() ) {
      std::vector<VipsImage*> in;
      if( par->needs_input() && !previous ) {
	// Here we have a problem: the operation we are trying to insert in the chain requires
	// a primary input image, but there is no previous image available... we give up
	return false;
      }

      // we add the previous image to the list of inputs, even if it is NULL
      //if(previous)
      in.push_back(previous);

      // Now we loop on the vector of extra inputs, and we include the corresponding
      // images in the input vector
      for(uint32_t iextra = 0; iextra < l->extra_inputs.size(); iextra++) {
	PF::Layer* lextra = get_layer( l->extra_inputs[iextra] );
	// If the extra input layer is not found we have a problem, better to give up
	// with an error.
	if( !lextra ) return false;
	PF::ViewNode* extra_node = view->get_node( lextra->get_id() );
	if( !extra_node ) return false;
	VipsImage* extra_img = extra_node->image;
	//VipsImage* extra_img = lextra->get_processor()->get_par()->get_image();
	// Similarly, if the extra input layer has no valid image associated to it
	// we have a problem and we gve up
	if( !extra_img ) return false;
	in.push_back( extra_img );
      }

      if( par->get_config_ui() ) {
	par->get_config_ui()->update_properties();
      }
#ifndef NDEBUG
      std::cout<<"Building layer \""<<l->get_name()<<"\"..."<<std::endl;
#endif
      unsigned int level = view->get_level();
      viewpar->import_settings( par );
      newimg = viewpar->build( in, 0, imap, omap, level );
      view->set_level( level );
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
      if( par->get_config_ui() ) {
	par->get_config_ui()->update();
      }
    } else {
      std::vector<VipsImage*> in;

      // we add the previous image to the list of inputs, even if it is NULL
      in.push_back( previous );

      // Then we build the chain for the sub-layers, passing the previous image as the
      // initial input
      VipsImage* isub = NULL;
      if( previous ) 
	isub = rebuild_chain( view, cs, 
			      previous->Xsize, previous->Ysize, 
			      l->sublayers, previous_layer );
      else
	isub = rebuild_chain( view, cs, 
			      previous->Xsize, previous->Ysize, 
			      l->sublayers, NULL );

      // we add the output of the sub-layers chain to the list of inputs, even if it is NULL
      in.push_back( isub );
      
      if( par->get_config_ui() ) par->get_config_ui()->update_properties();
#ifndef NDEBUG
      std::cout<<"Building layer \""<<l->get_name()<<"\"..."<<std::endl;
#endif
      unsigned int level = view->get_level();
      viewpar->import_settings( par );
      newimg = viewpar->build( in, 0, imap, omap, level );
      view->set_level( level );
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
      if( par->get_config_ui() ) par->get_config_ui()->update();
    }

    if( newimg ) {
      view->set_image( newimg, l->get_id() );
      //view.set_output( newimg );
      out = newimg;
      //previous = newimg;
      previous_layer = l;
    }
  }
  return out;
}


bool PF::LayerManager::rebuild_prepare()
{
#ifndef NDEBUG
  std::cout<<"PF::LayerManager::rebuild_prepare(): layers.size()="<<layers.size()<<std::endl;
#endif
  bool dirty = false;
  update_dirty( layers, dirty );

  if( !dirty ) {
    return false;
  }
  return true;
}


bool PF::LayerManager::rebuild( View* view, colorspace_t cs, int width, int height, VipsRect* area )
{
  VipsImage* output = rebuild_chain( view, cs, width, height, layers, NULL );
	//std::cout<<"LayerManager::rebuild(): chain rebuild finished."<<std::endl;
  view->set_output( output );
  view->update( area );
	//std::cout<<"LayerManager::rebuild(): view updated."<<std::endl;
  return true;
}


bool PF::LayerManager::rebuild_finalize()
{
  reset_dirty( layers );
  return true;
}




bool PF::LayerManager::rebuild_all(View* view, colorspace_t cs, int width, int height)
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

  VipsImage* output  = rebuild_chain( view, cs, width, height, layers, NULL );
  view->set_output( output );

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
