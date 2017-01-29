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

#include <stdlib.h>

#include "fast_demosaic.hh"
#include "../base/processor.hh"

//#define RT_EMU 1


PF::FastDemosaicPar::FastDemosaicPar(): 
  OpParBase(), invGrad( 0x10000 )
{
  //set up directional weight function
  for (int i=0; i<0x10000; i++)
    invGrad[i] = 1.0/SQR(1.0+i);

  set_demand_hint( VIPS_DEMAND_STYLE_FATSTRIP );
  set_type( "fast_demosaic" );
}


VipsImage* PF::FastDemosaicPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  int border = 6;

  //VipsImage **t = (VipsImage **)
  //  vips_object_local_array( VIPS_OBJECT( in[0] ), 12 );
  VipsImage* t[12];

  //  TOP BAND
  int i0 = 0;
  VipsImage* input = in[0];
  // Extract an horizontal top band at (0,1) and with size (in[0]->Xsize,border)
  if( vips_crop(input, &t[i0], 0, 1, input->Xsize, border, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #1" );
  // Flip the band vertically
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#1) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #1" );
  // Put the vertical band above the original image
  if( vips_join(t[i0+1], in[0], &t[i0+2], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #1-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #1" );


  //  BOTTOM BAND
  i0 += 3;
  // Extract an horizontal bottom band at (0,in[0]->Ysize-border-2) and with size (in[0]->Xsize,border)
  if( vips_crop(in[0], &t[i0], 0, in[0]->Ysize-border-1, in[0]->Xsize, border, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#2) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #2" );
  // Flip the band vertically
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#2) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #2" );
  // Put the vertical band below the previously joined image
  if( vips_join(t[i0-1], t[i0+1], &t[i0+2], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#2) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #2" );


  //  LEFT BAND
  i0 += 3;
  // Extract a vertical left band at (1,0) and with size (border,t[i0-1]->Ysize)
  //PF_PRINT_REF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] refcount before crop" );
  if( vips_crop(t[i0-1], &t[i0], 1, 0, border, t[i0-1]->Ysize, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#3) failed"<<std::endl;
    return NULL;
  }
  //PF_PRINT_REF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] refcount after crop" );
  //PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #3" );
  // Flip the band horizontally
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#3) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #3" );
  // Put the vertical band left of the previously joined image
  if( vips_join(t[i0+1], t[i0-1], &t[i0+2], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#3) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #3-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #3" );


  //  RIGHT BAND
  i0 += 3;
  // Extract a vertical right band at (t[i0-1]->Xsize-2,0) and with size (border,t[i0-1]->Ysize)
  if( vips_crop(t[i0-1], &t[i0], t[i0-1]->Xsize-border-1, 0, border, t[i0-1]->Ysize, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#4) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #4" );
  // Flip the band horizontally
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#4) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #4" );
  // Put the vertical band right of the previously joined image
  if( vips_join(t[i0-1], t[i0+1], &t[i0+2], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#4) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #4-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #4" );

  //std::cout<<"i0+2="<<i0+2<<std::endl;

  // Copy to extended image
  VipsImage* extended;
  if( vips_copy(t[i0+2], &extended, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_copy(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] after vips_copy()" );
  PF_UNREF( t[i0+2], "AmazeDemosaicPar::build(): t[i0+2] after vips_copy()" );

  set_image_hints( extended );

  std::vector<VipsImage*> in2; in2.push_back(extended);
  VipsImage* img = OpParBase::build( in2, first, NULL, NULL, level );
  PF_UNREF( extended, "AmazeDemosaicPar::build(): extended unref" );
  if( !img ) return NULL;

  VipsImage* cropped = img;
  /**/
  int result;
  result = vips_crop(img, &cropped, border, border, in[0]->Xsize, in[0]->Ysize, NULL);
  PF_UNREF( img, "AmazeDemosaicPar::build(): img unref" )
  if( result ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop() failed"<<std::endl;
    return NULL;
  }
  /**/

  VipsImage* out;
  int bands = 3;
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_RGB;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  vips_copy( cropped, &out,
	     "format", format,
	     "bands", bands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  //sprintf(tifname,"/tmp/level_%d-2.tif",(int)levels.size());
  //vips_image_write_to_file( out, tifname );
  //g_object_unref( img );
  PF_UNREF( cropped, "PF::FastDemosaicPar::build(): img unref" );

  return out;
}



PF::ProcessorBase* PF::new_fast_demosaic()
{
  return( new PF::Processor<PF::FastDemosaicPar,PF::FastDemosaicProc>() );
}




/*
void fast_demosaic(VipsRegion** ir, int n, int in_first,
		   VipsRegion* imap, VipsRegion* omap, 
		   VipsRegion* oreg, PF::FastDemosaicPar* par)
{
  LUTf& invGrad = par->get_inv_grad();

  Rect *r = &oreg->valid;
  int width = r->width;
  int height = r->height;
  int right = r->left + r->width - 1;
  int bottom = r->top + r->height - 1;
  int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 

  int xoffset = oreg->im->Xoffset;
  int yoffset = oreg->im->Yoffset;
  // Size of border region where to apply simple bilinear interpolation
  int border = 4;
  // Size of the extra border around the current region where to compute 
  // pixels for proper interpolation
  int margin = 2;

  int imgxmin = xoffset;
  int imgxmax = xoffset + oreg->im->Xsize - 1;
  int imgymin = yoffset;
  int imgymax = yoffset + oreg->im->Ysize - 1;

  // Minimum x and y values of the image area to be interpolated
  int xmin = imgxmin;
  if( r->left > (xmin+margin) ) xmin = r->left - margin;
  int ymin = imgymin;
  if( r->top > (ymin+margin) ) ymin = r->top - margin;
  // X and y start values of the region inside the border
  int xstart = xmin;
  if( xstart < (imgxmin+border) ) xstart = imgxmin + border;
  int ystart = ymin;
  if( ystart < (imgymin+border) ) ystart = imgymin + border;
  // X and y offsets between min and start
  int dx = xstart - xmin;
  int dy = ystart - ymin;

  // Maximum x and y values of the image area to be interpolated
  int xmax = imgxmax;
  if( (right+margin) < xmax ) xmax = right + margin;
  int ymax = yoffset + oreg->im->Ysize - 1;
  if( (bottom+margin) < ymax ) ymax = bottom + margin;
  // X and y end values of the region inside the border
  int xend = xmax;
  if( xend > (imgxmax-border) ) xend = imgxmax - border;
  int yend = ymax;
  if( yend > (imgymax-border) ) yend = imgymax - border;

  // Size of the image area where to store pixel values (including border)
  int xsize = xmax - xmin + 1;
  int ysize = ymax - ymin + 1;

  // Size of the image region excluding border
  int xsize2 = xstart - xend + 1;
  int ysize2 = ystart - yend + 1;

  float** red = new float*[ysize];
  red[0] = new float[xsize*ysize];
  for( int row = 1; row < ysize; row++ )
    red[row] = &(red[0][row*xsize]);

  float** green = new float*[ysize];
  green[0] = new float[xsize*ysize];
  for( int row = 1; row < ysize; row++ )
    green[row] = &(green[0][row*xsize]);

  float** blue = new float*[ysize];
  blue[0] = new float[xsize*ysize];
  for( int row = 1; row < ysize; row++ )
    blue[row] = &(blue[0][row*xsize]);

  const float clip_pt = 4;


  float* rawData[7];    
  float* pout;
  int x, x2, xout, y, pi, color;
  const int i = 3;

  // interpolate G using gradient weights
  for( y = dy; y < ysize2+dy; y++ ) {
    float	wtu, wtd, wtl, wtr;
    for( pi = 0; pi < 7; pi++ ) 
      rawData[pi] = ir ? (float*)VIPS_REGION_ADDR( ir[0], xmin, ymin+y+pi-3 ) : NULL; 
    pout = (float*)VIPS_REGION_ADDR( oreg, xmin, ymin+y ); 

    for( x = dx, x2 = dx*2; x < xsize2+dx; x++, x2+=2 ) {
      color = rawData[i][x2+1];
      if( color&1 ) {
	// green site
	green[y][x] = rawData[i][x2];
      } else {
	// compute directional weights using image gradients
	wtu = invGrad[(abs(rawData[i+1][x2]-rawData[i-1][x2])+
		       abs(rawData[i][x2]-rawData[i-2][x2])+
		       abs(rawData[i-1][x2]-rawData[i-3][x2]))/4];
	wtd = invGrad[(abs(rawData[i-1][x2]-rawData[i+1][x2])+
		       abs(rawData[i][x2]-rawData[i+2][x2])+
		       abs(rawData[i+1][x2]-rawData[i+3][x2]))/4];
	wtl = invGrad[(abs(rawData[i][x2+2]-rawData[i][x2-2])+
		       abs(rawData[i][x2]-rawData[i][x2-4])+
		       abs(rawData[i][x2-2]-rawData[i][x2-6]))/4];
	wtr = invGrad[(abs(rawData[i][x2-2]-rawData[i][x2+2])+
		       abs(rawData[i][x2]-rawData[i][x2+4])+
		       abs(rawData[i][x2+2]-rawData[i][x2+6]))/4];
	  
	// store in rgb array the interpolated G value at R/B grid points 
	// using directional weighted average
	green[y][x] = (wtu*rawData[i-1][x2]+wtd*rawData[i+1][x2]+
		       wtl*rawData[i][x2-2]+wtr*rawData[i][x2+2]) / (wtu+wtd+wtl+wtr);
      }
    }
  }


  // interpolate B/R colors at R/B sites
  // the region being considered is two pixels smaller than the one
  // used for green channel interpolation, as we need to acces green values
  // at x and y +/- 1
  for( y = dy+1; y < ysize2+dy-1; y++ ) {
    float	wtu, wtd, wtl, wtr;
    for( pi = 0; pi < 7; pi++ ) 
      rawData[pi] = ir ? (float*)VIPS_REGION_ADDR( ir[0], xmin, ymin+y+pi-3 ) : NULL; 
    pout = (float*)VIPS_REGION_ADDR( oreg, xmin, ymin+y ); 

    for( x = dx+1, x2 = x*2; x < xsize2+dx-1; x++, x2+=2 ) {
      color = rawData[i][x2+1];
      if( color == 0 ) {//R site
	red[y][x] = rawData[i][x2];
	blue[y][x] = 
	  green[y][x] - 0.25f*((green[y-1][x-1]+
				green[y-1][x+1]+
				green[y+1][x+1]+
				green[y+1][x-1]) -
			       PF::min(clip_pt,
				       rawData[i-1][x2-2]+
				       rawData[i-1][x2+2]+
				       rawData[i+1][x2+2]+
				       rawData[i+1][x2-2]));
      }
      if( color == 2 ) {//B site
	blue[y][x] = rawData[i][x2];
	red[y][x] = 
	  green[y][x] - 0.25f*((green[y-1][x-1]+
				green[y-1][x+1]+
				green[y+1][x+1]+
				green[y+1][x-1]) -
			       PF::min(clip_pt,
				       rawData[i-1][x2-2]+
				       rawData[i-1][x2+2]+
				       rawData[i+1][x2+2]+
				       rawData[i+1][x2-2]));
      }
    }
  }

  // interpolate R/B using color differences
  // the region being considered is two pixels smaller than the one
  // used in the previous step, as we need to acces red, green and blue
  // values at x and y +/- 1
  for( y = dy+2; y < ysize2+dy-2; y++ ) {
    float	wtu, wtd, wtl, wtr;
    for( pi = 0; pi < 7; pi++ ) 
      rawData[pi] = ir ? (float*)VIPS_REGION_ADDR( ir[0], xmin, ymin+y+pi-3 ) : NULL; 
    pout = (float*)VIPS_REGION_ADDR( oreg, xmin, ymin+y ); 

    for( x = dx+2, x2 = x*2; x < xsize2+dx-2; x++, x2+=2 ) {
      color = rawData[i][x2+1];

      //interpolate R and B colors at G sites
      if( color != 1 ) continue;
      red[y][x] = green[y][x] - 0.25f*((green[y-1][x]-red[y-1][x])+(green[y+1][x]-red[y+1][x])+
				       (green[y][x-1]-red[y][x-1])+(green[y][x+1]-red[y][x+1]));
      blue[y][x] = green[y][x] - 0.25f*((green[y-1][x]-blue[y-1][x])+(green[y+1][x]-blue[y+1][x])+
					(green[y][x-1]-blue[y][x-1])+(green[y][x+1]-blue[y][x+1]));
    }
  }
}
*/
