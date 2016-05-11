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

  cmsHPROFILE in_profile;

public:
  DefringePar();

  bool has_intensity() { return false; }
  bool needs_caching();

  int get_op_mode() { return op_mode.get_enum_value().first; }
  float get_radius() { return radius.get(); }
  float get_threshold() { return threshold.get(); }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
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



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class DefringeProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 2 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;

    DefringePar* opar = dynamic_cast<DefringePar*>(par);
    if( !opar ) return;

    bool bError = false;
    
    const float threshold = opar->get_threshold()/100.f;
    Rect *r = &oreg->valid;
    const int ch = oreg->im->Bands;
    const int width = r->width;
    const int height = r->height;

    float* in = NULL;
    float* pblur = NULL;
    float* out = NULL;

    const int order = 1; // 0,1,2
    const float sigma = fmax(0.1f, fabs(opar->get_radius()))/* * roi_in->scale / piece->iscale */; // TODO: get scale
    const int radius = ceil(2.0f * ceilf(sigma));
    
    float avg_edge_chroma = 0.0f;
    
    // save the fibonacci lattices in them later
    int *xy_avg = NULL;
    int *xy_small = NULL;
    float *edge_chroma = NULL;

    if(width < 2 * radius + 1 || height < 2 * radius + 1) 
      bError = true;

    // The loop should output only a few pixels, so just copy everything first
//TODO: should be a better way to copy images
    if (!bError) {
      for( int y = 0; y < height; y++ ) {
        pblur = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
  
        for( int x = 0; x < width * ch; x++ ) out[x] = pblur[x];
      }
    }
    
    // Pre-Compute Fibonacci Lattices
    int *tmp = NULL;

    int samples_wish = (int)(radius * radius);
    int sampleidx_avg = 0;
    
    // select samples by fibonacci number
    if (!bError) {
      if(samples_wish > 89)
      {
        sampleidx_avg = 12; // 144 samples
      }
      else if(samples_wish > 55)
      {
        sampleidx_avg = 11; // 89 samples
      }
      else if(samples_wish > 34)
      {
        sampleidx_avg = 10; // ..you get the idea
      }
      else if(samples_wish > 21)
      {
        sampleidx_avg = 9;
      }
      else if(samples_wish > 13)
      {
        sampleidx_avg = 8;
      }
      else
      { // don't use less than 13 samples
        sampleidx_avg = 7;
      }
    }
    
    const int sampleidx_small = sampleidx_avg - 1;
    const int small_radius = MAX(radius, 3);
    const int avg_radius = 24 + radius * 4;

    const int samples_small = (int)fib[sampleidx_small];
    const int samples_avg = (int)fib[sampleidx_avg];

    // precompute all required fibonacci lattices:
    if (!bError) {
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
        }
      }
      else
      {
        std::cout<<"Error allocating memory for fibonacci lattice in: defringe module"<<std::endl;
        bError = true;
      }
    }

    if (!bError) {
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
        }
      }
      else
      {
        std::cout<<"Error allocating memory for fibonacci lattice in: defringe module"<<std::endl;
        bError = true;
      }
    }
    
    if (!bError) {
      edge_chroma = (float*)malloc(width * height * sizeof(float));
      if (edge_chroma != NULL)
      {
        memset(edge_chroma, 0, width * height * sizeof(float));
        
        // TODO: can we use this?
    /*  #ifdef _OPENMP
      #pragma omp parallel for default(none) shared(width, height,                                                 \
                                                    d) reduction(+ : avg_edge_chroma) schedule(static)
      #endif*/
        for(int v = 0; v < height; v++)
        {
          in = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + v );
          out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + v );
          float *ec = edge_chroma + v * width;
          
          for(int t = 0; t < width; t++)
          {
            // edge-detect on color channels
            // method: difference of original to gaussian blurred image:
            float a = in[t * ch + 1] - out[t * ch + 1];
            float b = in[t * ch + 2] - out[t * ch + 2];
    
            float edge = (a * a + b * b); // range up to 2*(256)^2 -> approx. 0 to 131072
    
            // save local edge chroma in out[.. +3] , this is later compared with threshold
            ec[t] = edge;
            
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
        avg_edge_chroma = avg_edge_chroma / (width * height) + (10.0f/100.f) * FLT_EPSILON;
        thresh = fmax(0.1f/100.f, (4.0f/100.f) * threshold * avg_edge_chroma / MAGIC_THRESHOLD_COEFF);
      }
      else
      {
        // this fixed value will later be changed when doing local averaging, or kept as-is in "static" mode
        avg_edge_chroma = MAGIC_THRESHOLD_COEFF/100.f;
        thresh = fmax(0.1f/100.f, threshold);
      }
      thresh /= 100.f;
    }
    
    if (!bError) {
    // TODO: can we use this?
/*  #ifdef _OPENMP
  // dynamically/guided scheduled due to possible uneven edge-chroma distribution (thanks to rawtherapee code
  // for this hint!)
  #pragma omp parallel for default(none) shared(width, height, d, xy_small, xy_avg, xy_artifact)               \
      firstprivate(thresh, avg_edge_chroma) schedule(guided, 32)
  #endif*/
      for(int v = 0; v < height; v++)
      {
        in = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + v );
        out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + v );
        float *ec = edge_chroma + v * width;
        
        for(int t = 0; t < width; t++)
        {
          float local_thresh = thresh;
          // think of compiler setting "-funswitch-loops" to maybe improve these things:
          if(MODE_LOCAL_AVERAGE == opar->get_op_mode() && ec[t] > thresh)
          {
            float local_avg = 0.0f;
            // use some and not all values from the neigbourhood to speed things up:
            const int *tmp = xy_avg;
            for(int u = 0; u < samples_avg; u++)
            {
              int dx = *tmp++;
              int dy = *tmp++;
              int x = MAX(0, MIN(width - 1, t + dx));
              int y = MAX(0, MIN(height - 1, v + dy));
              local_avg += edge_chroma[y * width + x];
            }
            avg_edge_chroma = fmax(0.01f/100.f, (float)local_avg / samples_avg);
            local_thresh = fmax(0.1f/100.f, (4.0f/100.f) * threshold * avg_edge_chroma / MAGIC_THRESHOLD_COEFF);
          }
  
          if(edge_chroma[(size_t)v * width + t] > local_thresh
                     // reduces artifacts ("region growing by 1 pixel"):
                     || edge_chroma[(size_t)MAX(0, (v - 1)) * width + MAX(0, (t - 1))] > local_thresh
                     || edge_chroma[(size_t)MAX(0, (v - 1)) * width + t] > local_thresh
                     || edge_chroma[(size_t)MAX(0, (v - 1)) * width + MIN(width - 1, (t + 1))] > local_thresh
                     || edge_chroma[(size_t)v * width + MAX(0, (t - 1))] > local_thresh
                     || edge_chroma[(size_t)v * width + MIN(width - 1, (t + 1))] > local_thresh
                     || edge_chroma[(size_t)MIN(height - 1, (v + 1)) * width + MAX(0, (t - 1))] > local_thresh
                     || edge_chroma[(size_t)MIN(height - 1, (v + 1)) * width + t] > local_thresh
                     || edge_chroma[(size_t)MIN(height - 1, (v + 1)) * width + MIN(width - 1, (t + 1))]
                        > local_thresh)
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
              int x = MAX(0, MIN(width - 1, t + dx));
              int y = MAX(0, MIN(height - 1, v + dy));
              
              // inverse chroma weighted average of neigbouring pixels inside window
              // also taking average edge chromaticity into account (either global or local average)
  
              weight = 1.0f / (edge_chroma[(size_t)y * width + x] + avg_edge_chroma);
              float *in2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
  
              atot += weight * in2[x * ch + 1];
              btot += weight * in2[x * ch + 2];
              norm += weight;
            }
            
            // here we could try using a "balance" between original and changed value, this could be used to
            // reduce artifcats but on first tries results weren't very convincing, 
            // and there are blend settings available anyway
  
            double a = (atot / norm); // *balance + in[v*width*ch + t*ch +1]*(1.0-balance);
            double b = (btot / norm); // *balance + in[v*width*ch + t*ch +2]*(1.0-balance);
  
            out[t * ch + 1] = a;
            out[t * ch + 2] = b;
          }
          else
          {
            out[t * ch + 1] = in[t * ch + 1];
            out[t * ch + 2] = in[t * ch + 2];
          }
          out[t * ch] = in[t * ch];
        }
      }
    }

    if (bError) {
      // something went wrong, return original image
  //TODO: should be a better way to copy images
      for( int y = 0; y < height; y++ ) {
        in = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        out = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( int x = 0; x < width * ch; x++ ) out[x] = in[x];
      }
    }
      
    if (xy_small) free(xy_small);
    if (xy_avg) free(xy_avg);
    if (edge_chroma) free(edge_chroma);
    
  }
  
  void fib_latt(int *const x, int *const y, float radius, int step, int idx)
  {
    // idx < 1 because division by zero is also a problem in the following line
    if(idx >= sizeof(fib) / sizeof(float) - 1 || idx < 1)
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

};


ProcessorBase* new_defringe();

}

#endif 


