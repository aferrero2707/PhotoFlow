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

// NaN-safe clamping (NaN compares false, and will thus result in H)
#define CLAMPS(A, L, H) ((A) > (L) ? ((A) < (H) ? (A) : (H)) : (L))


class BlurBilateralSplatPar: public OpParBase
{
  float sigma_s;
  float sigma_r;
  float ss, sr;

  float scale_x, scale_y;
  int width, height;
  int ts, padding;

public:
  BlurBilateralSplatPar():
    OpParBase(), sigma_s(0), sigma_r(0), ts(128), padding(0)
  {
    set_type( "blur_bilateral_splat" );
    set_default_name( _("blilateral blur splat") );
  }


  //void set_iterations( int i ) { iterations.set( i ); }
  void set_sigma_s( float s ) { sigma_s = s; }
  void set_sigma_r( float s ) { sigma_r = s; }

  float get_ss() { return ss; }
  float get_sr() { return sr; }

  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_caching() { return false; }


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
  virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
  {
    /*
    float dx, dy, _l, _t;
    if( rout->left > 0 ) _l = roundf((rout->left+padding) * ss);
    else _l = roundf(rout->left * ss);
    rin->left = roundf(_l / ts) * ts;
    std::cout<<"BlurBilateralSplatPar::transform_inv(): ss="<<ss
        <<"  rout->left="<<rout->left<<"  l="<<_l
        <<"  rin->left="<<rin->left<<std::endl;
    if( rout->top > 0 ) _t = roundf((rout->top+padding) * ss);
    else _t = roundf(rout->top * ss);
    rin->top = roundf(_t / ts) * ts;
    std::cout<<"    rout->top="<<rout->top<<"  t="<<_t
        <<"  rin->top="<<rin->top<<std::endl;
    */
    float _w = roundf((rout->width-1)  * ss);
    float _h = roundf((rout->height-1) * ss);
    float _l = roundf((rout->left)   * ss);
    float _t = roundf((rout->top)    * ss);
    rin->left   = _l;
    rin->top    = _t;
    rin->width  = _w;
    rin->height = _h;
    if(false) {
    std::cout<<"BlurBilateralSplatPar::transform_inv(): ss="<<ss<<", ireg="<<rin->width<<"x"<<rin->height
          <<"+"<<rin->left<<","<<rin->top<<std::endl;
    std::cout<<"                                       oreg="<<rout->width<<"x"<<rout->height<<"+"<<rout->left<<","<<rout->top<<std::endl;
    }
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

    dt_bilateral_t* dt_b =
        dt_bilateral_init(srcimg->Xsize, srcimg->Ysize, ss, sr, 0);

    multiband_image(dt_b->size_x, dt_b->size_y, dt_b->size_z);

    scale_x = static_cast<float>(dt_b->size_x) / srcimg->Xsize;
    scale_y = static_cast<float>(dt_b->size_y) / srcimg->Ysize;

    if( true ) {
    std::cout<<"BlurBilateralSplatPar::build(): sigma_s="<<sigma_s<<"  ss="<<ss<<"  sr="<<sr
        <<"  W="<<srcimg->Xsize<<"  H="<<srcimg->Ysize
        <<"  size_x="<<dt_b->size_x<<"  size_y="<<dt_b->size_y<<"  size_z="<<dt_b->size_z
        <<"  scale_x="<<scale_x<<"  scale_y="<<scale_y<<"  level="<<level<<std::endl;
    }

    out = OpParBase::build( in, first, imap, omap, level );

    return out;
  }
};



template < OP_TEMPLATE_DEF >
class BlurBilateralSplatProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    std::cout<<"BlurBilateralSplatProc::render() called"<<std::endl;
  }
};

template < OP_TEMPLATE_DEF_CS_SPEC >
class BlurBilateralSplatProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_MULTIBAND) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) return;

    BlurBilateralSplatPar* opar = dynamic_cast<BlurBilateralSplatPar*>(par);
    if( !opar ) return;

    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int y;
    VipsImage* srcimg = ireg[0]->im;

    if( false && r->left<10000 && r->top<10000 ) {
      std::cout<<"BlurBilateralSplatProc::render(): ireg="<<ireg[0]->valid.width<<"x"<<ireg[0]->valid.height
          <<"+"<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<std::endl;
      std::cout<<"                                 oreg="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<" / "<<oreg->im->Bands<<std::endl;
    }

    T* pin  = (T*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top );
    T* pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top );
    int ilskip = VIPS_REGION_LSKIP( ireg[0] ) / sizeof(T);
    int olskip = VIPS_REGION_LSKIP( oreg ) / sizeof(T);

    vips_region_black( oreg );

    int verb = 0;
    //if(ireg[0]->valid.left<1000 && ireg[0]->valid.top<1000) verb = 1;
    dt_bilateral_t* dt_b = dt_bilateral_init(ireg[0]->valid.width, ireg[0]->valid.height,
        opar->get_ss(), opar->get_sr(), verb);
    dt_b->buf = pout;
    dt_b->size_x = r->width;
    dt_b->size_y = r->height;
    dt_b->size_z = oreg->im->Bands;
    dt_bilateral_splat(dt_b, pin, ilskip, olskip, verb);
    dt_bilateral_free(dt_b);
  }
};
