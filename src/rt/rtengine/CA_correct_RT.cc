////////////////////////////////////////////////////////////////
//
//      Chromatic Aberration Auto-correction
//
//      Original code from RawTherapee, adapted to PhotoFlow by A. Ferrero
//
//      copyright (c) 2008-2010  Emil Martinec <ejmartin@uchicago.edu>
//
//
// code dated: November 26, 2010
//
//  CA_correct_RT.cc is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#include <string.h>

//#include "rtengine.h"
#include "rawimagesource.hh"
#include "rt_math.h"
//#include "../rtgui/multilangmgr.h"
//#include "procparams.h"
#include "sleef.c"
#include "opthelper.h"

//#undef __SSE2__

//namespace rtengine {
namespace rtengine {

SSEFUNCTION void RawImageSource::CA_correct_RT(int winx, int winy, int winw, int winh,
    int tilex, int tiley, int tilew, int tileh, bool autoCA, float cared, float cablue)
{
// multithreaded and vectorized by Ingo Weyrich
    constexpr int ts = 192;
    constexpr int tsh = ts / 2;
    //shifts to location of vertical and diagonal neighbors
    constexpr int v1 = ts, v2 = 2 * ts, v3 = 3 * ts, v4 = 4 * ts; //, p1=-ts+1, p2=-2*ts+2, p3=-3*ts+3, m1=ts+1, m2=2*ts+2, m3=3*ts+3;

    // Test for RGB cfa
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            if(FC(i, j) == 3) {
                printf("CA correction supports only RGB Colour filter arrays\n");
                return;
            }

    volatile double progress = 0.0;

    if(plistener) {
        plistener->setProgress (progress);
    }

  //bool autoCA = true;
  //float cared = 0, cablue = 0;
    // local variables
    //const int width = W + (W & 1), height = H;
    const int width = winw + (winw & 1), height = winh;
    //temporary array to store simple interpolation of G
    //float *Gtmp = (float (*)) malloc ((height * width) / 2 * sizeof * Gtmp);
    int top, left;

    // temporary array to avoid race conflicts, only every second pixel needs to be saved here
    //float *RawDataTmp = (float*) malloc( (height * width) * sizeof(float) / 2);
    float *RawDataTmp = (float*) malloc( tileh * tilew * sizeof(float) / 2);

    float blockave[2][2] = {{0, 0}, {0, 0}}, blocksqave[2][2] = {{0, 0}, {0, 0}}, blockdenom[2][2] = {{0, 0}, {0, 0}}, blockvar[2][2];

    // Because we can't break parallel processing, we need a switch do handle the errors
    bool processpasstwo = true;

    constexpr int border = 8;
    constexpr int border2 = 16;

    const int vz1 = (height + border2) % (ts - border2) == 0 ? 1 : 0;
    const int hz1 = (width + border2) % (ts - border2) == 0 ? 1 : 0;
    const int vblsz = ceil((float)(height + border2) / (ts - border2) + 2 + vz1);
    const int hblsz = ceil((float)(width + border2) / (ts - border2) + 2 + hz1);

    //block CA shift values and weight assigned to block
    float* const blockwt = static_cast<float*>(calloc(vblsz * hblsz * (2 * 2 + 1), sizeof(float)));
    float (*blockshifts)[2][2] = (float (*)[2][2])(blockwt + vblsz * hblsz);

    //double fitparams[2][2][16];

    //order of 2d polynomial fit (polyord), and numpar=polyord^2
    int polyord = 4, numpar = 16;

    constexpr float eps = 1e-5f, eps2 = 1e-10f; //tolerance to avoid dividing by zero

    //#pragma omp parallel
    {
        int progresscounter = 0;

        //direction of the CA shift in a tile
        int GRBdir[2][3];

        int shifthfloor[3], shiftvfloor[3], shifthceil[3], shiftvceil[3];

        //local quadratic fit to shift data within a tile
        float   coeff[2][3][2];
        //measured CA shift parameters for a tile
        float   CAshift[2][2];
        //polynomial fit coefficients
        //residual CA shift amount within a plaquette
        float   shifthfrac[3], shiftvfrac[3];
        //per thread data for evaluation of block CA shift variance
        float   blockavethr[2][2] = {{0, 0}, {0, 0}}, blocksqavethr[2][2] = {{0, 0}, {0, 0}}, blockdenomthr[2][2] = {{0, 0}, {0, 0}};

        // assign working space
        constexpr int buffersize = sizeof(float) * ts * ts + 8 * sizeof(float) * ts * tsh + 8 * 64 + 63;
        char *buffer = (char *) malloc(buffersize);
        char *data = (char*)( ( uintptr_t(buffer) + uintptr_t(63)) / 64 * 64);

        // shift the beginning of all arrays but the first by 64 bytes to avoid cache miss conflicts on CPUs which have <=4-way associative L1-Cache

        //rgb data in a tile
        float* rgb[3];
        rgb[0]         = (float (*)) data;
        rgb[1]         = (float (*)) (data + sizeof(float) * ts * tsh + 1 * 64);
        rgb[2]         = (float (*)) (data + sizeof(float) * (ts * ts + ts * tsh) + 2 * 64);

        //high pass filter for R/B in vertical direction
        float *rbhpfh  = (float (*)) (data + 2 * sizeof(float) * ts * ts + 3 * 64);
        //high pass filter for R/B in horizontal direction
        float *rbhpfv  = (float (*)) (data + 2 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 4 * 64);
        //low pass filter for R/B in horizontal direction
        float *rblpfh  = (float (*)) (data + 3 * sizeof(float) * ts * ts + 5 * 64);
        //low pass filter for R/B in vertical direction
        float *rblpfv  = (float (*)) (data + 3 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 6 * 64);
        //low pass filter for colour differences in horizontal direction
        float *grblpfh = (float (*)) (data + 4 * sizeof(float) * ts * ts + 7 * 64);
        //low pass filter for colour differences in vertical direction
        float *grblpfv = (float (*)) (data + 4 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 8 * 64);
        float *grbdiff = rbhpfh; // there is no overlap in buffer usage => share
        //green interpolated to optical sample points for R/B
        float *gshift  = rbhpfv; // there is no overlap in buffer usage => share


    // Main algorithm: Tile loop
//#pragma omp for schedule(dynamic) collapse(2) nowait

        int nprocessedtiles = 0;

    //for (top = -border; top < height; top += TS - border2)
    //  for (left = -border; left < width; left += TS - border2) {
    for (top = tiley-border; top < tiley+tileh; top += ts - border2)
      for (left = tilex-border; left < tilex+tilew; left += ts - border2) {
        nprocessedtiles += 1;
        memset(buffer, 0, buffersize);
        float lblockshifts[2][2];
        const int vblock = ((top + border) / (ts - border2)) + 1;
        const int hblock = ((left + border) / (ts - border2)) + 1;
        //const int bottom = min(top + ts, height + border);
        //const int right  = min(left + ts, width + border);
        const int bottom = min(top + ts, tiley + tileh + border);
        const int right  = min(left + ts, tilex + tilew + border);
        const int rr1 = bottom - top;
        const int cc1 = right - left;

        if( false && tiley < 10 )
          std::cout<<"CA_correct_RT: left="<<left<<" top="<<top<<"    tilex="<<tilex<<" tiley="<<tiley<<"    tilew="<<tilew<<" tileh="<<tileh
          <<"    border="<<border
          <<"    rr1="<<rr1<<" cc1="<<cc1<<std::endl
          <<"    vblock="<<vblock<<" hblock="<<hblock<<std::endl;

        const int rrmin = top < 0 ? border : 0;
        const int rrmax = bottom > height ? height - top : rr1;
        const int ccmin = left < 0 ? border : 0;
        const int ccmax = right > width ? width - left : cc1;

        // rgb from input CFA data
        // rgb values should be floating point number between 0 and 1
        // after white balance multipliers are applied

#ifdef __SSE2__
        vfloat c65535v = F2V(65535.f);
        vmask gmask = _mm_set_epi32(0, 0xffffffff, 0, 0xffffffff);
#endif
        for (int rr = rrmin; rr < rrmax; rr++) {
          int row = rr + top;
          int cc = ccmin;
          int col = cc + left;
          int indx = row * width + col;
          int indx1 = rr * ts + cc;
#ifdef __SSE2___
          int c = FC(rr, cc);
          if(c & 1) {
            rgb[1][indx1] = rawData[row][col] / 65535.f;
            indx++;
            indx1++;
            cc++;
            col++;
            c = FC(rr, cc);
          }
          for (; cc < ccmax - 7; cc += 8, col += 8, indx += 8, indx1 += 8) {
            vfloat val1v = LVFU(rawData[row][col]) / c65535v;
            vfloat val2v = LVFU(rawData[row][col + 4]) / c65535v;
            STVFU(rgb[c][indx1 >> 1], _mm_shuffle_ps(val1v, val2v, _MM_SHUFFLE(2, 0, 2, 0)));
            vfloat gtmpv = LVFU(Gtmp[indx >> 1]);
            STVFU(rgb[1][indx1], vself(gmask, PERMUTEPS(gtmpv, _MM_SHUFFLE(1, 1, 0, 0)), val1v));
            STVFU(rgb[1][indx1 + 4], vself(gmask, PERMUTEPS(gtmpv, _MM_SHUFFLE(3, 3, 2, 2)), val2v));
          }
#endif
          for (; cc < ccmax; cc++, col++, indx++, indx1++) {
            int c = FC(rr, cc);
            rgb[c][indx1 >> ((c & 1) ^ 1)] = rawData[row][col] / 65535.f;
            //??? rgb[c][indx1] = (rawData[row][col]) / 65535.0f;
            //if(row<16 && col<16) printf("rawData[%d][%d](%d) = %f %f\n",row,col,c,(rawData[row][col]));
            /*
            if ((c & 1) == 0) {
              rgb[1][indx1] = Gtmp[indx >> 1];
            }
            */
          }
        }
        /*
        // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        //fill borders
        if (rrmin > 0) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = ccmin; cc < ccmax; cc++) {
              int c = FC(rr, cc);
              rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = rgb[c][((border2 - rr) * ts + cc) >> ((c & 1) ^ 1)];
              rgb[1][rr * ts + cc] = rgb[1][(border2 - rr) * ts + cc];
            }
        }

        if (rrmax < rr1) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = ccmin; cc < ccmax; cc++) {
              int c = FC(rr, cc);
              rgb[c][((rrmax + rr)*ts + cc) >> ((c & 1) ^ 1)] = (rawData[(height - rr - 2)][left + cc]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][(rrmax + rr)*ts + cc] = Gtmp[((height - rr - 2) * width + left + cc) >> 1];
              }
            }
        }

        if (ccmin > 0) {
          for (int rr = rrmin; rr < rrmax; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = rgb[c][(rr * ts + border2 - cc) >> ((c & 1) ^ 1)];
              rgb[1][rr * ts + cc] = rgb[1][rr * ts + border2 - cc];
            }
        }

        if (ccmax < cc1) {
          for (int rr = rrmin; rr < rrmax; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][(rr * ts + ccmax + cc) >> ((c & 1) ^ 1)] = (rawData[(top + rr)][(width - cc - 2)]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][rr * ts + ccmax + cc] = Gtmp[((top + rr) * width + (width - cc - 2)) >> 1];
              }
            }
        }

        //also, fill the image corners
        if (rrmin > 0 && ccmin > 0) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = (rawData[border2 - rr][border2 - cc]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][rr * ts + cc] = Gtmp[((border2 - rr) * width + border2 - cc) >> 1];
              }
            }
        }

        if (rrmax < rr1 && ccmax < cc1) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][((rrmax + rr)*ts + ccmax + cc) >> ((c & 1) ^ 1)] = (rawData[(height - rr - 2)][(width - cc - 2)]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][(rrmax + rr)*ts + ccmax + cc] = Gtmp[((height - rr - 2) * width + (width - cc - 2)) >> 1];
              }
            }
        }

        if (rrmin > 0 && ccmax < cc1) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][(rr * ts + ccmax + cc) >> ((c & 1) ^ 1)] = (rawData[(border2 - rr)][(width - cc - 2)]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][rr * ts + ccmax + cc] = Gtmp[((border2 - rr) * width + (width - cc - 2)) >> 1];
              }
            }
        }

        if (rrmax < rr1 && ccmin > 0) {
          for (int rr = 0; rr < border; rr++)
            for (int cc = 0; cc < border; cc++) {
              int c = FC(rr, cc);
              rgb[c][((rrmax + rr)*ts + cc) >> ((c & 1) ^ 1)] = (rawData[(height - rr - 2)][(border2 - cc)]) / 65535.f;
              if ((c & 1) == 0) {
                rgb[1][(rrmax + rr)*ts + cc] = Gtmp[((height - rr - 2) * width + (border2 - cc)) >> 1];
              }
            }
        }

        //end of border fill
        */

#ifdef __SSE2__
        vfloat onev = F2V(1.f);
        vfloat epsv = F2V(eps);
#endif
        for (int rr = 3; rr < rr1 - 3; rr++) {
          int row = rr + top;
          int cc = 3 + (FC(rr,3) & 1);
          int indx = rr * ts + cc;
          int c = FC(rr,cc);
#ifdef __SSE2__
          for (; cc < cc1 - 9; cc+=8, indx+=8) {
            //compute directional weights using image gradients
            vfloat rgb1mv1v = LC2VFU(rgb[1][indx - v1]);
            vfloat rgb1pv1v = LC2VFU(rgb[1][indx + v1]);
            vfloat rgbcv = LVFU(rgb[c][indx >> 1]);
            vfloat temp1v = epsv + vabsf(rgb1mv1v - rgb1pv1v);
            vfloat wtuv = onev / SQRV(temp1v + vabsf(rgbcv - LVFU(rgb[c][(indx - v2) >> 1])) + vabsf(rgb1mv1v - LC2VFU(rgb[1][indx - v3])));
            vfloat wtdv = onev / SQRV(temp1v + vabsf(rgbcv - LVFU(rgb[c][(indx + v2) >> 1])) + vabsf(rgb1pv1v - LC2VFU(rgb[1][indx + v3])));
            vfloat rgb1m1v = LC2VFU(rgb[1][indx - 1]);
            vfloat rgb1p1v = LC2VFU(rgb[1][indx + 1]);
            vfloat temp2v = epsv + vabsf(rgb1m1v - rgb1p1v);
            vfloat wtlv = onev / SQRV(temp2v + vabsf(rgbcv - LVFU(rgb[c][(indx - 2) >> 1])) + vabsf(rgb1m1v - LC2VFU(rgb[1][indx - 3])));
            vfloat wtrv = onev / SQRV(temp2v + vabsf(rgbcv - LVFU(rgb[c][(indx + 2) >> 1])) + vabsf(rgb1p1v - LC2VFU(rgb[1][indx + 3])));

            //store in rgb array the interpolated G value at R/B grid points using directional weighted average
            STC2VFU(rgb[1][indx], (wtuv * rgb1mv1v + wtdv * rgb1pv1v + wtlv * rgb1m1v + wtrv * rgb1p1v) / (wtuv + wtdv + wtlv + wtrv));
          }

#endif
          for (; cc < cc1 - 3; cc+=2, indx+=2) {
            //compute directional weights using image gradients
            float wtu = 1.f / SQR(eps + fabsf(rgb[1][indx + v1] - rgb[1][indx - v1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx - v2) >> 1]) + fabsf(rgb[1][indx - v1] - rgb[1][indx - v3]));
            float wtd = 1.f / SQR(eps + fabsf(rgb[1][indx - v1] - rgb[1][indx + v1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx + v2) >> 1]) + fabsf(rgb[1][indx + v1] - rgb[1][indx + v3]));
            float wtl = 1.f / SQR(eps + fabsf(rgb[1][indx + 1] - rgb[1][indx - 1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx - 2) >> 1]) + fabsf(rgb[1][indx - 1] - rgb[1][indx - 3]));
            float wtr = 1.f / SQR(eps + fabsf(rgb[1][indx - 1] - rgb[1][indx + 1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx + 2) >> 1]) + fabsf(rgb[1][indx + 1] - rgb[1][indx + 3]));

            //store in rgb array the interpolated G value at R/B grid points using directional weighted average
            rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
          }
          /*
          if (row > -1 && row < height) {
            int offset = (FC(row,max(left + 3, 0)) & 1);
            int col = max(left + 3, 0) + offset;
            int indx = rr * ts + 3 - (left < 0 ? (left+3) : 0) + offset;
#ifdef __SSE2__
            for(; col < min(cc1 + left - 3, width) - 7; col+=8, indx+=8) {
              STVFU(Gtmp[(row * width + col) >> 1], LC2VFU(rgb[1][indx]));
            }
#endif
            for(; col < min(cc1 + left - 3, width); col+=2, indx+=2) {
              Gtmp[(row * width + col) >> 1] = rgb[1][indx];
            }
          }
          */
        }
        // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        if (!autoCA) {
          //manual CA correction; use red/blue slider values to set CA shift parameters
          for (int rr = 3; rr < rr1 - 3; rr++)
            for (int cc = 3, indx = rr * ts + cc; cc < cc1 - 3; cc++, indx++) {
              int c = FC(rr, cc);

              if (c != 1) {
                //compute directional weights using image gradients
                float wtu = 1.f / SQR(eps + fabsf(rgb[1][(rr + 1) * ts + cc] - rgb[1][(rr - 1) * ts + cc]) + fabsf(rgb[c][(rr * ts + cc) >> 1] - rgb[c][((rr - 2) * ts + cc) >> 1]) + fabsf(rgb[1][(rr - 1) * ts + cc] - rgb[1][(rr - 3) * ts + cc]));
                float wtd = 1.f / SQR(eps + fabsf(rgb[1][(rr - 1) * ts + cc] - rgb[1][(rr + 1) * ts + cc]) + fabsf(rgb[c][(rr * ts + cc) >> 1] - rgb[c][((rr + 2) * ts + cc) >> 1]) + fabsf(rgb[1][(rr + 1) * ts + cc] - rgb[1][(rr + 3) * ts + cc]));
                float wtl = 1.f / SQR(eps + fabsf(rgb[1][rr * ts + cc + 1] - rgb[1][rr * ts + cc - 1]) + fabsf(rgb[c][(rr * ts + cc) >> 1] - rgb[c][(rr * ts + cc - 2) >> 1]) + fabsf(rgb[1][rr * ts + cc - 1] - rgb[1][rr * ts + cc - 3]));
                float wtr = 1.f / SQR(eps + fabsf(rgb[1][rr * ts + cc - 1] - rgb[1][rr * ts + cc + 1]) + fabsf(rgb[c][(rr * ts + cc) >> 1] - rgb[c][(rr * ts + cc + 2) >> 1]) + fabsf(rgb[1][rr * ts + cc + 1] - rgb[1][rr * ts + cc + 3]));

                //store in rgb array the interpolated G value at R/B grid points using directional weighted average
                rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
              }

            }

          float hfrac = -((float)(hblock - 0.5) / (hblsz - 2) - 0.5);
          float vfrac = -((float)(vblock - 0.5) / (vblsz - 2) - 0.5) * height / width;
          lblockshifts[0][0] = 2 * vfrac * cared;
          lblockshifts[0][1] = 2 * hfrac * cared;
          lblockshifts[1][0] = 2 * vfrac * cablue;
          lblockshifts[1][1] = 2 * hfrac * cablue;
        } else {
          //CA auto correction; use CA diagnostic pass to set shift parameters
          lblockshifts[0][0] = lblockshifts[0][1] = 0;
          lblockshifts[1][0] = lblockshifts[1][1] = 0;
          double powVblock = 1.0;
          for (int i = 0; i < polyord; i++) {
            double powHblock = powVblock;
            for (int j = 0; j < polyord; j++) {
              //printf("i= %d j= %d polycoeff= %f \n",i,j,fitparams[0][0][polyord*i+j]);
              lblockshifts[0][0] += powHblock * fitparams[0][0][polyord * i + j];
              lblockshifts[0][1] += powHblock * fitparams[0][1][polyord * i + j];
              lblockshifts[1][0] += powHblock * fitparams[1][0][polyord * i + j];
              lblockshifts[1][1] += powHblock * fitparams[1][1][polyord * i + j];
              powHblock *= hblock;
            }
            powVblock *= vblock;
          }
          constexpr float bslim = 3.99; //max allowed CA shift
          lblockshifts[0][0] = LIM(lblockshifts[0][0], -bslim, bslim);
          lblockshifts[0][1] = LIM(lblockshifts[0][1], -bslim, bslim);
          lblockshifts[1][0] = LIM(lblockshifts[1][0], -bslim, bslim);
          lblockshifts[1][1] = LIM(lblockshifts[1][1], -bslim, bslim);
        }//end of setting CA shift parameters

        //printf("vblock= %d hblock= %d vshift= %f hshift= %f \n",vblock,hblock,blockshifts[(vblock)*hblsz+hblock][0][0],blockshifts[(vblock)*hblsz+hblock][0][1]);

        for (int c = 0; c < 3; c += 2) {

          //some parameters for the bilinear interpolation
          shiftvfloor[c] = floor((float)lblockshifts[c>>1][0]);
          shiftvceil[c] = ceil((float)lblockshifts[c>>1][0]);
          shiftvfrac[c] = lblockshifts[c>>1][0] - shiftvfloor[c];

          shifthfloor[c] = floor((float)lblockshifts[c>>1][1]);
          shifthceil[c] = ceil((float)lblockshifts[c>>1][1]);
          shifthfrac[c] = lblockshifts[c>>1][1] - shifthfloor[c];

          GRBdir[0][c] = lblockshifts[c>>1][0] > 0 ? 2 : -2;
          GRBdir[1][c] = lblockshifts[c>>1][1] > 0 ? 2 : -2;

        }


        for (int rr = 4; rr < rr1 - 4; rr++) {
          int cc = 4 + (FC(rr, 2) & 1);
          int c = FC(rr, cc);
          int indx = (rr * ts + cc) >> 1;
          int indxfc = (rr + shiftvfloor[c]) * ts + cc + shifthceil[c];
          int indxff = (rr + shiftvfloor[c]) * ts + cc + shifthfloor[c];
          int indxcc = (rr + shiftvceil[c]) * ts + cc + shifthceil[c];
          int indxcf = (rr + shiftvceil[c]) * ts + cc + shifthfloor[c];
#ifdef __SSE2__
          vfloat shifthfracv = F2V(shifthfrac[c]);
          vfloat shiftvfracv = F2V(shiftvfrac[c]);
          for (; cc < cc1 - 10; cc += 8, indxfc += 8, indxff += 8, indxcc += 8, indxcf += 8, indx += 4) {
            //perform CA correction using colour ratios or colour differences
            vfloat Ginthfloorv = vintpf(shifthfracv, LC2VFU(rgb[1][indxfc]), LC2VFU(rgb[1][indxff]));
            vfloat Ginthceilv = vintpf(shifthfracv, LC2VFU(rgb[1][indxcc]), LC2VFU(rgb[1][indxcf]));
            //Gint is bilinear interpolation of G at CA shift point
            vfloat Gintv = vintpf(shiftvfracv, Ginthceilv, Ginthfloorv);

            //determine R/B at grid points using colour differences at shift point plus interpolated G value at grid point
            //but first we need to interpolate G-R/G-B to grid points...
            STVFU(grbdiff[indx], Gintv - LVFU(rgb[c][indx]));
            STVFU(gshift[indx], Gintv);
          }

#endif
          for (; cc < cc1 - 4; cc += 2, indxfc += 2, indxff += 2, indxcc += 2, indxcf += 2, ++indx) {
            //perform CA correction using colour ratios or colour differences
            float Ginthfloor = intp(shifthfrac[c], rgb[1][indxfc], rgb[1][indxff]);
            float Ginthceil = intp(shifthfrac[c], rgb[1][indxcc], rgb[1][indxcf]);
            //Gint is bilinear interpolation of G at CA shift point
            float Gint = intp(shiftvfrac[c], Ginthceil, Ginthfloor);

            //determine R/B at grid points using colour differences at shift point plus interpolated G value at grid point
            //but first we need to interpolate G-R/G-B to grid points...
            grbdiff[indx] = Gint - rgb[c][indx];
            gshift[indx] = Gint;
          }
        }

        shifthfrac[0] /= 2.f;
        shifthfrac[2] /= 2.f;
        shiftvfrac[0] /= 2.f;
        shiftvfrac[2] /= 2.f;

#ifdef __SSE2__
        vfloat zd25v = F2V(0.25f);
        //vfloat onev = F2V(1.f);
        vfloat zd5v = F2V(0.5f);
        //vfloat epsv = F2V(eps);
#endif
        for (int rr = 8; rr < rr1 - 8; rr++) {
          int cc = 8 + (FC(rr, 2) & 1);
          int c = FC(rr, cc);
          int GRBdir0 = GRBdir[0][c];
          int GRBdir1 = GRBdir[1][c];
#ifdef __SSE2__
          vfloat shifthfracc = F2V(shifthfrac[c]);
          vfloat shiftvfracc = F2V(shiftvfrac[c]);
          for (int indx = rr * ts + cc; cc < cc1 - 14; cc += 8, indx += 8) {
            //interpolate colour difference from optical R/B locations to grid locations
            vfloat grbdiffinthfloor = vintpf(shifthfracc, LVFU(grbdiff[(indx - GRBdir1) >> 1]), LVFU(grbdiff[indx >> 1]));
            vfloat grbdiffinthceil = vintpf(shifthfracc, LVFU(grbdiff[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1]), LVFU(grbdiff[((rr - GRBdir0) * ts + cc) >> 1]));
            //grbdiffint is bilinear interpolation of G-R/G-B at grid point
            vfloat grbdiffint = vintpf(shiftvfracc, grbdiffinthceil, grbdiffinthfloor);

            //now determine R/B at grid points using interpolated colour differences and interpolated G value at grid point
            vfloat cinv = LVFU(rgb[c][indx >> 1]);
            vfloat rinv = LC2VFU(rgb[1][indx]);
            vfloat RBint = rinv - grbdiffint;
            vmask cmask = vmaskf_ge(vabsf(RBint - cinv), zd25v * (RBint + cinv));
            if(_mm_movemask_ps((vfloat)cmask)) {
              // if for any of the 4 pixels the condition is true, do the math for all 4 pixels and mask the unused out at the end
              //gradient weights using difference from G at CA shift points and G at grid points
              vfloat p0 = onev / (epsv + vabsf(rinv - LVFU(gshift[indx >> 1])));
              vfloat p1 = onev / (epsv + vabsf(rinv - LVFU(gshift[(indx - GRBdir1) >> 1])));
              vfloat p2 = onev / (epsv + vabsf(rinv - LVFU(gshift[((rr - GRBdir0) * ts + cc) >> 1])));
              vfloat p3 = onev / (epsv + vabsf(rinv - LVFU(gshift[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1])));

              grbdiffint = vself(cmask, (p0 * LVFU(grbdiff[indx >> 1]) + p1 * LVFU(grbdiff[(indx - GRBdir1) >> 1]) +
                  p2 * LVFU(grbdiff[((rr - GRBdir0) * ts + cc) >> 1]) + p3 * LVFU(grbdiff[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1])) / (p0 + p1 + p2 + p3), grbdiffint);

            }
            vfloat grbdiffold = rinv - cinv;
            RBint = rinv - grbdiffint;
            RBint = vself(vmaskf_gt(vabsf(grbdiffold), vabsf(grbdiffint)), RBint, cinv);
            RBint = vself(vmaskf_lt(grbdiffold * grbdiffint, ZEROV), rinv - zd5v * (grbdiffold + grbdiffint), RBint);
            STVFU(rgb[c][indx >> 1], RBint);
          }
#endif
          for (int c = FC(rr, cc), indx = rr * ts + cc; cc < cc1 - 8; cc += 2, indx += 2) {
            float grbdiffold = rgb[1][indx] - rgb[c][indx >> 1];

            //interpolate colour difference from optical R/B locations to grid locations
            float grbdiffinthfloor = intp(shifthfrac[c], grbdiff[(indx - GRBdir1) >> 1], grbdiff[indx >> 1]);
            float grbdiffinthceil = intp(shifthfrac[c], grbdiff[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1], grbdiff[((rr - GRBdir0) * ts + cc) >> 1]);
            //grbdiffint is bilinear interpolation of G-R/G-B at grid point
            float grbdiffint = intp(shiftvfrac[c], grbdiffinthceil, grbdiffinthfloor);

            //now determine R/B at grid points using interpolated colour differences and interpolated G value at grid point
            float RBint = rgb[1][indx] - grbdiffint;

            if (fabsf(RBint - rgb[c][indx >> 1]) < 0.25f * (RBint + rgb[c][indx >> 1])) {
              if (fabsf(grbdiffold) > fabsf(grbdiffint) ) {
                rgb[c][indx >> 1] = RBint;
              }
            } else {

              //gradient weights using difference from G at CA shift points and G at grid points
              float p0 = 1.f / (eps + fabsf(rgb[1][indx] - gshift[indx >> 1]));
              float p1 = 1.f / (eps + fabsf(rgb[1][indx] - gshift[(indx - GRBdir1) >> 1]));
              float p2 = 1.f / (eps + fabsf(rgb[1][indx] - gshift[((rr - GRBdir0) * ts + cc) >> 1]));
              float p3 = 1.f / (eps + fabsf(rgb[1][indx] - gshift[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1]));

              grbdiffint = (p0 * grbdiff[indx >> 1] + p1 * grbdiff[(indx - GRBdir1) >> 1] +
                  p2 * grbdiff[((rr - GRBdir0) * ts + cc) >> 1] + p3 * grbdiff[((rr - GRBdir0) * ts + cc - GRBdir1) >> 1]) / (p0 + p1 + p2 + p3) ;

              //now determine R/B at grid points using interpolated colour differences and interpolated G value at grid point
              if (fabsf(grbdiffold) > fabsf(grbdiffint) ) {
                rgb[c][indx >> 1] = rgb[1][indx] - grbdiffint;
              }
            }

            //if colour difference interpolation overshot the correction, just desaturate
            if (grbdiffold * grbdiffint < 0) {
              rgb[c][indx >> 1] = rgb[1][indx] - 0.5f * (grbdiffold + grbdiffint);
            }
          }
        }

        // copy CA corrected results to temporary image matrix
        for (int rr = border; rr < rr1 - border; rr++) {
          //c = FC(rr + top, left + border + FC(rr + top, 2) & 1);
          int c = FC(rr, left + FC(rr, 2) & 1);
          int row = rr + top;
          int cc = border + (FC(rr, 2) & 1);
          int indx = ((row-tiley) * tilew + cc + left - tilex) >> 1;
          int indx_max = ((row-tiley) * tilew + cc1 - border + left - tilex) >> 1;
          int indx1 = (rr * ts + cc) >> 1;
#ifdef __SSE2___
          for (; indx < (row * width + cc1 - border - 7 + left) >> 1; indx+=4, indx1 += 4) {
            STVFU(RawDataTmp[indx], c65535v * LVFU(rgb[c][indx1]));
          }
#endif
          for (; indx < indx_max /*(row * width + cc1 - border + left) >> 1*/; indx++, indx1++) {
            //printf("c=%d indx1=%d indx=%d\n", c, indx1, indx);
            RawDataTmp[indx] =
                65535.f * rgb[c][indx1];
          }
          /*
          //for (row = rr + top, cc = border + (FC(rr, 2) & 1), indx = (row * width + cc + left) >> 1; cc < cc1 - border; cc += 2, indx++) {
          for (row = rr + top, cc = border + (FC(rr, 2) & 1), indx = ((row-tiley) * tilew + cc + left - tilex) >> 1; cc < cc1 - border; cc += 2, indx++) {
            col = cc + left;
            if( indx >= RawDataTmp_sz ) {
              std::cout<<"CA_correct_RT: row="<<row<<"  cc="<<cc<<"  left="<<left<<"  indx="<<indx<<"  border="<<border<<std::endl;
            }
            RawDataTmp[indx] = 65535.0f * rgb[c][(rr) * TS + cc] + 0.5f;
            if(false && top==0&&left==0 && row<16)
              std::cout<<"(1) row="<<row<<" col="<<cc+left<<"  RawDataTmp["<<indx<<"]="<<RawDataTmp[indx]<<std::endl;
            //image[indx][c] = CLIP((int)(65535.0*rgb[(rr)*TS+cc][c] + 0.5));//for dcraw implementation
          }
          */
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

//#pragma omp barrier
    // copy temporary image matrix back to image matrix
//#pragma omp for

    //for(row = 0; row < height; row++)
    //  for(col = 0 + (FC(row, 0) & 1), indx = (row * width + col) >> 1; col < width; col += 2, indx++) {
    for(int row = 0; row < tileh; row++)
      //for(col = 0 + (FC(row, 0) & 1), indx = ((row+tiley) * width + (col+tilex)) >> 1; col < tilew; col += 2, indx++) {
      for(int col = 0 + (FC(row, 0) & 1), indx = (row * tilew + col) >> 1; col < tilew; col += 2, indx++) {
    //for(row = tiley; row < tiley+tileh; row++)
    //  for(col = tilex + (FC(row, 0) & 1), indx = (row * width + col) >> 1; col < tilex+tilew; col += 2, indx++) {
        if(false && tiley==8&&tilex==8 && row<16 && col<16)
          std::cout<<"(2) row="<<row<<" col="<<col<<"  RawDataTmp["<<indx<<"]="<<RawDataTmp[indx]<<std::endl;
        rawData[row+tiley][col+tilex] = RawDataTmp[indx];
      }


    // clean up
    free(buffer);

    //printf("CA_correct_RT: nprocessedtiles=%d\n", nprocessedtiles);
    //printf("CA_correct_RT: tilew=%d tileh=%d\n", tilew, tileh);

  }


  //free(Gtmp);
  //free(buffer1);
  free(RawDataTmp);

#undef TS

}

}
