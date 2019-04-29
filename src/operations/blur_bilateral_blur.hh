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

class BlurBilateralBlurPar: public OpParBase
{
  int padding;
public:
  BlurBilateralBlurPar():
    OpParBase(), padding(2)
  {
    set_type( "blur_bilateral_blur" );
    set_default_name( _("blilateral blur blur") );
  }


  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_caching() { return false; }

  int get_padding() { return padding; }



  /* Function to derive the output area from the input area
   */
  virtual void transform(const VipsRect* rin, VipsRect* rout, int /*id*/)
  {
    int pad = padding;
    rout->left = rin->left+pad;
    rout->top = rin->top+pad;
    rout->width = rin->width-pad*2;
    rout->height = rin->height-pad*2;
  }

  /* Function to derive the area to be read from input images,
     based on the requested output area
  */
  virtual void transform_inv(const VipsRect* rout, VipsRect* rin, int /*id*/)
  {
    int pad = padding;
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
    //std::cout<<"BlurBilateralBlurPar::transform_inv(): ireg="<<rin->width<<"x"<<rin->height
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
    set_image_hints(srcimg);
    VipsImage* blurred = OpParBase::build( in, first, NULL, NULL, level );

    return blurred;
  }
};



template < OP_TEMPLATE_DEF >
class BlurBilateralBlurProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    std::cout<<"BlurBilateralBlurProc::render() called"<<std::endl;
  }
};

template < OP_TEMPLATE_DEF_CS_SPEC >
class BlurBilateralBlurProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_MULTIBAND) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) return;

    BlurBilateralBlurPar* opar = dynamic_cast<BlurBilateralBlurPar*>(par);
    if( !opar ) return;

    VipsRect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int y;
    VipsImage* srcimg = ireg[0]->im;

    if( false && r->left<10000 && r->top<10000 ) {
      std::cout<<"BlurBilateralBlurProc::render(): ireg="<<ireg[0]->valid.width<<"x"<<ireg[0]->valid.height
          <<"+"<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<std::endl;
      std::cout<<"                                 oreg="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;
    }

    int verb = 0;
    //if(ireg[0]->valid.left==0 && ireg[0]->valid.top==0) verb = 1;
    dt_bilateral_t* dt_b = (dt_bilateral_t *)malloc(sizeof(dt_bilateral_t));
    dt_b->size_x = ireg[0]->valid.width;
    dt_b->size_y = ireg[0]->valid.height;
    dt_b->size_z = ireg[0]->im->Bands;
    dt_b->buf = (float*)malloc(dt_b->size_x * dt_b->size_y * dt_b->size_z * sizeof(float));

    T* pin  = (T*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top );
    T* pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top );
    int ilskip = VIPS_REGION_LSKIP( ireg[0] ) / sizeof(T);
    int olskip = VIPS_REGION_LSKIP( oreg ) / sizeof(T);
    int ilsz = VIPS_REGION_SIZEOF_LINE(ireg[0]);
    int olsz = VIPS_REGION_SIZEOF_LINE(oreg);
    int lsz = dt_b->size_x * dt_b->size_z;

    T* p = dt_b->buf;
    for( y = 0; y < ireg[0]->valid.height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top + y );
      memcpy(p, pin, lsz * sizeof(float) );
      p += lsz;
    }

    dt_bilateral_blur(dt_b);

    p = dt_b->buf + 2*lsz + 2*dt_b->size_z;
    for( y = 0; y < height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      memcpy(pout, p, olsz );
      p += lsz;
    }

    free(dt_b->buf); free(dt_b);
  }
};
