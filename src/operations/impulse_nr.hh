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

#ifndef PF_VOLUME_H
#define PF_VOLUME_H

#include "../base/splinecurve.hh"
#include "../base/processor.hh"

namespace PF 
{

class ImpulseNRPar: public OpParBase
{
  Property<float> threshold;

  ProcessorBase* convert2lab;
  ProcessorBase* gauss_blur;
  ProcessorBase* impulse_nr_algo;

public:
  ImpulseNRPar();

  bool has_intensity() { return false; }
  bool needs_caching() { return false; }

  float get_threshold() { return threshold.get(); }
  void set_threshold( float t ) { threshold.set(t); }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



class ImpulseNR_RTAlgo_Par: public OpParBase
{
  Property<float> threshold;

public:
  ImpulseNR_RTAlgo_Par(): OpParBase(), threshold("threshold",this,0.5) {set_type("impulse_nr_RT");}

  bool has_intensity() { return false; }
  bool needs_caching() { return false; }

  int get_padding() { return 2; }

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


  float get_threshold() { return threshold.get(); }
  void set_threshold( float t ) { threshold.update(t); }
};



template < OP_TEMPLATE_DEF >
class ImpulseNRProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
    std::cout<<"ImpulseNRProc::render() called."<<std::endl;
  }
};


template < OP_TEMPLATE_DEF >
class ImpulseNR_RTAlgo_Proc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
    std::cout<<"ImpulseNR_RTAlgo_Proc::render() called."<<std::endl;
  }
};


template < OP_TEMPLATE_DEF_CS_SPEC >
class ImpulseNR_RTAlgo_Proc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    //std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB) called: n="<<n<<std::endl;
    if( n != 3 ) return;
    if( ireg[0] == NULL ) {std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB): ireg[0]==NULL"<<std::endl;return;}
    if( ireg[1] == NULL ) {std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB): ireg[1]==NULL"<<std::endl;return;}
    if( ireg[2] == NULL ) {std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB): ireg[2]==NULL"<<std::endl;return;}

    int iwidth = ireg[0]->valid.width;
    int iheight = ireg[0]->valid.height;

    //The cleaning algorithm starts here

    ImpulseNR_RTAlgo_Par* opar = dynamic_cast<ImpulseNR_RTAlgo_Par*>(par);
    if( !opar ) {
      std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB): opar==NULL"<<std::endl;
      return;
    }

    Rect *ir = &ireg[0]->valid;
    VipsRect roi = {ir->left+2, ir->top+2, ir->width-4, ir->height-4};
    Rect *r = &oreg->valid;
    vips_rect_intersectrect( &roi, r, &roi );
    int line_size = roi.width * oreg->im->Bands;
    int width = roi.width;
    int height = roi.height;
    int dx = r->left - ir->left;
    int dy = r->top - ir->top;

    // buffer for the highpass image
    float ** impish = new float *[iheight];
    for (int i = 0; i < iheight; i++) {
      impish[i] = new float [iwidth];
      memset (impish[i], 0, iwidth*sizeof(float));
    }
    //std::cout<<"ImpulseNR_RTAlgo_Proc::render(RGB):"<<std::endl
    //    <<"ir: "<<ir->width<<","<<ir->height<<"+"<<ir->left<<"+"<<ir->top<<std::endl
    //    <<"r:  "<<r->width<<","<<r->height<<"+"<<r->left<<"+"<<r->top<<std::endl;

    T* pin0;
    T* pin1;
    T* pin2;
    T* pin20;
    T* pin21;
    T* pin22;
    T* pout;
    int x, y, oy, x1, y1, pos, ipos, pos1;
    //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;

    const float eps = 1.0;
    float impthr = MAX(1.0, 5.5 - opar->get_threshold());
    float impthrDiv24 = impthr / 24.0f;
    float hpfabs, hfnbrave;

    for( y = 2; y < iheight-2; y++ ) {
      pin1 = (T*)VIPS_REGION_ADDR( ireg[1], ir->left, ir->top + y );
      pin2 = (T*)VIPS_REGION_ADDR( ireg[2], ir->left, ir->top + y );

      for( x = 2; x < iwidth-2; x++ ) {
        //intensity = 0;
        hpfabs = fabs( (float)pin1[x] - (float)pin2[x] );

        //block average of high pass data
        for( y1 = y-2, hfnbrave = 0; y1 <= y+2; y1++ ) {
          pin21 = (T*)VIPS_REGION_ADDR( ireg[1], ir->left+x-2, ir->top + y1 );
          pin22 = (T*)VIPS_REGION_ADDR( ireg[2], ir->left+x-2, ir->top + y1 );
          for (x1 = 0; x1 <= 5; x1++) {
            if( y1 == y && x1 == 2) continue;
            hfnbrave += fabs( (float)pin21[x1] - (float)pin22[x1] );
          }
        }

        impish[y][x] = (hpfabs > ((hfnbrave - hpfabs) * impthrDiv24));
        //std::cout<<y<<","<<x<<"  hpfabs="<<hpfabs
        //    <<"  ((hfnbrave - hpfabs) * impthrDiv24)="<<((hfnbrave - hpfabs) * impthrDiv24)
        //    <<"  impish="<<impish[y][x]<<std::endl;
      }
    }

    //now impulsive values have been identified
    float wtdsum[3], dirwt, norm, delta;
    for( y = 2, oy = 2 - dy; y < iheight-2; y++, oy++ ) {
      pin0 = (T*)VIPS_REGION_ADDR( ireg[0], ir->left, ir->top + y );
      pin1 = (T*)VIPS_REGION_ADDR( ireg[1], ir->left, ir->top + y );
      pin2 = (T*)VIPS_REGION_ADDR( ireg[2], ir->left, ir->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + oy );

      for( x = 2, ipos = 2*ireg[0]->im->Bands, pos = (2-dx)*oreg->im->Bands;
          x < iwidth-2; x++, ipos += ireg[0]->im->Bands, pos += oreg->im->Bands ) {

        //std::cout<<"Filling "<<r->left+(pos/oreg->im->Bands)<<","<<r->top+oy<<std::endl;

        if( !impish[y][x] ) {
          pout[pos] = pin0[ipos];
          pout[pos+1] = pin0[ipos+1];
          pout[pos+2] = pin0[ipos+2];
          continue;
        }

        norm = 0.0;
        wtdsum[0] = wtdsum[1] = wtdsum[2] = 0.0;

        for( y1 = y-2; y1 <= y+2; y1++ ) {
          pin20 = (T*)VIPS_REGION_ADDR( ireg[0], ir->left+x-2, ir->top + y1 );
          pin21 = (T*)VIPS_REGION_ADDR( ireg[1], ir->left+x-2, ir->top + y1 );
          pin22 = (T*)VIPS_REGION_ADDR( ireg[2], ir->left+x-2, ir->top + y1 );
          for (x1 = 0, pos1 = 0; x1 <= 5; x1++, pos1 += ireg[0]->im->Bands) {
            if( y1 == y && x1 == 2)
              continue;
            if (impish[y1][x+x1-2])
              continue;

            delta = (float)pin21[x1] - (float)pin1[x];
            dirwt = 1.0f/( delta*delta + eps ); //use more sophisticated rangefn???
            //std::cout<<"pin21["<<x1<<"]="<<pin21[x1]<<"  pin1["<<x<<"]="<<pin1[x]<<std::endl;
            //std::cout<<"  SQR((float)pin21[x1] - (float)pin1[x])="<<SQR((float)pin21[x1] - (float)pin1[x])
            //    <<"  dirwt="<<dirwt<<std::endl;
            wtdsum[0] += dirwt * pin20[pos1];
            wtdsum[1] += dirwt * pin20[pos1+1];
            wtdsum[2] += dirwt * pin20[pos1+2];
            norm += dirwt;
          }
        }

        //std::cout<<"norm="<<norm<<std::endl;

        if (norm) {
          pout[pos] = wtdsum[0] / norm; //low pass filter
          pout[pos+1] = wtdsum[1] / norm; //low pass filter
          pout[pos+2] = wtdsum[2] / norm; //low pass filter
        }
      }
    }
  }
};



ProcessorBase* new_impulse_nr();

}

#endif 


