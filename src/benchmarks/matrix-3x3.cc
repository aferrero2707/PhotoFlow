#include <string.h>
#include <vips/vips.h>

#ifdef __SSE2__
#include "xmmintrin.h"
#endif //__SSE2__

#define BENCHMARK
#include "../rt/rtengine/StopWatch.h"

float mrgb[3][3];

void* malloc_aligned(size_t size)
{
  void *mem = malloc(size+15+sizeof(void*));
  if (!mem) return mem;
  void *ptr = (void*)( ((uintptr_t) mem + 15 + sizeof(void*)) / 16 * 16 );
  uintptr_t* tptr = (uintptr_t*)ptr;
  tptr -= 1;
  *tptr = (uintptr_t)mem;
  //((void**)ptr)[-1] = mem;
  return ptr;
}

void free_aligned(void* ptr)
{
  if (ptr) free(((void**)ptr)[-1]);
}


void process( float* buf, float* obuf, size_t size, bool in_place, bool interleaved )
{
  size_t chunk_size = 64;
  size_t nchunks = size / chunk_size;

  long minTime = 0;
  long maxTime = 0;

  for(int i = 0; i < 20; i++) {
    //BENCHFUN;
    MyTime startTime;
    MyTime stopTime;
    startTime.set();
    float* ptr = in_place ? obuf : buf;
    //float r, g, b;
    float* optr = obuf;

    if(in_place) memcpy(obuf, buf, sizeof(float)*4*size);

    if(interleaved) {

      // RGBARGBARGBARGBA... channel arrangement

      for( int ci = 0; ci < nchunks; ci++ ) {
        //printf("processing chunk %d\n", ci);
        for( int pi = 0; pi < chunk_size; pi++ ) {
          if(true) {
            float r = ptr[0]; float g = ptr[1]; float b = ptr[2];
            optr[0] = mrgb[0][0] * r + mrgb[0][1] * g + mrgb[0][2] * b;
            optr[1] = mrgb[1][0] * r + mrgb[1][1] * g + mrgb[1][2] * b;
            optr[2] = mrgb[2][0] * r + mrgb[2][1] * g + mrgb[2][2] * b;
          } else {
            optr[0] = mrgb[0][0] * ptr[0] + mrgb[0][1] * ptr[1] + mrgb[0][2] * ptr[2];
            optr[1] = mrgb[1][0] * ptr[0] + mrgb[1][1] * ptr[1] + mrgb[1][2] * ptr[2];
            optr[2] = mrgb[2][0] * ptr[0] + mrgb[2][1] * ptr[1] + mrgb[2][2] * ptr[2];
          }
          ptr += 4; optr += 4;
        }
      }
    } else {

      // RRRR...GGGG...BBBB... channel arrangement

      float* rp = ptr;
      float* gp = &(ptr[size]);
      float* bp = &(ptr[size*2]);
      float* orp = optr;
      float* ogp = &(optr[size]);
      float* obp = &(optr[size*2]);
      for( int ci = 0; ci < nchunks; ci++ ) {
        //printf("processing chunk %d\n", ci);
        for( int pi = 0; pi < chunk_size; pi++ ) {
          if( true ) {
            float r = *rp; float g = *gp; float b = *bp;
            *orp = mrgb[0][0] * r + mrgb[0][1] * g + mrgb[0][2] * b;
            *ogp = mrgb[1][0] * r + mrgb[1][1] * g + mrgb[1][2] * b;
            *obp = mrgb[2][0] * r + mrgb[2][1] * g + mrgb[2][2] * b;
          } else {
            *orp = mrgb[0][0] * (*rp) + mrgb[0][1] * (*gp) + mrgb[0][2] * (*bp);
            *ogp = mrgb[1][0] * (*rp) + mrgb[1][1] * (*gp) + mrgb[1][2] * (*bp);
            *obp = mrgb[2][0] * (*rp) + mrgb[2][1] * (*gp) + mrgb[2][2] * (*bp);
          }
          rp+=1;  gp+=1;  bp+=1;
          orp+=1; ogp+=1; obp+=1;
        }
      }
    }
    stopTime.set();
    long elapsedTime = stopTime.etime(startTime) / 1000;
    //std::cout<<"process: elpased time = "<<elapsedTime<<std::endl;
    if( minTime==0 ) minTime = elapsedTime;
    else {
      if( minTime > elapsedTime ) minTime = elapsedTime;
    }
    if( maxTime==0 ) maxTime = elapsedTime;
    else {
      if( maxTime < elapsedTime ) maxTime = elapsedTime;
    }
  }
  std::cout<<"process: in_place="<<in_place<<"  interleaved="<<interleaved<<std::endl;
  std::cout<<"    minimum time = "<<minTime<<std::endl;
  std::cout<<"    maximum time = "<<maxTime<<std::endl;
}


void process_sse2( float* buf, float* obuf, size_t size, bool in_place, bool interleaved )
{
#ifdef __SSE2__
  size_t chunk_size = 64;
  size_t nchunks = size / chunk_size;

  long minTime = 0;
  long maxTime = 0;

  __m128 mrgbv[3][3];
  for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
          mrgbv[i][j] = _mm_set1_ps(mrgb[i][j]);
      }
  }

  for(int i = 0; i < 20; i++) {
    //BENCHFUN;
    MyTime startTime;
    MyTime stopTime;
    startTime.set();
    float* ptr = in_place ? obuf : buf;
    float* optr = obuf;

    if(in_place) memcpy(obuf, buf, sizeof(float)*4*size);

    if(interleaved) {

      // RGBARGBARGBARGBA... channel arrangement

      float rin[4] __attribute__ ((aligned (16)));
      float gin[4] __attribute__ ((aligned (16)));
      float bin[4] __attribute__ ((aligned (16)));
      float rout[4] __attribute__ ((aligned (16)));
      float gout[4] __attribute__ ((aligned (16)));
      float bout[4] __attribute__ ((aligned (16)));

      for( int ci = 0; ci < nchunks; ci++ ) {
        //printf("processing chunk %d\n", ci);
        for( int pi = 0; pi < chunk_size-3; pi+=4 ) {
          rin[0] = ptr[0]; gin[0] = ptr[1];  bin[0] = ptr[2];
          rin[1] = ptr[3]; gin[1] = ptr[4];  bin[1] = ptr[5];
          rin[2] = ptr[6]; gin[2] = ptr[7];  bin[2] = ptr[8];
          rin[3] = ptr[9]; gin[3] = ptr[10]; bin[3] = ptr[11];

          __m128 rv = _mm_loadu_ps(&rin[0]);
          __m128 gv = _mm_loadu_ps(&gin[0]);
          __m128 bv = _mm_loadu_ps(&bin[0]);

          _mm_storeu_ps(&rout[0], mrgbv[0][0]*rv + mrgbv[0][1]*gv + mrgbv[0][2]*bv);
          _mm_storeu_ps(&gout[0], mrgbv[1][0]*rv + mrgbv[1][1]*gv + mrgbv[1][2]*bv);
          _mm_storeu_ps(&bout[0], mrgbv[2][0]*rv + mrgbv[2][1]*gv + mrgbv[2][2]*bv);

          optr[0] = rout[0];  optr[1] = gout[0];  optr[2] = bout[0];
          optr[3] = rout[1];  optr[4] = gout[1];  optr[5] = bout[1];
          optr[6] = rout[2];  optr[7] = gout[2];  optr[8] = bout[2];
          optr[9] = rout[3];  optr[10] = gout[3]; optr[11] = bout[3];

          ptr += 16; optr += 16;
        }
      }
    } else {

      // RRRR...GGGG...BBBB... channel arrangement

      float* rp = ptr;
      float* gp = &(ptr[size]);
      float* bp = &(ptr[size*2]);
      float* orp = optr;
      float* ogp = &(optr[size]);
      float* obp = &(optr[size*2]);

      for( int ci = 0; ci < nchunks; ci++ ) {
        //printf("processing chunk %d\n", ci);
        for( int pi = 0; pi < chunk_size-3; pi+=4 ) {
          __m128 rv = _mm_loadu_ps(&rp[0]);
          __m128 gv = _mm_loadu_ps(&gp[0]);
          __m128 bv = _mm_loadu_ps(&bp[0]);

          _mm_storeu_ps(&orp[0], mrgbv[0][0]*rv + mrgbv[0][1]*gv + mrgbv[0][2]*bv);
          _mm_storeu_ps(&ogp[0], mrgbv[1][0]*rv + mrgbv[1][1]*gv + mrgbv[1][2]*bv);
          _mm_storeu_ps(&obp[0], mrgbv[2][0]*rv + mrgbv[2][1]*gv + mrgbv[2][2]*bv);

          rp+=4;  gp+=4;  bp+=4;
          orp+=4; ogp+=4; obp+=4;
        }
      }
    }
    stopTime.set();
    long elapsedTime = stopTime.etime(startTime) / 1000;
    //std::cout<<"process_sse2: elpased time = "<<elapsedTime<<std::endl;
    if( minTime==0 ) minTime = elapsedTime;
    else {
      if( minTime > elapsedTime ) minTime = elapsedTime;
    }
    if( maxTime==0 ) maxTime = elapsedTime;
    else {
      if( maxTime < elapsedTime ) maxTime = elapsedTime;
    }
  }
  std::cout<<"process_sse2: in_place="<<in_place<<"  interleaved="<<interleaved<<std::endl;
  std::cout<<"    minimum time = "<<minTime<<std::endl;
  std::cout<<"    maximum time = "<<maxTime<<std::endl;
#endif // __SSE2__
}


void process_sse2_shuffle( float* buf, float* obuf, size_t size, bool in_place, bool interleaved )
{
#ifdef __SSE2__
  size_t chunk_size = 64;
  size_t nchunks = size / chunk_size;

  long minTime = 0;
  long maxTime = 0;

  __m128 mrgbv[3][3];
  for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
          mrgbv[i][j] = _mm_set1_ps(mrgb[i][j]);
      }
  }

  for(int i = 0; i < 20; i++) {
    //BENCHFUN;
    MyTime startTime;
    MyTime stopTime;
    startTime.set();

    float* ptr = in_place ? obuf : buf;
    float* optr = obuf;

    if(in_place) memcpy(obuf, buf, sizeof(float)*4*size);

    for( int ci = 0; ci < nchunks; ci++ ) {
      //printf("processing chunk %d\n", ci);
      for( int pi = 0; pi < chunk_size-3; pi+=4 ) {
        __m128 rgbv[4];
        __m128 a0;
        __m128 a1;
        __m128 a2;
        __m128 a3;
        rgbv[0] = _mm_loadu_ps(&ptr[0]);
        rgbv[1] = _mm_loadu_ps(&ptr[4]);
        rgbv[2] = _mm_loadu_ps(&ptr[8]);
        rgbv[3] = _mm_loadu_ps(&ptr[12]);

        /**/
        a0 = _mm_unpacklo_ps(rgbv[0], rgbv[2]);
        a1 = _mm_unpacklo_ps(rgbv[1], rgbv[3]);
        a2 = _mm_unpackhi_ps(rgbv[0], rgbv[2]);
        a3 = _mm_unpackhi_ps(rgbv[1], rgbv[3]);

        rgbv[0] = _mm_unpacklo_ps(a0, a1);
        rgbv[1] = _mm_unpackhi_ps(a0, a1);
        rgbv[2] = _mm_unpacklo_ps(a2, a3);
        rgbv[3] = _mm_unpackhi_ps(a2, a3);
        /**/

        __m128 orv = mrgbv[0][0]*rgbv[0] + mrgbv[0][1]*rgbv[1] + mrgbv[0][2]*rgbv[2];
        __m128 ogv = mrgbv[1][0]*rgbv[0] + mrgbv[1][1]*rgbv[1] + mrgbv[1][2]*rgbv[2];
        __m128 obv = mrgbv[2][0]*rgbv[0] + mrgbv[2][1]*rgbv[1] + mrgbv[2][2]*rgbv[2];

        /**/
        a0 = _mm_unpacklo_ps(orv, obv);
        a1 = _mm_unpacklo_ps(ogv, rgbv[3]);
        a2 = _mm_unpackhi_ps(orv, obv);
        a3 = _mm_unpackhi_ps(ogv, rgbv[3]);

        rgbv[0] = _mm_unpacklo_ps(a0, a1);
        rgbv[1] = _mm_unpackhi_ps(a0, a1);
        rgbv[2] = _mm_unpacklo_ps(a2, a3);
        rgbv[3] = _mm_unpackhi_ps(a2, a3);
        /**/

        //rgbv[0] = orv;
        //rgbv[1] = ogv;
        //rgbv[2] = obv;

        _mm_storeu_ps(&optr[0], rgbv[0]);
        _mm_storeu_ps(&optr[4], rgbv[1]);
        _mm_storeu_ps(&optr[8], rgbv[2]);
        _mm_storeu_ps(&optr[12], rgbv[3]);

        ptr += 16; optr += 16;
      }
    }
    stopTime.set();
    long elapsedTime = stopTime.etime(startTime) / 1000;
    //std::cout<<"process_sse2: elpased time = "<<elapsedTime<<std::endl;
    if( minTime==0 ) minTime = elapsedTime;
    else {
      if( minTime > elapsedTime ) minTime = elapsedTime;
    }
    if( maxTime==0 ) maxTime = elapsedTime;
    else {
      if( maxTime < elapsedTime ) maxTime = elapsedTime;
    }
  }
  std::cout<<"process_sse2_shuffle: in_place="<<in_place<<"  interleaved="<<interleaved<<std::endl;
  std::cout<<"    minimum time = "<<minTime<<std::endl;
  std::cout<<"    maximum time = "<<maxTime<<std::endl;
#endif // __SSE2__
}



void process_sse2_alt( float* buf, float* obuf, size_t size, bool in_place, bool interleaved )
{
#ifdef __SSE2__
  size_t chunk_size = 64;
  size_t nchunks = size / chunk_size;

  long minTime = 0;
  long maxTime = 0;

  float mc0[4] __attribute__ ((aligned (16)));
  float mc1[4] __attribute__ ((aligned (16)));
  float mc2[4] __attribute__ ((aligned (16)));
  for(int i = 0; i < 3; i++) {
    mc0[i] = mrgb[i][0];
    mc1[i] = mrgb[i][1];
    mc2[i] = mrgb[i][2];
  }
  mc0[3] = mc1[3] = mc2[3] = 0;

  __m128 mcv[3];
  mcv[0] = _mm_loadu_ps(&mc0[0]);
  mcv[1] = _mm_loadu_ps(&mc1[0]);
  mcv[2] = _mm_loadu_ps(&mc2[0]);

  for(int i = 0; i < 20; i++) {
    //BENCHFUN;
    MyTime startTime;
    MyTime stopTime;
    startTime.set();
    float* ptr = in_place ? obuf : buf;
    float* optr = obuf;

    if(in_place) memcpy(obuf, buf, sizeof(float)*4*size);

    __m128 rv, bv, gv;

    float orgb[4] __attribute__ ((aligned (16)));

    for( int ci = 0; ci < nchunks; ci++ ) {
      //printf("processing chunk %d\n", ci);
      for( int pi = 0; pi < chunk_size-3; pi+=1 ) {
        rv = _mm_set1_ps(ptr[0]);
        gv = _mm_set1_ps(ptr[1]);
        bv = _mm_set1_ps(ptr[2]);

        _mm_storeu_ps(&orgb[0], mcv[0]*rv + mcv[1]*gv + mcv[2]*bv);

        optr[0] = orgb[0];  optr[1] = orgb[1];  optr[2] = orgb[2];

        ptr += 4; optr += 4;
      }
    }
    stopTime.set();
    long elapsedTime = stopTime.etime(startTime) / 1000;
    //std::cout<<"process_sse2: elpased time = "<<elapsedTime<<std::endl;
    if( minTime==0 ) minTime = elapsedTime;
    else {
      if( minTime > elapsedTime ) minTime = elapsedTime;
    }
    if( maxTime==0 ) maxTime = elapsedTime;
    else {
      if( maxTime < elapsedTime ) maxTime = elapsedTime;
    }
  }
  std::cout<<"process_sse2_alt: in_place="<<in_place<<"  interleaved="<<interleaved<<std::endl;
  std::cout<<"    minimum time = "<<minTime<<std::endl;
  std::cout<<"    maximum time = "<<maxTime<<std::endl;
#endif // __SSE2__
}





int 
main( int argc, char **argv )
{
  size_t size = 50*1024*1024;
  size_t chunk_size = 64;
  size_t nchunks = size / chunk_size;
  float* buf = (float*)malloc_aligned(sizeof(float)*4*size);
  float* obuf = (float*)malloc_aligned(sizeof(float)*4*size);

  memset(buf, 0, sizeof(float)*3*size);

  printf("buf: %p\n", (void*)buf);

  mrgb[0][0] = 0.5;
  mrgb[0][1] = 0.5;
  mrgb[0][2] = 0.5;
  mrgb[1][0] = 0.5;
  mrgb[1][1] = 0.5;
  mrgb[1][2] = 0.5;
  mrgb[2][0] = 0.5;
  mrgb[2][1] = 0.5;
  mrgb[2][2] = 0.5;

  memcpy(obuf, buf, sizeof(float)*4*size);

  //process(buf, obuf, size);
  process_sse2(buf, obuf, size, false, true);

  process(buf, obuf, size, false, false);
  //process(buf, obuf, size, true, false);
  process(buf, obuf, size, false, true);
  //process(buf, obuf, size, true, true);

  process_sse2(buf, obuf, size, false, false);
  //process_sse2(buf, obuf, size, true, false);
  process_sse2(buf, obuf, size, false, true);
  //process_sse2(buf, obuf, size, true, true);

  process_sse2_shuffle(buf, obuf, size, false, true);
  //process_sse2_shuffle(buf, obuf, size, true, true);

  process_sse2_alt(buf, obuf, size, false, true);
  //process_sse2_alt(buf, obuf, size, true, true);
  return( 0 );
}
