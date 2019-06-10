/*
 * ctmf.c - Constant-time median filtering
 * Copyright (C) 2006  Simon Perreault
 *
 * Reference: S. Perreault and P. Hébert, "Median Filtering in Constant Time",
 * IEEE Transactions on Image Processing, September 2007.
 *
 * This program has been obtained from http://nomis80.org/ctmf.html. No patent
 * covers this program, although it is subject to the following license:
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact:
 *  Laboratoire de vision et systèmes numériques
 *  Pavillon Adrien-Pouliot
 *  Université Laval
 *  Sainte-Foy, Québec, Canada
 *  G1K 7P4
 *
 *  perreaul@gel.ulaval.ca
 */

/* Standard C includes */
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Type declarations */
#ifdef _MSC_VER
#include <basetsd.h>
typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
#pragma warning( disable: 4799 )
#else
#include <stdint.h>
#endif

/* Intrinsic declarations */
#if defined(__SSE2__) || defined(__MMX__)
#if defined(__SSE2__)
#include <emmintrin.h>
#elif defined(__MMX__)
#include <mmintrin.h>
#endif
#if defined(__GNUC__)
#include <mm_malloc.h>
#elif defined(_MSC_VER)
#include <malloc.h>
#endif
#elif defined(__ALTIVEC__)
#include <altivec.h>
#endif

/* Compiler peculiarities */
#if defined(__GNUC__)
#include <stdint.h>
#define inline __inline__
#define align(x) __attribute__ ((aligned (x)))
#elif defined(_MSC_VER)
#define inline __inline
#define align(x) __declspec(align(x))
#else
#define inline
#define align(x)
#endif

#include "ctmf.h"

#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif


//#define DEBUG 1
#undef __SSE2__
#undef __MMX__
#undef __ALTIVEC__

/**
 * This structure represents a two-tier histogram. The first tier (known as the
 * "coarse" level) is 4 bit wide and the second tier (known as the "fine" level)
 * is 8 bit wide. Pixels inserted in the fine level also get inserted into the
 * coarse bucket designated by the 4 MSBs of the fine bucket value.
 *
 * The structure is aligned on 16 bytes, which is a prerequisite for SIMD
 * instructions. Each bucket is 16 bit wide, which means that extra care must be
 * taken to prevent overflow.
 */
typedef uint32_t ctmf_hist_t;
typedef struct align(16)
{
  ctmf_hist_t coarse[16];
  ctmf_hist_t fine[16][16];
} Histogram;

/**
 * HOP is short for Histogram OPeration. This macro makes an operation \a op on
 * histogram \a h for pixel value \a x. It takes care of handling both levels.
 */
#define HOP(h,x,op) \
    h.coarse[x>>CTMF_BITS2] op; \
    *((ctmf_hist_t*) h.fine + x) op;

#define COP(c,j,x,op) \
    h_coarse[ CTMF_BINS*(n*c+j) + (x>>CTMF_BITS2) ] op; \
    h_fine[ CTMF_BINS * (n*(CTMF_BINS*c+(x>>CTMF_BITS2)) + j) + (x & 0xF) ] op;

/**
 * Adds histograms \a x and \a y and stores the result in \a y. Makes use of
 * SSE2, MMX or Altivec, if available.
 */
#if defined(__SSE2__)
static inline void histogram_add( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(__m128i*) &y[0] = _mm_add_epi16( *(__m128i*) &y[0], *(__m128i*) &x[0] );
  *(__m128i*) &y[8] = _mm_add_epi16( *(__m128i*) &y[8], *(__m128i*) &x[8] );
}
#elif defined(__MMX__)
static inline void histogram_add( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(__m64*) &y[0]  = _mm_add_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
  *(__m64*) &y[4]  = _mm_add_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
  *(__m64*) &y[8]  = _mm_add_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
  *(__m64*) &y[12] = _mm_add_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_add( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(vector unsigned short*) &y[0] = vec_add( *(vector unsigned short*) &y[0], *(vector unsigned short*) &x[0] );
  *(vector unsigned short*) &y[8] = vec_add( *(vector unsigned short*) &y[8], *(vector unsigned short*) &x[8] );
}
#else
static inline void histogram_add( const ctmf_hist_t x[CTMF_BINS], ctmf_hist_t y[CTMF_BINS] )
{
  int i;
  for ( i = 0; i < CTMF_BINS; ++i ) {
    y[i] += x[i];
  }
}
#endif

/**
 * Subtracts histogram \a x from \a y and stores the result in \a y. Makes use
 * of SSE2, MMX or Altivec, if available.
 */
#if defined(__SSE2__)
static inline void histogram_sub( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(__m128i*) &y[0] = _mm_sub_epi16( *(__m128i*) &y[0], *(__m128i*) &x[0] );
  *(__m128i*) &y[8] = _mm_sub_epi16( *(__m128i*) &y[8], *(__m128i*) &x[8] );
}
#elif defined(__MMX__)
static inline void histogram_sub( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(__m64*) &y[0]  = _mm_sub_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
  *(__m64*) &y[4]  = _mm_sub_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
  *(__m64*) &y[8]  = _mm_sub_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
  *(__m64*) &y[12] = _mm_sub_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_sub( const uint16_t x[CTMF_BINS], uint16_t y[CTMF_BINS] )
{
  *(vector unsigned short*) &y[0] = vec_sub( *(vector unsigned short*) &y[0], *(vector unsigned short*) &x[0] );
  *(vector unsigned short*) &y[8] = vec_sub( *(vector unsigned short*) &y[8], *(vector unsigned short*) &x[8] );
}
#else
static inline void histogram_sub( const ctmf_hist_t x[CTMF_BINS], ctmf_hist_t y[CTMF_BINS] )
{
  int i;
  for ( i = 0; i < CTMF_BINS; ++i ) {
    y[i] -= x[i];
  }
}
#endif

static inline void histogram_muladd( const uint16_t a, const ctmf_hist_t x[CTMF_BINS],
    ctmf_hist_t y[CTMF_BINS] )
{
  int i;
  for ( i = 0; i < CTMF_BINS; ++i ) {
    y[i] += a * x[i];
  }
}



#define UPDATE_SEGMENT { \
    if ( luc[c][k] <= j-r ) { \
      memset( &H[c].fine[k], 0, CTMF_BINS * sizeof(ctmf_hist_t) ); \
      for ( luc[c][k] = j-r; luc[c][k] < MIN(j+r+1,n); ++luc[c][k] ) { \
        histogram_add( &h_fine[CTMF_BINS*(n*(CTMF_BINS*c+k)+luc[c][k])], H[c].fine[k] ); \
      } \
      if ( luc[c][k] < j+r+1 ) { \
        histogram_muladd( j+r+1 - n, &h_fine[CTMF_BINS*(n*(CTMF_BINS*c+k)+(n-1))], &H[c].fine[k][0] ); \
        luc[c][k] = j+r+1; \
      } \
    } \
    else { \
      for ( ; luc[c][k] < j+r+1; ++luc[c][k] ) { \
        histogram_sub( &h_fine[CTMF_BINS*(n*(CTMF_BINS*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] ); \
        histogram_add( &h_fine[CTMF_BINS*(n*(CTMF_BINS*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] ); \
      } \
    } \
    }


#define DAMPING_FACTOR 0.0f

static void ctmf_helper(
    const unsigned char* const src, float* const dst,
    const int width, const int height,
    const int src_step, const int dst_step,
    const int r, const int cn,
    const int pad_left, const int pad_right,
    const unsigned char threshold
)
{
  const int m = height, n = width;
  int i, j, k, c;
  const unsigned char *p, *q;

  Histogram H[4];
  ctmf_hist_t *h_coarse, *h_fine, luc[4][CTMF_BINS];

  assert( src );
  assert( dst );
  assert( r >= 0 );
  assert( width >= 2*r+1 );
  assert( height >= 2*r+1 );
  assert( src_step != 0 );
  assert( dst_step != 0 );

  /* SSE2 and MMX need aligned memory, provided by _mm_malloc(). */
#if defined(__SSE2__) || defined(__MMX__)
  h_coarse = (uint16_t*) _mm_malloc(  1 * CTMF_BINS * n * cn * sizeof(uint16_t), CTMF_BINS );
  h_fine   = (uint16_t*) _mm_malloc( CTMF_BINS * CTMF_BINS * n * cn * sizeof(uint16_t), CTMF_BINS );
  memset( h_coarse, 0,  1 * CTMF_BINS * n * cn * sizeof(uint16_t) );
  memset( h_fine,   0, CTMF_BINS * CTMF_BINS * n * cn * sizeof(uint16_t) );
#else
  h_coarse = (ctmf_hist_t*) calloc(  1 * CTMF_BINS * n * cn, sizeof(ctmf_hist_t) );
  h_fine   = (ctmf_hist_t*) calloc( CTMF_BINS * CTMF_BINS * n * cn, sizeof(ctmf_hist_t) );
#endif

  /* First row initialization *//*
    for ( j = 0; j < n; ++j ) {
        for ( c = 0; c < cn; ++c ) {
            COP( c, j, src[cn*j+c], += r+1 );
        }
    }*/
  for ( i = 0; i < 2*r+1; ++i ) {
    for ( j = 0; j < n; ++j ) {
      for ( c = 0; c < cn; ++c ) {
        COP( c, j, src[src_step*i+cn*j+c], ++ );
      }
    }
  }

  for ( i = r; i < m-r; ++i ) {

    if( i>r ) {
      /* Update column histograms for entire row. */
      p = src + src_step * MAX( 0, i-r-1 );
      q = p + cn * n;
      for ( j = 0; p != q; ++j ) {
        for ( c = 0; c < cn; ++c, ++p ) {
          COP( c, j, *p, -- );
        }
      }

      p = src + src_step * MIN( m-1, i+r );
      q = p + cn * n;
      for ( j = 0; p != q; ++j ) {
        for ( c = 0; c < cn; ++c, ++p ) {
          COP( c, j, *p, ++ );
        }
      }
    }

    /* First column initialization */
    memset( H, 0, cn*sizeof(H[0]) );
    memset( luc, 0, cn*sizeof(luc[0]) );
    if ( pad_left ) {
      for ( c = 0; c < cn; ++c ) {
        histogram_muladd( r, &h_coarse[CTMF_BINS*n*c], H[c].coarse );
      }
    }
    for ( j = 0; j < (pad_left ? r : 2*r); ++j ) {
      for ( c = 0; c < cn; ++c ) {
        histogram_add( &h_coarse[CTMF_BINS*(n*c+j)], H[c].coarse );
      }
    }
    for ( c = 0; c < cn; ++c ) {
      for ( k = 0; k < CTMF_BINS; ++k ) {
        histogram_muladd( 2*r+1, &h_fine[CTMF_BINS*n*(CTMF_BINS*c+k)], &H[c].fine[k][0] );
      }
    }

    for ( j = pad_left ? 0 : r; j < (pad_right ? n : n-r); ++j ) {
      for ( c = 0; c < cn; ++c ) {
        uint16_t t = 2*r*r + 2*r;
        float sum = 0;
        uint32_t *segment;
        int b;

        histogram_add( &h_coarse[CTMF_BINS*(n*c + MIN(j+r,n-1))], H[c].coarse );

        /*
        t = 0;
        // Find median at coarse level
        for ( k = 0; k < CTMF_BINS ; ++k ) {
          t += H[c].coarse[k];
        }
        t /= 2;
        for ( k = 0; k < CTMF_BINS ; ++k ) {
          sum += H[c].coarse[k];
          if ( sum > t ) {
            sum -= H[c].coarse[k];
            break;
          }
        }
       assert( k < CTMF_BINS );

        // Update corresponding histogram segment
       UPDATE_SEGMENT;

        float median = 0;
        // Find median in segment
        segment = H[c].fine[k];
        for ( b = 0; b < CTMF_BINS ; ++b ) {
          sum += segment[b];
#ifdef DEBUG
          printf("b=%d  segment[b]=%d  sum=%d  t=%d\n", (int)b, (int)segment[b], (int)sum, (int)t);
#endif
          if ( sum > t ) {
            if( b==0 || b==15 ) {
              median = CTMF_BINS*k + b;
            } else {
              float tot = segment[b-1] + segment[b] + segment[b+1];
              median = (float)(CTMF_BINS*k + b - 1) * segment[b-1];
              median += (float)(CTMF_BINS*k + b) * segment[b];
              median += (float)(CTMF_BINS*k + b + 1) * segment[b+1];
              median /= tot;
            }
#ifdef DEBUG
            printf("b=%d  sum=%d  dst=%f\n", (int)b, (int)sum, dst[dst_step*i+cn*j+c]);
#endif
            break;
          }
        }
        assert( b < CTMF_BINS );
        */

        float median = 0;

        int16_t srcpx = src[src_step*i+cn*j+c];
        int16_t srcmin = srcpx - threshold; if(srcmin<0) srcmin = 0;
        int16_t srcmax = srcpx + threshold; if(srcmax>255) srcmax = 255;

        unsigned char hcmin = srcmin >> CTMF_BITS2;
        unsigned char hcmax = (srcmax >> CTMF_BITS2) + 1;
#ifdef DEBUG
        printf("srcpx=%d  srcmin=%d  srcmax=%d  hcmin=%d  hcmax=%d\n",
            (int)srcpx, (int)srcmin, (int)srcmax, (int)hcmin, (int)hcmax);
#endif


        if( 0 && median > srcmin && median < srcmax ) {
          dst[dst_step*i+cn*j+c] = median;
        } else {

          int16_t x;

          sum = 0;
          uint32_t sub1 = 0, sub2 = 0;
          float medianc = 0;

          /* Update min and max histogram segment */
          k = hcmin;
          UPDATE_SEGMENT;
          k = hcmax - 1;
          if( k > hcmin ) {
            UPDATE_SEGMENT;
          }
#ifdef DEBUG
          for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
            if( k2>=hcmin && k2<hcmax )
              printf("%2d  %d +\n", k2, (int)H[c].coarse[k2]);
            else
              printf("%2d  %d -\n", k2, (int)H[c].coarse[k2]);
          }
#endif

          /* Find threshold */
          float tc = 0;
          for ( k = 0; k < hcmin; ++k ) {
            tc += DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  coarse=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)tc);
#endif
          }
          /* k = hcmin */
          x = (k<<CTMF_BITS2) & CTMF_CMASK;
          segment = H[c].fine[k];
          for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
            if( (srcpx-x-k2) > threshold ) tc += DAMPING_FACTOR * segment[k2];
            else tc += segment[k2];
#ifdef DEBUG
              printf("k=%d  k2=%d  segment=%d  tc=%d\n", (int)k, (int)k2, (int)segment[k2], (int)tc);
#endif
          }
          /* hcmin+1 <= k < hcmax-1 */
          for ( k = hcmin+1; k < hcmax-1; ++k ) {
            tc += H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  coarse=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)tc);
#endif
          }
          /* k = hcmax-1 */
          x = (k<<CTMF_BITS2) & CTMF_CMASK;
          segment = H[c].fine[k];
          for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
            int s;
            if( (x+k2-srcpx) > threshold ) s = DAMPING_FACTOR * segment[k2];
            else s = segment[k2];
            tc += s;
#ifdef DEBUG
            printf("k=%d  k2=%d  segment=%d  s=%d  tc=%d\n", (int)k, (int)k2, (int)segment[k2], s, (int)tc);
#endif
          }
          /* k >= hcmax */
          for ( k = hcmax; k < CTMF_BINS ; ++k ) {
            tc += DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
            printf("k=%d  coarse=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)tc);
#endif
          }
          tc /= 2; //tc += 1;
#ifdef DEBUG
              printf("==> tc=%d\n", (int)tc);
#endif


          /* Find median at coarse level */
          int found = 0;
          for ( k = 0; k < hcmin; ++k ) {
            sum += DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  coarse=%d  sum=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)sum, (int)tc);
#endif
            if ( sum > tc ) {
              sum -= DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  sum=%d  tc=%d\n", (int)k, (int)sum, (int)tc);
#endif
              break;
            }
          }

          if( k != hcmin ) {
            UPDATE_SEGMENT;
          }
          x = (k<<CTMF_BITS2) & CTMF_CMASK;
          segment = H[c].fine[k];
          for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
            if( (srcpx-x-k2) > threshold ) sum += DAMPING_FACTOR * segment[k2];
            else sum += segment[k2];
#ifdef DEBUG
            printf("x=%d  c=%d  k=%d  k2=%d  x+k2=%d  srcpx-x-k2=%d  segment[k2]=%d  sum=%d\n",
                (int)x, (int)c, (int)k, (int)k2, (int)(x+k2), (int)(srcpx-x-k2), (int)segment[k2], (int)sum);
#endif
            if ( sum > tc ) {
              if( k2==0 || k2==15 ) {
                medianc = CTMF_BINS*k + k2;
              } else {
                float tot = segment[k2-1] + segment[k2] + segment[k2+1];
                medianc = (float)(CTMF_BINS*k + k2 - 1) * segment[k2-1];
                medianc += (float)(CTMF_BINS*k + k2) * segment[k2];
                medianc += (float)(CTMF_BINS*k + k2 + 1) * segment[k2+1];
                medianc /= tot;
              }
              found = 1;
              break;
            }
          }

          if( found == 0 ) {
            for ( k = hcmin+1; k < hcmax-1; ++k ) {
              sum += H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  coarse=%d  sum=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)sum, (int)tc);
#endif
              if ( sum > tc ) {
                sum -= H[c].coarse[k];
#ifdef DEBUG
                printf("k=%d  sum=%d  tc=%d\n", (int)k, (int)sum, (int)tc);
#endif
                break;
              }
            }

            if( k != (hcmax-1) ) {
              UPDATE_SEGMENT;
            }
            x = (k<<CTMF_BITS2) & CTMF_CMASK;
            segment = H[c].fine[k];
            for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
              if( (x+k2-srcpx) > threshold ) sum += DAMPING_FACTOR * segment[k2];
              else sum += segment[k2];
#ifdef DEBUG
            printf("x=%d  c=%d  k=%d  k2=%d  x+k2=%d  srcpx-x-k2=%d  segment[k2]=%d  sum=%d\n",
                (int)x, (int)c, (int)k, (int)k2, (int)(x+k2), (int)(srcpx-x-k2), (int)segment[k2], (int)sum);
#endif
              if ( sum > tc ) {
                if( k2==0 || k2==15 ) {
                  medianc = CTMF_BINS*k + k2;
                } else {
                  float tot = segment[k2-1] + segment[k2] + segment[k2+1];
                  medianc = (float)(CTMF_BINS*k + k2 - 1) * segment[k2-1];
                  medianc += (float)(CTMF_BINS*k + k2) * segment[k2];
                  medianc += (float)(CTMF_BINS*k + k2 + 1) * segment[k2+1];
                  medianc /= tot;
                }
                found = 1;
                break;
              }
            }
          }

          if( found == 0 ) {
            for ( k = hcmax; k < CTMF_BINS; ++k ) {
              sum += DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
              printf("k=%d  coarse=%d  sum=%d  tc=%d\n", (int)k, (int)H[c].coarse[k], (int)sum, (int)tc);
#endif
              if ( sum > tc ) {
                sum -= DAMPING_FACTOR * H[c].coarse[k];
#ifdef DEBUG
                printf("k=%d  sum=%d  tc=%d\n", (int)k, (int)sum, (int)tc);
#endif
                break;
              }
            }

            if( k < CTMF_BINS ) {
              UPDATE_SEGMENT;
              x = (k<<CTMF_BITS2) & CTMF_CMASK;
              segment = H[c].fine[k];
              for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
                sum += DAMPING_FACTOR * segment[k2];
#ifdef DEBUG
            printf("x=%d  c=%d  k=%d  k2=%d  x+k2=%d  srcpx-x-k2=%d  segment[k2]=%d  sum=%d\n",
                (int)x, (int)c, (int)k, (int)k2, (int)(x+k2), (int)(srcpx-x-k2), (int)segment[k2], (int)sum);
#endif
                if ( sum > tc ) {
                  if( k2==0 || k2==15 ) {
                    medianc = CTMF_BINS*k + k2;
                  } else {
                    float tot = segment[k2-1] + segment[k2] + segment[k2+1];
                    medianc = (float)(CTMF_BINS*k + k2 - 1) * segment[k2-1];
                    medianc += (float)(CTMF_BINS*k + k2) * segment[k2];
                    medianc += (float)(CTMF_BINS*k + k2 + 1) * segment[k2+1];
                    medianc /= tot;
                  }
                  found = 1;
                  break;
                }
              }
            } else {
              medianc = CTMF_MAX;
            }
          }

          //#ifdef DEBUG
          if(k>15){
            printf("sub1=%d  sub2=%d  sum=%d\n", (int)sub1, (int)sub2, (int)sum);
            for ( int k2 = 0; k2 < CTMF_BINS ; ++k2 ) {
              if( k2>=hcmin && k2<hcmax )
                printf("%2d  %d +\n", k2, (int)H[c].coarse[k2]);
              else
                printf("%2d  %d -\n", k2, (int)H[c].coarse[k2]);
            }
            printf("i=%d  j=%d  srcpx=%d  thr=%d  min=%d  max=%d  hcmin=%d  hcmax=%d  t=%d  tc=%d  sum=%d  k=%d\n",
                (int)i, (int)j, (int)srcpx, (int)threshold, (int)srcmin, (int)srcmax,  (int)hcmin, (int)hcmax,
                (int)t, (int)tc, (int)sum, (int)k);
          }
          //#endif
          assert( k < CTMF_BINS );

          dst[dst_step*i+cn*j+c] = medianc; // * delta + srcpx * (1.0f - delta);
#ifdef DEBUG
          printf("srcpx=%f  medianc=%f  out=%f\n",
              (float)srcpx, medianc, dst[dst_step*i+cn*j+c]);
#endif
        }

        histogram_sub( &h_coarse[CTMF_BINS*(n*c+MAX(j-r,0))], H[c].coarse );
      }
    }
  }

#if defined(__SSE2__) || defined(__MMX__)
  _mm_empty();
  _mm_free(h_coarse);
  _mm_free(h_fine);
#else
  free(h_coarse);
  free(h_fine);
#endif
}

/**
 * \brief Constant-time median filtering
 *
 * This function does a median filtering of an 8-bit image. The source image is
 * processed as if it was padded with zeros. The median kernel is square with
 * odd dimensions. Images of arbitrary size may be processed.
 *
 * To process multi-channel images, you must call this function multiple times,
 * changing the source and destination adresses and steps such that each channel
 * is processed as an independent single-channel image.
 *
 * Processing images of arbitrary bit depth is not supported.
 *
 * The computing time is O(1) per pixel, independent of the radius of the
 * filter. The algorithm's initialization is O(r*width), but it is negligible.
 * Memory usage is simple: it will be as big as the cache size, or smaller if
 * the image is small. For efficiency, the histograms' bins are 16-bit wide.
 * This may become too small and lead to overflow as \a r increases.
 *
 * \param src           Source image data.
 * \param dst           Destination image data. Must be preallocated.
 * \param width         Image width, in pixels.
 * \param height        Image height, in pixels.
 * \param src_step      Distance between adjacent pixels on the same column in
 *                      the source image, in bytes.
 * \param dst_step      Distance between adjacent pixels on the same column in
 *                      the destination image, in bytes.
 * \param r             Median filter radius. The kernel will be a 2*r+1 by
 *                      2*r+1 square.
 * \param cn            Number of channels. For example, a grayscale image would
 *                      have cn=1 while an RGB image would have cn=3.
 * \param memsize       Maximum amount of memory to use, in bytes. Set this to
 *                      the size of the L2 cache, then vary it slightly and
 *                      measure the processing time to find the optimal value.
 *                      For example, a 512 kB L2 cache would have
 *                      memsize=512*1024 initially.
 */
void ctmf(
    const unsigned char* const src, float* const dst,
    const int width, const int height,
    const int src_step, const int dst_step,
    const int r, const int cn, const unsigned char threshold,
    const long unsigned int memsize
)
{
  /*
   * Processing the image in vertical stripes is an optimization made
   * necessary by the limited size of the CPU cache. Each histogram is 544
   * bytes big and therefore I can fit a limited number of them in the cache.
   * That number may sometimes be smaller than the image width, which would be
   * the number of histograms I would need without stripes.
   *
   * I need to keep histograms in the cache so that they are available
   * quickly when processing a new row. Each row needs access to the previous
   * row's histograms. If there are too many histograms to fit in the cache,
   * thrashing to RAM happens.
   *
   * To solve this problem, I figure out the maximum number of histograms
   * that can fit in cache. From this is determined the number of stripes in
   * an image. The formulas below make the stripes all the same size and use
   * as few stripes as possible.
   *
   * Note that each stripe causes an overlap on the neighboring stripes, as
   * when mowing the lawn. That overlap is proportional to r. When the overlap
   * is a significant size in comparison with the stripe size, then we are not
   * O(1) anymore, but O(r). In fact, we have been O(r) all along, but the
   * initialization term was neglected, as it has been (and rightly so) in B.
   * Weiss, "Fast Median and Bilateral Filtering", SIGGRAPH, 2006. Processing
   * by stripes only makes that initialization term bigger.
   *
   * Also, note that the leftmost and rightmost stripes don't need overlap.
   * A flag is passed to ctmf_helper() so that it treats these cases as if the
   * image was zero-padded.
   */
  int stripes = (int) ceil( (double) (width - 2*r) / (memsize / sizeof(Histogram) - 2*r) );
  int stripe_size = (int) ceil( (double) ( width + stripes*2*r - 2*r ) / stripes );

  int i;

  for ( i = 0; i < width; i += stripe_size - 2*r ) {
    int stripe = stripe_size;
    /* Make sure that the filter kernel fits into one stripe. */
    if ( i + stripe_size - 2*r >= width || width - (i + stripe_size - 2*r) < 2*r+1 ) {
      stripe = width - i;
    }

    ctmf_helper( src + cn*i, dst + cn*i, stripe, height, src_step, dst_step, r, cn,
        /*i == 0*/0, /*stripe == width - i*/0, threshold );

    if ( stripe == width - i ) {
      break;
    }
  }
}
