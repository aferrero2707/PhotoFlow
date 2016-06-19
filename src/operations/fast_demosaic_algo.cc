/* 
 */

////////////////////////////////////////////////////////////////
//
//		Fast demosaicing algorythm
//
//		copyright (c) 2008-2010  Emil Martinec <ejmartin@uchicago.edu>
//
//
// code dated: August 26, 2010
//
//	fast_demo.cc is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////


/*

    Fast demosaicing algorithm adapted from the original implementation 
    in RawTherapee (http://rawtherapee.com/)

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#include <stdlib.h>
#include <math.h>

//#define RT_EMU

#include "fast_demosaic.hh"


void PF::fast_demosaic(VipsRegion** ir, int n, int in_first,
    VipsRegion* imap, VipsRegion* omap,
    VipsRegion* oreg, PF::FastDemosaicPar* par)
{
  PF_LUTf& invGrad = par->get_inv_grad();

  Rect *r = &oreg->valid;
  int width = r->width;
  int height = r->height;

#ifndef NDEBUG
  if(r->left==0)std::cout<<"fast_demosaic(): left="<<r->left<<"  top="<<r->top<<std::endl;
#endif

  // Size of border region where to apply simple bilinear interpolation
  int bord = 4;
  // Size of the extra border around the current region where to compute 
  // pixels for proper interpolation
  int margin = 2;

  VipsRect r_img = {0, 0, ir[in_first]->im->Xsize, ir[in_first]->im->Ysize};
  VipsRect r_img2 = {bord, bord, ir[in_first]->im->Xsize - 2*bord, ir[in_first]->im->Ysize - 2*bord};

  VipsRect r_raw = {r->left - 5, r->top - 5, r->width + 10, r->height + 10};
  vips_rect_intersectrect (&r_raw, &r_img, &r_raw);

  VipsRect r_step3 = {r->left, r->top, r->width, r->height};
  vips_rect_intersectrect (&r_step3, &r_img2, &r_step3);

  VipsRect r_step2 = {r_step3.left-1, r_step3.top-1, r_step3.width+2, r_step3.height+2};
  vips_rect_intersectrect (&r_step2, &r_img2, &r_step2);

  VipsRect r_step1 = {r_step2.left-1, r_step2.top-1, r_step2.width+2, r_step2.height+2};
  vips_rect_intersectrect (&r_step1, &r_img2, &r_step1);

  int imgxmin = 0;
  int imgxmax = oreg->im->Xsize - 1;
  int imgymin = 0;
  int imgymax = oreg->im->Ysize - 1;

#ifdef RT_EMU
  /* RawTherapee emulation */
  float initialGain = 1.53516;
  float clip_pt = 4*65535*initialGain;
#else
  // No clipping?
  float clip_pt = 4000000;
#endif

  int left = r->left;
  int right = r->left + width - 1;
  int left1 = left - 1; if( left1 < 0 ) left1 = 0;
  int left2 = left1 - 1; if( left2 < 0 ) left2 = 0;
  int right1 = right + 1; if( right1 >= imgxmax ) right1 = imgxmax;
  int right2 = right1 + 1; if( right2 >= imgxmax ) right2 = imgxmax;

  int top = r->top;
  int bottom = r->top + height - 1;
  int top1 = top - 1; if( top1 < 0 ) top1 = 0;
  int top2 = top1 - 1; if( top2 < 0 ) top2 = 0;
  int bottom1 = bottom + 1; if( bottom1 >= imgymax ) bottom1 = imgymax;
  int bottom2 = bottom1 + 1; if( bottom2 >= imgymax ) bottom2 = imgymax;

  int xstart = left; if( xstart < bord ) xstart = bord;
  int xstart1 = left1; if( xstart1 < bord ) xstart1 = bord;
  int xstart2 = left2; if( xstart2 < bord ) xstart2 = bord;

  int ystart = top; if( ystart < bord ) ystart = bord;
  int ystart1 = top1; if( ystart1 < bord ) ystart1 = bord;
  int ystart2 = top2; if( ystart2 < bord ) ystart2 = bord;

  int xend = right; if( xend > (imgxmax-bord) ) xend = imgxmax-bord;
  int xend1 = right1; if( xend1 > (imgxmax-bord) ) xend1 = imgxmax-bord;
  int xend2 = right2; if( xend2 > (imgxmax-bord) ) xend2 = imgxmax-bord;

  int yend = bottom; if( yend > (imgymax-bord) ) yend = imgymax-bord;
  int yend1 = bottom1; if( yend1 > (imgymax-bord) ) yend1 = imgymax-bord;
  int yend2 = bottom2; if( yend2 > (imgymax-bord) ) yend2 = imgymax-bord;

  int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 

  int x, x2, xout, y, pi, color;

  PF::RawMatrix rawData;
  rawData.init( r_raw.width, r_raw.height, r_raw.top, r_raw.left );
  for( y = 0; y < r_raw.height; y++ ) {
    PF::raw_pixel_t* ptr = ir ? (PF::raw_pixel_t*)VIPS_REGION_ADDR( ir[0], r_raw.left, y+r_raw.top ) : NULL; 
    rawData.set_row( y+r_raw.top, ptr );
  }
  //if( r_raw.left==0 && r_raw.top==0 ) {
  //	std::cout<<"rawData[0][0] = "<<rawData[0][0]<<"  c="<<(int)rawData[0].color(0)<<std::endl;
  //}
  PF::Array2D<float> red, green, blue;
  red.Init( r_raw.width, r_raw.height, r_raw.top, r_raw.left );
  green.Init( r_raw.width, r_raw.height, r_raw.top, r_raw.left );
  blue.Init( r_raw.width, r_raw.height, r_raw.top, r_raw.left );


  /*
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  //first, interpolate borders using bilinear
  for (int i=top2; i<=bottom2; i++) {

    float sum[6];

    for (int j=xstart2; j<bord; j++) {//first few columns
      for (int c=0; c<6; c++) sum[c]=0;
      for (int i1=i-1; i1<i+2; i1++)
	for (int j1=j-1; j1<j+2; j1++) {
	  if ((i1 > -1) && (i1 <= bottom2) && (j1 > -1)) {
	    //int c = FC(i1,j1);
	    int c=rawData[i1].color(j1);
	    sum[c] += rawData[i1][j1];
	    sum[c+3]++;
	  }
	}
      //int c=FC(i,j);
      int c=rawData[i].color(j);
      if (c==1) {
	red[i][j]=sum[0]/sum[3];
	green[i][j]=rawData[i][j];
	blue[i][j]=sum[2]/sum[5];
      } else {
	green[i][j]=sum[1]/sum[4];
	if (c==0) {
	  red[i][j]=rawData[i][j];
	  blue[i][j]=sum[2]/sum[5];
	} else {
	  red[i][j]=sum[0]/sum[3];
	  blue[i][j]=rawData[i][j];
	}
      }
    }//j

    for (int j=xend2+1; j<=right2; j++) {//last few columns
      for (int c=0; c<6; c++) sum[c]=0;
      for (int i1=i-1; i1<i+2; i1++)
	for (int j1=j-1; j1<j+2; j1++) {
	  if ((i1 > -1) && (i1 <= bottom2 ) && (j1 <= right2)) {
	    //int c = FC(i1,j1);
	    int c = rawData[i1].color(j1);
	    sum[c] += rawData[i1][j1];
	    sum[c+3]++;
	  }
	}
      //int c=FC(i,j);
      int c=rawData[i].color(j);
      if (c==1) {
	red[i][j]=sum[0]/sum[3];
	green[i][j]=rawData[i][j];
	blue[i][j]=sum[2]/sum[5];
      } else {
	green[i][j]=sum[1]/sum[4];
	if (c==0) {
	  red[i][j]=rawData[i][j];
	  blue[i][j]=sum[2]/sum[5];
	} else {
	  red[i][j]=sum[0]/sum[3];
	  blue[i][j]=rawData[i][j];
	}
      }
    }//j
  }//i

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  for (int j=xstart2; j <= xend2; j++) {
    float sum[6];

    for (int i=top2; i<bord; i++) {//first few rows
      for (int c=0; c<6; c++) sum[c]=0;
      for (int i1=i-1; i1<i+2; i1++)
	for (int j1=j-1; j1<j+2; j1++) {
	  if ((j1 > -1) && (j1 <= right2) && (i1 > -1)) {
	    //int c = FC(i1,j1);
	    std::cout<<"  j="<<j<<"  i="<<i<<"  j1="<<j1<<"  i1="<<i1<<"  bord="<<bord<<std::endl;
	    int c = rawData[i1].color(j1);
	    sum[c] += rawData[i1][j1];
	    sum[c+3]++;
	  }
	}
      //int c=FC(i,j);
      int c = rawData[i].color(j);
      if (c==1) {
	red[i][j]=sum[0]/sum[3];
	green[i][j]=rawData[i][j];
	blue[i][j]=sum[2]/sum[5];
      } else {
	green[i][j]=sum[1]/sum[4];
	if (c==0) {
	  red[i][j]=rawData[i][j];
	  blue[i][j]=sum[2]/sum[5];
	} else {
	  red[i][j]=sum[0]/sum[3];
	  blue[i][j]=rawData[i][j];
	}
      }
    }//i

    for (int i=yend2+1; i<=bottom2; i++) {//last few rows
      for (int c=0; c<6; c++) sum[c]=0;
      for (int i1=i-1; i1<i+2; i1++)
	for (int j1=j-1; j1<j+2; j1++) {
	  if  ((j1 > -1) && (j1 <= right2) && (i1 <= bottom2)) {
	    //int c = FC(i1,j1);
	    int c = rawData[i1].color(j1);
	    sum[c] += rawData[i1][j1];
	    sum[c+3]++;
	  }
	}
      //int c=FC(i,j);
      int c = rawData[i].color(j);
      if (c==1) {
	red[i][j]=sum[0]/sum[3];
	green[i][j]=rawData[i][j];
	blue[i][j]=sum[2]/sum[5];
      } else {
	green[i][j]=sum[1]/sum[4];
	if (c==0) {
	  red[i][j]=rawData[i][j];
	  blue[i][j]=sum[2]/sum[5];
	} else {
	  red[i][j]=sum[0]/sum[3];
	  blue[i][j]=rawData[i][j];
	}
      }
    }//i
  }//j
   */

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // interpolate G using gradient weights
  for (int i=ystart2; i <= yend2; i++) {
    float	wtu, wtd, wtl, wtr;
    float	iwtu, iwtd, iwtl, iwtr;
    for (int j=xstart2; j <= xend2; j++) {
      color = rawData[i].color(j);
      if (color&1) {
        green[i][j] = rawData[i][j];
        //red[i][j] = green[i][j];
        //blue[i][j] = green[i][j];
      } else {
        //compute directional weights using image gradients
        iwtu = (fabs(rawData[i+1][j]-
            rawData[i-1][j])+
            fabs(rawData[i][j]-
                rawData[i-2][j])+
                fabs(rawData[i-1][j]-
                    rawData[i-3][j])) /4;
        wtu=invGrad[(int)iwtu];
        iwtd = (fabs(rawData[i-1][j]-
            rawData[i+1][j])+
            fabs(rawData[i][j]-
                rawData[i+2][j])+
                fabs(rawData[i+1][j]-
                    rawData[i+3][j])) /4;
        wtd=invGrad[(int)iwtd];
        iwtl = (fabs(rawData[i][j+1]-
            rawData[i][j-1])+
            fabs(rawData[i][j]-
                rawData[i][j-2])+
                fabs(rawData[i][j-1]-
                    rawData[i][j-3])) /4;
        wtl=invGrad[(int)iwtl];
        iwtr = (fabs(rawData[i][j-1]-
            rawData[i][j+1])+
            fabs(rawData[i][j]-
                rawData[i][j+2])+
                fabs(rawData[i][j+1]-
                    rawData[i][j+3])) /4;
        wtr=invGrad[(int)iwtr];

        //store in rgb array the interpolated G value at R/B grid points using directional weighted average
        green[i][j]=(wtu*rawData[i-1][j]+wtd*rawData[i+1][j]+wtl*rawData[i][j-1]+wtr*rawData[i][j+1]) / (wtu+wtd+wtl+wtr);
        //red[i][j] = green[i][j];
        //blue[i][j] = green[i][j];

        if(false && i<14 && j<14) {
          for(int ii = -3; ii <= 3; ii++) {
            std::cout<<"	";
            for(int jj = -3; jj <= 3; jj++) {
              std::cout<<rawData[i+ii][j+jj]<<"  ";
            }
            std::cout<<std::endl;
          }
          std::cout<<"	i="<<i<<"  j="<<j
              <<"  wtu="<<wtu
              <<"  wtd="<<wtd
              <<"  wtl="<<wtl
              <<"  wtr="<<wtr
              <<"  iwtu="<<iwtu
              <<"  iwtd="<<iwtd
              <<"  iwtl="<<iwtl
              <<"  iwtr="<<iwtr
              <<std::endl;
        }
      }
      if(false && i<14 && j<14)
        std::cout<<"step #1	i="<<i<<"  j="<<j<<"  green[i][j]="<<green[i][j]<<std::endl;
    }
  }


  for (int i=ystart1; i <= yend1; i++) {
    float	wtu, wtd, wtl, wtr;
    color = rawData[i].color(xstart1);
    int dx = color&1;
    for (int j=xstart1+dx; j <= xend1; j+=2) {
      color = rawData[i].color(j);
      if( (color&1) != 0 )
        continue;
      //interpolate B/R colors at R/B sites
      if (color==0) {//R site
        red[i][j] = rawData[i][j];
        blue[i][j] = green[i][j] - 0.25f*((green[i-1][j-1]+green[i-1][j+1]+green[i+1][j+1]+green[i+1][j-1]) -
            PF::min(static_cast<float>(clip_pt),rawData[i-1][j-1]+rawData[i-1][j+1]+rawData[i+1][j+1]+rawData[i+1][j-1]));
      } else {//B site
        red[i][j] = green[i][j] - 0.25f*((green[i-1][j-1]+green[i-1][j+1]+green[i+1][j+1]+green[i+1][j-1]) -
            PF::min(static_cast<float>(clip_pt),rawData[i-1][j-1]+rawData[i-1][j+1]+rawData[i+1][j+1]+rawData[i+1][j-1]));
        blue[i][j] = rawData[i][j];
      }
      if(false && i<14 && j<14)
        std::cout<<"step #2	i="<<i<<"  j="<<j
        <<"  red[i][j]="<<red[i][j]
                                 <<"  blue[i][j]="<<blue[i][j]
                                                            <<std::endl;
    }
  }


  // interpolate R/B using color differences
  for (int i=ystart; i <= yend; i++) {
    float	wtu, wtd, wtl, wtr;
    color = rawData[i].color(xstart);
    int dx = 1 - (color&1);
    for (int j=xstart+dx; j <= xend; j+=2) {
      color = rawData[i].color(j);
      if( (color&1) != 1 ) 
        continue;

      //interpolate R and B colors at G sites
      red[i][j] = green[i][j] - 0.25f*((green[i-1][j]-red[i-1][j])+(green[i+1][j]-red[i+1][j])+
          (green[i][j-1]-red[i][j-1])+(green[i][j+1]-red[i][j+1]));
      blue[i][j] = green[i][j] - 0.25f*((green[i-1][j]-blue[i-1][j])+(green[i+1][j]-blue[i+1][j])+
          (green[i][j-1]-blue[i][j-1])+(green[i][j+1]-blue[i][j+1]));
      if(false && i<14 && j<14)
        std::cout<<"step #3	i="<<i<<"  j="<<j
        <<"  red[i][j]="<<red[i][j]
                                 <<"  blue[i][j]="<<blue[i][j]
                                                            <<std::endl;
    }
  }

  int xx = 0;
  for( y = 0; y < r->height; y++ ) {
    float* ptr = (float*)VIPS_REGION_ADDR( oreg, r->left, y+r->top ); 
    for( x = 0, xx = 0; x < width; x++, xx+=3 ) {
#ifdef RT_EMU
      /* RawTherapee emulation */
      ptr[x*3] = red[y+r->top][x+r->left]/65535;
      ptr[x*3+1] = green[y+r->top][x+r->left]/65535;
      ptr[x*3+2] = blue[y+r->top][x+r->left]/65535;
#else
      //ptr[xx] = CLAMP( red[y+r->top][x+r->left], 0, 1 );
      //ptr[xx+1] = CLAMP( green[y+r->top][x+r->left], 0, 1 );
      //ptr[xx+2] = CLAMP( blue[y+r->top][x+r->left], 0, 1 );
      ptr[xx] = red[y+r->top][x+r->left];
      ptr[xx+1] = green[y+r->top][x+r->left];
      ptr[xx+2] = blue[y+r->top][x+r->left];
#endif
      if( r->top < 10 && r-> left < 10 )
        std::cout<<"r="<<r->top+y<<" c="<<r->left+x<<"  raw="
        <<rawData[r->top+y][r->left+x]<<"  rgb="<<ptr[xx]<<","<<ptr[xx+1]<<","<<ptr[xx+2]<<std::endl;
    }
  }
}


