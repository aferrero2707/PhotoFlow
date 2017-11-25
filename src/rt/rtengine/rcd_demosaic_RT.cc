/***
 *
 *   Bayer CFA Demosaicing using Integrated Gaussian Vector on Color Differences
 *   Revision 1.0 - 2013/02/28
 *
 *   Copyright (c) 2007-2013 Luis Sanz Rodriguez
 *   Using High Order Interpolation technique by Jim S, Jimmy Li, and Sharmil Randhawa
 *
 *   Contact info: luis.sanz.rodriguez@gmail.com
 *
 *   This code is distributed under a GNU General Public License, version 3.
 *   Visit <http://www.gnu.org/licenses/> for more information.
 *
 ***/
// Adapted to RT by Jacques Desmis 3/2013
// SSE version by Ingo Weyrich 5/2013
// Adapted to PhotoFlow 08/2014
#include <string.h>
#include <vector>
#include <array>

//#include "rtengine.h"
#include "rawimagesource.hh"
#include "rt_math.h"
//#include "../rtgui/multilangmgr.h"
//#include "procparams.h"
#include "sleef.c"
#include "opthelper.h"


//#undef CLIP
//#define CLIP(x) x




/**
 * RATIO CORRECTED DEMOSAICING
 * Luis Sanz Rodriguez (luis.sanz.rodriguez(at)gmail(dot)com)
 *
 * Release 2.2 @ 171117
 *
 * Original code from https://github.com/LuisSR/RCD-Demosaicing
 * Adapted from RawTherapee: https://raw.githubusercontent.com/Beep6581/RawTherapee/rcd-demosaic/rtengine/demosaic_algos.cc
 * Licensed under the GNU GPL version 3
 */

#undef _OPENMP


//namespace rtengine {
namespace rtengine {

void RawImageSource::rcd_demosaic_RT(int winx, int winy, int winw, int winh,
    int tilex, int tiley, int tilew, int tileh)
{
  // Image dimensions
  const int iwidth=winw, iheight=winh;
  // Input tile dimensions
  const int width=tilew, height=tileh;

  std::vector<float> cfa(width * height);
  std::vector< std::array<float, 3> > rgb(width * height);

  int border = 8;
  bool verbose = false;

#ifdef _OPENMP
    #pragma omp parallel for
#endif
  for (int row = 0, row2 = tiley; row < height; row++, row2++) {
    for (int col = 0, col2 = tilex, indx = row * width + col;
        col < width; col++, col2++, indx++) {
      int c = FC(row, col);
      cfa[indx] = rgb[indx][c] = CLIP(rawData[row2][col2]) / 65535.f;
    }
  }

  if (plistener) {
    plistener->setProgress(0.05);
  }
  // ------------------------------------------------------------------------
  /* RT
    int row, col, indx, c;
   */
  int w1 = width, w2 = 2 * width, w3 = 3 * width, w4 = 4 * width;

  //Tolerance to avoid dividing by zero
  static const float eps = 1e-5, epssq = 1e-10;


  /* RT
        //Gradients
        float N_Grad, E_Grad, W_Grad, S_Grad, NW_Grad, NE_Grad, SW_Grad, SE_Grad;

        //Pixel estimation
        float N_Est, E_Est, W_Est, S_Est, NW_Est, NE_Est, SW_Est, SE_Est, V_Est, H_Est, P_Est, Q_Est;

        //Directional discrimination
        //float V_Stat, H_Stat, P_Stat, Q_Stat;
        float VH_Central_Value, VH_Neighbour_Value, PQ_Central_Value, PQ_Neighbour_Value;
   */
  float ( *VH_Dir ), ( *VH_Disc ), ( *PQ_Dir ), ( *PQ_Disc );

  //Low pass filter
  float ( *lpf );


  /**
   * STEP 1: Find cardinal and diagonal interpolation directions
   */

  int x0 = 221, y0 = 509;

  VH_Dir = ( float ( * ) ) calloc( width * height, sizeof *VH_Dir ); //merror ( VH_Dir, "rcd_demosaicing_171117()" );
  PQ_Dir = ( float ( * ) ) calloc( width * height, sizeof *PQ_Dir ); //merror ( PQ_Dir, "rcd_demosaicing_171117()" );

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int row = 4; row < height - 4; row++ ) {
    for (int col = 4, indx = row * width + col; col < width - 4; col++, indx++ ) {

      //Calculate h/v local discrimination
      float V_Stat = 0.f - 18.0f  *  cfa[indx] * cfa[indx - w1] - 18.0f * cfa[indx] * cfa[indx + w1] - 36.0f * cfa[indx] * cfa[indx - w2] - 36.0f * cfa[indx] * cfa[indx + w2] + 18.0f * cfa[indx] * cfa[indx - w3] + 18.0f * cfa[indx] * cfa[indx + w3] - 2.0f * cfa[indx] * cfa[indx - w4] - 2.0f * cfa[indx] * cfa[indx + w4] + 38.0f * cfa[indx] * cfa[indx] - 70.0f * cfa[indx - w1] * cfa[indx + w1] - 12.0f * cfa[indx - w1] * cfa[indx - w2] + 24.0f * cfa[indx - w1] * cfa[indx + w2] - 38.0f * cfa[indx - w1] * cfa[indx - w3] + 16.0f * cfa[indx - w1] * cfa[indx + w3] + 12.0f * cfa[indx - w1] * cfa[indx - w4] - 6.0f * cfa[indx - w1] * cfa[indx + w4] + 46.0f * cfa[indx - w1] * cfa[indx - w1] + 24.0f * cfa[indx + w1] * cfa[indx - w2] - 12.0f * cfa[indx + w1] * cfa[indx + w2] + 16.0f * cfa[indx + w1] * cfa[indx - w3] - 38.0f * cfa[indx + w1] * cfa[indx + w3] - 6.0f * cfa[indx + w1] * cfa[indx - w4] + 12.0f * cfa[indx + w1] * cfa[indx + w4] + 46.0f * cfa[indx + w1] * cfa[indx + w1] + 14.0f * cfa[indx - w2] * cfa[indx + w2] - 12.0f * cfa[indx - w2] * cfa[indx + w3] - 2.0f * cfa[indx - w2] * cfa[indx - w4] + 2.0f * cfa[indx - w2] * cfa[indx + w4] + 11.0f * cfa[indx - w2] * cfa[indx - w2] - 12.0f * cfa[indx + w2] * cfa[indx - w3] + 2.0f * cfa[indx + w2] * cfa[indx - w4] - 2.0f * cfa[indx + w2] * cfa[indx + w4] + 11.0f * cfa[indx + w2] * cfa[indx + w2] + 2.0f * cfa[indx - w3] * cfa[indx + w3] - 6.0f * cfa[indx - w3] * cfa[indx - w4] + 10.0f * cfa[indx - w3] * cfa[indx - w3] - 6.0f * cfa[indx + w3] * cfa[indx + w4] + 10.0f * cfa[indx + w3] * cfa[indx + w3] + 1.0f * cfa[indx - w4] * cfa[indx - w4] + 1.0f * cfa[indx + w4] * cfa[indx + w4];
      float H_Stat = 0.f - 18.0f  *  cfa[indx] * cfa[indx -  1] - 18.0f * cfa[indx] * cfa[indx +  1] - 36.0f * cfa[indx] * cfa[indx -  2] - 36.0f * cfa[indx] * cfa[indx +  2] + 18.0f * cfa[indx] * cfa[indx -  3] + 18.0f * cfa[indx] * cfa[indx +  3] - 2.0f * cfa[indx] * cfa[indx -  4] - 2.0f * cfa[indx] * cfa[indx +  4] + 38.0f * cfa[indx] * cfa[indx] - 70.0f * cfa[indx -  1] * cfa[indx +  1] - 12.0f * cfa[indx -  1] * cfa[indx -  2] + 24.0f * cfa[indx -  1] * cfa[indx +  2] - 38.0f * cfa[indx -  1] * cfa[indx -  3] + 16.0f * cfa[indx -  1] * cfa[indx +  3] + 12.0f * cfa[indx -  1] * cfa[indx -  4] - 6.0f * cfa[indx -  1] * cfa[indx +  4] + 46.0f * cfa[indx -  1] * cfa[indx -  1] + 24.0f * cfa[indx +  1] * cfa[indx -  2] - 12.0f * cfa[indx +  1] * cfa[indx +  2] + 16.0f * cfa[indx +  1] * cfa[indx -  3] - 38.0f * cfa[indx +  1] * cfa[indx +  3] - 6.0f * cfa[indx +  1] * cfa[indx -  4] + 12.0f * cfa[indx +  1] * cfa[indx +  4] + 46.0f * cfa[indx +  1] * cfa[indx +  1] + 14.0f * cfa[indx -  2] * cfa[indx +  2] - 12.0f * cfa[indx -  2] * cfa[indx +  3] - 2.0f * cfa[indx -  2] * cfa[indx -  4] + 2.0f * cfa[indx -  2] * cfa[indx +  4] + 11.0f * cfa[indx -  2] * cfa[indx -  2] - 12.0f * cfa[indx +  2] * cfa[indx -  3] + 2.0f * cfa[indx +  2] * cfa[indx -  4] - 2.0f * cfa[indx +  2] * cfa[indx +  4] + 11.0f * cfa[indx +  2] * cfa[indx +  2] + 2.0f * cfa[indx -  3] * cfa[indx +  3] - 6.0f * cfa[indx -  3] * cfa[indx -  4] + 10.0f * cfa[indx -  3] * cfa[indx -  3] - 6.0f * cfa[indx +  3] * cfa[indx +  4] + 10.0f * cfa[indx +  3] * cfa[indx +  3] + 1.0f * cfa[indx -  4] * cfa[indx -  4] + 1.0f * cfa[indx +  4] * cfa[indx +  4];
      V_Stat += epssq;
      H_Stat += epssq;

      VH_Dir[indx] = V_Stat / (fabs(V_Stat) + fabs(H_Stat));

      //Calculate m/p local discrimination
      float P_Stat = 0.f - 18.0f * cfa[indx] * cfa[indx - w1 - 1] - 18.0f * cfa[indx] * cfa[indx + w1 + 1] - 36.0f * cfa[indx] * cfa[indx - w2 - 2] - 36.0f * cfa[indx] * cfa[indx + w2 + 2] + 18.0f * cfa[indx] * cfa[indx - w3 - 3] + 18.0f * cfa[indx] * cfa[indx + w3 + 3] - 2.0f * cfa[indx] * cfa[indx - w4 - 4] - 2.0f * cfa[indx] * cfa[indx + w4 + 4] + 38.0f * cfa[indx] * cfa[indx] - 70.0f * cfa[indx - w1 - 1] * cfa[indx + w1 + 1] - 12.0f * cfa[indx - w1 - 1] * cfa[indx - w2 - 2] + 24.0f * cfa[indx - w1 - 1] * cfa[indx + w2 + 2] - 38.0f * cfa[indx - w1 - 1] * cfa[indx - w3 - 3] + 16.0f * cfa[indx - w1 - 1] * cfa[indx + w3 + 3] + 12.0f * cfa[indx - w1 - 1] * cfa[indx - w4 - 4] - 6.0f * cfa[indx - w1 - 1] * cfa[indx + w4 + 4] + 46.0f * cfa[indx - w1 - 1] * cfa[indx - w1 - 1] + 24.0f * cfa[indx + w1 + 1] * cfa[indx - w2 - 2] - 12.0f * cfa[indx + w1 + 1] * cfa[indx + w2 + 2] + 16.0f * cfa[indx + w1 + 1] * cfa[indx - w3 - 3] - 38.0f * cfa[indx + w1 + 1] * cfa[indx + w3 + 3] - 6.0f * cfa[indx + w1 + 1] * cfa[indx - w4 - 4] + 12.0f * cfa[indx + w1 + 1] * cfa[indx + w4 + 4] + 46.0f * cfa[indx + w1 + 1] * cfa[indx + w1 + 1] + 14.0f * cfa[indx - w2 - 2] * cfa[indx + w2 + 2] - 12.0f * cfa[indx - w2 - 2] * cfa[indx + w3 + 3] - 2.0f * cfa[indx - w2 - 2] * cfa[indx - w4 - 4] + 2.0f * cfa[indx - w2 - 2] * cfa[indx + w4 + 4] + 11.0f * cfa[indx - w2 - 2] * cfa[indx - w2 - 2] - 12.0f * cfa[indx + w2 + 2] * cfa[indx - w3 - 3] + 2 * cfa[indx + w2 + 2] * cfa[indx - w4 - 4] - 2.0f * cfa[indx + w2 + 2] * cfa[indx + w4 + 4] + 11.0f * cfa[indx + w2 + 2] * cfa[indx + w2 + 2] + 2.0f * cfa[indx - w3 - 3] * cfa[indx + w3 + 3] - 6.0f * cfa[indx - w3 - 3] * cfa[indx - w4 - 4] + 10.0f * cfa[indx - w3 - 3] * cfa[indx - w3 - 3] - 6.0f * cfa[indx + w3 + 3] * cfa[indx + w4 + 4] + 10.0f * cfa[indx + w3 + 3] * cfa[indx + w3 + 3] + 1.0f * cfa[indx - w4 - 4] * cfa[indx - w4 - 4] + 1.0f * cfa[indx + w4 + 4] * cfa[indx + w4 + 4];
      float Q_Stat = 0.f - 18.0f * cfa[indx] * cfa[indx + w1 - 1] - 18.0f * cfa[indx] * cfa[indx - w1 + 1] - 36.0f * cfa[indx] * cfa[indx + w2 - 2] - 36.0f * cfa[indx] * cfa[indx - w2 + 2] + 18.0f * cfa[indx] * cfa[indx + w3 - 3] + 18.0f * cfa[indx] * cfa[indx - w3 + 3] - 2.0f * cfa[indx] * cfa[indx + w4 - 4] - 2.0f * cfa[indx] * cfa[indx - w4 + 4] + 38.0f * cfa[indx] * cfa[indx] - 70.0f * cfa[indx + w1 - 1] * cfa[indx - w1 + 1] - 12.0f * cfa[indx + w1 - 1] * cfa[indx + w2 - 2] + 24.0f * cfa[indx + w1 - 1] * cfa[indx - w2 + 2] - 38.0f * cfa[indx + w1 - 1] * cfa[indx + w3 - 3] + 16.0f * cfa[indx + w1 - 1] * cfa[indx - w3 + 3] + 12.0f * cfa[indx + w1 - 1] * cfa[indx + w4 - 4] - 6.0f * cfa[indx + w1 - 1] * cfa[indx - w4 + 4] + 46.0f * cfa[indx + w1 - 1] * cfa[indx + w1 - 1] + 24.0f * cfa[indx - w1 + 1] * cfa[indx + w2 - 2] - 12.0f * cfa[indx - w1 + 1] * cfa[indx - w2 + 2] + 16.0f * cfa[indx - w1 + 1] * cfa[indx + w3 - 3] - 38.0f * cfa[indx - w1 + 1] * cfa[indx - w3 + 3] - 6.0f * cfa[indx - w1 + 1] * cfa[indx + w4 - 4] + 12.0f * cfa[indx - w1 + 1] * cfa[indx - w4 + 4] + 46.0f * cfa[indx - w1 + 1] * cfa[indx - w1 + 1] + 14.0f * cfa[indx + w2 - 2] * cfa[indx - w2 + 2] - 12.0f * cfa[indx + w2 - 2] * cfa[indx - w3 + 3] - 2.0f * cfa[indx + w2 - 2] * cfa[indx + w4 - 4] + 2.0f * cfa[indx + w2 - 2] * cfa[indx - w4 + 4] + 11.0f * cfa[indx + w2 - 2] * cfa[indx + w2 - 2] - 12.0f * cfa[indx - w2 + 2] * cfa[indx + w3 - 3] + 2 * cfa[indx - w2 + 2] * cfa[indx + w4 - 4] - 2.0f * cfa[indx - w2 + 2] * cfa[indx - w4 + 4] + 11.0f * cfa[indx - w2 + 2] * cfa[indx - w2 + 2] + 2.0f * cfa[indx + w3 - 3] * cfa[indx - w3 + 3] - 6.0f * cfa[indx + w3 - 3] * cfa[indx + w4 - 4] + 10.0f * cfa[indx + w3 - 3] * cfa[indx + w3 - 3] - 6.0f * cfa[indx - w3 + 3] * cfa[indx - w4 + 4] + 10.0f * cfa[indx - w3 + 3] * cfa[indx - w3 + 3] + 1.0f * cfa[indx + w4 - 4] * cfa[indx + w4 - 4] + 1.0f * cfa[indx - w4 + 4] * cfa[indx - w4 + 4];
      P_Stat += epssq;
      Q_Stat += epssq;


      PQ_Dir[indx] = P_Stat / ( fabs(P_Stat) + fabs(Q_Stat) );

      //if(tilex<10 && tiley<10 && row<6 && col<6)
      //  printf("indx=%d  VH_Dir=%f  V_Stat=%e  H_Stat=%e  PQ_Dir=%f\n", indx, VH_Dir[indx], V_Stat, H_Stat, PQ_Dir[indx]);

    }
  }

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.2);
  }
  // -------------------------------------------------------------------------


  VH_Disc = ( float ( * ) ) calloc( width * height, sizeof *VH_Disc ); //merror ( VH_Disc, "rcd_demosaicing_171117()" );
  PQ_Disc = ( float ( * ) ) calloc( width * height, sizeof *PQ_Disc ); //merror ( PQ_Disc, "rcd_demosaicing_171117()" );

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for ( int row = 4; row < height - 4; row++ ) {
    for ( int col = 4, indx = row * width + col; col < width - 4; col++, indx++ ) {

      //Refined h/v local discrimination
      float VH_Central_Value   = VH_Dir[indx];
      float VH_Neighbour_Value = 0.25f * (VH_Dir[indx - w1 - 1] + VH_Dir[indx - w1 + 1] + VH_Dir[indx + w1 - 1] + VH_Dir[indx + w1 + 1]);

      VH_Disc[indx] = ( fabs( 0.5f - VH_Central_Value ) < fabs( 0.5f - VH_Neighbour_Value ) ) ? VH_Neighbour_Value : VH_Central_Value;

      //Refined m/p local discrimination
      float PQ_Central_Value   = PQ_Dir[indx];
      float PQ_Neighbour_Value = 0.25f * (PQ_Dir[indx - w1 - 1] + PQ_Dir[indx - w1 + 1] + PQ_Dir[indx + w1 - 1] + PQ_Dir[indx + w1 + 1]);

      PQ_Disc[indx] = ( fabs( 0.5f - PQ_Central_Value ) < fabs( 0.5f - PQ_Neighbour_Value ) ) ? PQ_Neighbour_Value : PQ_Central_Value;

    }
  }

  free( VH_Dir );
  free( PQ_Dir );

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.4);
  }
  // ------------------------------------------------------------------------

  /**
   * STEP 2: Calculate the low pass filter
   */

  lpf = ( float ( * ) ) calloc( width * height, sizeof *lpf ); //merror ( lpf, "rcd_demosaicing_171117()" );

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for ( int row = 1; row < height - 1; row++ ) {
    for ( int col = 1, indx = row * width + col; col < width - 1; col++, indx++ ) {

      //Low pass filter incorporating red and blue local samples
      lpf[indx] = 0.25f * cfa[indx] + 0.125f * ( cfa[indx - w1] + cfa[indx + w1] + cfa[indx - 1] + cfa[indx + 1] ) + 0.0625f * ( cfa[indx - w1 - 1] + cfa[indx - w1 + 1] + cfa[indx + w1 - 1] + cfa[indx + w1 + 1] );

    }
  }

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.5);
  }
  // ------------------------------------------------------------------------

  /**
   * STEP 3: Populate the green channel
   */
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for ( int row = 4; row < height - 4; row++ ) {
    for ( int col = 4 + ( FC( row, 0 )&1 ), indx = row * width + col; col < width - 4; col += 2, indx += 2 ) {

      //Cardinal gradients
      float N_Grad = eps + fabs( cfa[indx - w1] - cfa[indx + w1] ) + fabs( cfa[indx] - cfa[indx - w2] ) + fabs( cfa[indx - w1] - cfa[indx - w3] ) + fabs( cfa[indx - w2] - cfa[indx - w4] );
      float S_Grad = eps + fabs( cfa[indx + w1] - cfa[indx - w1] ) + fabs( cfa[indx] - cfa[indx + w2] ) + fabs( cfa[indx + w1] - cfa[indx + w3] ) + fabs( cfa[indx + w2] - cfa[indx + w4] );
      float W_Grad = eps + fabs( cfa[indx -  1] - cfa[indx +  1] ) + fabs( cfa[indx] - cfa[indx -  2] ) + fabs( cfa[indx -  1] - cfa[indx -  3] ) + fabs( cfa[indx -  2] - cfa[indx -  4] );
      float E_Grad = eps + fabs( cfa[indx +  1] - cfa[indx -  1] ) + fabs( cfa[indx] - cfa[indx +  2] ) + fabs( cfa[indx +  1] - cfa[indx +  3] ) + fabs( cfa[indx +  2] - cfa[indx +  4] );

      //Cardinal pixel estimations
      float N_Est = cfa[indx - w1] * ( 1.f + ( lpf[indx] - lpf[indx - w2] ) / ( eps + lpf[indx] + lpf[indx - w2] ) );
      float S_Est = cfa[indx + w1] * ( 1.f + ( lpf[indx] - lpf[indx + w2] ) / ( eps + lpf[indx] + lpf[indx + w2] ) );
      float W_Est = cfa[indx -  1] * ( 1.f + ( lpf[indx] - lpf[indx -  2] ) / ( eps + lpf[indx] + lpf[indx -  2] ) );
      float E_Est = cfa[indx +  1] * ( 1.f + ( lpf[indx] - lpf[indx +  2] ) / ( eps + lpf[indx] + lpf[indx +  2] ) );

      //Interpolate G@R & G@B
      float V_Est = ( S_Grad * N_Est + N_Grad * S_Est ) / ( N_Grad + S_Grad );
      float H_Est = ( W_Grad * E_Est + E_Grad * W_Est ) / ( E_Grad + W_Grad );

      rgb[indx][1] = LIM( VH_Disc[indx] * H_Est + ( 1.0f - VH_Disc[indx] ) * V_Est, 0.f, 1.f );

    }
  }

  free( lpf );

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.7);
  }
  // -------------------------------------------------------------------------

  /**
   * STEP 4: Populate the red and blue channel
   */
  for ( int row = 4; row < height - 4; row++ ) {
    for ( int col = 4 + ( FC( row, 0 )&1 ), indx = row * width + col, c = 2 - FC( row, col ); col < width - 4; col += 2, indx += 2 ) {

      //Diagonal gradients
      float NW_Grad = eps + fabs( rgb[indx - w1 - 1][c] - rgb[indx + w1 + 1][c] ) + fabs( rgb[indx - w1 - 1][c] - rgb[indx - w3 - 3][c] ) + fabs( rgb[indx][1] - rgb[indx - w2 - 2][1] );
      float NE_Grad = eps + fabs( rgb[indx - w1 + 1][c] - rgb[indx + w1 - 1][c] ) + fabs( rgb[indx - w1 + 1][c] - rgb[indx - w3 + 3][c] ) + fabs( rgb[indx][1] - rgb[indx - w2 + 2][1] );
      float SW_Grad = eps + fabs( rgb[indx + w1 - 1][c] - rgb[indx - w1 + 1][c] ) + fabs( rgb[indx + w1 - 1][c] - rgb[indx + w3 - 3][c] ) + fabs( rgb[indx][1] - rgb[indx + w2 - 2][1] );
      float SE_Grad = eps + fabs( rgb[indx + w1 + 1][c] - rgb[indx - w1 - 1][c] ) + fabs( rgb[indx + w1 + 1][c] - rgb[indx + w3 + 3][c] ) + fabs( rgb[indx][1] - rgb[indx + w2 + 2][1] );

      //Diagonal colour differences
      float NW_Est = rgb[indx - w1 - 1][c] - rgb[indx - w1 - 1][1];
      float NE_Est = rgb[indx - w1 + 1][c] - rgb[indx - w1 + 1][1];
      float SW_Est = rgb[indx + w1 - 1][c] - rgb[indx + w1 - 1][1];
      float SE_Est = rgb[indx + w1 + 1][c] - rgb[indx + w1 + 1][1];

      //Interpolate R@B and B@R
      float P_Est = ( NW_Grad * SE_Est + SE_Grad * NW_Est ) / ( NW_Grad + SE_Grad );
      float Q_Est = ( NE_Grad * SW_Est + SW_Grad * NE_Est ) / ( NE_Grad + SW_Grad );

      rgb[indx][c] = LIM( rgb[indx][1] + ( 1.0f - PQ_Disc[indx] ) * P_Est + PQ_Disc[indx] * Q_Est, 0.f, 1.f );

    }
  }

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.825);
  }
  // -------------------------------------------------------------------------

  for ( int row = 4; row < height - 4; row++ ) {
    for ( int col = 4 + ( FC( row, 1 )&1 ), indx = row * width + col; col < width - 4; col += 2, indx += 2 ) {

      for ( int c = 0; c <= 2; c += 2 ) {

        //Cardinal gradients
        float N_Grad = eps + fabs( rgb[indx][1] - rgb[indx - w2][1] ) + fabs( rgb[indx - w1][c] - rgb[indx + w1][c] ) + fabs( rgb[indx - w1][c] - rgb[indx - w3][c] );
        float S_Grad = eps + fabs( rgb[indx][1] - rgb[indx + w2][1] ) + fabs( rgb[indx + w1][c] - rgb[indx - w1][c] ) + fabs( rgb[indx + w1][c] - rgb[indx + w3][c] );
        float W_Grad = eps + fabs( rgb[indx][1] - rgb[indx -  2][1] ) + fabs( rgb[indx -  1][c] - rgb[indx +  1][c] ) + fabs( rgb[indx -  1][c] - rgb[indx -  3][c] );
        float E_Grad = eps + fabs( rgb[indx][1] - rgb[indx +  2][1] ) + fabs( rgb[indx +  1][c] - rgb[indx -  1][c] ) + fabs( rgb[indx +  1][c] - rgb[indx +  3][c] );

        //Cardinal colour differences
        float N_Est = rgb[indx - w1][c] - rgb[indx - w1][1];
        float S_Est = rgb[indx + w1][c] - rgb[indx + w1][1];
        float W_Est = rgb[indx -  1][c] - rgb[indx -  1][1];
        float E_Est = rgb[indx +  1][c] - rgb[indx +  1][1];

        //Interpolate R@G and B@G
        float V_Est = ( N_Grad * S_Est + S_Grad * N_Est ) / ( N_Grad + S_Grad );
        float H_Est = ( E_Grad * W_Est + W_Grad * E_Est ) / ( E_Grad + W_Grad );

        rgb[indx][c] = LIM( rgb[indx][1] + ( 1.0f - VH_Disc[indx] ) * V_Est + VH_Disc[indx] * H_Est, 0.f, 1.f );

      }
    }
  }

  free( PQ_Disc );
  free( VH_Disc );

  // RT ---------------------------------------------------------------------
  if (plistener) {
    plistener->setProgress(0.95);
  }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int row = border, row2 = tiley+row; row < height-border; ++row, ++row2) {
    for (int col = border, col2 = tilex+col, idx = row * width + col;
        col < width-border; ++col, ++col2, ++idx) {
      red[row2][col2] = CLIP(rgb[idx][0] * 65535.f);
      green[row2][col2] = CLIP(rgb[idx][1] * 65535.f);
      blue[row2][col2] = CLIP(rgb[idx][2] * 65535.f);
    }
  }

  if (plistener) {
    plistener->setProgress(1);
  }
  // -------------------------------------------------------------------------
}

}
