/* Compile with

   gcc rawsave.c `pkg-config vips --cflags --libs`

 */

#include <vips/vips.h>
#include <vector>
#include <iostream>
#include <string>


#undef __SSE2__


#define THRESHOLD 1.f
#define BETA 0.0f
#define STRENGTH 1.0f
#define LOGL 1


// NaN-safe clamping (NaN compares false, and will thus result in H)
#define CLAMPS(A, L, H) ((A) > (L) ? ((A) < (H) ? (A) : (H)) : (L))


#ifdef _WIN32
void dt_free_align(void *mem);
#define dt_free_align_ptr dt_free_align
#else
#define dt_free_align(A) free(A)
#define dt_free_align_ptr free
#endif

void *dt_alloc_align(size_t alignment, size_t size)
{
  void *ptr = NULL;
#if defined(__FreeBSD_version) && __FreeBSD_version < 700013
  ptr = malloc(size);
#elif defined(_WIN32)
  ptr = _aligned_malloc(size, alignment);
#else
  if(posix_memalign(&ptr, alignment, size)) return NULL;
#endif
  if( ptr ) memset(ptr, 0, size);
  return ptr;
}

#ifdef _WIN32
void dt_free_align(void *mem)
{
  _aligned_free(mem);
}
#endif



// downsample width/height to given level
static inline int dl(int size, const int level)
{
  for(int l=0;l<level;l++)
    size = (size-1)/2+1;
  return size;
}



// upsamnple width/height to given level
static inline int ul(int size, const int level)
{
  for(int l=0;l<level;l++)
    size = size * 2;
  return size;
}



// needs a boundary of 1 or 2px around i,j or else it will crash.
// (translates to a 1px boundary around the corresponding pixel in the coarse buffer)
// more precisely, 1<=i<wd-1 for even wd and
//                 1<=i<wd-2 for odd wd (j likewise with ht)
static inline float ll_expand_gaussian(
    const float *const coarse,
    int i,
    int j,
    int wd,
    int ht,
    bool verbose=false)
{
  //if(i < 1) i = 1;
  //if(i > (wd-3)) i = wd - 3;
  //if(j < 1) j = 1;
  //if(j > (ht)) j = ht - 3;
  assert(i > 0);
  assert(i < wd-1);
  assert(j > 0);
  assert(j < ht-1);
  assert(j/2 + 1 < (ht-1)/2+1);
  assert(i/2 + 1 < (wd-1)/2+1);
  const int cw = (wd-1)/2+1;
  const int ind = (j/2)*cw+i/2;
  const int type = (i&1) + 2*(j&1);
  if(verbose) printf("ll_expand_gaussian(%d, %d): wd=%d  ht=%d  cw=%d  ind=%d  type=%d\n", i, j, wd, ht, cw, ind, type);
  // case 0:     case 1:     case 2:     case 3:
  //  x . x . x   x . x . x   x . x . x   x . x . x
  //  . . . . .   . . . . .   . .[.]. .   .[.]. . .
  //  x .[x]. x   x[.]x . x   x . x . x   x . x . x
  //  . . . . .   . . . . .   . . . . .   . . . . .
  //  x . x . x   x . x . x   x . x . x   x . x . x
  switch((i&1) + 2*(j&1))
  {
  case 0: // both are even, 3x3 stencil
    if(verbose) printf("    %f %f %f\n", coarse[ind-cw-1], coarse[ind-cw], coarse[ind-cw+1]);
    if(verbose) printf("    %f %f %f\n", coarse[ind-1], coarse[ind], coarse[ind+1]);
    if(verbose) printf("    %f %f %f\n", coarse[ind+cw-1], coarse[ind+cw], coarse[ind+cw+1]);
    return 4./256. * (
        6.0f*(coarse[ind-cw] + coarse[ind-1] + 6.0f*coarse[ind] + coarse[ind+1] + coarse[ind+cw])
        + coarse[ind-cw-1] + coarse[ind-cw+1] + coarse[ind+cw-1] + coarse[ind+cw+1]);
  case 1: // i is odd, 2x3 stencil
    return 4./256. * (
        24.0*(coarse[ind] + coarse[ind+1]) +
        4.0*(coarse[ind-cw] + coarse[ind-cw+1] + coarse[ind+cw] + coarse[ind+cw+1]));
  case 2: // j is odd, 3x2 stencil
    return 4./256. * (
        24.0*(coarse[ind] + coarse[ind+cw]) +
        4.0*(coarse[ind-1] + coarse[ind+1] + coarse[ind+cw-1] + coarse[ind+cw+1]));
  default: // case 3: // both are odd, 2x2 stencil
    return .25f * (coarse[ind] + coarse[ind+1] + coarse[ind+cw] + coarse[ind+cw+1]);
  }
}



// helper to fill in one pixel boundary by copying it
static inline void ll_fill_boundary1(
    float *const input,
    const int wd,
    const int ht)
{
  for(int j=1;j<ht-1;j++) input[j*wd] = input[j*wd+1];
  for(int j=1;j<ht-1;j++) input[j*wd+wd-1] = input[j*wd+wd-2];
  memcpy(input,    input+wd, sizeof(float)*wd);
  memcpy(input+wd*(ht-1), input+wd*(ht-2), sizeof(float)*wd);
}

// helper to fill in two pixels boundary by copying it
static inline void ll_fill_boundary2(
    float *const input,
    const int wd,
    const int ht)
{
  for(int j=1;j<ht-1;j++) input[j*wd] = input[j*wd+1];
  if(wd & 1) for(int j=1;j<ht-1;j++) input[j*wd+wd-1] = input[j*wd+wd-2];
  else       for(int j=1;j<ht-1;j++) input[j*wd+wd-1] = input[j*wd+wd-2] = input[j*wd+wd-3];
  memcpy(input, input+wd, sizeof(float)*wd);
  if(!(ht & 1)) memcpy(input+wd*(ht-2), input+wd*(ht-3), sizeof(float)*wd);
  memcpy(input+wd*(ht-1), input+wd*(ht-2), sizeof(float)*wd);
}

static inline void gauss_expand(
    const float *const input, // coarse input
    float *const fine,        // upsampled, blurry output
    const int wd,             // fine res
    const int ht)
{
#ifdef _OPENMP
#pragma omp parallel for default(none) schedule(static) collapse(2)
#endif
  for(int j=1;j<((ht-1)&~1);j++)  // even ht: two px boundary. odd ht: one px.
    for(int i=1;i<((wd-1)&~1);i++)
      fine[j*wd+i] = ll_expand_gaussian(input, i, j, wd, ht);
  ll_fill_boundary2(fine, wd, ht);
}



#if defined(__SSE2__)
static inline void gauss_reduce_sse2(
    const float *const input, // fine input buffer
    float *const coarse,      // coarse scale, blurred input buf
    const int wd,             // fine res
    const int ht)
{
  // blur, store only coarse res
  const int cw = (wd-1)/2+1, ch = (ht-1)/2+1;

  // this version is inspired by opencv's pyrDown_ :
  // - allocate 5 rows of ring buffer (aligned)
  // - for coarse res y
  //   - fill 5 coarse-res row buffers with 1 4 6 4 1 weights (reuse some from last time)
  //   - do vertical convolution via sse and write to coarse output buf

  const int stride = ((cw+8)&~7); // assure sse alignment of rows
  float *ringbuf = dt_alloc_align(64, sizeof(*ringbuf)*stride*5);
  float *rows[5] = {0};
  int rowj = 0; // we initialised this many rows so far

  for(int j=1;j<ch-1;j++)
  {
    // horizontal pass, convolve with 1 4 6 4 1 kernel and decimate
    for(;rowj<=2*j+2;rowj++)
    {
      float *const row = ringbuf + (rowj % 5)*stride;
      const float *const in = input + rowj*wd;
#ifdef _OPENMP
#pragma omp parallel for schedule(static) default(none)
#endif
      for(int i=1;i<cw-1;i++)
        row[i] = 6*in[2*i] + 4*(in[2*i-1]+in[2*i+1]) + in[2*i-2] + in[2*i+2];
    }

    // init row pointers
    for(int k=0;k<5;k++)
      rows[k] = ringbuf + ((2*j-2+k)%5)*stride;

    // vertical pass, convolve and decimate using SIMD:
    // note that we're ignoring the (1..cw-1) buffer limit, we'll pull in
    // garbage and fix it later by border filling.
    float *const out = coarse + j*cw;
    const float *const row0 = rows[0], *const row1 = rows[1],
        *const row2 = rows[2], *const row3 = rows[3], *const row4 = rows[4];
    const __m128 four = _mm_set1_ps(4.f), scale = _mm_set1_ps(1.f/256.f);
#ifdef _OPENMP
#pragma omp parallel for schedule(static) default(none)
#endif
    for(int i=0;i<=cw-8;i+=8)
    {
      __m128 r0, r1, r2, r3, r4, t0, t1;
      r0 = _mm_load_ps(row0 + i);
      r1 = _mm_load_ps(row1 + i);
      r2 = _mm_load_ps(row2 + i);
      r3 = _mm_load_ps(row3 + i);
      r4 = _mm_load_ps(row4 + i);
      r0 = _mm_add_ps(r0, r4);
      r1 = _mm_add_ps(_mm_add_ps(r1, r3), r2);
      r0 = _mm_add_ps(r0, _mm_add_ps(r2, r2));
      t0 = _mm_add_ps(r0, _mm_mul_ps(r1, four));

      r0 = _mm_load_ps(row0 + i + 4);
      r1 = _mm_load_ps(row1 + i + 4);
      r2 = _mm_load_ps(row2 + i + 4);
      r3 = _mm_load_ps(row3 + i + 4);
      r4 = _mm_load_ps(row4 + i + 4);
      r0 = _mm_add_ps(r0, r4);
      r1 = _mm_add_ps(_mm_add_ps(r1, r3), r2);
      r0 = _mm_add_ps(r0, _mm_add_ps(r2, r2));
      t1 = _mm_add_ps(r0, _mm_mul_ps(r1, four));

      t0 = _mm_mul_ps(t0, scale);
      t1 = _mm_mul_ps(t1, scale);

      _mm_storeu_ps(out + i, t0);
      _mm_storeu_ps(out + i + 4, t1);
    }
    // process the rest
    for(int i=cw&~7;i<cw-1;i++)
      out[i] = (6*row2[i] + 4*(row1[i] + row3[i]) + row0[i] + row4[i])*(1.0f/256.0f);
  }
  dt_free_align(ringbuf);
  ll_fill_boundary1(coarse, cw, ch);
}
#endif

static inline void gauss_reduce(
    const float *const input, // fine input buffer
    float *const coarse,      // coarse scale, blurred input buf
    const int wd,             // fine res
    const int ht)
{
  // blur, store only coarse res
  const int cw = (wd-1)/2+1, ch = (ht-1)/2+1;

  // this is the scalar (non-simd) code:
  const float a = 0.4f;
  const float w[5] = {1.f/4.f-a/2.f, 1.f/4.f, a, 1.f/4.f, 1.f/4.f-a/2.f};
  memset(coarse, 0, sizeof(float)*cw*ch);
  // direct 5x5 stencil only on required pixels:
#ifdef _OPENMP
#pragma omp parallel for schedule(static) default(none) collapse(2)
#endif
  for(int j=1;j<ch-1;j++) for(int i=1;i<cw-1;i++)
    for(int jj=-2;jj<=2;jj++) for(int ii=-2;ii<=2;ii++)
      coarse[j*cw+i] += input[(2*j+jj)*wd+2*i+ii] * w[ii+2] * w[jj+2];
  ll_fill_boundary1(coarse, cw, ch);
}



// allocate output buffer with monochrome brightness channel from input, padded
// up by max_supp on all four sides, dimensions written to wd2 ht2
static inline float *ll_pad_input(
    const float *const input,
    const int wd,
    const int ht,
    const int max_supp,
    int *wd2,
    int *ht2)
{
  const int stride = 1;
  *wd2 = 2*max_supp + wd;
  *ht2 = 2*max_supp + ht;
  float *const out = (float*)dt_alloc_align(64, *wd2**ht2*sizeof(*out));

  if( (*wd2) == wd && (*ht2) == ht ) {
    memcpy( out, input, *wd2**ht2*sizeof(*out) );
    return out;
  }

  { // pad by replication:
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) default(none) shared(wd2, ht2)
#endif
    for(int j=0;j<ht;j++)
    {
      for(int i=0;i<max_supp;i++)
        out[(j+max_supp)**wd2+i] = input[stride*wd*j]; // L -> [0,1]
      for(int i=0;i<wd;i++)
        out[(j+max_supp)**wd2+i+max_supp] = input[stride*(wd*j+i)]; // L -> [0,1]
      for(int i=wd+max_supp;i<*wd2;i++)
        out[(j+max_supp)**wd2+i] = input[stride*(j*wd+wd-1)]; // L -> [0,1]
    }
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) default(none) shared(wd2, ht2)
#endif
    for(int j=0;j<max_supp;j++)
      memcpy(out + *wd2*j, out+max_supp**wd2, sizeof(float)**wd2);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) default(none) shared(wd2, ht2)
#endif
    for(int j=max_supp+ht;j<*ht2;j++)
      memcpy(out + *wd2*j, out + *wd2*(max_supp+ht-1), sizeof(float)**wd2);
  }
  return out;
}



static inline float ll_laplacian(
    const float *const coarse,   // coarse res gaussian
    const float *const fine,     // fine res gaussian
    const int i,                 // fine index
    const int j,
    const int wd,                // fine width
    const int ht,
    bool verbose=false)                // fine height
{
  const float c = ll_expand_gaussian(coarse,
      CLAMPS(i, 1, ((wd-1)&~1)-1), CLAMPS(j, 1, ((ht-1)&~1)-1), wd, ht);
  const float l = fine[j*wd+i] - c;
  if(verbose) printf("ll_laplacian(%d, %d): c=%f  fine=%f  laplacian=%f\n", i, j, c, fine[j*wd+i], l);
  return l;
}



class Image
{
public:
  std::string name;
  int width, height, padding;
  float* pixels;

  Image(): width(0), height(0), pixels(NULL) {}
  ~Image()
  {
    if(pixels) {
      //std::cout<<"Freeing pixels from "<<name<<std::endl;
      dt_free_align(pixels);
    }
  }

  void set_name(std::string n) { name = n; }

  void zero()
  {
    memset(pixels, 0, sizeof(float)*width*height);
  }

  float get_pixel(int x, int y)
  {
    //printf("setting pixel %d,%d to %f\n", x, y, val);
    return pixels[y*width+x];
  }

  void set_pixel(int x, int y, float val)
  {
    //printf("setting pixel %d,%d from %f to %f\n", x, y, pixels[y*width+x], val);
    pixels[y*width+x] = val;
  }

  void add_to_pixel(int x, int y, float val)
  {
    //printf("setting pixel %d,%d to %f\n", x, y, val);
    pixels[y*width+x] += val;
  }

  void add_image(Image& img, float sign=1.0f)
  {
    for(int y = 0; y < height; y++) {
      for(int x = 0; x < width; x++) {
        pixels[y*width+x] += img.pixels[y*width+x] * sign;
      }
    }
  }

  bool crop(int x0, int y0, int w, int h, Image& out)
  {
    if( (x0+w) > width ) w = width - x0;
    if( (y0+h) > height ) h = height - y0;
    //printf("crop: %d,%d -> %d,%d\n", x0, y0, x0+w-1, y0+h-1);
    out.pixels = (float*)dt_alloc_align(64, sizeof(float)*w*h);
    if( !out.pixels ) return false;
    out.width = w; out.height = h;

    for(int y = 0; y < h; y++) {
      float* line = pixels + width*(y+y0) + x0;
      float* oline = out.pixels + out.width*y;
      memcpy( oline, line, sizeof(float)*out.width );
    }
    //printf("Image::crop()\n");
    //print(); out.print();
    //print(x0+w/2, y0+h/2); out.print(w/2, h/2);
    return true;
  }

  bool copy(Image& out)
  {
    out.pixels = (float*)dt_alloc_align(64, sizeof(float)*width*height);
    if( !out.pixels ) return false;
    out.width = width; out.height = height;
    memcpy( out.pixels, pixels, sizeof(float)*width*height );
    return true;
  }

  void print_full()
  {
    for(int y = 0; y < height; y++) {
      //printf("%d\n", y);
      for(int x = 0; x < width; x++) {
        //printf("  %d: %f\n", x, pixels[y*width+x]);
        printf("\t%f", pixels[y*width+x]);
      }
      printf("\n");
    }

    //int y = dl(height, 1), x = dl(width, 1);
    //printf("%d %d -> %f\n", y, x, pixels[y*width+x]);
  }

  void print()
  {
    for(int y = height/2; y <= height/2; y++) {
      //printf("%d\n", y);
      for(int x = 0; x < width; x++) {
        //printf("  %d: %f\n", x, pixels[y*width+x]);
        printf("\t%f", pixels[y*width+x]);
      }
      printf("\n");
    }

    //int y = dl(height, 1), x = dl(width, 1);
    //printf("%d %d -> %f\n", y, x, pixels[y*width+x]);
  }

  void print(int x, int y)
  {
    printf("%d %d -> %f\n", y, x, pixels[y*width+x]);
  }
};


#define max_levels 30
int pyramid_get_num_levels(int wd, int ht)
{
  int nl = MIN(max_levels, 31-__builtin_clz(MAX(wd,ht))+1);
  return nl;
}


int pyramid_get_padding(int num_levels)
{
  int p = 1 << (num_levels);
  return p;
}



class GaussianPyramid
{
public:
  Image input;
  Image padded[max_levels];
  Image laplacian[max_levels];
  Image output[max_levels];
  int width, height;
  int num_levels;
  int max_supp;
  bool verbose;
public:
  GaussianPyramid(const float *const input, int wd, int ht, int padding, bool add_padding=true, bool verbose=false);
  //~GaussianPyramid() {printf("Destroying gaussian pyramid\n");}

  void fill_laplacian();
  void remap();
};


GaussianPyramid::GaussianPyramid(const float *const in, int wd, int ht, int nl, bool add_padding, bool v)
{
  char tstr[500];
  verbose = v;
  width = wd, height = ht;
  //input.pixels = (float*)in;
  //input.width = wd; input.height = ht;
  // don't divide by 2 more often than we can:
  num_levels = nl;
  int last_level = num_levels-1;
  max_supp = (add_padding) ? pyramid_get_padding(last_level) : 0;
  if(verbose) printf("GaussianPyramid: size=%dx%d levels=%d padding=%d\n", wd, ht, num_levels, max_supp);
  int w, h;
  padded[0].pixels = ll_pad_input(in, wd, ht, max_supp, &w, &h);
  padded[0].width = w; padded[0].height = h;
  sprintf(tstr,"padded[0]"); padded[0].set_name(tstr);
  if(verbose) printf("GaussianPyramid: level 0 filled, size=%dx%d\n", w, h);
  if(verbose) padded[0].print();

  // allocate pyramid pointers for padded input
  for(int l=1;l<=last_level;l++) {
    padded[l].pixels = (float*)dt_alloc_align(64, sizeof(float)*dl(w,l)*dl(h,l));
    padded[l].width = dl(w,l); padded[l].height = dl(h,l);
    sprintf(tstr,"padded[%d]", l); padded[l].set_name(tstr);
  }

  // allocate pyramid pointers for laplacian coefficients
  //for(int l=0;l<=last_level;l++) {
  //  laplacian[l].pixels = (float*)dt_alloc_align(64, sizeof(float)*dl(w,l)*dl(h,l));
  //  laplacian[l].width = dl(w,l); laplacian[l].height = dl(h,l);
  //}

  // allocate pyramid pointers for output
  //for(int l=0;l<=last_level;l++) {
  //  output[l].pixels = (float*)dt_alloc_align(64, sizeof(float)*dl(w,l)*dl(h,l));
  //  output[l].width = dl(w,l); output[l].height = dl(h,l);
  //}

  // create gauss pyramid of padded input, write coarse directly to output
#if defined(__SSE2__)
  if(use_sse2)
  {
    for(int l=1;l<last_level;l++)
      gauss_reduce_sse2(padded[l-1], padded[l], dl(w,l-1), dl(h,l-1));
    gauss_reduce_sse2(padded[last_level-1], output[last_level], dl(w,last_level-1), dl(h,last_level-1));
  }
  else
#endif
  {
    for(int l=1;l<=last_level;l++) {
      gauss_reduce(padded[l-1].pixels, padded[l].pixels, dl(w,l-1), dl(h,l-1));
      if(verbose) printf("GaussianPyramid: level %d filled, size=%dx%d\n", l, dl(w,l), dl(h,l));
      if(verbose) padded[l].print();
    }
    //gauss_reduce(padded[last_level-1].pixels, padded[last_level].pixels, dl(w,last_level-1), dl(h,last_level-1));
    //if(verbose) printf("GaussianPyramid: level %d filled, size=%dx%d\n", last_level, dl(w,last_level), dl(h,last_level));
  }
  if(verbose) padded[last_level].print();
}



void GaussianPyramid::fill_laplacian()
{
  int last_level = num_levels-1;
  if(verbose) printf("\n\n================\n\n");

  for(int l=last_level;l>0;l--) {
    gauss_expand(padded[l].pixels, output[l-1].pixels, output[l-1].width, output[l-1].height);
    if(verbose) printf("GaussianPyramid: level %d expanded, size=%dx%d\n", l, output[l-1].width, output[l-1].height);
    if(verbose) output[l-1].print();
  }

  if(verbose) printf("\n\n================\n\n");

  for(int l=last_level-1;l>=0;l--) {
    padded[l].copy(laplacian[l]);
    laplacian[l].add_image(output[l], -1);
    if(verbose) printf("GaussianPyramid: laplacian level %d created, size=%dx%d\n", l, laplacian[l].width, laplacian[l].height);
    if(verbose) laplacian[l].print();
    laplacian[l].zero();
  }

  //float test = ll_expand_gaussian( padded[2].pixels, 1, 2,
  //    laplacian[1].width, laplacian[1].height);
  //printf("ll_expand_gaussian(%d, %d, %d): %f\n", 2, 1, 2, test);
}


void GaussianPyramid::remap()
{
  int last_level = num_levels-1;
  if(verbose) printf("\n\n================\n\n");

  output[last_level].pixels =
      (float*)dt_alloc_align(64, sizeof(float)*padded[last_level].width*padded[last_level].height);
  output[last_level].width = padded[last_level].width; output[last_level].height = padded[last_level].height;
  padded[last_level].copy(output[last_level]);

  for(int l=last_level-1;l>=0;l--) {

    int padding0 = 1<<(l+1);
    int padding = 1<<(last_level-l);
    int padding2 = padding / 2;
    int K = 3*( (1<<(l+2)) - 1 );
    if(true || verbose) printf("remap: level=%d  padding0=%d  padding=%d  K=%d\n", l, padding0, padding, K);

    output[l].pixels = (float*)dt_alloc_align(64, sizeof(float)*padded[l].width*padded[l].height);
    output[l].width = padded[l].width; output[l].height = padded[l].height;

    gauss_expand(output[l+1].pixels, output[l].pixels, output[l].width, output[l].height);
    if(verbose) printf("remap: output[%d]:\n", l+1);
    if(verbose) output[l+1].print();
    if(verbose) printf("remap: output[%d] expanded:\n", l+1);
    if(verbose) output[l].print();

    //getchar(); continue;

    float thr = THRESHOLD;
    int Dl = (last_level-1) - l;
    for(int il = 0; il < Dl; il++) thr *= 1.;

    if(verbose) printf("remap: laplacian[%d] before remap:\n", l);
    if(verbose) laplacian[l].print();

    // pixel coordinates in laplacian level l
    //int lpx = dl(width,l)-1, lpy = dl(height,l)-1;
    //int lpx = 1, lpy = 1;
    //for(int lpy = 1; lpy < padded[l].height-1; lpy++) {
    for(int lpy = padding-1; lpy < padded[l].height-padding+1; lpy++) {
    //for(int lpy = padding2-1; lpy < padded[l].height-padding2+1; lpy++) {
    //for(int lpy = padded[l].height/2; lpy <= padded[l].height/2; lpy++) {
      {
        int step = (padded[l].height-(padding*2))/10;
        //printf("lpy=%d  step=%d\n", lpy, step);
        if( (step>0) && ((lpy+padding+1)%step) == 0 ) {printf("*"); fflush(stdout);}
      }
      //for(int lpx = 1; lpx < padded[l].width-1; lpx++) {
      for(int lpx = padding-1; lpx < padded[l].width-padding+1; lpx++) {
      //for(int lpx = padding2-1; lpx < padded[l].width-padding2+1; lpx++) {
      //for(int lpx = padded[l].width-2; lpx <= padded[l].width-2; lpx++) {
        if(false && (verbose && (lpy == padded[l].height/2))) printf("pixel coordinates in laplacian level %d: %d,%d\n", l, lpx, lpy);
        // pixel coordinates in gassian level l+1
        //int gpx = dl(lpx,1), gpy = dl(lpy,1);
        int gpx = lpx/2, gpy = lpy/2;
        //printf("pixel coordinates in gaussian level %d: %d,%d\n", l+1, gpx, gpy);

        // pixel area to be remapped in 0th level
        int roisz = 3;
        int lleft = ul(gpx-roisz, 1);
        int ltop = ul(gpy-roisz, 1);
        int lright = ul(gpx+roisz+1, 1) - 1;
        int lbottom = ul(gpy+roisz+1, 1) - 1;

        int l0left = ul(gpx-roisz,l+1);
        int l0top = ul(gpy-roisz,l+1);
        int l0right = ul(gpx+roisz+1,l+1)-1;
        int l0bottom = ul(gpy+roisz+1,l+1)-1;

        //int w = 5, h = 5;
        /*int l0left = ul(lleft,l);
    int l0top = ul(ltop,l);
    int w = 4, h = 4;
    int w0 = ul(w,l)+1;
    int h0 = ul(h,l)+1;*/
        //w0 = padded[0].width; h0 = padded[0].height;

        if(true && verbose && (lpy == padded[l].height/2)) printf("RoI in level %d: %d,%d -> %d, %d\n", l, lleft, ltop, lright, lbottom );
        if(true && (verbose && (lpy == padded[l].height/2))) printf("RoI in level 0: %d,%d -> %d, %d\n", l0left, l0top, l0right, l0bottom);
        //printf("remap: level=%d  laplacian pixel=%d,%d  downsampled=%d,%d  input=%d,%d -> %d,%d (%dx%d), padding=%d\n",
        //    l, lpx, lpy, gpx, gpy, l0left, l0top, l0left+w0-1, l0top+h0-1, w0, h0, padding);
        int llpx = (lpx%2 == 1) ? roisz*2+1 : roisz*2;
        int llpy = (lpy%2 == 1) ? roisz*2+1 : roisz*2;
        if(lleft < 0) llpx += lleft;
        if(ltop < 0) llpy += ltop;
        if(true && (verbose && (lpy == padded[l].height/2))) printf("llpx=%d  llpy=%d\n", llpx, llpy);

        if(lleft < 0) lleft = 0;
        if(ltop < 0) ltop = 0;
        if(lright >= padded[l].width) lright = padded[l].width - 1;
        if(lbottom >= padded[l].height) lbottom = padded[l].height - 1;
        if(l0left < 0) l0left = 0;
        if(l0top < 0) l0top = 0;
        if(l0right >= padded[0].width) l0right = padded[0].width - 1;
        if(l0bottom >= padded[0].height) l0bottom = padded[0].height - 1;

        int w0 = l0right - l0left + 1; //ul(w,l+1);
        int h0 = l0bottom - l0top + 1; //ul(h,l+1);

        if(true && verbose && (lpy == padded[l].height/2)) printf("RoI in level %d: %d,%d -> %d, %d\n", l, lleft, ltop, lright, lbottom );
        if(true && (verbose && (lpy == padded[l].height/2))) printf("RoI in level 0: %d,%d -> %d, %d\n", l0left, l0top, l0right, l0bottom);

        Image cropped;
        padded[0].crop( l0left, l0top, w0, h0, cropped );
        cropped.set_name("cropped");
        if(true && verbose && (lpy == padded[l].height/2)) printf("cropped before remap: \n");
        if(true && verbose && (lpy == padded[l].height/2)) cropped.print();

        float g = padded[l].get_pixel(lpx, lpy);
        if(true && verbose && (lpy == padded[l].height/2)) printf("padded[%d].get_pixel(%d, %d)=%f\n", l, lpx, lpy, g);
        for(int y = 0; y < cropped.height; y++) {
          for(int x = 0; x < cropped.width; x++) {
            float val = cropped.get_pixel(x, y);
            float delta = val - g;
            if( delta > thr ) {
              float diff = val - (g+thr);
              cropped.set_pixel(x, y, g+thr+(diff*BETA));
            }
            else if( delta < -thr ) {
              float diff = (g-thr) - val;
              cropped.set_pixel(x, y, g-thr-(diff*BETA));
            }
          }
        }
        if(true && verbose && (lpy == padded[l].height/2)) printf("cropped after remap: \n");
        if(true && verbose && (lpy == padded[l].height/2)) cropped.print();

        //continue;

        if(false && verbose && (lpy == padded[l].height/2)) printf("remap: level=%d  laplacian pixel=%d,%d  numlevels=%d\n", l, lpx, lpy, pyramid_get_num_levels(cropped.width, cropped.height));
        GaussianPyramid pyr( cropped.pixels, cropped.width, cropped.height, l+2, false, false&&(lpy == padded[l].height/2) );
        if(true && verbose && (lpy == padded[l].height/2)) printf("pyr.padded[0]:\n");
        if(true && verbose && (lpy == padded[l].height/2)) pyr.padded[0].print();
        if(true && verbose && (lpy == padded[l].height/2)) printf("pyr.padded[%d]:\n", l);
        if(true && verbose && (lpy == padded[l].height/2)) pyr.padded[l].print();
        if(true && verbose && (lpy == padded[l].height/2)) printf("pyr.padded[%d]:\n", l+1);
        if(true && verbose && (lpy == padded[l].height/2)) pyr.padded[l+1].print();
        //if(l<3) getchar();
        //continue;
        //if(verbose) pyr.padded[l].print();
        if( pyramid_get_num_levels(cropped.width, cropped.height) >= (l+2) ) {
          //pyr.padded[l+1].print();

          //float test = ll_expand_gaussian( pyr.padded[l+1].pixels, lpx-lleft, lpy-ltop,
          //    laplacian[l].width, laplacian[l].height);
          //printf("ll_expand_gaussian(%d, %d, %d): %f\n", l+1, lpx-lleft, lpy-ltop, test);

          bool verb = false && (lpy == padded[l].height/2);

          // this mimics the 2-pixels border that is added when expanding a gaussian level
          int llpx2 = llpx;
          if(llpx2 < 1) llpx2 = 1;
          if(llpx2 > (pyr.padded[l].width-3)) llpx2 = pyr.padded[l].width - 3;
          int llpy2 = llpy;
          if(llpy2 < 1) llpy2 = 1;
          if(llpy2 > (pyr.padded[l].height-3)) llpy2 = pyr.padded[l].height - 3;
          float c = ll_expand_gaussian(pyr.padded[l+1].pixels, llpx2, llpy2,
              //CLAMPS(i, 1, ((wd-1)&~1)-1), CLAMPS(j, 1, ((ht-1)&~1)-1),
              pyr.padded[l].width, pyr.padded[l].height, true && verbose && (lpy == padded[l].height/2));
          float f = pyr.padded[l].pixels[(llpy)*pyr.padded[l].width + (llpx)];
          float lap = f - c;
          if(true && verbose && (lpy == padded[l].height/2))
            printf("laplacian(%d, %d): c=%f  fine=%f  laplacian=%f\n", lpx, lpy, c, f, lap);
          //float lap = ll_laplacian( output[l+1].pixels, pyr.padded[l].pixels,
          //    lpx-lleft, lpy-ltop, pyr.padded[l].width, pyr.padded[l].height, verb );
          //printf("remap: level=%d  laplacian pixel=%d,%d  laplacian=%f\n", l, lpx, lpy, lap);

          //int indx = laplacian[l].width * lpy + lpx;
          //laplacian[l].pixels[indx] = lap;

          int indx = output[l].width * lpy + lpx;
          output[l].pixels[indx] += lap;

          //printf("remap: output[%d] before add:\n", l);
          //output[l].print();
          //output[l].add_to_pixel(lpx, lpy, lap);
          //break;
        }
        if(true && verbose && (lpy == padded[l].height/2)) printf("\n");
      }
      //break;
    }
    if(verbose) printf("remap: output[%d] before add:\n", l);
    if(verbose) output[l].print();
    if(verbose) printf("\n");
    //ll_fill_boundary1(laplacian[l].pixels, laplacian[l].width, laplacian[l].height);
    //if(verbose) printf("remap: laplacian[%d]:\n", l);
    //if(verbose) laplacian[l].print();
    //output[l].add_image(laplacian[l]);
    if(verbose) printf("remap: output[%d] after add:\n", l);
    if(verbose) output[l].print_full();
    printf("\n");
    //if(l<4) getchar();
    //if( l==0 ) break;
  }
}


class LaplacianPyramid
{
public:
  std::vector< std::pair<VipsImage*, void*> > gauss_buffers, laplacian_buffers;

  LaplacianPyramid(void *data,
      size_t size,
      int width,
      int height,
      int bands,
      VipsBandFormat format, int nlevels);
  ~LaplacianPyramid();

  VipsImage* collapse();

  void get_roi(int level, int x, int y, VipsRect& roi, VipsRect& roi_out);

  void remap();
};


LaplacianPyramid::LaplacianPyramid(void *data,
    size_t size,
    int width,
    int height,
    int bands,
    VipsBandFormat format, int nlevels)
{
  VipsImage* g0 = vips_image_new_from_memory( data, size, width, height, bands, format );
  VipsImage* in = g0;
  size_t array_sz;
  gauss_buffers.push_back( std::make_pair(g0, data) );
  for(int i = 0; i < nlevels-1; i++) {
    VipsImage* blurred;
    VipsImage* g;
    if( vips_gaussblur( in, &blurred, 0.7, NULL ) ) break;
    if( vips_shrink( in, &g, 2, 2, NULL ) ) break;
    VIPS_UNREF( blurred );
    size_t array_sz;
    void* gbuf = vips_image_write_to_memory( g, &array_sz );
    if( !gbuf ) break;
    VipsImage* g2 = vips_image_new_from_memory( gbuf, array_sz, g->Xsize, g->Ysize, g0->Bands, g0->BandFmt );
    gauss_buffers.push_back( std::make_pair(g2, gbuf) );

    VipsImage* upscaled;
    if( vips_resize( g2, &upscaled, 2, "kernel", VIPS_KERNEL_LINEAR, NULL ) ) break;

    VipsImage* l;
    if( vips_subtract( in, upscaled, &l, NULL ) ) break;
    void* lbuf = vips_image_write_to_memory( l, &array_sz );
    if( !lbuf ) break;
    VipsImage* l2 = vips_image_new_from_memory( lbuf, array_sz, l->Xsize, l->Ysize, g0->Bands, g0->BandFmt );
    laplacian_buffers.push_back( std::make_pair(l2, lbuf) );

    //printf("LaplacianPyramid: level %d completed, size=%d %d\n", i, l2->Xsize, l2->Ysize);

    char tstr[500];
    sprintf(tstr, "/tmp/g%02d.tif", i);
    //vips_tiffsave( g, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
    //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
    sprintf(tstr, "/tmp/l%02d.tif", i);
    //vips_tiffsave( l, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
    //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );

    in = g2;

    if( in->Xsize < 4 || in->Ysize < 4 ) break;
  }
  laplacian_buffers.push_back( gauss_buffers.back() );
  //printf("LaplacianPyramid: residual level %lu completed, size=%d %d\n", laplacian_buffers.size()-1,
  //    gauss_buffers.back().first->Xsize, gauss_buffers.back().first->Ysize);
}


LaplacianPyramid::~LaplacianPyramid()
{
  for(size_t i = 0; i < gauss_buffers.size(); i++) {
    VIPS_UNREF(gauss_buffers[i].first);
    free(gauss_buffers[i].second);
  }

  for(size_t i = 0; i < laplacian_buffers.size()-1; i++) {
    VIPS_UNREF(laplacian_buffers[i].first);
    free(laplacian_buffers[i].second);
  }
}


VipsImage* LaplacianPyramid::collapse()
{
  char tstr[500];
  VipsImage* result = NULL;
  int N = laplacian_buffers.size();
  VipsImage* g = laplacian_buffers[N-1].first;
  sprintf(tstr, "/tmp/cg%02d.tif", N-1);
  //vips_tiffsave( g, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
  //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
  for(int i = N-2; i >= 0; i--) {
    VipsImage* l = laplacian_buffers[i].first;
    sprintf(tstr, "/tmp/cl%02d.tif", i);
    vips_tiffsave( l, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
        "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );

    VipsImage* upscaled;
    if( vips_resize( g, &upscaled, 2, "kernel", VIPS_KERNEL_LINEAR, NULL ) ) return NULL;

    VipsImage* gnew;
    if( vips_add( l, upscaled, &gnew, NULL ) ) return NULL;

    g = gnew;
    sprintf(tstr, "/tmp/cg%02d.tif", i);
    //vips_tiffsave( g, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
    //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
  }
  result = g;

  //sprintf(tstr, "/tmp/collapsed.tif");
  //vips_tiffsave( result, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
  //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );


  return result;
}


void LaplacianPyramid::get_roi(int level, int x, int y, VipsRect& roi, VipsRect& roi_out)
{
  int level2 = level;
  if( level == (laplacian_buffers.size()-1) ) {
    roi.left = (x - 1);
    roi.top = (y - 1);
    level2 -= 1;
  } else {
    roi.left = (x/2 - 1);
    roi.top = (y/2 - 1);
  }
  roi.width = 3;
  roi.height = 3;

  for(int i = 0; i <= level2; i++) {
    roi.left *= 2;
    roi.top *= 2;
    roi.width *= 2;
    roi.height *= 2;
  }

  VipsRect r0 = {0, 0, gauss_buffers.front().first->Xsize, gauss_buffers.front().first->Ysize};
  vips_rect_intersectrect( &roi, &r0, &roi );

  roi_out.left = roi.left;
  roi_out.top = roi.top;
  roi_out.width = roi.width;
  roi_out.height = roi.height;
  for(int i = 0; i < level; i++) {
    roi_out.left /= 2;
    roi_out.top /= 2;
    roi_out.width /= 2;
    roi_out.height /= 2;
  }
}


void LaplacianPyramid::remap()
{
  char tstr[500];
  int N = laplacian_buffers.size();
  for(int i = N-1; i >= 0; i--) {
    VipsImage* g = laplacian_buffers[i].first;
    sprintf(tstr, "/tmp/rg%02d.tif", i);
    //vips_tiffsave( g, tstr, "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
    //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );
    float* buf = (float*)laplacian_buffers[i].second;

    int width = g->Xsize;
    int height = g->Ysize;
    //for(int y = 0; y < g->Ysize; y++) {
    for(int y = 0; y < g->Ysize/10; y++) {
      float* line = buf + y * width * g->Bands;
      //for(int x = 0; x < g->Xsize; x++) {
      printf("remapping line %d\n", y);
      for(int x = 0; x < g->Xsize/10; x++) {
        float* px = line + x * g->Bands;
        VipsRect roi, roi_out;
        get_roi(i, x, y, roi, roi_out);
        //printf("level=%d  y=%d  x=%d\n  roi=%d,%d+%d+%d\n  roi_out=%d,%d+%d+%d\n  px=%f,%f,%f\n",
        //    i, y, x, roi.width, roi.height, roi.left, roi.top,
        //    roi_out.width, roi_out.height, roi_out.left, roi_out.top,
        //    px[0], px[1], px[2]);

        //VipsImage* cropped;
        //if( vips_crop(gauss_buffers.front().first, &cropped,
        //    roi.left, roi.top, roi.width, roi.height, NULL) ) return;

        size_t cropped_size = sizeof(float) * roi.width * roi.height * g->Bands;
        float* cropped_buf = (float*)malloc(cropped_size);

        int dx = x - roi_out.left;
        int dy = y - roi_out.top;

        //printf("dx=%d  dy=%d\n", dx, dy);

        LaplacianPyramid p(cropped_buf, cropped_size, roi.width, roi.height, g->Bands, g->BandFmt, i+2);
        float* rbuf = NULL;
        rbuf = (float*)p.laplacian_buffers[i].second;

        //printf("  output:\n");
        //for(int y = 0; y < roi_out.height; y++) {
        for(int y = dy; y <= dy; y++) {
          float* line = rbuf + roi_out.width*g->Bands*y;
          //for(int x = 0; x < roi_out.width; x++) {
          for(int x = dx; x <= dx; x++) {
            //printf("    y=%d x=%d  (", y, x);
            for(int b = 0; b < g->Bands; b++) {
              //printf("%f ", line[x*g->Bands+b]);
            }
            //printf(")\n");
          }
          //printf("\n");
        }
      }
    }
    //break;
    printf("level %d remapped\n", i);
  }
}



int
main( int argc, char **argv )
{
  VipsImage *image;

  // Create VipsImage from given file
  image = vips_image_new_from_file( argv[1], NULL );
  if( !image ) {
    printf("Failed to load \"%s\"\n",argv[1]);
    return 1;
  }

  printf("image:            %p\n",image);
  printf("# of bands:       %d\n",image->Bands);
  printf("band format:      %d\n",image->BandFmt);
  printf("type:             %d\n",image->Type);
  printf("image dimensions: %d x %d\n",image->Xsize,image->Ysize);

  size_t array_sz;
  float* buf = (float*)vips_image_write_to_memory( image, &array_sz );
  if( !buf ) return 1;

  float log2 = log(2);
  float* logl = (float*)malloc(sizeof(float) * image->Xsize * image->Ysize);
  for(int y = 0; y < image->Ysize; y++) {
    for(int x = 0; x < image->Xsize; x++) {
      float R = buf[(y*image->Xsize+x)*image->Bands];
      float G = buf[(y*image->Xsize+x)*image->Bands+1];
      float B = buf[(y*image->Xsize+x)*image->Bands+2];
      float L = 0.2126 * R + 0.7152 * G + 0.0722 * B;
      if( L < 1.0e-15 ) L = 1.0e-15;
#ifdef LOGL
      logl[y*image->Xsize+x] = log(L) / log2;
#else
      logl[y*image->Xsize+x] = L;
#endif
    }
  }
  float* input = (float*)logl;


  int nl = pyramid_get_num_levels(image->Xsize, image->Ysize) - 1;
  GaussianPyramid gp((float*)input, image->Xsize, image->Ysize, nl, true, false);
  //gp.fill_laplacian();
  gp.remap();
  //return 0;

  /*VipsImage* out = vips_image_new_from_memory( input,
      sizeof(float) * image->Xsize * image->Ysize,
      image->Xsize, image->Ysize, 1, image->BandFmt );*/

  //Image& inputimg = gp.padded[0];
  //VipsImage* in = vips_image_new_from_memory( inputimg.pixels,
  //    sizeof(float)*inputimg.width*inputimg.height,
  //    inputimg.width, inputimg.height, 1, image->BandFmt );
  //vips_tiffsave( in, "/tmp/in.tif", "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
  //    "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );


  int padding = (gp.output[0].width - image->Xsize) / 2;
  printf("padding: %d\n", padding);

  Image inputimg;
  inputimg.pixels = input;
  inputimg.width = image->Xsize; inputimg.height = image->Ysize;
  printf("input:\n"); inputimg.print();
  Image cropped;
  gp.output[0].crop(padding, padding, image->Xsize, image->Ysize, cropped);
  printf("cropped output:\n"); cropped.print();

  //Image& outimg = gp.output[0];
  Image& outimg = cropped;

  for(int y = 0; y < image->Ysize; y++) {
    for(int x = 0; x < image->Xsize; x++) {
      float R = buf[(y*image->Xsize+x)*image->Bands];
      float G = buf[(y*image->Xsize+x)*image->Bands+1];
      float B = buf[(y*image->Xsize+x)*image->Bands+2];
      float L = 0.2126 * R + 0.7152 * G + 0.0722 * B;
      float rlL = cropped.pixels[y*image->Xsize+x];
#ifdef LOGL
      float rL = STRENGTH*pow(2, rlL) + (1.0f-STRENGTH)*L;
#else
      float rL = STRENGTH*rlL + (1.0f-STRENGTH)*L;
#endif
      float ratio = (L > 1.0e-15) ? rL / L : 0;
      //if( y == image->Ysize/2 ) printf("L=%f  rlL=%f  rL=%f ratio=%f\n", L, rlL, rL, ratio);
      R *= ratio;
      G *= ratio;
      B *= ratio;
      buf[(y*image->Xsize+x)*image->Bands] = R;
      buf[(y*image->Xsize+x)*image->Bands+1] = G;
      buf[(y*image->Xsize+x)*image->Bands+2] = B;
    }
  }

  //VipsImage* out = vips_image_new_from_memory( outimg.pixels,
  //    sizeof(float)*outimg.width*outimg.height,
  //    outimg.width, outimg.height, 1, image->BandFmt );
  VipsImage* out = vips_image_new_from_memory( buf, sizeof(float) * image->Xsize * image->Ysize * image->Bands,
      image->Xsize, image->Ysize, image->Bands, image->BandFmt );
  vips_tiffsave( out, "/tmp/out.tif", "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
      "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );

  //LaplacianPyramid p( buf, array_sz, image->Xsize, image->Ysize, image->Bands, image->BandFmt, 2 );

  //p.remap();

  g_object_unref( image );

  return( 0 );
}
