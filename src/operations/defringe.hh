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

#ifndef PF_DEFRINGE_H
#define PF_DEFRINGE_H

namespace PF 
{

#define MAGIC_THRESHOLD_COEFF 33.0f

const float fib[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233 };

enum defringe_method_t
{
  MODE_GLOBAL_AVERAGE,
  MODE_LOCAL_AVERAGE,
  MODE_STATIC
};

class DefringePar: public OpParBase
{
  PropertyBase op_mode;
  Property<float> radius;
  Property<float> threshold;

  ProcessorBase* gauss;
  ProcessorBase* convert2lab;
  ProcessorBase* convert2input;
  ProcessorBase* defringe_algo;

  PF::ICCProfile* in_profile;

public:
  DefringePar();

  bool has_intensity() { return false; }
  bool needs_caching();

  int get_op_mode() { return op_mode.get_enum_value().first; }
  float get_radius() { return radius.get(); }
  float get_threshold() { return threshold.get(); }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



class DefringeAlgoPar: public OpParBase
{
  defringe_method_t op_mode;
  float sigma;
  int radius;
  float threshold;

  int* xy_avg;
  int* xy_small;
  int samples_avg;
  int samples_small;
  int dmax, dmax_small;

public:
  DefringeAlgoPar(): OpParBase()
  {
    radius = threshold = 0;
    xy_avg = xy_small = NULL;
    dmax = dmax_small = 0;
  }

  ~DefringeAlgoPar()
  {
    if( xy_avg ) free( xy_avg );
    if( xy_small ) free( xy_small );
  }

  int get_padding() { return( (op_mode == MODE_LOCAL_AVERAGE) ? dmax : dmax_small ); }

  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
  {
    int pad = get_padding();
    rout->left = rin->left+pad;
    rout->top = rin->top+pad;
    rout->width = rin->width-pad*2;
    rout->height = rin->height-pad*2;
  }

  /* Function to derive the area to be read from input images,
     based on the requested output area
  */
  virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
  {
    int pad = get_padding();
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
  }


  void set_op_mode( defringe_method_t m ) { op_mode = m; }
  defringe_method_t get_op_mode() { return op_mode; }
  void set_sigma( float s ) { sigma = s; }
  int get_radius() { return radius; }
  void set_threshold( float t ) { threshold = t; }
  float get_threshold() { return threshold; }

  int get_samples_avg() { return samples_avg; }
  int* get_xy_avg() { return xy_avg; }
  int get_samples_small() { return samples_small; }
  int* get_xy_small() { return xy_small; }

  void fib_latt(int *const x, int *const y, float radius, int step, int idx)
  {
    // idx < 1 because division by zero is also a problem in the following line
    if(idx >= (int)(sizeof(fib) / sizeof(float)) - 1 || idx < 1)
    {
      *x = 0;
      *y = 0;
      std::cout<<"Fibonacci lattice index wrong/out of bounds in: defringe module"<<std::endl;
      return;
    }
    float px = step / fib[idx], py = step * (fib[idx + 1] / fib[idx]);
    py -= (int)py;
    float dx = px * radius, dy = py * radius;
    *x = round(dx - radius / 2.0);
    *y = round(dy - radius / 2.0);
  }

  bool fb_init()
  {
    const float sigma_tmp = fmax(0.1f, fabs(sigma));
    radius = ceil(2.0f * ceilf(sigma_tmp));
    int samples_wish = (int)(radius * radius);
    int sampleidx_avg = 0;
    dmax = 0;
    dmax_small = 0;

    // select samples by fibonacci number
    if(samples_wish > 89) {
      sampleidx_avg = 12; // 144 samples
    } else if(samples_wish > 55) {
      sampleidx_avg = 11; // 89 samples
    } else if(samples_wish > 34) {
      sampleidx_avg = 10; // ..you get the idea
    } else if(samples_wish > 21) {
      sampleidx_avg = 9;
    } else if(samples_wish > 13) {
      sampleidx_avg = 8;
    } else {
      // don't use less than 13 samples
      sampleidx_avg = 7;
    }

    const int sampleidx_small = sampleidx_avg - 1;
    const int small_radius = MAX(radius, 3);
    const int avg_radius = 24 + radius * 4;

    samples_small = (int)fib[sampleidx_small];
    samples_avg = (int)fib[sampleidx_avg];

    int* tmp;

    // precompute all required fibonacci lattices:
    if( xy_avg ) free( xy_avg );
    xy_avg = (int*)malloc((size_t)2 * sizeof(int) * samples_avg);
    if (xy_avg != NULL)
    {
      tmp = xy_avg;
      for(int u = 0; u < samples_avg; u++)
      {
        int dx, dy;
        fib_latt(&dx, &dy, avg_radius, u, sampleidx_avg);
        *tmp++ = dx;
        *tmp++ = dy;
        if( dmax < abs(dx) ) dmax = abs(dx);
        if( dmax < abs(dy) ) dmax = abs(dy);
      }
    }
    else
    {
      std::cout<<"Error allocating memory for fibonacci lattice in: defringe module"<<std::endl;
      return false;
    }

    if( xy_small ) free( xy_small );
    xy_small = (int*)malloc((size_t)2 * sizeof(int) * samples_small);
    if (xy_small != NULL)
    {
      tmp = xy_small;
      for(int u = 0; u < samples_small; u++)
      {
        int dx, dy;
        fib_latt(&dx, &dy, small_radius, u, sampleidx_small);
        *tmp++ = dx;
        *tmp++ = dy;
        if( dmax_small < abs(dx) ) dmax_small = abs(dx);
        if( dmax_small < abs(dy) ) dmax_small = abs(dy);
      }
    }
    else
    {
      std::cout<<"Error allocating memory for fibonacci lattice in: defringe module"<<std::endl;
      return false;
    }

    return true;
  }
};



template < OP_TEMPLATE_DEF >
class DefringeProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF >
class DefringeAlgoProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class DefringeAlgoProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 2 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;

    DefringeAlgoPar* opar = dynamic_cast<DefringeAlgoPar*>(par);
    if( !opar ) return;

#define RESCALE_LAB(ab) ((ab)/255.f)

    bool bError = false;
    
    const float threshold = RESCALE_LAB(opar->get_threshold());
    Rect *ir = &ireg[0]->valid;
    Rect *r = &oreg->valid;
    const int ch = oreg->im->Bands;
    const int iwidth = ir->width;
    const int iheight = ir->height;
    const int width = r->width;
    const int height = r->height;
    const int ilsz = iwidth*ch;
    const int lsz = width*ch;
    const int border = r->top - ir->top;

    float* in = NULL;
    float* pblur = NULL;
    float* out = NULL;

    //const int order = 1; // 0,1,2
    const int radius = opar->get_radius();
    
    float avg_edge_chroma = 0.0f;
    
    // save the fibonacci lattices in them later
    int *xy_avg = NULL;
    int *xy_small = NULL;
    float *edge_chroma = NULL;

    if(width < 2 * radius + 1 || height < 2 * radius + 1) 
      bError = true;

    // Pre-Computed Fibonacci Lattices
    int *tmp = NULL;

    const int samples_small = opar->get_samples_small();
    const int samples_avg = opar->get_samples_avg();

    xy_avg = opar->get_xy_avg();
    xy_small = opar->get_xy_small();

    if (!bError) {
      edge_chroma = (float*)malloc(iwidth * iheight * sizeof(float));
      if (edge_chroma != NULL)
      {
        memset(edge_chroma, 0, iwidth * iheight * sizeof(float));
        
        for(int v = 0; v < iheight; v++)
        {
          in = (float*)VIPS_REGION_ADDR( ireg[0], ir->left, ir->top + v );
          pblur = (float*)VIPS_REGION_ADDR( ireg[1], ir->left, ir->top + v );
          float *ec = edge_chroma + v * iwidth;
          
          for(int t = 0, tt = 0; t < ilsz; t+=ch, tt++)
          {
            // edge-detect on color channels
            // method: difference of original to gaussian blurred image:
            //std::cout<<"defringe: t="<<t<<"  ilsz="<<ilsz<<std::endl;
            float a = in[t + 1] - pblur[t + 1];
            float b = in[t + 2] - pblur[t + 2];
    
            float edge = (a * a + b * b); // range up to 2*(256)^2 -> approx. 0 to 131072
    
            // save local edge chroma in out[.. +3] , this is later compared with threshold
            ec[tt] = edge;
            
            // the average chroma of the edge-layer in the roi
            if(MODE_GLOBAL_AVERAGE == opar->get_op_mode()) avg_edge_chroma += edge;
          }
        }
      }
      else
      {
        std::cout<<"Error allocating memory for edge detection in: defringe module"<<std::endl;
        bError = true;
      }
    }
    
    float thresh = 0;
    if (!bError) {
      if(MODE_GLOBAL_AVERAGE == opar->get_op_mode())
      {
        avg_edge_chroma = avg_edge_chroma / (iwidth * iheight) + RESCALE_LAB(10.0f) * FLT_EPSILON;
        thresh = fmax(RESCALE_LAB(0.1f), RESCALE_LAB(4.0f) * threshold * avg_edge_chroma / MAGIC_THRESHOLD_COEFF);
      }
      else
      {
        // this fixed value will later be changed when doing local averaging, or kept as-is in "static" mode
        avg_edge_chroma = RESCALE_LAB(RESCALE_LAB(MAGIC_THRESHOLD_COEFF));
        thresh = fmax(RESCALE_LAB(0.1f), threshold);
      }
      thresh = RESCALE_LAB(thresh);
    }
    
    if (!bError) {
      for(int v = 0, vv = border; v < height; v++, vv++)
      {
        in = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + v );
        out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + v );

        int l = vv * iwidth;
        int lprev = (vv - 1) * iwidth;
        int lnext = (vv + 1) * iwidth;
        float *ec = edge_chroma + l;
        
        for(int t = 0, tt = border; t < lsz; t+=ch, tt++)
        {
          float local_thresh = thresh;
          // think of compiler setting "-funswitch-loops" to maybe improve these things:
          if(MODE_LOCAL_AVERAGE == opar->get_op_mode() && ec[tt] > thresh)
          {
            float local_avg = 0.0f;
            // use some and not all values from the neigbourhood to speed things up:
            const int *tmp = xy_avg;
            for(int u = 0; u < samples_avg; u++)
            {
              int dx = *tmp++;
              int dy = *tmp++;
              int x = tt + dx; //provided that the region padding is correct, this is not needed: MAX(0, MIN(width - 1, t + dx));
              int y = vv + dy; //provided that the region padding is correct, this is not needed: MAX(0, MIN(height - 1, v + dy));
              local_avg += edge_chroma[y * iwidth + x];
            }
            avg_edge_chroma = fmax(RESCALE_LAB(0.01f), (float)local_avg / samples_avg);
            local_thresh = fmax(RESCALE_LAB(0.1f), RESCALE_LAB(4.0f) * threshold * avg_edge_chroma / MAGIC_THRESHOLD_COEFF);
          }
  
          if(edge_chroma[(size_t)vv * iwidth + tt] > local_thresh
              // reduces artifacts ("region growing by 1 pixel"):
              || edge_chroma[lprev + tt - 1] > local_thresh
              || edge_chroma[lprev + tt] > local_thresh
              || edge_chroma[lprev + tt + 1] > local_thresh
              || edge_chroma[l + tt - 1] > local_thresh
              || edge_chroma[l + tt + 1] > local_thresh
              || edge_chroma[lnext + tt - 1] > local_thresh
              || edge_chroma[lnext + tt] > local_thresh
              || edge_chroma[lnext + tt + 1] > local_thresh)
          {
            float atot = 0.f, btot = 0.f;
            float norm = 0.f;
            float weight = 0.f;

            // it seems better to use only some pixels from a larger window instead of all pixels from a smaller window
            // we use a fibonacci lattice for that, samples amount need to be a fibonacci number, this can then be
            // scaled to a certain radius
  
            // use some neighbourhood pixels for lowest chroma average
            const int *tmp = xy_small;
            for(int u = 0; u < samples_small; u++)
            {
              int dx = *tmp++;
              int dy = *tmp++;
              int x = tt + dx; //provided that the region padding is correct, this is not needed: MAX(0, MIN(width - 1, t + dx));
              int y = vv + dy; //provided that the region padding is correct, this is not needed: MAX(0, MIN(height - 1, v + dy));
              
              // inverse chroma weighted average of neigbouring pixels inside window
              // also taking average edge chromaticity into account (either global or local average)
  
              weight = 1.0f / (edge_chroma[(size_t)y * iwidth + x] + avg_edge_chroma);
              float *in2 = (float*)VIPS_REGION_ADDR( ireg[0], ir->left, ir->top + y );
  
              atot += weight * in2[x * ch + 1];
              btot += weight * in2[x * ch + 2];
              norm += weight;
            }
            
            // here we could try using a "balance" between original and changed value, this could be used to
            // reduce artifcats but on first tries results weren't very convincing, 
            // and there are blend settings available anyway
  
            double a = (atot / norm);
            double b = (btot / norm);
  
            out[t + 1] = a;
            out[t + 2] = b;
          }
          else
          {
            out[t + 1] = in[t + 1];
            out[t + 2] = in[t + 2];
          }
          out[t] = in[t];

        }
      }
    }

    if (bError) {
      // something went wrong, return original image
      for( int y = 0; y < height; y++ ) {
        in = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( int x = 0; x < width * ch; x++ ) out[x] = in[x];
      }
    }
      
    if (edge_chroma) free(edge_chroma);
  }
  
};


ProcessorBase* new_defringe();

ProcessorBase* new_defringe_algo();

}

#endif 


