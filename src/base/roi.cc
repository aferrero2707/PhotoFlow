#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "roi.hh"


#ifdef _WIN32
void pf_free_align(void *mem);
#define pf_free_align_ptr pf_free_align
#else
#define pf_free_align(A) free(A)
#define pf_free_align_ptr free
#endif




static void *pf_alloc_align(size_t alignment, size_t size)
{
#if defined(__FreeBSD_version) && __FreeBSD_version < 700013
  return malloc(size);
#elif defined(_WIN32)
  return _aligned_malloc(size, alignment);
#else
  void *ptr = NULL;
  if(posix_memalign(&ptr, alignment, size)) return NULL;
  return ptr;
#endif
}

#ifdef _WIN32
void pf_free_align(void *mem)
{
  _aligned_free(mem);
}
#endif




//using namespace PF;

PF::rp_roi_t* PF::rp_roi_new(PF::rp_roi_rect_t* rect, int nchannels)
{
  int left = rect->left, top = rect->top, width = rect->width, height = rect->height;
  int ci, ri, ch;
  int alignment = 16;
  int aligned_width = (width/4) * 4 + 4;
  rp_roi_t* roi = (rp_roi_t*)malloc(sizeof(rp_roi_t));
  if(roi == NULL) return roi;

  roi->rect.left = left;
  roi->rect.top = top;
  roi->rect.width = width;
  roi->rect.height = height;
  roi->nchannels = nchannels;
  roi->buf = NULL;
  roi->data = NULL;

  /* Sanity checks */
  if( width <= 0 || height <=0 || nchannels <= 0) {
    rp_roi_free(roi);
    return NULL;
  }

  /* roi->data = (float***)malloc( sizeof(float**) * nchannels ); */
  if( roi->data == NULL ) {
    rp_roi_free(roi);
    return NULL;
  }
  memset(roi->data, 0, sizeof(float**) * nchannels);

  for( ch = 0; ch < nchannels; ch+=1 ) {
    /* roi->data[ch] = (float**)malloc( sizeof(float*) * height ); */
    roi->data[ch] = (float**)pf_alloc_align( alignment, sizeof(float*) * height );
    if( !roi->data[ch] ) {
      rp_roi_free(roi);
      return NULL;
    }
    //if( posix_memalign( (void**)(&(roi->data[ch])), alignment, sizeof(float*) * height ) != 0 ) {
    //  rp_roi_free(roi);
    //  return NULL;
    //}
    memset(roi->data[ch], 0, sizeof(float*) * height);
    roi->data[ch] = roi->data[ch] - top;
  }

  /* We need a buffer to store the pixel values */
  /* roi->buf = (float*)malloc( sizeof(float) * aligned_width * height * nchannels ); */
  roi->buf = (float*)pf_alloc_align( alignment, sizeof(float) * aligned_width * height * nchannels );
  if( !roi->buf ) {
    rp_roi_free(roi);
    return NULL;
  }
  //if( posix_memalign( (void**)(&(roi->buf)), alignment, sizeof(float) * aligned_width * height * nchannels ) != 0 ) {
  //  rp_roi_free(roi);
  //  return NULL;
  //}

  /* init row pointers */
  for( ch = 0; ch < nchannels; ch+=1 ) {
    for( ri = 0; ri < height; ri+=1 ) {
      roi->data[ch][ri+top] = roi->buf + (aligned_width*height*ch - left);
    }
  }

  return roi;
}


PF::rp_roi_t* PF::rp_roi_new_from_data(PF::rp_roi_rect_t* rect, PF::rp_roi_rect_t* rect_in, int nchannels, int rowstride, int interleaved, float* input, ...)
{
  int left = rect->left, top = rect->top, width = rect->width, height = rect->height;
  int left_in = rect_in->left, top_in = rect_in->top;
  int ci, ri, ch;
  int alignment = 16;
  int aligned_width = (width/4) * 4 + 4;
  rp_roi_t* roi = (rp_roi_t*)malloc(sizeof(rp_roi_t));
  if(roi == NULL) return roi;

  roi->rect.left = left;
  roi->rect.top = top;
  roi->rect.width = width;
  roi->rect.height = height;
  roi->nchannels = nchannels;
  roi->buf = NULL;
  roi->data = NULL;

  /* Sanity checks */
  if( width <= 0 || height <=0 || nchannels <= 0 ) {
    rp_roi_free(roi);
    return NULL;
  }
  if( rect->width != rect_in->width || rect->height != rect_in->height ) {
    rp_roi_free(roi);
    return NULL;
  }

  roi->data = (float***)malloc( sizeof(float**) * nchannels );
  if( roi->data == NULL ) {
    rp_roi_free(roi);
    return NULL;
  }
  memset(roi->data, 0, sizeof(float**) * nchannels);

  for( ch = 0; ch < nchannels; ch+=1 ) {
    /* roi->data[ch] = (float**)malloc( sizeof(float*) * height ); */
    roi->data[ch] = (float**)pf_alloc_align( alignment, sizeof(float*) * height );
    if( !roi->data[ch] ) {
      rp_roi_free(roi);
      return NULL;
    }
    //if( posix_memalign( (void**)(&(roi->data[ch])), alignment, sizeof(float*) * height ) != 0 ) {
    //  rp_roi_free(roi);
    //  return NULL;
    //}
    memset(roi->data[ch], 0, sizeof(float*) * height);
    roi->data[ch] = roi->data[ch] - top;
  }

  if(interleaved > 0) {
    /* If the input data is interleaved, we need a buffer to store the non-interleaved pixel values */
    /* roi->buf = (float*)malloc( sizeof(float) * aligned_width * height * nchannels ); */
    roi->buf = (float*)pf_alloc_align( alignment, sizeof(float) * aligned_width * height * nchannels );
    if( !roi->buf ) {
      rp_roi_free(roi);
      return NULL;
    }
    //if( posix_memalign( (void**)(&(roi->buf)), alignment, sizeof(float) * aligned_width * height * nchannels ) != 0 ) {
    //  rp_roi_free(roi);
    //  return NULL;
    //}

    /* init row pointers */
    for( ch = 0; ch < nchannels; ch+=1 ) {
      for( ri = 0; ri < height; ri+=1 ) {
        roi->data[ch][ri+top] = roi->buf + (aligned_width*height*ch + aligned_width*ri - left);
      }
    }

    /* copy the interleaved pixels into the non-interleaved RoI buffer */
    for( ri = 0; ri < height; ri+=1 ) {
      for( ci = 0; ci < width; ci+=1 ) {
        for( ch = 0; ch < nchannels; ch+=1 ) {
          roi->data[ch][ri+top][ci+left] = input[(ri+top_in)*rowstride + (ci+left_in)*nchannels + ch];
          if( false && ri==0 && ci==0 ) {
            printf("RoI: roi->data[%d][%d+%d][%d+%d] = input[(%d+%d)*%d + (%d+%d)*%d + %d] = %f\n",
                ch, ri, top, ci, left, ri, top_in, rowstride, ci, left_in, nchannels, ch,
                roi->data[ch][ri+top][ci+left]);
          }
        }
      }
    }
  } else {
    /* If the input data is non-interleaved, we can directly copy pointers.
       In this case we expect one input buffer for each image channel. */
  }

  return roi;
}


void PF::rp_roi_free(PF::rp_roi_t* roi)
{
  if( !roi ) return;
  int ch;
  int nchannels = roi->nchannels;
  int top = roi->rect.top;

  if( roi->buf ) pf_free_align( roi->buf );

  if( roi->data ) {
    for( ch = 0; ch < nchannels; ch+=1 ) {
      pf_free_align( roi->data[ch] + top );
    }
    free( roi->data );
  }

  free(roi);
}
