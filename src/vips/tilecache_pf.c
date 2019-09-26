/* Simple tile or line cache.
 *
 * This isn't the same as the sinkscreen cache: we don't sub-divide, and we 
 * single-thread our callee.
 *
 * 23/8/06
 * 	- take ownership of reused tiles in case the cache is being shared
 * 13/2/07
 * 	- release ownership after fillng with pixels in case we read across
 * 	  threads
 * 4/2/10
 * 	- gtkdoc
 * 12/12/10
 * 	- use im_prepare_to() and avoid making a sequence for every cache tile
 * 5/12/11
 * 	- rework as a class
 * 23/6/12
 * 	- listen for "minimise" signal
 * 23/8/12
 * 	- split to line and tile cache
 * 	- use a hash table instead of a list
 * 13/9/12
 * 	- oops, linecache was oversized
 * 12/11/12
 * 	- make linecache 50% larger to give some slop room
 * 8/10/12
 * 	- make it optionally threaded
 * 21/2/13
 * 	- could deadlock if downstream raised an error (thanks Todd)
 * 25/4/13
 * 	- cache minimisation is optional, see "persistent" flag
 * 26/8/14 Lovell
 * 	- free the hash table in _dispose()
 * 11/7/16
 * 	- terminate on tile calc error
 * 7/3/17
 * 	- remove "access" on linecache, use the base class instead
 */

/*

    This file is part of VIPS.
    
    VIPS is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a cache of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA

 */

/*

    These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

/*
#define VIPS_DEBUG_RED
#define VIPS_DEBUG
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/
#include <vips/intl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vips/vips.h>
//#include <vips/internal.h>
#include <vips/debug.h>

//#include "tilecache_pf.h"

#define PHF_TILE_SIZE 128

/* A tile in cache can be in one of three states:
 *
 * DATA		- the tile holds valid pixels 
 * CALC		- some thread somewhere is calculating it
 * PEND   - some thread somewhere wants it
 * FREE   - the tile is not assigned to any cache
 */
typedef enum PhFTileState {
	VIPS_TILE_STATE_DATA,
	VIPS_TILE_STATE_CALC,
  VIPS_TILE_STATE_PEND,
  VIPS_TILE_STATE_FREE
} PhFTileState;

/* A tile in our cache.
 */
typedef struct _PhFTile {
	struct _PhFBlockCache *cache;

	PhFTileState state;

	VipsRegion *region;		/* Region with private mem for data */

	/* We count how many threads are relying on this tile. This tile can't
	 * be flushed if ref_count > 0.
	 */
	int ref_count; 

	/* Tile position. Just use left/top to calculate a hash. This is the
	 * key for the hash table. Don't use region->valid in case the region
	 * pointer is NULL.
	 */
	VipsRect pos; 

	int time;			/* Time of last use for flush */
} PhFTile;


/* The global pool of tiles available to the different caches
 */
#define PHF_MAX_NUMBER_OF_TILES 5000
typedef struct _PhFTilePool {
  PhFTile tiles[PHF_MAX_NUMBER_OF_TILES];

  GMutex *lock;     /* Lock the global pool */
  int debug;
} PhFTilePool;


static PhFTilePool phf_tile_pool;
static int phf_tile_pool_initialised = 0;

static GMutex *phf_tile_ref_lock = NULL;     /* Lock tiles referencing/unreferencing */

static void phf_tile_pool_init()
{
  int i;
  if( phf_tile_pool_initialised == 1 ) return;

  printf("phf_tile_pool_init(): called\n");

  for(i = 0; i < PHF_MAX_NUMBER_OF_TILES; i++) {
    phf_tile_pool.tiles[i].cache = NULL;
    phf_tile_pool.tiles[i].state = VIPS_TILE_STATE_FREE;
    phf_tile_pool.tiles[i].region = NULL;
    phf_tile_pool.tiles[i].ref_count = 0;
    phf_tile_pool.tiles[i].time = 0;
  }
  phf_tile_pool.lock = vips_g_mutex_new();

  phf_tile_ref_lock = vips_g_mutex_new();

  phf_tile_pool.debug = 0;
  const gchar* tilecache_debug = g_getenv("PHF_TILECACHE_DEBUG");
  if( tilecache_debug && !strcmp(tilecache_debug,"1") ) phf_tile_pool.debug = 1;
  if( tilecache_debug && !strcmp(tilecache_debug,"2") ) phf_tile_pool.debug = 2;

  phf_tile_pool_initialised = 1;
}


static void phf_tile_pool_lock()
{
  g_mutex_lock( phf_tile_pool.lock );
}


static void phf_tile_pool_unlock()
{
  g_mutex_unlock( phf_tile_pool.lock );
}


typedef struct _PhFBlockCache {
  VipsOperation parent_instance;

	VipsImage *in;
  VipsImage *out;
	int tile_width, tile_width2;
	int tile_height;
	int max_tiles;
	VipsAccess access;
	gboolean threaded;
	gboolean persistent;

	int time;			/* Update ticks for LRU here */
	int ntiles;			/* Current cache size */
	GMutex *lock;			/* Lock everything here */
	GCond *new_tile;		/* A new tile is ready */
	GHashTable *tiles;		/* Tiles, hashed by coordinates */
} PhFBlockCache;

typedef VipsOperationClass PhFBlockCacheClass;

G_DEFINE_ABSTRACT_TYPE( PhFBlockCache, phf_block_cache,
    VIPS_TYPE_OPERATION );

#define PHF_TYPE_BLOCK_CACHE (phf_block_cache_get_type())


static void
phf_tile_dispose( PhFTile *tile )
{
  PhFBlockCache *cache = tile->cache;

  VIPS_DEBUG_MSG_RED( "phf_tile_dispose: tile %d, %d (%p)\n",
    tile->pos.left, tile->pos.top, tile );
  if(phf_tile_pool.debug > 1) {
    printf("phf_tile_dispose: tile %d, %d (%p), cache: %p  ref_count: %d  state: %d\n",
      tile->pos.left, tile->pos.top, tile, cache, tile->ref_count, tile->state );
    //fflush(stdout);
  }

  g_assert( cache );

  cache->ntiles -= 1;
  g_assert( cache->ntiles >= 0 );
  tile->cache = NULL;
  tile->ref_count = 0;
  tile->state = VIPS_TILE_STATE_FREE;

  VIPS_UNREF( tile->region );
  tile->region = NULL;

  /*vips_free( tile );*/
}


/* Find a free tile or grab one from another cache.
 */
static PhFTile *
phf_tile_pool_get_tile( int now )
{
  PhFTile* result = NULL;
  PhFTile* t = NULL;
  int i, oldest = now;

  /* We loop on the tile pool twice. In the first pass we look for free tiles.
   * If no free tile is availabe, in the second pass we look for data tiles
   * with zero reference counting, and we take the oldet one.
   */
  for(i = 0; i < PHF_MAX_NUMBER_OF_TILES; i++) {
    if( phf_tile_pool.tiles[i].state == VIPS_TILE_STATE_FREE ) {
      result = &(phf_tile_pool.tiles[i]);
      //printf("phf_tile_pool_get_tile: grabbed free tile: %p\n", result);
      break;
    }
  }

  if( result == NULL ) {
    for(i = 0; i < PHF_MAX_NUMBER_OF_TILES; i++) {
      t = &(phf_tile_pool.tiles[i]);
      if( t->state == VIPS_TILE_STATE_DATA && t->ref_count == 0 ) {
        if( (result == NULL) || (t->time < oldest) ) {
          result = &(phf_tile_pool.tiles[i]);
          oldest = t->time;
        }
      }
    }
  }

  if( (result != NULL) && (result->cache != NULL) ) {
    PhFBlockCache* cache = result->cache;
    //printf("phf_tile_pool_get_tile: reusing existing tile: %p in cache: %p at %d x %d\n",
    //    result, cache, result->pos.left, result->pos.top);
    g_mutex_lock( cache->lock );
    g_hash_table_remove( cache->tiles, &(result->pos) );
    //printf("phf_tile_pool_get_tile: after g_hash_table_remove, result->cache: %p\n", result->cache);
    //phf_tile_dispose( result );
    //printf("phf_tile_pool_get_tile: after phf_tile_dispose, cache: %d\n", result->cache);
    g_mutex_unlock( cache->lock );
  }

  return result;
}


static gint64
phf_get_time( void )
{
#ifdef HAVE_MONOTONIC_TIME
  return( g_get_monotonic_time() );
#else
  GTimeVal time;

  g_get_current_time( &time );

  return( (gint64) time.tv_usec );
#endif
}


static void
phf_tile_dispose_cb( gpointer key, gpointer value, gpointer user_data )
{
  PhFTile *tile = (PhFTile *) value;
  phf_tile_dispose( tile );
}

static void
phf_block_cache_drop_all( PhFBlockCache *cache )
{
	/* FIXME this is a disaster if active threads are working on tiles. We
	 * should have something to block new requests, and only dispose once
	 * all tiles are unreffed.
	 */
  //g_hash_table_foreach( cache->tiles, phf_tile_dispose_cb, NULL );
	g_hash_table_remove_all( cache->tiles ); 
}

static void
phf_block_cache_dispose( GObject *gobject )
{
	PhFBlockCache *cache = (PhFBlockCache *) gobject;
  printf("phf_block_cache_dispose(): called, cache: %p  cache->tiles: %p\n", cache, cache->tiles);
  if( cache->tiles )
    printf("  cache hash table size: %d\n", (int)g_hash_table_size(cache->tiles));

	phf_block_cache_drop_all( cache );
  printf("phf_block_cache_dispose(): after phf_block_cache_drop_all\n");
	VIPS_FREEF( vips_g_mutex_free, cache->lock );
	VIPS_FREEF( vips_g_cond_free, cache->new_tile );

	if( cache->tiles )
		g_assert( g_hash_table_size( cache->tiles ) == 0 );
	VIPS_FREEF( g_hash_table_destroy, cache->tiles );

	G_OBJECT_CLASS( phf_block_cache_parent_class )->dispose( gobject );
}

static void
phf_tile_touch( PhFTile *tile )
{
	g_assert( tile->cache->ntiles >= 0 );

	tile->time = tile->cache->time++;
}

static int
phf_tile_init_region( PhFTile *tile )
{
  if( !(tile->region = vips_region_new( tile->cache->in )) ) {
    g_mutex_lock( tile->cache->lock );
    g_hash_table_remove( tile->cache->tiles, &tile->pos );
    g_mutex_unlock( tile->cache->lock );
    return( -1 );
  }

  vips__region_no_ownership( tile->region );

  if( vips_region_buffer( tile->region, &tile->pos ) )
    return( -1 );

  return( 0 );
}

static int
phf_tile_move( PhFTile *tile, int x, int y )
{
  /* We are changing x/y and therefore the hash value. We must unlink
   * from the old hash position and relink at the new place.
   */
  //gint64 time = phf_get_time();
  g_hash_table_steal( tile->cache->tiles, &tile->pos );
  //gint64 time2 = phf_get_time();
  //printf("vips_tile_move(): g_hash_table_steal took %d\n", (int)(time2 - time));

  tile->pos.left = x;
  tile->pos.top = y;
  tile->pos.width = tile->cache->tile_width;
  tile->pos.height = tile->cache->tile_height;

  //time = phf_get_time();
  g_hash_table_insert( tile->cache->tiles, &tile->pos, tile );
  //time2 = phf_get_time();
  //printf("vips_tile_move(): g_hash_table_insert took %d\n", (int)(time2 - time));

  //printf("phf_tile_move: tile %p added to cache %p at %d x %d\n", tile, tile->cache, tile->pos.left, tile->pos.top);

  if(tile->region ) {
    //time = phf_get_time();
    if( vips_region_buffer( tile->region, &tile->pos ) )
      return( -1 );
    //time2 = phf_get_time();
    //printf("vips_tile_move(): vips_region_buffer took %d\n", (int)(time2 - time));
  }

  /* No data yet, but someone must want it.
   */
  tile->state = VIPS_TILE_STATE_PEND;

  return( 0 );
}

static PhFTile *
phf_tile_new( PhFBlockCache *cache, int x, int y )
{
  PhFTile *tile;

  if( !(tile = VIPS_NEW( NULL, PhFTile )) )
    return( NULL );

  tile->cache = cache;
  tile->state = VIPS_TILE_STATE_PEND;
  tile->ref_count = 0;
  tile->region = NULL;
  tile->time = cache->time;
  tile->pos.left = x;
  tile->pos.top = y;
  tile->pos.width = cache->tile_width;
  tile->pos.height = cache->tile_height;
  //gint64 time = phf_get_time();
  g_hash_table_insert( cache->tiles, &tile->pos, tile );
  //gint64 time2 = phf_get_time();
  //printf("phf_tile_new(): g_hash_table_insert took %d\n", (int)(time2 - time));
  g_assert( cache->ntiles >= 0 );
  cache->ntiles += 1;

  /*
  time = phf_get_time();
  if( !(tile->region = vips_region_new( cache->in )) ) {
    g_hash_table_remove( cache->tiles, &tile->pos );
    return( NULL );
  }
  time2 = phf_get_time();
  printf("phf_tile_new(): vips_region_new took %d\n", (int)(time2 - time));

  vips__region_no_ownership( tile->region );
  */

  //time = phf_get_time();
  if( phf_tile_move( tile, x, y ) ) {
    g_hash_table_remove( cache->tiles, &tile->pos );
    return( NULL );
  }
  //time2 = phf_get_time();
  //printf("phf_tile_new(): phf_tile_move took %d\n", (int)(time2 - time));

  return( tile );
}

/* Assign a tile to a given cache
 */
static int
phf_tile_assign( PhFTile *tile, PhFBlockCache *cache, int x, int y )
{
  if(phf_tile_pool.debug > 1) {
    printf("phf_tile_assign: tile %d, %d (%p), cache: %p  ref_count: %d  state: %d\n",
      tile->pos.left, tile->pos.top, tile, cache, tile->ref_count, tile->state );
    //fflush(stdout);
  }
  tile->cache = cache;
  tile->state = VIPS_TILE_STATE_PEND;
  tile->ref_count = 0;
  tile->region = NULL;
  tile->time = cache->time;
  tile->pos.left = x;
  tile->pos.top = y;
  tile->pos.width = cache->tile_width;
  tile->pos.height = cache->tile_height;
  //gint64 time = phf_get_time();
  g_hash_table_insert( cache->tiles, &tile->pos, tile );
  //gint64 time2 = phf_get_time();
  //printf("phf_tile_new(): g_hash_table_insert took %d\n", (int)(time2 - time));
  g_assert( cache->ntiles >= 0 );
  cache->ntiles += 1;

  /*
  time = phf_get_time();
  if( !(tile->region = vips_region_new( cache->in )) ) {
    g_hash_table_remove( cache->tiles, &tile->pos );
    return( NULL );
  }
  time2 = phf_get_time();
  printf("phf_tile_new(): vips_region_new took %d\n", (int)(time2 - time));

  vips__region_no_ownership( tile->region );
  */

  //time = phf_get_time();
  //printf("phf_tile_assign: before phf_tile_move()\n");
  if( phf_tile_move( tile, x, y ) ) {
    g_hash_table_remove( cache->tiles, &tile->pos );
    return( -1 );
  }
  //printf("phf_tile_assign: after phf_tile_move()\n");
  //time2 = phf_get_time();
  //printf("phf_tile_new(): phf_tile_move took %d\n", (int)(time2 - time));

  return( 0 );
}

/* Do we have a tile in the cache?
 */
static PhFTile *
phf_tile_search( PhFBlockCache *cache, int x, int y )
{
	VipsRect pos;
	PhFTile *tile;

	pos.left = x;
	pos.top = y;
	pos.width = cache->tile_width;
	pos.height = cache->tile_height;
	tile = (PhFTile *) g_hash_table_lookup( cache->tiles, &pos );

	return( tile );
}

typedef struct _PhFTileSearch {
	PhFTile *tile;

	int oldest;
	int topmost;
} PhFTileSearch;

static void 
phf_tile_search_recycle( gpointer key, gpointer value, gpointer user_data )
{
	PhFTile *tile = (PhFTile *) value;
	PhFBlockCache *cache = tile->cache;
	PhFTileSearch *search = (PhFTileSearch *) user_data;

	/* Only consider unreffed tiles for recycling.
	 */
	if( !tile->ref_count ) {
		switch( cache->access ) {
		case VIPS_ACCESS_RANDOM:
			if( tile->time < search->oldest ) {
				search->oldest = tile->time;
				search->tile = tile;
			}
			break;

		case VIPS_ACCESS_SEQUENTIAL:
		case VIPS_ACCESS_SEQUENTIAL_UNBUFFERED:
			if( tile->pos.top < search->topmost ) {
				search->topmost = tile->pos.top;
				search->tile = tile;
			}
			break;

		default:
			g_assert_not_reached();
		}
	}
}

/* Find existing tile, make a new tile, or if we have a full set of tiles, 
 * reuse one.
 */
static PhFTile *
phf_tile_find( PhFBlockCache *cache, int x, int y )
{
	PhFTile *tile;
	PhFTileSearch search;

	/* In cache already?
	 */
  //gint64 time = phf_get_time();
	if( (tile = phf_tile_search( cache, x, y )) ) {
		VIPS_DEBUG_MSG_RED( "phf_tile_find: "
			"tile %d x %d in cache\n", x, y ); 
		//printf( "phf_tile_find: "
	  //    "tile %p (%d x %d) in cache, state: %d\n", tile, x, y, tile->state );
	  //gint64 time2 = phf_get_time();
	  //printf("phf_tile_find(): phf_tile_search took %d\n", (int)(time2 - time));
		return( tile );
	}
  //gint64 time2 = phf_get_time();
  //printf("phf_tile_find(): phf_tile_search took %d\n", (int)(time2 - time));

	/* Get a tile from the pool. It can be either a yet-not-assigned tile,
	 * or an unused data tile from this or another cache. The oldest one is grabbed.
	 */
  //printf("phf_tile_find(): before phf_tile_pool_get_tile()\n");
	if( !(tile = phf_tile_pool_get_tile(cache->time)) ) {
	  printf("phf_tile_find(): phf_tile_pool_get_tile() failed\n");
	  return NULL;
	}
  //printf("phf_tile_find(): after phf_tile_pool_get_tile()\n");
  if( phf_tile_assign( tile, cache, x, y ) ) {
    printf("phf_tile_find(): phf_tile_assign() failed\n");
    return( NULL );
  }
  //gint64 time2 = phf_get_time();
  //printf("phf_tile_find(): phf_tile_new took %d\n", (int)(time2 - time));
  //printf("phf_tile_find: tile %p assigned to %d x %d, cache: %p\n", tile, x, y, tile->cache);
  return( tile );

	/* PhFBlockCache not full?
	 */
	if( cache->max_tiles == -1 ||
		cache->ntiles < cache->max_tiles ) {
		VIPS_DEBUG_MSG_RED( "phf_tile_find: "
			"making new tile at %d x %d\n", x, y ); 
	  //gint64 time = phf_get_time();
    //printf( "phf_tile_find: "
    //    "making new tile at %d x %d + %d + %d\n", cache->tile_width, cache->tile_height, x, y );
		if( !(tile = phf_tile_new( cache, x, y )) )
			return( NULL );
	  //gint64 time2 = phf_get_time();
	  //printf("phf_tile_find(): phf_tile_new took %d\n", (int)(time2 - time));

		return( tile );
	}

	/* Reuse an old one.
	 */
	search.oldest = cache->time;
	search.topmost = cache->in->Ysize;
	search.tile = NULL;
	g_hash_table_foreach( cache->tiles, phf_tile_search_recycle, &search );
	tile = search.tile; 

	if( !tile ) {
		/* There are no tiles we can reuse -- we have to make another
		 * for now. They will get culled down again next time around.
		 */
		if( !(tile = phf_tile_new( cache, x, y )) )
			return( NULL );

		return( tile );
	}

	VIPS_DEBUG_MSG_RED( "phf_tile_find: reusing tile %d x %d\n",
		tile->pos.left, tile->pos.top );
	//printf( "phf_tile_find: reusing tile %p -> %d x %d\n",
	//    tile, tile->pos.left, tile->pos.top );

	if( phf_tile_move( tile, x, y ) )
		return( NULL );

	return( tile );
}

static gboolean            
phf_tile_unlocked( gpointer key, gpointer value, gpointer user_data )
{
	PhFTile *tile = (PhFTile *) value;

	return( !tile->ref_count );
}

static void
phf_block_cache_minimise( VipsImage *image, PhFBlockCache *cache )
{
	/* We can't drop tiles that are in use.
	 */
	g_mutex_lock( cache->lock );

	printf("phf_block_cache_minimise() called\n");
  if( cache->tiles )
    printf("  cache hash table size: %d\n", (int)g_hash_table_size(cache->tiles));

	g_hash_table_foreach_remove( cache->tiles, 
		phf_tile_unlocked, NULL );

	g_mutex_unlock( cache->lock );
}

static int
phf_block_cache_build( VipsObject *object )
{
	PhFBlockCache *cache = (PhFBlockCache *) object;

	VIPS_DEBUG_MSG( "phf_block_cache_build:\n" );

	if( VIPS_OBJECT_CLASS( phf_block_cache_parent_class )->
		build( object ) )
		return( -1 );

	VIPS_DEBUG_MSG( "phf_block_cache_build: max size = %g MB\n",
		(cache->max_tiles * cache->tile_width * cache->tile_height *
		 	VIPS_IMAGE_SIZEOF_PEL( cache->in )) / (1024 * 1024.0) );

  g_object_set( cache, "out", vips_image_new(), NULL );
  if(phf_tile_pool.debug>1)
    printf("phf_block_cache_build(): out ref count: %p->%d\n",
      G_OBJECT(cache->out), G_OBJECT(cache->out)->ref_count);

  if( !cache->persistent ) {
    if(phf_tile_pool.debug>1)
      printf("phf_block_cache_build: connecting \"minimise\" signal from cache->out(%p)\n", cache->out);
    g_signal_connect( cache->out, "minimise",
        G_CALLBACK( phf_block_cache_minimise ), cache );
  }

  return( 0 );
}

static void
phf_block_cache_class_init( PhFBlockCacheClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( class );
	VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( class );

	VIPS_DEBUG_MSG( "phf_block_cache_class_init\n" );

	gobject_class->dispose = phf_block_cache_dispose;
	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	vobject_class->nickname = "blockcache";
	vobject_class->description = _( "cache an image" );
	vobject_class->build = phf_block_cache_build;

	operation_class->flags = VIPS_OPERATION_SEQUENTIAL;

	VIPS_ARG_IMAGE( class, "in", 1, 
		_( "Input" ), 
		_( "Input image" ),
		VIPS_ARGUMENT_REQUIRED_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, in ) );

  VIPS_ARG_IMAGE( class, "out", 2,
    _( "Output" ),
    _( "Output image" ),
    VIPS_ARGUMENT_REQUIRED_OUTPUT,
    G_STRUCT_OFFSET( PhFBlockCache, out ) );

  /*VIPS_ARG_INT( class, "tile_height", 4,
		_( "Tile height" ), 
		_( "Tile height in pixels" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, tile_height ),
		1, 1000000, 128 );
  */
	VIPS_ARG_ENUM( class, "access", 6, 
		_( "Access" ), 
		_( "Expected access pattern" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, access ),
		VIPS_TYPE_ACCESS, VIPS_ACCESS_RANDOM );

	VIPS_ARG_BOOL( class, "threaded", 7, 
		_( "Threaded" ), 
		_( "Allow threaded access" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, threaded ),
		TRUE );

	VIPS_ARG_BOOL( class, "persistent", 8,
		_( "Persistent" ), 
		_( "Keep cache between evaluations" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, persistent ),
		TRUE );
}

static unsigned int
phf_rect_hash( VipsRect *pos )
{
	guint hash;

	/* We could shift down by the tile size?
	 *
	 * X discrimination is more important than Y, since
	 * most tiles will have a similar Y. 
	 */
	hash = pos->left ^ (pos->top << 16);

	return( hash );
}

static gboolean 
phf_rect_equal( VipsRect *a, VipsRect *b )
{
	return( a->left == b->left && a->top == b->top );
}

static void
phf_block_cache_init( PhFBlockCache *cache )
{
	cache->tile_width = 128;
	cache->tile_height = 128;
	cache->max_tiles = 1000;
	cache->access = VIPS_ACCESS_RANDOM;
	cache->threaded = FALSE;
	cache->persistent = FALSE;

	cache->time = 0;
	cache->ntiles = 0;
	cache->lock = vips_g_mutex_new();
	cache->new_tile = vips_g_cond_new();
	cache->tiles = g_hash_table_new_full( 
		(GHashFunc) phf_rect_hash,
		(GEqualFunc) phf_rect_equal,
		NULL,
		(GDestroyNotify) phf_tile_dispose );
}

typedef struct _PhFTileCache {
	PhFBlockCache parent_instance;

} PhFTileCache;

typedef PhFBlockCacheClass PhFTileCacheClass;

G_DEFINE_TYPE( PhFTileCache, phf_tile_cache, PHF_TYPE_BLOCK_CACHE );

static void
phf_tile_unref( PhFTile *tile )
{
  g_mutex_lock(phf_tile_ref_lock);
  if(phf_tile_pool.debug) {
    printf("phf_tile_unref: tile: %p  tile->cache: %p  ref_count: %d\n", tile, tile->cache, tile->ref_count);
    //fflush(stdout);
  }
	g_assert( tile->ref_count > 0 );

	tile->ref_count -= 1;
  g_mutex_unlock(phf_tile_ref_lock);
}

static void
phf_tile_ref( PhFTile *tile )
{
  g_mutex_lock(phf_tile_ref_lock);
	tile->ref_count += 1;
	if(phf_tile_pool.debug) {
	  printf("phf_tile_ref:   tile: %p  tile->cache: %p  ref_count: %d\n", tile, tile->cache, tile->ref_count);
	  //fflush(stdout);
	}

	g_assert( tile->ref_count > 0 );
  g_mutex_unlock(phf_tile_ref_lock);
}

static void
phf_tile_cache_unref( GSList *work )
{
	GSList *p;

	for( p = work; p; p = p->next ) {
	  PhFTile * tile = (PhFTile *) p->data;
	  if(phf_tile_pool.debug > 1) {
	    printf("phf_tile_cache_unref: tile: %p  tile->cache: %p  ref_count: %d\n", tile, tile->cache, tile->ref_count);
	    //fflush(stdout);
	  }
		phf_tile_unref( (PhFTile *) p->data );
    if(phf_tile_pool.debug > 1) {
      printf("phf_tile_cache_unref: tile: %p unreferenced\n", tile);
      //fflush(stdout);
    }
	}

	g_slist_free( work );
}

/* Make a set of work tiles.
 */
static GSList *
phf_tile_cache_ref( PhFBlockCache *cache, VipsRect *r )
{
	const int tw = cache->tile_width;
	const int th = cache->tile_height;

	/* Find top left of tiles we need.
	 */
	const int xs = (r->left / tw) * tw;
	const int ys = (r->top / th) * th;

	GSList *work;
	PhFTile *tile;
	int x, y, n;

  phf_tile_pool_lock();
	/* Ref all the tiles we will need.
	 */
	work = NULL;
	n = 0;
	//printf("phf_tile_cache_ref(): start\n");
	for( y = ys; y < VIPS_RECT_BOTTOM( r ); y += th ) {
		for( x = xs; x < VIPS_RECT_RIGHT( r ); x += tw ) {

		  //printf("phf_tile_cache_ref(): y=%d x=%d\n", y, x);
		  /* Lock the global tile pool until the tile is reff'ed,
		   * so that we do not risk that it gets re-used by another thread as well
		   */
		  //phf_tile_pool_lock();
      //gint64 time = phf_get_time();
		  tile = phf_tile_find( cache, x, y );
			if( !tile ) {
	      printf("phf_tile_cache_ref(): phf_tile_find: %p\n", tile);
			  g_assert(tile);
				phf_tile_cache_unref( work );
	      phf_tile_pool_unlock();
				return( NULL );
			}
      //gint64 time2 = phf_get_time();
      //printf("phf_tile_cache_ref(): phf_tile_find took %d, tile=%p, tile->cache=%p\n", (int)(time2 - time), tile, tile->cache);

			phf_tile_touch( tile );
      //printf("phf_tile_cache_ref(): after phf_tile_touch()\n");

	    if(phf_tile_pool.debug > 1) {
	      printf("phf_tile_cache_ref: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
	      //fflush(stdout);
	    }
	    phf_tile_ref( tile );
	    if(phf_tile_pool.debug > 1) {
	      printf("phf_tile_cache_ref: tile: %p  referenced\n", tile);
	      //fflush(stdout);
	    }
	    //printf("phf_tile_cache_ref(): after phf_tile_ref()\n");

			/* We must append, since we want to keep tile ordering
			 * for sequential sources.
			 */
			//time = phf_get_time();
			work = g_slist_append( work, tile );
			//time2 = phf_get_time();
			//printf("phf_tile_cache_ref(): g_slist_append took %d\n", (int)(time2 - time));

      /* The global tile pool can now be safely unlocked, since we own the tile
       */
      //phf_tile_pool_unlock();

      VIPS_DEBUG_MSG_RED( "phf_tile_cache_ref: "
				"tile %d, %d (%p)\n", x, y, tile ); 

			n += 1;
		}
	}

  GSList *p;
  for( p = work; p; p = p->next ) {
    tile = (PhFTile *) p->data;
    if(tile->ref_count < 1) {
      printf("phf_tile_cache_gen: tile %p wrong ref_count %d, state=%d, region=%p, cache=%p\n",
        tile, tile->ref_count, tile->state, tile->region, tile->cache);
      //fflush(stdout);
    }
    g_assert( tile->ref_count > 0 );
  }

  phf_tile_pool_unlock();
	//printf("phf_tile_cache_ref(): appended %d tiles\n", n);
	return( work );
}

static void
phf_tile_paste( PhFTile *tile, VipsRegion *or )
{
	VipsRect hit;

	/* The part of the tile that we need.
	 */
	vips_rect_intersectrect( &or->valid, &tile->pos, &hit );
	if( !vips_rect_isempty( &hit ) )
		vips_region_copy( tile->region, or, &hit, hit.left, hit.top ); 
}


/* Also called from phf_line_cache_gen(), beware.
 */
static int
phf_tile_cache_gen( VipsRegion *or,
	void *seq, void *a, void *b, gboolean *stop )
{
	VipsRegion *in = (VipsRegion *) seq;
	PhFBlockCache *cache = (PhFBlockCache *) b;
	VipsRect *r = &or->valid;

	PhFTile *tile;
  GSList *work;
  GSList *work2;
	GSList *p;
	int result;
  gint64 locked_tot;
  gint64 waiting_tot;

	result = 0;
  locked_tot = 0;
  waiting_tot = 0;

  VIPS_DEBUG_MSG_RED( "phf_tile_cache_gen: "
    "left = %d, top = %d, width = %d, height = %d\n",
    r->left, r->top, r->width, r->height );
  //printf( "phf_tile_cache_gen: thread=%p, "
  //    "left = %d, top = %d, width = %d, height = %d, threaded=%d\n",
  //    g_thread_self(), r->left, r->top, r->width, r->height, cache->threaded );
  //if( cache->tiles )
  //  printf("  cache hash table size: %d\n", (int)g_hash_table_size(cache->tiles));


  /* Ref all the tiles we will need.
   */
  //gint64 time_ = phf_get_time();
  work = phf_tile_cache_ref( cache, r );
  //gint64 time2_ = phf_get_time();
  //printf("phf_tile_cache_gen(): cache_ref took %d\n", (int)(time2_ - time_));

	VIPS_GATE_START( "phf_tile_cache_gen: wait1" );
	//gint64 time = phf_get_time();
	g_mutex_lock( cache->lock );
	//printf("phf_tile_cache_gen: cache locked by thread=%p\n", g_thread_self());
  //gint64 time2 = phf_get_time();
  //gint64 tstart = phf_get_time();
	VIPS_GATE_STOP( "phf_tile_cache_gen: wait1" );
	//printf("phf_tile_cache_gen: thread=%p, lock 1 took %d\n", g_thread_self(), (int)(time2 - time));
  //printf("phf_tile_cache_gen: thread=%p, tstart set at beginning\n",
  //            g_thread_self());

	for( p = work; p; p = p->next ) {
    tile = (PhFTile *) p->data;
    if(tile->ref_count < 1) {
      printf("phf_tile_cache_gen[1]: tile %p wrong ref_count %d, state=%d, region=%p, cache=%p\n",
        tile, tile->ref_count, tile->state, tile->region, tile->cache);
      //fflush(stdout);
    }
    g_assert( tile->ref_count > 0 );
	}

	int iter = 0;
  int nprocessed = 0;
  int npasted = 0;
	while( work ) {
    /* Search for data tiles: easy, we can just paste those in.
     */
	  gint64 time2 = phf_get_time();
	  iter += 1;
    int nchecked=0;
		for(;;) {
		  nchecked=0;
			for( p = work; p; p = p->next ) { 
				tile = (PhFTile *) p->data;
				nchecked += 1;
				if( tile->state == VIPS_TILE_STATE_DATA ) 
					break;
			}

			if( !p )
				break;

      if(phf_tile_pool.debug > 1) {
        printf("phf_tile_cache_gen: start of tile pasting: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
        //fflush(stdout);
      }
			VIPS_DEBUG_MSG_RED( "phf_tile_cache_gen: "
				"pasting %p\n", tile ); 
			//printf("phf_tile_cache_gen: pasting %p\n", tile );

			if(tile->ref_count < 1) {
			  printf("phf_tile_cache_gen[2]: tile %p wrong ref_count %d, state=%d, region=%p, cache=%p\n",
			    tile, tile->ref_count, tile->state, tile->region, tile->cache);
			  //fflush(stdout);
			}
			g_assert( tile->ref_count > 0 );

      /*printf("visp_tile_cache_gen(): before phf_tile_paste()\n");*/
			//gint64 time_ = phf_get_time();
      //gint64 tstop = phf_get_time();
      //locked_tot += tstop - tstart;
      //g_mutex_unlock(cache->lock);
			phf_tile_paste( tile, or );
      //g_mutex_lock(cache->lock);
      //tstart = phf_get_time();
			npasted += 1;
      //printf("visp_tile_cache_gen: thread=%p, iter=%d, phf_tile_paste called\n"
      //    "                     %dx%d+%d+%d -> %dx%d+%d+%d\n", g_thread_self(), iter,
      //    tile->pos.width, tile->pos.height, tile->pos.left, tile->pos.top,
      //    or->valid.width, or->valid.height, or->valid.left, or->valid.top);
      //gint64 time2_ = phf_get_time();
      //printf("phf_tile_cache_gen: phf_tile_paste took %d\n"

			/* We're done with this tile.
			 */
			work = g_slist_remove( work, tile );
      //time2_ = phf_get_time();
      //printf("phf_tile_cache_gen: phf_tile_paste + remove took %d\n", (int)(time2_ - time_));
      if(phf_tile_pool.debug > 1) {
        printf("phf_tile_cache_gen: before phf_tile_unref: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
        //fflush(stdout);
      }
			phf_tile_unref( tile );
      if(phf_tile_pool.debug > 1) {
        printf("phf_tile_cache_gen: tile: %p unreferenced\n", tile);
        //fflush(stdout);
      }
      //time2_ = phf_get_time();
      //printf("phf_tile_cache_gen: phf_tile_paste + remove + unref took %d\n", (int)(time2_ - time_));
		}
    gint64 time3 = phf_get_time();
    //printf("phf_tile_cache_gen: thread=%p, iter=%d, data tile paste took %d, pasted=%d, checked=%d\n", g_thread_self(), iter, (int)(time3 - time2), npasted, nchecked);

		/* Calculate the first PEND tile we find on the work list. We
		 * don't calculate all PEND tiles since after the first, more
		 * DATA tiles might heve been made available by other threads
		 * and we want to get them out of the way as soon as we can.
		 */
		for( p = work; p; p = p->next ) { 
			tile = (PhFTile *) p->data;

			//printf("phf_tile_cache_gen(): tile->state=%d\n", tile->state);

      if(phf_tile_pool.debug > 1) {
        printf("phf_tile_cache_gen: checking tile before preparation: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
        //fflush(stdout);
      }
			if( tile->state == VIPS_TILE_STATE_PEND ) {
				tile->state = VIPS_TILE_STATE_CALC;

				VIPS_DEBUG_MSG_RED( "phf_tile_cache_gen: "
					"calc of %p\n", tile ); 

	      if(phf_tile_pool.debug > 1) {
	        printf("phf_tile_cache_gen: start of tile preparation: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
	        //fflush(stdout);
	      }
	      g_assert( tile->ref_count > 0 );

				/* In threaded mode, we let other threads run
				 * while we calc this tile. In non-threaded
				 * mode, we keep the lock and make 'em wait.
				 */
			  //gint64 tstop = phf_get_time();
			  //locked_tot += tstop - tstart;
        //printf("phf_tile_cache_gen: thread=%p, iter=%d, time elpased before phf_region_prepare_to: %d\n",
        //    g_thread_self(), iter, (int)(tstop - tstart));
        //printf("phf_tile_cache_gen: thread=%p, cache unlocked\n", g_thread_self());
				if( cache->threaded ) 
					g_mutex_unlock( cache->lock );
			  //gint64 time3 = phf_get_time();

			  if( !tile->region ) {
			    //printf("phf_tile_cache_gen(): calling phf_tile_init_region()\n");
			    phf_tile_init_region( tile );
          //printf("phf_tile_cache_gen(): phf_tile_init_region() finished\n");
			  }

        g_assert( tile->ref_count > 0 );
        g_assert( tile->region );

	      result = vips_region_prepare_to( in,
					tile->region, 
					&tile->pos, 
					tile->pos.left, tile->pos.top );
				nprocessed += 1;
				/*
        printf("phf_tile_cache_gen: thread=%p, iter=%d, vips_region_prepare_to finished\n"
            "                     in->im=%dx%d\n"
            "                     in->valid=%dx%d+%d+%d\n"
            "                     tile->region->valid=%dx%d+%d+%d\n"
            "                     tile->pos=%dx%d+%d+%d\n", g_thread_self(), iter,
            in->im->Xsize, in->im->Ysize,
            in->valid.width, in->valid.height, in->valid.left, in->valid.top,
            tile->region->valid.width, tile->region->valid.height, tile->region->valid.left, tile->region->valid.top,
            tile->pos.width, tile->pos.height, tile->pos.left, tile->pos.top);
				*/

				if( cache->threaded ) {
					VIPS_GATE_START( "phf_tile_cache_gen: "
						"wait2" );
					//time = phf_get_time();
					g_mutex_lock( cache->lock );
	        //printf("phf_tile_cache_gen: thread=%p, cache locked\n", g_thread_self());
					//time2 = phf_get_time();
					VIPS_GATE_STOP( "phf_tile_cache_gen: "
						"wait2" );
					//printf("phf_tile_cache_gen(): lock 2 took %d\n", (int)(time2 - time));
				}

        if(phf_tile_pool.debug > 1) {
          printf("phf_tile_cache_gen: end of tile preparation: cache: %p  tile: %p  tile->cache: %p  ref_count: %d\n", cache, tile, tile->cache, tile->ref_count);
          //fflush(stdout);
        }
	      g_assert( tile->ref_count > 0 );
        //tstart = phf_get_time();
        //printf("phf_tile_cache_gen: thread=%p, iter=%d, tstart set after vips_region_prepare_to\n",
        //            g_thread_self(), iter);
				/* If there was an error calculating this
				 * tile, black it out and terminate
				 * calculation. We have to stop so we can
				 * support things like --fail on jpegload.
				 *
				 * Don't return early, we'd deadlock. 
				 */
				if( result ) {
					VIPS_DEBUG_MSG_RED( 
						"phf_tile_cache_gen: "
						"error on tile %p\n", tile ); 

					g_warning( _( "error in tile %d x %d" ),
						tile->pos.left, tile->pos.top );

					vips_region_black( tile->region );

					*stop = TRUE;
				}

				tile->state = VIPS_TILE_STATE_DATA;

				phf_tile_touch( tile );

				/* Let everyone know there's a new DATA tile. 
				 * They need to all check their work lists.
				 */
				g_cond_broadcast( cache->new_tile );

				break;
			}
		}

		/* There are no PEND or DATA tiles, we must need a tile some
		 * other thread is currently calculating.
		 *
		 * We must block until the CALC tiles we need are done.
		 */
		if( !p && 
			work ) {
			for( p = work; p; p = p->next ) { 
				tile = (PhFTile *) p->data;

				g_assert( tile->state == VIPS_TILE_STATE_CALC );
			}

			VIPS_DEBUG_MSG_RED( "phf_tile_cache_gen: waiting\n" );

			VIPS_GATE_START( "phf_tile_cache_gen: wait3" );

      //gint64 tstop = phf_get_time();
      //locked_tot += tstop - tstart;
      //printf("phf_tile_cache_gen: thread=%p, iter=%d, time elpased before g_cond_wait: %d\n",
      //    g_thread_self(), iter, (int)(tstop - tstart));
      //printf("phf_tile_cache_gen: cache unlocked by thread=%p\n", g_thread_self());
			g_cond_wait( cache->new_tile, cache->lock );
		  //printf("phf_tile_cache_gen: cache locked by thread=%p\n", g_thread_self());
			//tstart = phf_get_time();
      //printf("phf_tile_cache_gen: thread=%p, iter=%d, waited %d\n",
      //            g_thread_self(), iter, (int)(tstart-tstop));
			//waiting_tot += tstart - tstop;

			VIPS_GATE_STOP( "phf_tile_cache_gen: wait3" );

			VIPS_DEBUG_MSG( "phf_tile_cache_gen: awake!\n" );
		}
	}

  //gint64 tstop = phf_get_time();
  //locked_tot += tstop - tstart;
  //printf("phf_tile_cache_gen: thread=%p, iter=%d, total time elapsed before end: %d\n",
  //    g_thread_self(), iter, (int)(tstop - tstart));
  //printf("phf_tile_cache_gen: thread=%p, ts=%dx%d, iter=%d, total=%d, waiting=%d, pasted=%d, processed=%d\n",
  //        g_thread_self(), cache->tile_width, cache->tile_height, iter,
  //        (int)(tstop-time), (int)waiting_tot, npasted, nprocessed);
  if( 0 && locked_tot > 200 ) {
    printf("phf_tile_cache_gen: thread=%p, iter=%d, total time spend in locked state: %ld, pasted=%d, processed=%d",
        g_thread_self(), iter, locked_tot, npasted, nprocessed);
    printf("   ********************");
    printf("\n");
  }
  //printf("phf_tile_cache_gen: cache unlocked by thread=%p\n", g_thread_self());
	g_mutex_unlock( cache->lock );
  //printf( "phf_tile_cache_gen: thread=%p, finished\n", g_thread_self() );

	return( result );
}

static int
phf_tile_cache_build( VipsObject *object )
{
	PhFBlockCache *block_cache = (PhFBlockCache *) object;
	PhFTileCache *cache = (PhFTileCache *) object;

	VIPS_DEBUG_MSG( "phf_tile_cache_build\n" );

	if( VIPS_OBJECT_CLASS( phf_tile_cache_parent_class )->
		build( object ) )
		return( -1 );

	if( vips_image_pio_input( block_cache->in ) )
		return( -1 );

	if( vips_image_pipelinev( block_cache->out,
		VIPS_DEMAND_STYLE_SMALLTILE, block_cache->in, NULL ) )
		return( -1 );

	if( vips_image_generate( block_cache->out,
		vips_start_one, phf_tile_cache_gen, vips_stop_one,
		block_cache->in, cache ) )
		return( -1 );

  printf("phf_tile_cache_build(): out ref count: %p->%d\n",
      G_OBJECT(block_cache->out), G_OBJECT(block_cache->out)->ref_count);
	return( 0 );
}

static void
phf_tile_cache_class_init( PhFTileCacheClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( class );

	VIPS_DEBUG_MSG( "phf_tile_cache_class_init\n" );

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	vobject_class->nickname = "phf_tilecache";
	vobject_class->description = _( "cache an image as a set of tiles" );
	vobject_class->build = phf_tile_cache_build;

	/*VIPS_ARG_INT( class, "tile_width", 3,
		_( "Tile width" ), 
		_( "Tile width in pixels" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, tile_width2 ),
		1, 1000000, 128 );

	VIPS_ARG_INT( class, "max_tiles", 5, 
		_( "Max tiles" ), 
		_( "Maximum number of tiles to cache" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( PhFBlockCache, max_tiles ),
		-1, 1000000, 1000 );
  */
}

static void
phf_tile_cache_init( PhFTileCache *cache )
{
  phf_tile_pool_init();
  cache->parent_instance.tile_width = PHF_TILE_SIZE;
}

/**
 * phf_tilecache:
 * @in: input image
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Optional arguments:
 *
 * * @tile_width: width of tiles in cache
 * * @tile_height: height of tiles in cache
 * * @max_tiles: maximum number of tiles to cache
 * * @access: hint expected access pattern #VipsAccess
 * * @threaded: allow many threads
 * * @persistent: don't drop cache at end of computation
 *
 * This operation behaves rather like vips_copy() between images
 * @in and @out, except that it keeps a cache of computed pixels. 
 * This cache is made of up to @max_tiles tiles (a value of -1 
 * means any number of tiles), and each tile is of size @tile_width
 * by @tile_height pixels. 
 *
 * Each cache tile is made with a single call to 
 * vips_region_prepare(). 
 *
 * When the cache fills, a tile is chosen for reuse. If @access is
 * #VIPS_ACCESS_RANDOM, then the least-recently-used tile is reused. If 
 * @access is #VIPS_ACCESS_SEQUENTIAL 
 * the top-most tile is reused.
 *
 * By default, @tile_width and @tile_height are 128 pixels, and the operation
 * will cache up to 1,000 tiles. @access defaults to #VIPS_ACCESS_RANDOM.
 *
 * Normally, only a single thread at once is allowed to calculate tiles. If
 * you set @threaded to %TRUE, phf_tilecache() will allow many threads to
 * calculate tiles at once, and share the cache between them.
 *
 * Normally the cache is dropped when computation finishes. Set @persistent to
 * %TRUE to keep the cache between computations.
 *
 * See also: phf_cache(), phf_linecache().
 *
 * Returns: 0 on success, -1 on error.
 */
int
phf_tilecache( VipsImage *in, VipsImage **out, ... )
{
	va_list ap;
	int result;

  //printf("phf_tilecache(): in ref count (before): %p->%d\n",
  //    G_OBJECT(in), G_OBJECT(in)->ref_count);

	va_start( ap, out );
	result = vips_call_split( "phf_tilecache", ap, in, out );
	va_end( ap );

  //printf("phf_tilecache(): in ref count (after): %p->%d\n",
  //    G_OBJECT(in), G_OBJECT(in)->ref_count);
  //printf("phf_tilecache(): out ref count: %p->%d\n",
  //    G_OBJECT(*out), G_OBJECT(*out)->ref_count);

	return( result );
}

typedef struct _PhFLineCache {
	PhFBlockCache parent_instance;

} PhFLineCache;

typedef PhFBlockCacheClass PhFLineCacheClass;

G_DEFINE_TYPE( PhFLineCache, phf_line_cache, PHF_TYPE_BLOCK_CACHE );

static int
phf_line_cache_gen( VipsRegion *or,
	void *seq, void *a, void *b, gboolean *stop )
{
	PhFBlockCache *block_cache = (PhFBlockCache *) b;

	VIPS_GATE_START( "phf_line_cache_gen: wait" );

	g_mutex_lock( block_cache->lock );

	VIPS_GATE_STOP( "phf_line_cache_gen: wait" );

	/* We size up the cache to the largest request.
	 */
	if( or->valid.height > 
		block_cache->max_tiles * block_cache->tile_height ) {
		block_cache->max_tiles = 
			1 + (or->valid.height / block_cache->tile_height);
		VIPS_DEBUG_MSG( "phf_line_cache_gen: bumped max_tiles to %d\n",
			block_cache->max_tiles ); 
	}

	g_mutex_unlock( block_cache->lock );

	return( phf_tile_cache_gen( or, seq, a, b, stop ) );
}

static int
phf_line_cache_build( VipsObject *object )
{
	PhFBlockCache *block_cache = (PhFBlockCache *) object;
	PhFLineCache *cache = (PhFLineCache *) object;

	int tile_width;
	int tile_height;
	int n_lines;

	VIPS_DEBUG_MSG( "phf_line_cache_build\n" );

	if( !vips_object_argument_isset( object, "access" ) ) 
		block_cache->access = VIPS_ACCESS_SEQUENTIAL;

	if( VIPS_OBJECT_CLASS( phf_line_cache_parent_class )->
		build( object ) )
		return( -1 );

	/* This can go up with request size, see phf_line_cache_gen().
	 */
	vips_get_tile_size( block_cache->in,
		&tile_width, &tile_height, &n_lines );
	block_cache->tile_width = block_cache->in->Xsize;

	/* Output has two buffers n_lines height, so 2 * n_lines is the maximum
	 * non-locality from threading. Add another n_lines for conv / reducev
	 * etc. 
	 */
	block_cache->max_tiles = 3 * n_lines / block_cache->tile_height;

	VIPS_DEBUG_MSG( "phf_line_cache_build: n_lines = %d\n",
		n_lines );
	VIPS_DEBUG_MSG( "phf_line_cache_build: max_tiles = %d\n",
		block_cache->max_tiles ); 
	VIPS_DEBUG_MSG( "phf_line_cache_build: tile_height = %d\n",
		block_cache->tile_height ); 
	VIPS_DEBUG_MSG( "phf_line_cache_build: max size = %g MB\n",
		(block_cache->max_tiles * 
		 block_cache->tile_width * 
		 block_cache->tile_height * 
		 VIPS_IMAGE_SIZEOF_PEL( block_cache->in )) / (1024 * 1024.0) );

	if( vips_image_pio_input( block_cache->in ) )
		return( -1 );

	if( vips_image_pipelinev( block_cache->out,
		VIPS_DEMAND_STYLE_THINSTRIP, block_cache->in, NULL ) )
		return( -1 );

	if( vips_image_generate( block_cache->out,
		vips_start_one, phf_line_cache_gen, vips_stop_one,
		block_cache->in, cache ) )
		return( -1 );

	return( 0 );
}

static void
phf_line_cache_class_init( PhFLineCacheClass *class )
{
	VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( class );

	VIPS_DEBUG_MSG( "phf_line_cache_class_init\n" );

	vobject_class->nickname = "linecache";
	vobject_class->description = _( "cache an image as a set of lines" );
	vobject_class->build = phf_line_cache_build;

}

static void
phf_line_cache_init( PhFLineCache *cache )
{
}

/**
 * phf_linecache:
 * @in: input image
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Optional arguments:
 *
 * * @access: hint expected access pattern #VipsAccess
 * * @tile_height: height of tiles in cache
 * * @threaded: allow many threads
 *
 * This operation behaves rather like vips_copy() between images
 * @in and @out, except that it keeps a cache of computed scanlines. 
 *
 * The number of lines cached is enough for a small amount of non-local
 * access. 
 *
 * Each cache tile is made with a single call to 
 * vips_region_prepare(). 
 *
 * When the cache fills, a tile is chosen for reuse. If @access is
 * #VIPS_ACCESS_RANDOM, then the least-recently-used tile is reused. If 
 * @access is #VIPS_ACCESS_SEQUENTIAL, then 
 * the top-most tile is reused. @access defaults to #VIPS_ACCESS_RANDOM.
 *
 * @tile_height can be used to set the size of the strips that
 * phf_linecache() uses. The default is 1 (a single scanline).
 *
 * Normally, only a single thread at once is allowed to calculate tiles. If
 * you set @threaded to %TRUE, phf_linecache() will allow many threads to
 * calculate tiles at once and share the cache between them.
 *
 * See also: phf_cache(), phf_tilecache().
 *
 * Returns: 0 on success, -1 on error.
 */
int
pf_linecache( VipsImage *in, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "linecache", ap, in, out );
	va_end( ap );

	return( result );
}
