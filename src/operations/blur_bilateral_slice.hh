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


class BlurBilateralSlicePar: public OpParBase
{
  float sigma_s;
  float sigma_r;
  float ss, sr;

  float scale_x, scale_y;
  int full_width, full_height;

public:
  BlurBilateralSlicePar():
    OpParBase(), sigma_s(0), sigma_r(0)
  {
    set_type( "blur_bilateral_slice" );
    set_default_name( _("blilateral blur slice") );
  }

  //void set_iterations( int i ) { iterations.set( i ); }
  void set_full_width( int w )  { full_width  = w; }
  void set_full_height( int h ) { full_height = h; }

  void set_sigma_s( float s ) { sigma_s = s; }
  void set_sigma_r( float s ) { sigma_r = s; }

  float get_ss() { return ss; }
  float get_sr() { return sr; }

  float get_sigma_s() { return sigma_s; }
  float get_sigma_r() { return sigma_r; }

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_caching() { return true; }


  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
  {
    rout->left = rin->left*scale_x;
    rout->top = rin->top*scale_y;
    rout->width = rin->width*scale_x;
    rout->height = rin->height*scale_y;
  }

  /* Function to derive the area to be read from input images,
       based on the requested output area
   */
  virtual void transform_inv(const Rect* rout, Rect* rin, int id)
  {
    if( id == 0 ) {
    float _w = roundf(rout->width  / ss);
    float _h = roundf(rout->height / ss);
    float _l = roundf(rout->left   / ss);
    float _t = roundf(rout->top    / ss);
    rin->left   = CLAMPS((int)_l, 0, 6000);
    rin->top    = CLAMPS((int)_t, 0, 6000);
    rin->width  = CLAMPS((int)_w, 4, 6000) + 1;
    rin->height = CLAMPS((int)_h, 4, 6000) + 1;
    //rin->left = rout->left/scale_x;
    //rin->top = rout->top/scale_y;
    //rin->width = rout->width/scale_x;
    //rin->height = rout->height/scale_y;
    } else {
      rin->left = rout->left;
      rin->top = rout->top;
      rin->width = rout->width;
      rin->height = rout->height;
    }
    //std::cout<<"BlurBilateralSlicePar::transform_inv(): id="<<id<<"  ss="<<ss<<", ireg="<<rin->width<<"x"<<rin->height
    //      <<"+"<<rin->left<<","<<rin->top<<std::endl;
    //std::cout<<"                                       oreg="<<rout->width<<"x"<<rout->height<<"+"<<rout->left<<","<<rout->top<<std::endl;
  }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    VipsImage* srcimg = NULL;
    if( in.size() > 0 ) srcimg = in[0];
    VipsImage* out = srcimg;
    if( !out ) return NULL;

    ss = sigma_s;
    sr = sigma_r;

    out = OpParBase::build( in, first, imap, omap, level );

    return out;
  }
};



template < OP_TEMPLATE_DEF >
class BlurBilateralSliceProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    std::cout<<"BlurBilateralSliceProc::render() called"<<std::endl;
  }
};

template < OP_TEMPLATE_DEF_CS_SPEC >
class BlurBilateralSliceProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_GRAYSCALE) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) return;

    BlurBilateralSlicePar* opar = dynamic_cast<BlurBilateralSlicePar*>(par);
    if( !opar ) return;

    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int y;
    VipsImage* srcimg = ireg[0]->im;

    if( false && r->left<10000 && r->top<10000 ) {
      std::cout<<"BlurBilateralSliceProc::render(): ireg[0]="<<ireg[0]->valid.width<<"x"<<ireg[0]->valid.height
          <<"+"<<ireg[0]->valid.left<<","<<ireg[0]->valid.top
          //<<"  ireg[1]="<<ireg[1]->valid.width<<"x"<<ireg[1]->valid.height
          //<<"+"<<ireg[1]->valid.left<<","<<ireg[1]->valid.top
          <<std::endl;
      std::cout<<"                                  oreg="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;
    }

    T* pin  = (T*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top );
    T* pin2  = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top );
    T* pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top );
    int ilskip = VIPS_REGION_LSKIP( ireg[1] ) / sizeof(T);
    int lskip  = VIPS_REGION_LSKIP( ireg[0] ) / sizeof(T);
    int olskip = VIPS_REGION_LSKIP( oreg ) / sizeof(T);

    int verb = 0;
    //if(ireg[0]->valid.left<1000 && ireg[0]->valid.top<1000) verb = 1;
    dt_bilateral_t* dt_b = dt_bilateral_init(width, height,
        opar->get_ss(), opar->get_sr(), verb);
    dt_b->buf = pin;
    dt_bilateral_slice(dt_b, pin2, pout, ilskip, lskip, olskip, -1);
    //std::cout<<"pout[0]="<<*pout<<std::endl;

    dt_bilateral_free(dt_b);
  }
};
