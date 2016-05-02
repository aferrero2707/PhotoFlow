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

#include "pf_mkstemp.hh"
#include "photoflow.hh"
#include "rawbuffer.hh"


void PF::PencilMask::init(unsigned int s, float op, float sm)
{
  //std::cout<<"PencilMask::init("<<s<<", "<<op<<", "<<sm<<") called. mask="<<mask<<std::endl;
  if( mask ) {
    for( unsigned int i = 0; i < size; i++) delete[] mask[i];
    delete[] mask;
    mask = NULL;
  }

  size = s;
  opacity = op;
  smoothness = sm;
  if( size == 0 ) return;

  mask = new float*[size];
  for( unsigned int i = 0; i < size; i++) mask[i] = new float[size];

  int xc = size/2;
  int yc = xc;
  float rmin = (0.99999f-smoothness)*xc;
  float rmax = xc;
  //std::cout<<"PencilMask::init(): rmin="<<rmin<<"  rmax="<<rmax<<std::endl;
  float dr = rmax-rmin;
  float dr2 = dr*dr;
  float minus = -1.0f;
  for( unsigned int x = 0; x < size; x++) {
    float dx = x-xc;
    float dx2 = dx*dx;
    for( unsigned int y = 0; y < size; y++) {
      float dy = y-yc;
      float r2 = dx2 + dy*dy;
      float r = sqrt(r2);
      float rr = (r - rmin)/dr;
      //std::cout<<"r="<<r<<"  rmin="<<rmin<<"  dr="<<dr<<std::endl;
      float rr2 = rr*2;
      float sigma = 0.08;
      //sigma = sigma*sigma;
      //float val = (r<rmin) ? 1 : ( (r>=rmax) ? 0 : (rmax-r)/(rmax-rmin) );
      //float val = (r<rmin) ? 1 : ( (r>=rmax) ? 0 : exp(minus*(r-rmin)*(r-rmin)*3.5/dr2) );
      //float val = (r<rmin) ? 1 : ( (r>=rmax) ? 0 : exp(-1.0f*(rr)/(2.0f*sigma)) );
      float val = 0;
      if( rr < 0 ) val = 1;
      else if( rr < 1 ) {
        if( rr2 < 1 ) val = 1.0f-(rr2*rr2)/2;
        else val = (2.0f-rr2)*(2.0f-rr2)/2;
        //float exp = 1.8;
        //if( rr2 < 1 ) val = 1.0f-pow(rr2,exp)/2;
        //else val = pow(2.0f-rr2,exp)/2;
      }
      //std::cout<<"x="<<x<<"  y="<<y<<"  rr="<<rr<<"  val="<<val<<std::endl;
      //float val2 = sqrt( val );
      mask[y][x] = val*opacity;
      //if(r>(rmin-1) && r<(rmin+1)) std::cout<<"r="<<r<<"  (rmin="<<rmin<<")  mask["<<y<<"]["<<x<<"]="<<mask[y][x]<<std::endl;
    }
  }
}



PF::RawBuffer::RawBuffer():
      image( NULL ),
      fd(-1)
{
  bands = 1;
  xsize = 100; ysize = 100;
  format = VIPS_FORMAT_NOTSET;
  coding = VIPS_CODING_NONE;
}


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
for( unsigned int ch = 0; ch < bands; ch++ ) {					\
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
  if( fd < 0 ) {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    //fd = mkostemp( fname, O_CREAT|O_RDWR|O_TRUNC );
    fd = pf_mkstemp( fname );
    if( fd >= 0 )
      file_name = fname;
  }

  if( fd < 0 )
    return;

  if( bands > 16 )
    return;

  if( bgdcol.size() < bands )
    return;

  size_t sizeofpel;

  bgd_color = bgdcol;

  off_t seek_result = lseek( fd, 0, SEEK_SET );

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
    for( unsigned int ch = 0; ch < bands; ch++ ) {
      val[ch] = (unsigned short int)(bgd_color[ch]*FormatInfo<unsigned short int>::RANGE + FormatInfo<unsigned short int>::MIN);
    }
    unsigned short int* tbuf = (unsigned short int*)buf;
    for( unsigned int x  = 0; x < xsize; x++ ) {
      for( unsigned int ch = 0; ch < bands; ch++ ) {
        tbuf[x*bands+ch] = val[ch];
      }
    }
    for( unsigned int y = 0; y < ysize; y++ ) {
      ssize_t write_res = write( fd, buf, sizeof(unsigned short int)*xsize*bands );
      if( write_res != sizeof(unsigned short int)*xsize*bands )
        break;
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
  default:
    return;
  }
  pxmask = NULL;

  stroke_ranges.clear();
  for( unsigned int y = 0; y < ysize; y++ )
    stroke_ranges.push_back( std::list< std::pair<unsigned int, unsigned int> >() );

  //return;

  if( image ) {
    //g_object_unref( image );
    PF_UNREF( image, "PF::RawBuffer::init()" );
  }
  //close(fd);
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
}




#define DRAW_ROW( TYPE ) {																							\
    TYPE val[16];																													\
    for( unsigned int ch = 0; ch < bands; ch++ ) {																	\
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
        for( unsigned int ch = 0; ch < bands; ch++ ) {															\
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
        for( unsigned int ch = 0; ch < bands; ch++ ) {																\
          x = col*bands + ch;																								\
          tbuf[x] = val[ch];																								\
        }																																		\
      }									\
} \
off_t result = lseek( fd, offset, SEEK_SET );					\
if(result<0) perror("draw_row(): lseek failed");					\
size_t bufsize = sizeof(TYPE)*bands*(endcol-startcol+1);	\
write( fd, buf, bufsize );						\
}


void PF::RawBuffer::draw_row( Pencil& pen, unsigned int row, 
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
  default:
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



void PF::RawBuffer::draw_point( Pencil& pen, unsigned int x0, unsigned int y0,
    VipsRect& update, bool update_pyramid )
{
  if( fd < 0 )
    return;

  //std::cout<<"RawBuffer::draw_point("<<x0<<","<<y0<<"): fd="<<fd<<std::endl;
  //std::cout<<"Pencil color: ";
  //for( int ch = 0; ch < bands; ch++ ) {
  //	std::cout<<pen.get_channel(ch)<<"  ";
  //}
  //std::cout<<std::endl;
  //x0 = pen.get_size() - 1;
  //y0 = pen.get_size() - 1;
  for(int y = 0; y <= pen.get_size(); y++ ) {
    int row1 = y0 - y;
    int row2 = y0 + y;
    //int L = pen.get_size() - y;
    int D = (int)sqrt( pen.get_size()*pen.get_size() - y*y );
    int startcol = x0 - D;
    if( startcol < 0 ) 
      startcol = 0;
    int endcol = x0 + D;
    if( endcol >= (int)xsize )
      endcol = xsize - 1;

    //endcol = x0;


    if( row1 >= 0 )
      draw_row( pen, row1, startcol, endcol );
    if( (row2 != row1) && (row2 < (int)ysize) )
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

  //return;

  if( update_pyramid ) 
    pyramid.update( update );
}



void PF::RawBuffer::draw_segment( Pencil& pen, Segment& segment )
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
      if( endcol >= (int)xsize )
        endcol = xsize - 1;


      if( row1 >= 0 )
        draw_row( pen, row1, startcol, endcol );
      if( (row2 != row1) && (row2 < (int)ysize) )
        draw_row( pen, row2, startcol, endcol );

      //if( i==1 ) break;
    }
  }
}


void PF::RawBuffer::start_stroke()
{
  if( buf ) free( buf );
  buf = NULL;
  switch( get_format() ) {
  case VIPS_FORMAT_UCHAR:
    buf = malloc( sizeof(unsigned char)*xsize*bands );
    memset(buf, 0, sizeof(unsigned char)*xsize*bands );
    break;
  case VIPS_FORMAT_USHORT:
    buf = malloc( sizeof(unsigned short int)*xsize*bands );
    memset(buf, 0, sizeof(unsigned short int)*xsize*bands );
    break;
  case VIPS_FORMAT_FLOAT:
    buf = malloc( sizeof(float)*xsize*bands );
    memset(buf, 0, sizeof(float)*xsize*bands );
    break;
  case VIPS_FORMAT_DOUBLE:
    buf = malloc( sizeof(double)*xsize*bands );
    memset(buf, 0, sizeof(double)*xsize*bands );
    break;
  default:
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
