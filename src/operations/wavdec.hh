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

/* The Wavelet Decompose algorithm is based on the GIMP's Wavelet Decompose plugin, by Marco Rossini 
 * 
 * http://registry.gimp.org/node/11742
 * 
 * */


#ifndef PF_WAVDEC_H
#define PF_WAVDEC_H

#include <string.h>
#include "../base/processor.hh"

namespace PF 
{


class WavDecPar: public OpParBase
{
  Property<int> numScales, currScale, preview_scale, initial_lev;
  Property<float> blendFactor;
  
  ProcessorBase* wavdec_algo;

public:
  WavDecPar();

  bool has_intensity() { return false; }
//  bool needs_caching() { return true; }

  int get_numScales() { return numScales.get(); }
  void set_numScales(int a) { numScales.set(a); }
  int get_currScale() { return currScale.get(); }
  void set_currScale(int a) { currScale.set(a); }
  int get_initial_lev() { return initial_lev.get(); }
  void set_initial_lev(int s) { initial_lev.set(s); }
  int get_preview_scale() { return preview_scale.get(); }
  void set_preview_scale(int s) { preview_scale.set(s); }

  float get_blendFactor() { return blendFactor.get(); }
  void set_blendFactor(float a) { blendFactor.set(a); }

  int get_padding(int nScales, int initial_lev) { return pow(2, nScales+initial_lev); }
  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  
  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};

class WavDecAlgoPar: public OpParBase
{
  int numScales, currScale;
  float blendFactor;
  int initial_lev;
  int preview_scale;
  int padding;

public:
  WavDecAlgoPar(): OpParBase()
  {
      numScales = 0;
      currScale = 0;
      blendFactor = .5f;
      initial_lev = 0;
      preview_scale = 1;
      padding = 0;
  }
  
  int get_numScales() { return numScales; }
  void set_numScales(int a) { numScales=a; }
  int get_currScale() { return currScale; }
  void set_currScale(int a) { currScale=a; }
  int get_padding() { return padding; }
  void set_padding(int p) { padding = p; }
  int get_initial_lev() { return initial_lev; }
  void set_initial_lev(int p) { initial_lev = p; }
  int get_preview_scale() { return preview_scale; }
  void set_preview_scale(int p) { preview_scale = p; }

  float get_blendFactor() { return (float)blendFactor; }
  void set_blendFactor(float a) { blendFactor=a; }

  static int get_maxScales(const int width, const int height)
  {
    int maxscale = 0;
    
    // smallest edge must be higher than or equal to 2^scales 
    unsigned int size = MIN(width, height);
    while (size >>= 1) maxscale++;

    return maxscale;
  } 

  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout)
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
  virtual void transform_inv(const Rect* rout, Rect* rin)
  {
    int pad = get_padding();
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
  }


};



template < OP_TEMPLATE_DEF >
class WavDecProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};

template < OP_TEMPLATE_DEF >
class WavDecAlgoProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class WavDecAlgoProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
    float *wd_layers; // buffer for internal use
    float *wd_image;
    int wd_ch;
    int wd_width;
    int wd_height;
    int wd_scales;
    int wd_return_layer;
    float wd_blend_factor; // .128f
    int wd_max_scales;
    int wd_initial_lev;
    float wd_preview_scale;
    
#define INDEX_WT_IMAGE(ch, index) (((index)*ch)+c)

    
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) {
      std::cout<<"WavDecAlgoProc::render ireg[0] == NULL"<<std::endl;
      return;
    }

    WavDecAlgoPar* opar = dynamic_cast<WavDecAlgoPar*>(par);
    if( !opar ) {
      std::cout<<"WavDecAlgoProc::render opar == NULL"<<std::endl;
      return;
    }

    Rect *r = &oreg->valid;
    Rect *ir = &ireg[0]->valid;
    
    wd_width = ir->width;
    wd_height = ir->height;
    wd_ch = oreg->im->Bands;
    
    const int i_line_size = wd_width * wd_ch;
    
    wd_image = (float*)malloc(wd_width * wd_height * wd_ch * sizeof(float));
    memset(wd_image, 0, wd_width * wd_height * wd_ch * sizeof(float));

    for( int y = 0; y < wd_height; y++ ) {
      float *pin = (float*)VIPS_REGION_ADDR( ireg[0], ir->left, ir->top + y );
      float *pwd = wd_image + y * i_line_size;

      memcpy(pwd, pin, i_line_size * sizeof(float));
    }
    
    wd_initial_lev = (int)opar->get_initial_lev();
    wd_scales = (int)opar->get_numScales();
    wd_return_layer = (int)opar->get_currScale();
    wd_blend_factor = (float)opar->get_blendFactor();
    wd_preview_scale = (float)opar->get_preview_scale()+1;
    wd_preview_scale = 1.f / wd_preview_scale;
    wd_max_scales = opar->get_maxScales(wd_width, wd_height);
    if (wd_scales > wd_max_scales)
    {
      wd_scales = wd_max_scales;
      std::cout<<"WavDecAlgoProc::render: max scale is "<<wd_max_scales<<" for this image preview size"<<std::endl;
    }
    if (wd_return_layer > (wd_scales+1)) wd_return_layer = wd_scales+1;

//    std::cout<<"WavDecAlgoProc::render: max scale is "<<wd_max_scales<<" for this image preview size"<<std::endl;

    if (wd_scales > 0)
      dwt_decompose();
      
    const int o_line_size = r->width*wd_ch;
    
    for( int y = 0; y < r->height; y++ ) {
      float *pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      float *pwd = wd_image + (y+abs(ir->height-r->height)/2) * i_line_size + (abs(ir->width-r->width)/2) * wd_ch;

      memcpy(pout, pwd, o_line_size * sizeof(float));
    }

    if (wd_image) free(wd_image);
    
  }
  

private:
  void dwt_decompose()
  {  
    /* this function prepares for decomposing, which is done in the function dwt_wavelet_decompose() */
    if (wd_scales > wd_max_scales) wd_scales = wd_max_scales;
    if (wd_return_layer > (wd_scales+1)) wd_return_layer = wd_scales+1;
    
    if (wd_return_layer == 0) {
      wd_layers = (float *)malloc(wd_width * wd_height * wd_ch * sizeof(float));
      if (wd_layers == NULL)
      {
        std::cout<<"not enough memory for wavelet decomposition"<<std::endl;
        goto cleanup;
      }
      memset(wd_layers, 0, wd_width * wd_height * wd_ch * sizeof(float));
    }
    else {
      wd_layers = NULL;
    }
    
    dwt_wavelet_decompose();

    
  cleanup:
      if (wd_layers) 
      {
        free(wd_layers);
        wd_layers = NULL;
      }

  }

  void dwt_wavelet_decompose()
  {
    float *temp = NULL;
    unsigned int lpass, hpass;
    float *buffer[2] = {0, 0};
    int bcontinue = 1;
    
    //const float lpass_add = sqrtf(.25f);
    //const float lpass_mult = (1.f / 16.f);
    const float lpass_add = 0;//8.f/257.f;//sqrtf(.25f);
    const float lpass_mult = (1.f / 16.f);//( lpass_add * 2.f );//(1.f / 16.f);
    const float lpass_sub = wd_blend_factor; //.128f;
    
    const int size = wd_width * wd_height * wd_ch;
    const int i_line_size = wd_width * wd_ch;

    /* image buffers */
    buffer[0] = wd_image;
    /* temporary storage */
    buffer[1] = (float *)malloc(size * sizeof(float));
    if (buffer[1] == NULL)
    {
      std::cout<<"not enough memory for wavelet decomposition"<<std::endl;
      goto cleanup;
    }
    memset(buffer[1], 0, size * sizeof(float));

    temp = (float *)malloc(MAX(wd_width, wd_height) * wd_ch * sizeof(float));
    if (temp == NULL)
    {
      std::cout<<"not enough memory for wavelet decomposition"<<std::endl;
      goto cleanup;
    }
    memset(temp, 0, MAX(wd_width, wd_height) * wd_ch * sizeof(float));

    /* iterate over wavelet scales */
    lpass = 1;
    hpass = 0;
    for (unsigned int lev = 0; lev < wd_scales && bcontinue; lev++) 
    {
      lpass = (1 - (lev & 1));

      for (int row = 0; row < wd_height; row++) 
      {
        dwt_hat_transform(temp, buffer[hpass] + (row * i_line_size), 1, wd_width, 1 << (lev+wd_initial_lev));
        memcpy(&(buffer[lpass][row * i_line_size]), temp, i_line_size * sizeof(float));
      }
      
      for (int col = 0; col < wd_width; col++) 
      {
        dwt_hat_transform(temp, buffer[lpass] + col*wd_ch, wd_width, wd_height, 1 << (lev+wd_initial_lev));
        for (int row = 0; row < wd_height; row++) 
        {
          for (int c = 0; c < wd_ch; c++) 
          buffer[lpass][INDEX_WT_IMAGE(wd_ch, row * wd_width + col)] = temp[INDEX_WT_IMAGE(wd_ch, row)];
        }
      }
        
      for (int i = 0; i < size; i++) 
      {
        //buffer[lpass][i] = ( buffer[lpass][i] + lpass_add ) * lpass_mult;
        buffer[lpass][i] *= lpass_mult;
        buffer[hpass][i] -= buffer[lpass][i] - lpass_sub;
      }
    
      if (wd_return_layer == 0)
      {
        dwt_add_layer(buffer[hpass], lev + 1);
      }
      else if (wd_return_layer == ((int)(lev + 1)))
      {
        dwt_get_image_layer(buffer[hpass]);
    
        bcontinue = 0;
      }
      
      hpass = lpass;
    }

    //  Wavelet residual
    if (wd_return_layer == (wd_scales+1))
    {
      dwt_get_image_layer(buffer[lpass]);
    }
    else if (wd_return_layer == 0)
    {
      dwt_add_layer(buffer[hpass], wd_scales+1);
      
      dwt_get_image_layer(wd_layers);
    }

  cleanup:
    if (temp) free(temp);
    if (buffer[1]) free(buffer[1]);

  }

  void dwt_add_layer(float *const img, const int n_scale)
  {
    const float lpass_sub = wd_blend_factor; //.128f;
    const int buff_size = wd_width * wd_height * wd_ch;
    
    if (n_scale == wd_scales+1)
    {
      for (int i = 0; i < buff_size; i++)
        wd_layers[i] += img[i];
    }
    else
    {
      for (int i = 0; i < buff_size; i++)
        wd_layers[i] += img[i] - lpass_sub;
    }

  }

  void dwt_get_image_layer(float *const layer)
  {
    if (wd_image != layer) {
      memcpy(wd_image, layer, wd_width * wd_height * wd_ch * sizeof(float));
    }
  }

  void dwt_hat_transform(float *temp, float *const base, const int st, const int size, int sc)
  {
    int i, c;
    const float hat_mult = 2.f;
    sc = std::min(size, (int)(sc * wd_preview_scale));
    
    for (i = 0; i < sc; i++)
    {
      for (c = 0; c < wd_ch; c++, temp++)
      {
        *temp = hat_mult * base[INDEX_WT_IMAGE(wd_ch, st * i)] + base[INDEX_WT_IMAGE(wd_ch, st * (sc - i))] + base[INDEX_WT_IMAGE(wd_ch, st * (i + sc))];
      }
    }
    for (; i + sc < size; i++)
    {
      for (c = 0; c < wd_ch; c++, temp++)
      {
        *temp = hat_mult * base[INDEX_WT_IMAGE(wd_ch, st * i)] + base[INDEX_WT_IMAGE(wd_ch, st * (i - sc))] + base[INDEX_WT_IMAGE(wd_ch, st * (i + sc))];
      }
    }
    for (; i < size; i++)
    {
      for (c = 0; c < wd_ch; c++, temp++)
      {
        *temp = hat_mult * base[INDEX_WT_IMAGE(wd_ch, st * i)] + base[INDEX_WT_IMAGE(wd_ch, st * (i - sc))]
                                                               + base[INDEX_WT_IMAGE(wd_ch, st * (2 * size - 2 - (i + sc)))];
      }
    }
    
  }


};


ProcessorBase* new_wavdec_algo();
ProcessorBase* new_wavdec();

}

#endif 
