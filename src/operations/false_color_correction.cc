/* 
	 False color correction after RAW image demosaicing
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

 *  These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/
 *
 *  Source code adapted from the original RawTherapee version.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "../base/array2d.hh"
#include "../base/processor.hh"
#include "../rt/rtengine/median.h"
#include "false_color_correction.hh"

//#define RT_EMU 1


PF::FalseColorCorrectionPar::FalseColorCorrectionPar(): 
  OpParBase()
{
  set_type( "false_color_correction" );
}


VipsImage* PF::FalseColorCorrectionPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  VipsImage* extended;
  if( vips_embed(in[0], &extended, 2, 2, in[0]->Xsize+4, in[0]->Ysize+4, "extend", VIPS_EXTEND_MIRROR, NULL) ) {
    std::cout<<"FalseColorCorrectionPar::build(): vips_embed() failed."<<std::endl;
    return NULL;
  }
  set_image_hints( extended );

  std::vector<VipsImage*> in2; in2.push_back(extended);
  VipsImage* out = OpParBase::build( in2, first, NULL, NULL, level );
  PF_UNREF( extended, "FalseColorCorrectionPar::build(): extended unref" )

  VipsImage* cropped = out;
  int result;
  result = vips_crop(out, &cropped, 2, 2, in[0]->Xsize, in[0]->Ysize, NULL);
  PF_UNREF( out, "FalseColorCorrectionPar::build(): out unref" )
  if( result ) {
    std::cout<<"FalseColorCorrectionPar::build(): vip_crop() failed"<<std::endl;
    return NULL;
  }

  return cropped;
}


static inline void convert_row_to_YIQ (float* r, float* Y, float* I, float* Q, int W) 
{
	int j, j2;
  for (j=0, j2=0; j<W; j++, j2+=3) {
    Y[j] = .299 * r[j2] + .587 * r[j2+1] + .114 * r[j2+2];
    I[j] = .596 * r[j2] - .275 * r[j2+1] - .321 * r[j2+2];
    Q[j] = .212 * r[j2] - .523 * r[j2+1] + .311 * r[j2+2];
  }
}

static inline void convert_row_to_RGB (float* r, float* Y, float* I, float* Q, int W) {
	int j, j2;
  for (j=0, j2=0; j<W; j++, j2+=3) {
    r[j2] = Y[j] + 0.956*I[j] + 0.621*Q[j];
    r[j2+1] = Y[j] - 0.272*I[j] - 0.647*Q[j];
    r[j2+2] = Y[j] - 1.105*I[j] + 1.702*Q[j];
  }
}

void PF::false_color_correction(VipsRegion* ir, VipsRegion* oreg)
{
	// Extra pixels around the output region needed to properly compute the color correction
	// One pixel is needed for the final blurring, and one for the median filering step
	int pad1 = 1;
	int pad2 = 2;

	// Image area minus the padding border
	const VipsRect img_rect = {
		0, 0, ir->im->Xsize, ir->im->Ysize
	};
	const VipsRect img_rect_pad1 = {
		pad1, pad1, ir->im->Xsize-pad1*2, ir->im->Ysize-pad1*2
	};
	const VipsRect img_rect_pad2 = {
		pad2, pad2, ir->im->Xsize-pad2*2, ir->im->Ysize-pad2*2
	};

	// Area to be updated in the output 
	VipsRect out_rect;
	vips_rect_intersectrect( &img_rect_pad2, &(oreg->valid), &out_rect );

	// Area on which the median filter has to be applied 
	// One pixel larger than the output area
	VipsRect mf_rect = {
		out_rect.left-1,
		out_rect.top-1,
		out_rect.width+2,
		out_rect.height+2
	};
	vips_rect_intersectrect( &img_rect_pad1, &mf_rect, &mf_rect );
	vips_rect_intersectrect( &(ir->valid), &mf_rect, &mf_rect );

	// Input area
	// Two pixels larger than the output area
	VipsRect in_rect = {
		out_rect.left-2,
		out_rect.top-2,
		out_rect.width+4,
		out_rect.height+4
	};
	vips_rect_intersectrect( &img_rect, &in_rect, &in_rect );
	vips_rect_intersectrect( &(ir->valid), &in_rect, &in_rect );

	const int W = in_rect.width;
  constexpr float onebynine = 1.f / 9.f;

	int row_from = mf_rect.top;
	int row_to = VIPS_RECT_BOTTOM(&mf_rect);

	PF::Array2D<float> rbconv_Y;  rbconv_Y.Init(W,3,0,0);
  PF::Array2D<float> rbconv_I; rbconv_I.Init(W,3,0,0);
  PF::Array2D<float> rbconv_Q; rbconv_Q.Init(W,3,0,0);
  PF::Array2D<float> rbout_I; rbout_I.Init(W,3,0,0);
  PF::Array2D<float> rbout_Q; rbout_Q.Init(W,3,0,0);

  float* row_I = new float[W];
  float* row_Q = new float[W];

  float buffer[12];
  float* pre1 = &buffer[0];
  float* pre2 = &buffer[3];
  float* post1 = &buffer[6];
  float* post2 = &buffer[9];

  int px=(row_from-1)%3, cx=row_from%3, nx=0;

	float* p;
	float* pout;
	p = (float*)VIPS_REGION_ADDR( ir, in_rect.left, row_from-1 );  
	convert_row_to_YIQ( p, rbconv_Y[px], rbconv_I[px], rbconv_Q[px], W );
	p = (float*)VIPS_REGION_ADDR( ir, in_rect.left, row_from );  
	convert_row_to_YIQ( p, rbconv_Y[cx], rbconv_I[cx], rbconv_Q[cx], W );

  for (int j = 0; j < W; j++) {
      rbout_I[px][j] = rbconv_I[px][j];
      rbout_Q[px][j] = rbconv_Q[px][j];
  }

  for (int i = row_from; i < row_to; i++) {

      px = (i - 1) % 3;
      cx = i % 3;
      nx = (i + 1) % 3;
		
		p = (float*)VIPS_REGION_ADDR( ir, in_rect.left, i+1 );  
		convert_row_to_YIQ( p, rbconv_Y[nx], rbconv_I[nx], rbconv_Q[nx], W );

    pre1[0] = rbconv_I[px][0], pre1[1] = rbconv_I[cx][0], pre1[2] = rbconv_I[nx][0];
     pre2[0] = rbconv_I[px][1], pre2[1] = rbconv_I[cx][1], pre2[2] = rbconv_I[nx][1];

     // fill first element in rbout_I
     rbout_I[cx][0] = rbconv_I[cx][0];

     // median I channel
     for (int j = 1; j < W - 2; j += 2) {
         post1[0] = rbconv_I[px][j + 1], post1[1] = rbconv_I[cx][j + 1], post1[2] = rbconv_I[nx][j + 1];
         const auto middle = middle4of6(pre2[0], pre2[1], pre2[2], post1[0], post1[1], post1[2]);
         rbout_I[cx][j] = median(pre1[0], pre1[1], pre1[2], middle[0], middle[1], middle[2], middle[3]);
         post2[0] = rbconv_I[px][j + 2], post2[1] = rbconv_I[cx][j + 2], post2[2] = rbconv_I[nx][j + 2];
         rbout_I[cx][j + 1] = median(post2[0], post2[1], post2[2], middle[0], middle[1], middle[2], middle[3]);
         std::swap(pre1, post1);
         std::swap(pre2, post2);
     }

     // fill last elements in rbout_I
     rbout_I[cx][W - 1] = rbconv_I[cx][W - 1];
     rbout_I[cx][W - 2] = rbconv_I[cx][W - 2];

     pre1[0] = rbconv_Q[px][0], pre1[1] = rbconv_Q[cx][0], pre1[2] = rbconv_Q[nx][0];
     pre2[0] = rbconv_Q[px][1], pre2[1] = rbconv_Q[cx][1], pre2[2] = rbconv_Q[nx][1];

     // fill first element in rbout_Q
     rbout_Q[cx][0] = rbconv_Q[cx][0];

     // median Q channel
     for (int j = 1; j < W - 2; j += 2) {
         post1[0] = rbconv_Q[px][j + 1], post1[1] = rbconv_Q[cx][j + 1], post1[2] = rbconv_Q[nx][j + 1];
         const auto middle = middle4of6(pre2[0], pre2[1], pre2[2], post1[0], post1[1], post1[2]);
         rbout_Q[cx][j] = median(pre1[0], pre1[1], pre1[2], middle[0], middle[1], middle[2], middle[3]);
         post2[0] = rbconv_Q[px][j + 2], post2[1] = rbconv_Q[cx][j + 2], post2[2] = rbconv_Q[nx][j + 2];
         rbout_Q[cx][j + 1] = median(post2[0], post2[1], post2[2], middle[0], middle[1], middle[2], middle[3]);
         std::swap(pre1, post1);
         std::swap(pre2, post2);
     }

     // fill last elements in rbout_Q
     rbout_Q[cx][W - 1] = rbconv_Q[cx][W - 1];
     rbout_Q[cx][W - 2] = rbconv_Q[cx][W - 2];
		
		// blur i-1th row
		if (i>(row_from+1)) {
			for (int j=2; j<W-2; j++) {
				row_I[j] = (rbout_I[px][j - 1] + rbout_I[px][j] + rbout_I[px][j + 1] + rbout_I[cx][j - 1] + rbout_I[cx][j] + rbout_I[cx][j + 1] + rbout_I[nx][j - 1] + rbout_I[nx][j] + rbout_I[nx][j + 1]) * onebynine;
				row_Q[j] = (rbout_Q[px][j - 1] + rbout_Q[px][j] + rbout_Q[px][j + 1] + rbout_Q[cx][j - 1] + rbout_Q[cx][j] + rbout_Q[cx][j + 1] + rbout_Q[nx][j - 1] + rbout_Q[nx][j] + rbout_Q[nx][j + 1]) * onebynine;
			}
			row_I[0] = rbout_I[px][0];
			row_Q[0] = rbout_Q[px][0];
			row_I[W-1] = rbout_I[px][W-1];
			row_Q[W-1] = rbout_Q[px][W-1];
			pout = (float*)VIPS_REGION_ADDR( oreg, out_rect.left, i-1 );  
			convert_row_to_RGB (pout, rbconv_Y[px]+pad2, row_I+pad2, row_Q+pad2, out_rect.width);
		}
	}

	// blur last 3 row and finalize H-1th row
	/*
	for (int j=1; j<W-1; j++) {
		row_I[j] = (rbout_I[px][j-1]+rbout_I[px][j]+rbout_I[px][j+1]+rbout_I[cx][j-1]+rbout_I[cx][j]+rbout_I[cx][j+1]+rbconv_I[nx][j-1]+rbconv_I[nx][j]+rbconv_I[nx][j+1])/9;
		row_Q[j] = (rbout_Q[px][j-1]+rbout_Q[px][j]+rbout_Q[px][j+1]+rbout_Q[cx][j-1]+rbout_Q[cx][j]+rbout_Q[cx][j+1]+rbconv_Q[nx][j-1]+rbconv_Q[nx][j]+rbconv_Q[nx][j+1])/9;
	}
	row_I[0] = rbout_I[cx][0];
	row_Q[0] = rbout_Q[cx][0];
	row_I[W-1] = rbout_I[cx][W-1];
	row_Q[W-1] = rbout_Q[cx][W-1];
	convert_row_to_RGB (im->r(row_to-1), im->g(row_to-1), im->b(row_to-1), rbconv_Y[cx], row_I, row_Q, W);
	*/
}

