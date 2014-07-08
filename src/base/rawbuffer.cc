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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <stdlib.h>

#include <iostream>

#include "photoflow.hh"
#include "rawbuffer.hh"


PF::RawBuffer::RawBuffer(std::string fname):
  file_name( fname ),
  image( NULL )
{
  bands = 1;
  xsize = 100; ysize = 100;
  format = VIPS_FORMAT_NOTSET;
  coding = VIPS_CODING_NONE;
  fd = open( file_name.c_str(), O_CREAT|O_RDWR|O_TRUNC, S_IRWXU );
}


#define INIT_BUF( TYPE ) {											\
  sizeofpel = sizeof(TYPE)*bands;\ 
buf = malloc( sizeofpel*xsize );	\
if( !buf ) break;				\
TYPE val[16];			\
for( int ch = 0; ch < bands; ch++ ) {					\
	val[ch] = (TYPE)(bgd_color[ch]*FormatInfo<TYPE>::RANGE + FormatInfo<TYPE>::MIN); \
 }									\
TYPE* tbuf = (TYPE*)buf;						\
for( unsigned int x  = 0; x < xsize; x++ ) {					\
	for( unsigned int ch = 0; ch < bands; ch++ ) {					\
		tbuf[x*bands+ch] = val[ch];						\
	}									\
 }									\
for( unsigned int y = 0; y < ysize; y++ ) {				\
	write( fd, buf, sizeof(TYPE)*xsize*bands );				\
 }									\
free( buf );								\
buf = NULL;								\
}									


void PF::RawBuffer::init( const std::vector<float>& bgdcol)
{
  if( fd < 0 )
    return;

  if( bands > 16 )
    return;
  
  if( bgdcol.size() < bands )
    return;

  size_t sizeofpel;

  bgd_color = bgdcol;

  lseek( fd, 0, SEEK_SET );

  switch( get_format() ) {
  case VIPS_FORMAT_UCHAR:
    INIT_BUF( unsigned char );
    break;
  case VIPS_FORMAT_USHORT:
    //INIT_BUF( unsigned short int );
    {
      sizeofpel = sizeof(unsigned short int)*bands;
      buf = malloc( sizeofpel*xsize );
      if( !buf ) break;
      unsigned short int val[16];
      for( int ch = 0; ch < bands; ch++ ) {
				val[ch] = (unsigned short int)(bgd_color[ch]*FormatInfo<unsigned short int>::RANGE + FormatInfo<unsigned short int>::MIN);
      }
      unsigned short int* tbuf = (unsigned short int*)buf;
      for( unsigned int x  = 0; x < xsize; x++ ) {
				for( unsigned int ch = 0; ch < bands; ch++ ) {
					tbuf[x*bands+ch] = val[ch];
				}
      }
      for( unsigned int y = 0; y < ysize; y++ ) {
				write( fd, buf, sizeof(unsigned short int)*xsize*bands );
      }
      free( buf );
      buf = NULL;
    }									
    break;
  case VIPS_FORMAT_FLOAT:
    INIT_BUF( float );
    break;
  case VIPS_FORMAT_DOUBLE:
    INIT_BUF( double );
    break;
  }
  pxmask = NULL;
  
  if( image ) {
    //g_object_unref( image );
    PF_UNREF( image, "PF::RawBuffer::init()" );
  }
  VipsImage* tempimg;
  vips_rawload( file_name.c_str(), &tempimg, xsize, ysize, sizeofpel, NULL );
  vips_copy( tempimg, &image, 
						 "format", format,
						 "bands", bands,
						 "coding", coding,
						 "interpretation", interpretation,
						 NULL );

  PF_UNREF( tempimg, "PF::RawBuffer::init() after rawload()" );
  pyramid.init( image, fd );

  //unsigned int level = 4;
  //PF::PyramidLevel* l = pyramid.get_level( level );

  stroke_ranges.clear();
  for( unsigned int y = 0; y < ysize; y++ )
    stroke_ranges.push_back( std::list< std::pair<unsigned int, unsigned int> >() );
}




#define DRAW_ROW( TYPE ) {																							\
  TYPE val[16];																													\
  for( int ch = 0; ch < bands; ch++ ) {																	\
    val[ch] = (TYPE)(pen.get_channel(ch)*FormatInfo<TYPE>::RANGE + FormatInfo<TYPE>::MIN); \
  }																																			\
  unsigned int x;																												\
  TYPE* tbuf = (TYPE*)buf;																							\
  off_t offset = (off_t(xsize)*row+startcol)*sizeof(TYPE)*bands;				\
  lseek( fd, offset, SEEK_SET );																				\
  unsigned int col, col2;																								\
  unsigned int npx = endcol-startcol+1;																	\
  if( pen.get_opacity() < 1 ) {																					\
    read( fd, buf, sizeof(TYPE)*bands*(endcol-startcol+1) );						\
    TYPE oldval;																												\
    float transparency = 1.0f - pen.get_opacity();											\
    for( col = 0, col2 = startcol; col < npx; col++, col2++ ) {					\
      if( pxmask[col2] == 0 ) {																					\
				continue;																												\
      }																																	\
      for( int ch = 0; ch < bands; ch++ ) {															\
				x = col*bands + ch;																							\
				oldval = tbuf[x];																								\
				tbuf[x] = (TYPE)(pen.get_opacity()*val[ch] + transparency*oldval); \
      }																																	\
    }																																		\
  } else {																															\
	for( col = 0, col2 = startcol; col < npx; col++, col2++ ) {						\
		if( pxmask[col2] == 0 ) {																						\
			continue;																													\
		}																																		\
		for( int ch = 0; ch < bands; ch++ ) {																\
			x = col*bands + ch;																								\
			tbuf[x] = val[ch];																								\
		}																																		\
	}									\    
} \
lseek( fd, offset, SEEK_SET );					\
size_t bufsize = sizeof(unsigned short int)*bands*(endcol-startcol+1);	\
write( fd, buf, bufsize );						\
}


void PF::RawBuffer::draw_row( Pen& pen, unsigned int row, 
															unsigned int startcol, unsigned int endcol )
{
  if( fd < 0 )
    return;

  if( pen.get_color().size() < bands )
    return;

	//std::cout<<"RawBuffer::draw_row("<<row<<","<<startcol<<","<<endcol<<")"<<std::endl;
  int npixels = endcol-startcol+1;
  memset( &(pxmask[startcol]), 0xFF, npixels );
  
  std::list< std::pair<unsigned int, unsigned int> >::iterator ri;
  for( ri = stroke_ranges[row].begin(); ri != stroke_ranges[row].end(); ++ri ) {
    int nexcluded = ri->second-ri->first+1;
    memset( &(pxmask[ri->first]), 0x0, nexcluded );
    //npixels -= nexcluded;
  }

  if( npixels <= 0 ) 
    return;

  size_t pixel_size = 0;
  switch( get_format() ) {
  case VIPS_FORMAT_UCHAR:
    DRAW_ROW( unsigned char );
    break;
  case VIPS_FORMAT_USHORT:
    DRAW_ROW( unsigned short int );
    /*
			{
      unsigned short int val[16];								
      for( int ch = 0; ch < bands; ch++ ) {					
			val[ch] = (unsigned short int)(pen.get_channel(ch)*FormatInfo<unsigned short int>::RANGE + FormatInfo<unsigned short int>::MIN); 
      }									
      unsigned int x;						
      unsigned short int* tbuf = (unsigned short int*)buf;						
      off_t offset = (off_t(xsize)*row+startcol)*sizeof(unsigned short int)*bands;	
      std::cout<<"  offset="<<offset<<std::endl;
      lseek( fd, offset, SEEK_SET );					
      unsigned int npx = endcol-startcol+1;
      if( pen.get_opacity() < 1 ) {				
			read( fd, buf, sizeof(unsigned short int)*bands*(endcol-startcol+1) );	
			unsigned short int oldval;						
			float transparency = 1.0f - pen.get_opacity();		
			for( unsigned int col = startcol; col <= endcol; col++ ) {	
			//for( unsigned int col = 0; col <= npx; col++ ) {	
			for( int ch = 0; ch < bands; ch++ ) {					
	    x = col*bands + ch;				
	    oldval = tbuf[x];						
	    tbuf[x] = (unsigned short int)(pen.get_opacity()*val[ch] + transparency*oldval); 
			}									
			}									
      } else {
			for( unsigned int col = 0; col <= npx; col++ ) {	
			for( int ch = 0; ch < bands; ch++ ) {					
	    x = col*bands + ch;						
	    tbuf[x] = val[ch];						
			}									
			}									    
      } 
      lseek( fd, offset, SEEK_SET );					
      size_t bufsize = sizeof(unsigned short int)*bands*(endcol-startcol+1);
      std::cout<<"  bufsize="<<bufsize<<std::endl;
      //write( fd, buf, bufsize );	
      write( fd, &(tbuf[startcol*bands]), bufsize );
      //write( fd, &(tbuf[startcol*bands]), sizeof(TYPE)*bands*(endcol-startcol+1) ); \
			}
    */
    break;
  case VIPS_FORMAT_FLOAT:
    DRAW_ROW( float );
    break;
  case VIPS_FORMAT_DOUBLE:
    DRAW_ROW( double );
    break;
  }

  // Merge existing painted ranges with the newly painted pixels
  if( stroke_ranges[row].empty() ) {
    stroke_ranges[row].push_back( std::make_pair(startcol, endcol) );
  } else {
    bool inserted = false;
    for( ri = stroke_ranges[row].begin(); ri != stroke_ranges[row].end(); ++ri ) {
      if( startcol < ri->first ) {
				// The new range is just after the current one, it's time to include the
				// new range in the list
				stroke_ranges[row].insert( ri, std::make_pair(startcol,endcol) );
				inserted = true;
				break;
      }
    }
    if( !inserted )
      stroke_ranges[row].push_back( std::make_pair(startcol, endcol) );
  }
}



void PF::RawBuffer::draw_point( Pen& pen, unsigned int x0, unsigned int y0,
																VipsRect& update, bool update_pyramid )
{
	std::cout<<"RawBuffer::draw_point("<<x0<<","<<y0<<"): fd="<<fd<<std::endl;
  if( fd < 0 )
    return;

  for(int y = 0; y <= pen.get_size(); y++ ) {
    int row1 = y0 - y;
    int row2 = y0 + y;
    //int L = pen.get_size() - y;
    int D = (int)sqrt( pen.get_size()*pen.get_size() - y*y );
    int startcol = x0 - D;
    if( startcol < 0 ) 
      startcol = 0;
    int endcol = x0 + D;
    if( endcol >= xsize ) 
      endcol = xsize - 1;


    if( row1 >= 0 )
      draw_row( pen, row1, startcol, endcol );
    if( (row2 != row1) && (row2 < ysize) )
      draw_row( pen, row2, startcol, endcol );
  }
	//fsync( fd );

  VipsRect area;
  area.left = x0 - pen.get_size();
  area.top = y0 - pen.get_size();
  area.width = area.height = pen.get_size()*2 + 1;

  VipsRect img;
  img.top = img.left = 0;
  img.width = xsize;
  img.height = ysize;

  vips_rect_intersectrect (&img, &area, &update);

  if( update_pyramid ) 
    pyramid.update( update );
}



void PF::RawBuffer::draw_segment( Pen& pen, Segment& segment )
{
  if( fd < 0 )
    return;

  // Draw the circles at the beginning and at the end of the segment
  int x0[2], y0[2];
  x0[0] = segment.get_x1();
  y0[0] = segment.get_y1();
  x0[1] = segment.get_x2();
  y0[1] = segment.get_y2();
  for( unsigned int i = 0; i < 2; i++ ) {
    //for(int y = pen.get_size(); y >= 0; y-- ) {
    for(int y = 0; y <= pen.get_size(); y++ ) {
      int row1 = y0[i] - y;
      int row2 = y0[i] + y;
      //int L = pen.get_size() - y;
      int D = (int)sqrt( pen.get_size()*pen.get_size() - y*y );
      int startcol = x0[i] - D;
      if( startcol < 0 ) 
				startcol = 0;
      int endcol = x0[i] + D;
      if( endcol >= xsize ) 
				endcol = xsize - 1;


      if( row1 >= 0 )
				draw_row( pen, row1, startcol, endcol );
      if( (row2 != row1) && (row2 < ysize) )
				draw_row( pen, row2, startcol, endcol );

      //if( i==1 ) break;
    }
  }
}


void PF::RawBuffer::start_stroke()
{
  if( buf ) free( buf );
  switch( get_format() ) {
  case VIPS_FORMAT_UCHAR:
    buf = malloc( sizeof(unsigned char)*xsize*bands );
    break;
  case VIPS_FORMAT_USHORT:
    buf = malloc( sizeof(unsigned short int)*xsize*bands );
    break;
  case VIPS_FORMAT_FLOAT:
    buf = malloc( sizeof(float)*xsize*bands );
    break;
  case VIPS_FORMAT_DOUBLE:
    buf = malloc( sizeof(double)*xsize*bands );
    break;
  }
  if( !buf ) return;
  
  if( pxmask ) delete pxmask;
  pxmask = new unsigned char[xsize];
  for( unsigned int y = 0; y < ysize; y++ )
    stroke_ranges[y].clear();
}


void PF::RawBuffer::end_stroke()
{
  if( buf ) free( buf );
  buf = NULL;
  if( pxmask ) delete pxmask;
  pxmask = NULL;
  for( unsigned int y = 0; y < ysize; y++ )
    stroke_ranges[y].clear();
}

//void PF::RawBuffer::draw_stroke( Stroke& stroke );
