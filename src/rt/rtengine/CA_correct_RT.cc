////////////////////////////////////////////////////////////////
//
//			AMaZE demosaic algorithm
// (Aliasing Minimization and Zipper Elimination)
//
//	copyright (c) 2008-2010  Emil Martinec <ejmartin@uchicago.edu>
//
// incorporating ideas of Luis Sanz Rodrigues and Paul Lee
//
// code dated: May 27, 2010
//
//	amaze_interpolate_RT.cc is free software: you can redistribute it and/or modify
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

#include <string.h>

//#include "rtengine.h"
#include "rawimagesource.hh"
#include "rt_math.h"
//#include "../rtgui/multilangmgr.h"
//#include "procparams.h"
#include "sleef.c"
#include "opthelper.h"

//namespace rtengine {
namespace rtengine {

SSEFUNCTION void RawImageSource::CA_correct_RT(int winx, int winy, int winw, int winh,
    int tilex, int tiley, int tilew, int tileh, bool autoCA, float cared, float cablue)
{
  // multithreaded by Ingo Weyrich
#define TS 128      // Tile size
#define TSH 64      // Half Tile size
#define PIX_SORT(a,b) { if ((a)>(b)) {temp=(a);(a)=(b);(b)=temp;} }

  // Test for RGB cfa
  for(int i = 0; i < 2; i++)
    for(int j = 0; j < 2; j++)
      if(FC(i, j) == 3) {
        printf("CA correction supports only RGB Color filter arrays\n");
        return;
      }

  volatile double progress = 0.0;

  //if(plistener) {
  //    plistener->setProgress (progress);
  //}

  //bool autoCA = true;
  //float cared = 0, cablue = 0;
  // local variables
  int width = winw, height = winh;
  //temporary array to store simple interpolation of G
  float (*Gtmp);
  Gtmp = (float (*)) calloc ((height) * (width), sizeof * Gtmp);

  // temporary array to avoid race conflicts, only every second pixel needs to be saved here
  float (*RawDataTmp);
  RawDataTmp = (float*) malloc( height * width * sizeof(float) / 2);

  float   blockave[2][3] = {{0, 0, 0}, {0, 0, 0}}, blocksqave[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockdenom[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockvar[2][3];

  // Because we can't break parallel processing, we need a switch do handle the errors
  bool processpasstwo = true;

  //block CA shift values and weight assigned to block
  char        *buffer1;               // vblsz*hblsz*(3*2+1)
  float       (*blockwt);             // vblsz*hblsz
  float       (*blockshifts)[3][2];   // vblsz*hblsz*3*2


  const int border = 8;
  const int border2 = 16;

  int vz1, hz1;

  if((height + border2) % (TS - border2) == 0) {
    vz1 = 1;
  } else {
    vz1 = 0;
  }

  if((width + border2) % (TS - border2) == 0) {
    hz1 = 1;
  } else {
    hz1 = 0;
  }

  int vblsz, hblsz;
  vblsz = ceil((float)(height + border2) / (TS - border2) + 2 + vz1);
  hblsz = ceil((float)(width + border2) / (TS - border2) + 2 + hz1);

  buffer1 = (char *) malloc(vblsz * hblsz * (3 * 2 + 1) * sizeof(float));
  //merror(buffer1,"CA_correct()");
  memset(buffer1, 0, vblsz * hblsz * (3 * 2 + 1)*sizeof(float));
  // block CA shifts
  blockwt     = (float (*))           (buffer1);
  blockshifts = (float (*)[3][2])     (buffer1 + (vblsz * hblsz * sizeof(float)));

  //double fitparams[3][2][16];

  //order of 2d polynomial fit (polyord), and numpar=polyord^2
  int polyord = 4, numpar = 16;

  //#pragma omp parallel shared(Gtmp,width,height,blockave,blocksqave,blockdenom,blockvar,blockwt,blockshifts,fitparams,polyord,numpar)
  {
    int progresscounter = 0;

    int rrmin, rrmax, ccmin, ccmax;
    int top, left, row, col;
    int rr, cc, c, indx, indx1, i, j, k, m, n, dir;
    //number of pixels in a tile contributing to the CA shift diagnostic
    int areawt[2][3];
    //direction of the CA shift in a tile
    int GRBdir[2][3];
    //offset data of the plaquette where the optical R/B data are sampled
    int offset[2][3];
    int shifthfloor[3], shiftvfloor[3], shifthceil[3], shiftvceil[3];
    //number of tiles in the image
    int vblock, hblock;
    //int verbose=1;
    //flag indicating success or failure of polynomial fit
    int res;
    //shifts to location of vertical and diagonal neighbors
    const int v1 = TS, v2 = 2 * TS, v3 = 3 * TS, v4 = 4 * TS; //, p1=-TS+1, p2=-2*TS+2, p3=-3*TS+3, m1=TS+1, m2=2*TS+2, m3=3*TS+3;

    float eps = 1e-5f, eps2 = 1e-10f; //tolerance to avoid dividing by zero

    //adaptive weights for green interpolation
    float   wtu, wtd, wtl, wtr;
    //local quadratic fit to shift data within a tile
    float   coeff[2][3][3];
    //measured CA shift parameters for a tile
    float   CAshift[2][3];
    //polynomial fit coefficients
    //residual CA shift amount within a plaquette
    float   shifthfrac[3], shiftvfrac[3];
    //temporary storage for median filter
    float   temp, p[9];
    //temporary parameters for tile CA evaluation
    float   gdiff, deltgrb;
    //interpolated G at edge of plaquette
    float   Ginthfloor, Ginthceil, Gint, RBint, gradwt;
    //interpolated color difference at edge of plaquette
    float   grbdiffinthfloor, grbdiffinthceil, grbdiffint, grbdiffold;
    //per thread data for evaluation of block CA shift variance
    float   blockavethr[2][3] = {{0, 0, 0}, {0, 0, 0}}, blocksqavethr[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockdenomthr[2][3] = {{0, 0, 0}, {0, 0, 0}}; //, blockvarthr[2][3];

    //low and high pass 1D filters of G in vertical/horizontal directions
    float   glpfh, glpfv;

    //max allowed CA shift
    const float bslim = 3.99;
    //gaussians for low pass filtering of G and R/B
    //static const float gaussg[5] = {0.171582, 0.15839, 0.124594, 0.083518, 0.0477063};//sig=2.5
    //static const float gaussrb[3] = {0.332406, 0.241376, 0.0924212};//sig=1.25

    //block CA shift values and weight assigned to block

    char        *buffer;            // TS*TS*16
    //rgb data in a tile
    float* rgb[3];
    //color differences
    float         (*grbdiff);       // TS*TS*4
    //green interpolated to optical sample points for R/B
    float         (*gshift);        // TS*TS*4
    //high pass filter for R/B in vertical direction
    float         (*rbhpfh);        // TS*TS*4
    //high pass filter for R/B in horizontal direction
    float         (*rbhpfv);        // TS*TS*4
    //low pass filter for R/B in horizontal direction
    float         (*rblpfh);        // TS*TS*4
    //low pass filter for R/B in vertical direction
    float         (*rblpfv);        // TS*TS*4
    //low pass filter for color differences in horizontal direction
    float         (*grblpfh);       // TS*TS*4
    //low pass filter for color differences in vertical direction
    float         (*grblpfv);       // TS*TS*4


    // assign working space; this would not be necessary
    // if the algorithm is part of the larger pre-interpolation processing
    buffer = (char *) malloc(3 * sizeof(float) * TS * TS + 8 * sizeof(float) * TS * TSH + 10 * 64 + 64);
    //merror(buffer,"CA_correct()");
    memset(buffer, 0, 3 * sizeof(float)*TS * TS + 8 * sizeof(float)*TS * TSH + 10 * 64 + 64);

    char    *data;
    data    = buffer;

    //  buffers aligned to size of cacheline
    //  data = (char*)( ( uintptr_t(buffer) + uintptr_t(63)) / 64 * 64);


    // shift the beginning of all arrays but the first by 64 bytes to avoid cache miss conflicts on CPUs which have <=4-way associative L1-Cache
    rgb[0]      = (float (*))       data;
    rgb[1]      = (float (*))       (data + 1 * sizeof(float) * TS * TS + 1 * 64);
    rgb[2]      = (float (*))       (data + 2 * sizeof(float) * TS * TS + 2 * 64);
    grbdiff     = (float (*))       (data + 3 * sizeof(float) * TS * TS + 3 * 64);
    gshift      = (float (*))       (data + 3 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 4 * 64);
    rbhpfh      = (float (*))       (data + 4 * sizeof(float) * TS * TS + 5 * 64);
    rbhpfv      = (float (*))       (data + 4 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 6 * 64);
    rblpfh      = (float (*))       (data + 5 * sizeof(float) * TS * TS + 7 * 64);
    rblpfv      = (float (*))       (data + 5 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 8 * 64);
    grblpfh     = (float (*))       (data + 6 * sizeof(float) * TS * TS + 9 * 64);
    grblpfv     = (float (*))       (data + 6 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 10 * 64);


    // Main algorithm: Tile loop
#pragma omp for schedule(dynamic) collapse(2) nowait

    //for (top = -border; top < height; top += TS - border2)
    //  for (left = -border; left < width; left += TS - border2) {
    for (top = tiley-border; top < tiley+tileh; top += TS - border2)
      for (left = tilex-border; left < tilex+tilew; left += TS - border2) {

        //std::cout<<"CA_correct_RT: top="<<top<<" left="<<left<<std::endl;
        vblock = ((top + border) / (TS - border2)) + 1;
        hblock = ((left + border) / (TS - border2)) + 1;
        //int bottom = min(top + TS, height + border);
        //int right  = min(left + TS, width + border);
        int bottom = min(top + TS, tiley + tileh + border);
        int right  = min(left + TS, tilex + tilew + border);
        int rr1 = bottom - top;
        int cc1 = right - left;

        //t1_init = clock();
        if (top < 0) {
          rrmin = border;
        } else {
          rrmin = 0;
        }

        if (left < 0) {
          ccmin = border;
        } else {
          ccmin = 0;
        }

        if (bottom > height) {
          rrmax = height - top;
        } else {
          rrmax = rr1;
        }

        if (right > width) {
          ccmax = width - left;
        } else {
          ccmax = cc1;
        }

        // rgb from input CFA data
        // rgb values should be floating point number between 0 and 1
        // after white balance multipliers are applied

        for (rr = rrmin; rr < rrmax; rr++)
          for (row = rr + top, cc = ccmin; cc < ccmax; cc++) {
            col = cc + left;
            c = FC(rr, cc);
            indx = row * width + col;
            indx1 = rr * TS + cc;
            //rgb[indx1][c] = image[indx][c]/65535.0f;
            rgb[c][indx1] = (rawData[row][col]) / 65535.0f;
            //rgb[indx1][c] = image[indx][c]/65535.0f;//for dcraw implementation
            //if(row<16 && col<16) printf("rawData[%d][%d](%d) = %f %f\n",row,col,c,(rawData[row][col]));

            //if ((c & 1) == 0) {
            //  rgb[1][indx1] = Gtmp[indx];
            //}
          }

        // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        //fill borders
        if (rrmin > 0) {
          for (rr = 0; rr < border; rr++)
            for (cc = ccmin; cc < ccmax; cc++) {
              c = FC(rr, cc);
              rgb[c][rr * TS + cc] = rgb[c][(border2 - rr) * TS + cc];
              rgb[1][rr * TS + cc] = rgb[1][(border2 - rr) * TS + cc];
            }
        }

        if (rrmax < rr1) {
          for (rr = 0; rr < border; rr++)
            for (cc = ccmin; cc < ccmax; cc++) {
              c = FC(rr, cc);
              rgb[c][(rrmax + rr)*TS + cc] = (rawData[(height - rr - 2)][left + cc]) / 65535.0f;
              //rgb[(rrmax+rr)*TS+cc][c] = (image[(height-rr-2)*width+left+cc][c])/65535.0f;//for dcraw implementation

              rgb[1][(rrmax + rr)*TS + cc] = Gtmp[(height - rr - 2) * width + left + cc];
            }
        }

        if (ccmin > 0) {
          for (rr = rrmin; rr < rrmax; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][rr * TS + cc] = rgb[c][rr * TS + border2 - cc];
              rgb[1][rr * TS + cc] = rgb[1][rr * TS + border2 - cc];
            }
        }

        if (ccmax < cc1) {
          for (rr = rrmin; rr < rrmax; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][rr * TS + ccmax + cc] = (rawData[(top + rr)][(width - cc - 2)]) / 65535.0f;
              //rgb[rr*TS+ccmax+cc][c] = (image[(top+rr)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation

              rgb[1][rr * TS + ccmax + cc] = Gtmp[(top + rr) * width + (width - cc - 2)];
            }
        }

        //also, fill the image corners
        if (rrmin > 0 && ccmin > 0) {
          for (rr = 0; rr < border; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][(rr)*TS + cc] = (rawData[border2 - rr][border2 - cc]) / 65535.0f;
              //rgb[(rr)*TS+cc][c] = (rgb[(border2-rr)*TS+(border2-cc)][c]);//for dcraw implementation

              rgb[1][(rr)*TS + cc] = Gtmp[(border2 - rr) * width + border2 - cc];
            }
        }

        if (rrmax < rr1 && ccmax < cc1) {
          for (rr = 0; rr < border; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][(rrmax + rr)*TS + ccmax + cc] = (rawData[(height - rr - 2)][(width - cc - 2)]) / 65535.0f;
              //rgb[(rrmax+rr)*TS+ccmax+cc][c] = (image[(height-rr-2)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation

              rgb[1][(rrmax + rr)*TS + ccmax + cc] = Gtmp[(height - rr - 2) * width + (width - cc - 2)];
            }
        }

        if (rrmin > 0 && ccmax < cc1) {
          for (rr = 0; rr < border; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][(rr)*TS + ccmax + cc] = (rawData[(border2 - rr)][(width - cc - 2)]) / 65535.0f;
              //rgb[(rr)*TS+ccmax+cc][c] = (image[(border2-rr)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation

              rgb[1][(rr)*TS + ccmax + cc] = Gtmp[(border2 - rr) * width + (width - cc - 2)];
            }
        }

        if (rrmax < rr1 && ccmin > 0) {
          for (rr = 0; rr < border; rr++)
            for (cc = 0; cc < border; cc++) {
              c = FC(rr, cc);
              rgb[c][(rrmax + rr)*TS + cc] = (rawData[(height - rr - 2)][(border2 - cc)]) / 65535.0f;
              //rgb[(rrmax+rr)*TS+cc][c] = (image[(height-rr-2)*width+(border2-cc)][c])/65535.0f;//for dcraw implementation

              rgb[1][(rrmax + rr)*TS + cc] = Gtmp[(height - rr - 2) * width + (border2 - cc)];
            }
        }

        //end of border fill

        for (rr = 3; rr < rr1 - 3; rr++)
          for (row = rr + top, cc = 3, indx = rr * TS + cc; cc < cc1 - 3; cc++, indx++) {
            col = cc + left;
            c = FC(rr, cc);

            if (c != 1) {
              //compute directional weights using image gradients
              wtu = 1.0 / SQR(eps + fabsf(rgb[1][indx + v1] - rgb[1][indx - v1]) + fabsf(rgb[c][indx] - rgb[c][indx - v2]) + fabsf(rgb[1][indx - v1] - rgb[1][indx - v3]));
              wtd = 1.0 / SQR(eps + fabsf(rgb[1][indx - v1] - rgb[1][indx + v1]) + fabsf(rgb[c][indx] - rgb[c][indx + v2]) + fabsf(rgb[1][indx + v1] - rgb[1][indx + v3]));
              wtl = 1.0 / SQR(eps + fabsf(rgb[1][indx + 1] - rgb[1][indx - 1]) + fabsf(rgb[c][indx] - rgb[c][indx - 2]) + fabsf(rgb[1][indx - 1] - rgb[1][indx - 3]));
              wtr = 1.0 / SQR(eps + fabsf(rgb[1][indx - 1] - rgb[1][indx + 1]) + fabsf(rgb[c][indx] - rgb[c][indx + 2]) + fabsf(rgb[1][indx + 1] - rgb[1][indx + 3]));

              //store in rgb array the interpolated G value at R/B grid points using directional weighted average
              rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
            }

            //if (row > -1 && row < height && col > -1 && col < width) {
            //    Gtmp[row * width + col] = rgb[1][indx];
            //}
          }

        // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        if (!autoCA) {
          //manual CA correction; use red/blue slider values to set CA shift parameters
          for (rr = 3; rr < rr1 - 3; rr++)
            for (row = rr + top, cc = 3, indx = rr * TS + cc; cc < cc1 - 3; cc++, indx++) {
              col = cc + left;
              c = FC(rr, cc);

              if (c != 1) {
                //compute directional weights using image gradients
                wtu = 1.0 / SQR(eps + fabsf(rgb[1][(rr + 1) * TS + cc] - rgb[1][(rr - 1) * TS + cc]) + fabsf(rgb[c][(rr) * TS + cc] - rgb[c][(rr - 2) * TS + cc]) + fabsf(rgb[1][(rr - 1) * TS + cc] - rgb[1][(rr - 3) * TS + cc]));
                wtd = 1.0 / SQR(eps + fabsf(rgb[1][(rr - 1) * TS + cc] - rgb[1][(rr + 1) * TS + cc]) + fabsf(rgb[c][(rr) * TS + cc] - rgb[c][(rr + 2) * TS + cc]) + fabsf(rgb[1][(rr + 1) * TS + cc] - rgb[1][(rr + 3) * TS + cc]));
                wtl = 1.0 / SQR(eps + fabsf(rgb[1][(rr) * TS + cc + 1] - rgb[1][(rr) * TS + cc - 1]) + fabsf(rgb[c][(rr) * TS + cc] - rgb[c][(rr) * TS + cc - 2]) + fabsf(rgb[1][(rr) * TS + cc - 1] - rgb[1][(rr) * TS + cc - 3]));
                wtr = 1.0 / SQR(eps + fabsf(rgb[1][(rr) * TS + cc - 1] - rgb[1][(rr) * TS + cc + 1]) + fabsf(rgb[c][(rr) * TS + cc] - rgb[c][(rr) * TS + cc + 2]) + fabsf(rgb[1][(rr) * TS + cc + 1] - rgb[1][(rr) * TS + cc + 3]));

                //store in rgb array the interpolated G value at R/B grid points using directional weighted average
                rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
              }

              if (row > -1 && row < height && col > -1 && col < width) {
                Gtmp[row * width + col] = rgb[1][indx];
              }
            }

          float hfrac = -((float)(hblock - 0.5) / (hblsz - 2) - 0.5);
          float vfrac = -((float)(vblock - 0.5) / (vblsz - 2) - 0.5) * height / width;
          blockshifts[(vblock)*hblsz + hblock][0][0] = 2 * vfrac * cared;
          blockshifts[(vblock)*hblsz + hblock][0][1] = 2 * hfrac * cared;
          blockshifts[(vblock)*hblsz + hblock][2][0] = 2 * vfrac * cablue;
          blockshifts[(vblock)*hblsz + hblock][2][1] = 2 * hfrac * cablue;
        } else {
          //CA auto correction; use CA diagnostic pass to set shift parameters
          blockshifts[(vblock)*hblsz + hblock][0][0] = blockshifts[(vblock) * hblsz + hblock][0][1] = 0;
          blockshifts[(vblock)*hblsz + hblock][2][0] = blockshifts[(vblock) * hblsz + hblock][2][1] = 0;

          for (i = 0; i < polyord; i++)
            for (j = 0; j < polyord; j++) {
              //printf("i= %d j= %d polycoeff= %f \n",i,j,fitparams[0][0][polyord*i+j]);
              blockshifts[(vblock)*hblsz + hblock][0][0] += (float)pow((float)vblock, i) * pow((float)hblock, j) * fitparams[0][0][polyord * i + j];
              blockshifts[(vblock)*hblsz + hblock][0][1] += (float)pow((float)vblock, i) * pow((float)hblock, j) * fitparams[0][1][polyord * i + j];
              blockshifts[(vblock)*hblsz + hblock][2][0] += (float)pow((float)vblock, i) * pow((float)hblock, j) * fitparams[2][0][polyord * i + j];
              blockshifts[(vblock)*hblsz + hblock][2][1] += (float)pow((float)vblock, i) * pow((float)hblock, j) * fitparams[2][1][polyord * i + j];
            }

          blockshifts[(vblock)*hblsz + hblock][0][0] = LIM(blockshifts[(vblock) * hblsz + hblock][0][0], -bslim, bslim);
          blockshifts[(vblock)*hblsz + hblock][0][1] = LIM(blockshifts[(vblock) * hblsz + hblock][0][1], -bslim, bslim);
          blockshifts[(vblock)*hblsz + hblock][2][0] = LIM(blockshifts[(vblock) * hblsz + hblock][2][0], -bslim, bslim);
          blockshifts[(vblock)*hblsz + hblock][2][1] = LIM(blockshifts[(vblock) * hblsz + hblock][2][1], -bslim, bslim);
        }//end of setting CA shift parameters

        //printf("vblock= %d hblock= %d vshift= %f hshift= %f \n",vblock,hblock,blockshifts[(vblock)*hblsz+hblock][0][0],blockshifts[(vblock)*hblsz+hblock][0][1]);

        for (c = 0; c < 3; c += 2) {

          //some parameters for the bilinear interpolation
          shiftvfloor[c] = floor((float)blockshifts[(vblock) * hblsz + hblock][c][0]);
          shiftvceil[c] = ceil((float)blockshifts[(vblock) * hblsz + hblock][c][0]);
          shiftvfrac[c] = blockshifts[(vblock) * hblsz + hblock][c][0] - shiftvfloor[c];

          shifthfloor[c] = floor((float)blockshifts[(vblock) * hblsz + hblock][c][1]);
          shifthceil[c] = ceil((float)blockshifts[(vblock) * hblsz + hblock][c][1]);
          shifthfrac[c] = blockshifts[(vblock) * hblsz + hblock][c][1] - shifthfloor[c];


          if (blockshifts[(vblock)*hblsz + hblock][c][0] > 0) {
            GRBdir[0][c] = 1;
          } else {
            GRBdir[0][c] = -1;
          }

          if (blockshifts[(vblock)*hblsz + hblock][c][1] > 0) {
            GRBdir[1][c] = 1;
          } else {
            GRBdir[1][c] = -1;
          }

        }


        for (rr = 4; rr < rr1 - 4; rr++)
          for (cc = 4 + (FC(rr, 2) & 1), c = FC(rr, cc); cc < cc1 - 4; cc += 2) {
            //perform CA correction using color ratios or color differences

            Ginthfloor = (1 - shifthfrac[c]) * rgb[1][(rr + shiftvfloor[c]) * TS + cc + shifthfloor[c]] + (shifthfrac[c]) * rgb[1][(rr + shiftvfloor[c]) * TS + cc + shifthceil[c]];
            Ginthceil = (1 - shifthfrac[c]) * rgb[1][(rr + shiftvceil[c]) * TS + cc + shifthfloor[c]] + (shifthfrac[c]) * rgb[1][(rr + shiftvceil[c]) * TS + cc + shifthceil[c]];
            //Gint is blinear interpolation of G at CA shift point
            Gint = (1 - shiftvfrac[c]) * Ginthfloor + (shiftvfrac[c]) * Ginthceil;

            //determine R/B at grid points using color differences at shift point plus interpolated G value at grid point
            //but first we need to interpolate G-R/G-B to grid points...
            grbdiff[((rr)*TS + cc) >> 1] = Gint - rgb[c][(rr) * TS + cc];
            gshift[((rr)*TS + cc) >> 1] = Gint;
          }

        for (rr = 8; rr < rr1 - 8; rr++)
          for (cc = 8 + (FC(rr, 2) & 1), c = FC(rr, cc), indx = rr * TS + cc; cc < cc1 - 8; cc += 2, indx += 2) {

            //if (rgb[indx][c]>clip_pt || Gtmp[indx]>clip_pt) continue;

            grbdiffold = rgb[1][indx] - rgb[c][indx];

            //interpolate color difference from optical R/B locations to grid locations
            grbdiffinthfloor = (1.0f - shifthfrac[c] / 2.0f) * grbdiff[indx >> 1] + (shifthfrac[c] / 2.0f) * grbdiff[(indx - 2 * GRBdir[1][c]) >> 1];
            grbdiffinthceil = (1.0f - shifthfrac[c] / 2.0f) * grbdiff[((rr - 2 * GRBdir[0][c]) * TS + cc) >> 1] + (shifthfrac[c] / 2.0f) * grbdiff[((rr - 2 * GRBdir[0][c]) * TS + cc - 2 * GRBdir[1][c]) >> 1];
            //grbdiffint is bilinear interpolation of G-R/G-B at grid point
            grbdiffint = (1.0f - shiftvfrac[c] / 2.0f) * grbdiffinthfloor + (shiftvfrac[c] / 2.0f) * grbdiffinthceil;

            //now determine R/B at grid points using interpolated color differences and interpolated G value at grid point
            RBint = rgb[1][indx] - grbdiffint;

            if (fabsf(RBint - rgb[c][indx]) < 0.25f * (RBint + rgb[c][indx])) {
              if (fabsf(grbdiffold) > fabsf(grbdiffint) ) {
                rgb[c][indx] = RBint;
              }
            } else {

              //gradient weights using difference from G at CA shift points and G at grid points
              p[0] = 1.0f / (eps + fabsf(rgb[1][indx] - gshift[indx >> 1]));
              p[1] = 1.0f / (eps + fabsf(rgb[1][indx] - gshift[(indx - 2 * GRBdir[1][c]) >> 1]));
              p[2] = 1.0f / (eps + fabsf(rgb[1][indx] - gshift[((rr - 2 * GRBdir[0][c]) * TS + cc) >> 1]));
              p[3] = 1.0f / (eps + fabsf(rgb[1][indx] - gshift[((rr - 2 * GRBdir[0][c]) * TS + cc - 2 * GRBdir[1][c]) >> 1]));

              grbdiffint = (p[0] * grbdiff[indx >> 1] + p[1] * grbdiff[(indx - 2 * GRBdir[1][c]) >> 1] +
                  p[2] * grbdiff[((rr - 2 * GRBdir[0][c]) * TS + cc) >> 1] + p[3] * grbdiff[((rr - 2 * GRBdir[0][c]) * TS + cc - 2 * GRBdir[1][c]) >> 1]) / (p[0] + p[1] + p[2] + p[3]);

              //now determine R/B at grid points using interpolated color differences and interpolated G value at grid point
              if (fabsf(grbdiffold) > fabsf(grbdiffint) ) {
                rgb[c][indx] = rgb[1][indx] - grbdiffint;
              }
            }

            //if color difference interpolation overshot the correction, just desaturate
            if (grbdiffold * grbdiffint < 0) {
              rgb[c][indx] = rgb[1][indx] - 0.5f * (grbdiffold + grbdiffint);
            }
          }

        // copy CA corrected results to temporary image matrix
        for (rr = border; rr < rr1 - border; rr++) {
          //c = FC(rr + top, left + border + FC(rr + top, 2) & 1);
          c = FC(rr, left + FC(rr, 2) & 1);

          for (row = rr + top, cc = border + (FC(rr, 2) & 1), indx = (row * width + cc + left) >> 1; cc < cc1 - border; cc += 2, indx++) {
            col = cc + left;
            RawDataTmp[indx] = 65535.0f * rgb[c][(rr) * TS + cc] + 0.5f;
            //if(top==0&&left==0 && row<16 && col<16) std::cout<<"(1) row="<<row<<" col="<<cc+left<<"  RawDataTmp["<<indx<<"]="<<RawDataTmp[indx]<<std::endl;
            //image[indx][c] = CLIP((int)(65535.0*rgb[(rr)*TS+cc][c] + 0.5));//for dcraw implementation
          }
        }
        /*
      if(plistener) {
        progresscounter++;

        if(progresscounter % 8 == 0)
#pragma omp critical
        {
          progress += (double)(8.0 * (TS - border2) * (TS - border2)) / (2 * height * width);

          if (progress > 1.0) {
            progress = 1.0;
          }

          plistener->setProgress(progress);
        }
      }
         */
      }

#pragma omp barrier
    // copy temporary image matrix back to image matrix
#pragma omp for

    //for(row = 0; row < height; row++)
    //  for(col = 0 + (FC(row, 0) & 1), indx = (row * width + col) >> 1; col < width; col += 2, indx++) {
    for(row = 0; row < tileh; row++)
      for(col = 0 + (FC(row, 0) & 1), indx = ((row+tiley) * width + (col+tilex)) >> 1; col < tilew; col += 2, indx++) {
    //for(row = tiley; row < tiley+tileh; row++)
    //  for(col = tilex + (FC(row, 0) & 1), indx = (row * width + col) >> 1; col < tilex+tilew; col += 2, indx++) {
        //if(tiley==8&&tilex==8 && row<16 && col<16) std::cout<<"(2) row="<<row<<" col="<<col<<"  RawDataTmp["<<indx<<"]="<<RawDataTmp[indx]<<std::endl;
        rawData[row+tiley][col+tilex] = RawDataTmp[indx];
      }


    // clean up
    free(buffer);


  }

  free(Gtmp);
  free(buffer1);
  free(RawDataTmp);

#undef TS

}

}
