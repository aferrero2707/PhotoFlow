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

#ifndef PF_GUIDED_FILTER_H
#define PF_GUIDED_FILTER_H

#include "../base/operation.hh"

#include "padded_op.hh"

namespace PF 
{

class GuidedFilterPar: public PaddedOpPar
{
  Property<float> radius;
  float radius_real;
  Property<float> threshold;
  Property<int> subsampling;
  int subsampling_real;

  ICCProfile* icc_data;
  Property<bool> convert_to_perceptual;
public:
  GuidedFilterPar();

  ICCProfile* get_icc_data() { return icc_data; }

  bool has_intensity() { return false; }
  bool needs_caching();
  void set_convert_to_perceptual(bool val) { convert_to_perceptual.set(val); }
  bool get_convert_to_perceptual() { return convert_to_perceptual.get(); }

  void set_radius(float r) { radius.set(r); }
  float get_radius() { return radius_real; }
  void set_threshold(float t) { threshold.set(t); }
  float get_threshold() { return threshold.get(); }

  void set_subsampling(int s) { subsampling.set(s); }
  int get_subsampling() { return subsampling_real; }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class GuidedFilterProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};


void guidedFilter(const PixelMatrix<float> &guide, const PixelMatrix<float> &src,
    PixelMatrix<float> &dst, int r, float epsilon, int subsampling);


template < OP_TEMPLATE_DEF_TYPE_SPEC >
class GuidedFilterProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
  ICCProfile* profile;
  GuidedFilterPar* opar;

  void fill_L_matrices(int rw, int rh, PixelMatrix<float>& rgbin,
      PixelMatrix<float>& Lin, PixelMatrix<float>& Lguide)
  {
    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* L = Lin[y];
      float* Lg = Lguide[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          //*L = (row[0]>0) ? powf( row[0], 1./2.4 ) : row[0];
          *L = (row[0]>1.0e-16) ? log10( row[0] ) : -16;
        } else {
          *L = row[0];
        }
        *Lg = *L;

        row += 1;
        L += 1;
        Lg += 1;
      }
    }
  }

  void fill_RGB_matrices(int rw, int rh, PixelMatrix<float>& rgbin,
      PixelMatrix<float>& rin, PixelMatrix<float>& gin, PixelMatrix<float>& bin,
      PixelMatrix<float>& rguide, PixelMatrix<float>& gguide, PixelMatrix<float>& bguide)
  {
    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* rr = rin[y];
      float* gr = gin[y];
      float* br = bin[y];
      float* rrg = rguide[y];
      float* grg = gguide[y];
      float* brg = bguide[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          //*rr = (row[0]>0) ? powf( row[0], 1./2.4 ) : row[0];
          //*gr = (row[1]>0) ? powf( row[1], 1./2.4 ) : row[1];
          //*br = (row[2]>0) ? powf( row[2], 1./2.4 ) : row[2];
          *rr = (row[0]>1.0e-16) ? log10( row[0] ) : -16;
          *gr = (row[1]>1.0e-16) ? log10( row[1] ) : -16;
          *br = (row[2]>1.0e-16) ? log10( row[2] ) : -16;
        } else {
          *rr = row[0];
          *gr = row[1];
          *br = row[2];
        }
        *rrg = *rr;
        *grg = *gr;
        *brg = *br;

        row += 3;
        rr += 1;
        gr += 1;
        br += 1;
        rrg += 1;
        grg += 1;
        brg += 1;
      }
    }
  }

  void fill_RGBL_matrices(int rw, int rh, PixelMatrix<float>& rgbin,
      PixelMatrix<float>& rin, PixelMatrix<float>& gin, PixelMatrix<float>& bin, PixelMatrix<float>& Lin,
      PixelMatrix<float>& rguide, PixelMatrix<float>& gguide, PixelMatrix<float>& bguide, PixelMatrix<float>& Lguide)
  {
    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* rr = rin[y];
      float* gr = gin[y];
      float* br = bin[y];
      float* Lr = Lin[y];
      float* rrg = rguide[y];
      float* grg = gguide[y];
      float* brg = bguide[y];
      float* Lrg = Lguide[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        if( opar->get_convert_to_perceptual() && profile && profile->is_linear() ) {
          //*rr = (row[0]>0) ? powf( row[0], 1./2.4 ) : row[0];
          //*gr = (row[1]>0) ? powf( row[1], 1./2.4 ) : row[1];
          //*br = (row[2]>0) ? powf( row[2], 1./2.4 ) : row[2];
          //*Lr = (row[3]>0) ? powf( row[3], 1./2.4 ) : row[3];
          *rr = (row[0]>1.0e-16) ? log10( row[0] ) : -16;
          *gr = (row[1]>1.0e-16) ? log10( row[1] ) : -16;
          *br = (row[2]>1.0e-16) ? log10( row[2] ) : -16;
          *Lr = (row[3]>1.0e-16) ? log10( row[3] ) : -16;
        } else {
          *rr = row[0];
          *gr = row[1];
          *br = row[2];
          *Lr = row[3];
        }
        *rrg = *rr;
        *grg = *gr;
        *brg = *br;
        *Lrg = *Lr;

        row += 4;
        rr += 1; gr += 1; br += 1; Lr += 1;
        rrg += 1; grg += 1; brg += 1; Lrg += 1;
      }
    }
  }

  void fill_RGB_out(int rw, int rh, int dx, int dy, PixelMatrix<float>& rgbout,
      PixelMatrix<float>& rout, PixelMatrix<float>& gout, PixelMatrix<float>& bout)
  {
    int x, y, z;

   for(y = 0; y < rh; y++) {
      float* rr = rout[y+dy];
      float* gr = gout[y+dy];
      float* br = bout[y+dy];
      float* row = rgbout[y];
      rr += dx; gr += dx; br += dx;
      for( x = 0; x < rw; x++ ) {
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          //row[0] = (*rr>0) ? powf( *rr, 2.4 ) : *rr;
          //row[1] = (*gr>0) ? powf( *gr, 2.4 ) : *gr;
          //row[2] = (*br>0) ? powf( *br, 2.4 ) : *br;
          row[0] = pow( 10, *rr );
          row[1] = pow( 10, *gr );
          row[2] = pow( 10, *br );
        } else {
          row[0] = *rr;
          row[1] = *gr;
          row[2] = *br;
        }
        row += 3;
        rr += 1;
        gr += 1;
        br += 1;
      }
    }
  }

  void fill_RGBL_out(int rw, int rh, int dx, int dy, PixelMatrix<float>& rgbout,
      PixelMatrix<float>& rout, PixelMatrix<float>& gout, PixelMatrix<float>& bout, PixelMatrix<float>& Lout)
  {
    int x, y, z;

   for(y = 0; y < rh; y++) {
      float* rr = rout[y+dy];
      float* gr = gout[y+dy];
      float* br = bout[y+dy];
      float* Lr = Lout[y+dy];
      float* row = rgbout[y];
      rr += dx; gr += dx; br += dx; Lr += dx;
      for( x = 0; x < rw; x++ ) {
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          //row[0] = (*rr>0) ? powf( *rr, 2.4 ) : *rr;
          //row[1] = (*gr>0) ? powf( *gr, 2.4 ) : *gr;
          //row[2] = (*br>0) ? powf( *br, 2.4 ) : *br;
          //row[3] = (*Lr>0) ? powf( *Lr, 2.4 ) : *Lr;
          row[0] = pow( 10, *rr );
          row[1] = pow( 10, *gr );
          row[2] = pow( 10, *br );
          row[3] = pow( 10, *Lr );
        } else {
          row[0] = *rr;
          row[1] = *gr;
          row[2] = *br;
          row[3] = *Lr;
        }
        row += 4;
        rr += 1;
        gr += 1;
        br += 1;
        Lr += 1;
      }
    }
  }

  void fill_L_out(int rw, int rh, int dx, int dy,
      PixelMatrix<float>& rgbout, PixelMatrix<float>& Lout)
  {
    int x, y, z;

   for(y = 0; y < rh; y++) {
      float* L = Lout[y+dy];
      float* row = rgbout[y];
      L += dx;
      for( x = 0; x < rw; x++ ) {
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          //row[0] = (*L>0) ? powf( *L, 2.4 ) : *L;
          row[0] = pow( 10, *L );
        } else {
          row[0] = *L;
        }
        row += 1;
        L += 1;
      }
    }
  }

public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    //std::cout<<"GuidedFilterProc::render: this="<<this<<"  in_first="<<in_first<<std::endl;

    if( n != 1 ) return;
    if( ireg[0] == NULL ) return;

    opar = dynamic_cast<GuidedFilterPar*>(par);
    if( !opar ) return;

    int es = VIPS_IMAGE_SIZEOF_ELEMENT( ireg[0]->im );
    int ips = VIPS_IMAGE_SIZEOF_PEL( ireg[0]->im );
    const int ops = VIPS_IMAGE_SIZEOF_PEL( oreg->im );

    //const int order = 1; // 0,1,2
    //const float radius = opar->get_radius();
    //const float thresfold = opar->get_threshold();

    int r = opar->get_radius();
    float epsilon = opar->get_threshold();
    int subsampling = opar->get_subsampling();

    profile = opar->get_icc_data();
    if( opar->get_convert_to_perceptual() && profile && profile->is_linear() ) epsilon *= 10;

    int offsx = 0;
    int offsy = 0;
    int shiftx = (ireg[0]->valid.left/subsampling+1) * subsampling - ireg[0]->valid.left;
    int shifty = (ireg[0]->valid.top/subsampling+1) * subsampling - ireg[0]->valid.top;
    int rw = ireg[0]->valid.width - shiftx - subsampling;
    rw = (rw/subsampling+1) * subsampling;
    int rh = ireg[0]->valid.height - shifty - subsampling;
    rh = (rh/subsampling+1) * subsampling;
    int ileft = ireg[0]->valid.left + shiftx;
    int itop = ireg[0]->valid.top + shifty;
    float* p = (float*)VIPS_REGION_ADDR( ireg[0], ileft, itop );
    int rowstride = VIPS_REGION_LSKIP(ireg[0]) / sizeof(float);
    PixelMatrix<float> rgbin(p, rw, rh, rowstride, offsy, offsx);

    /*
    std::cout<<"GuidedFilterProc::render: Bands="<<ireg[0]->im->Bands<<std::endl;
    std::cout<<"GuidedFilterProc::render: radius="<<r<<"  subsampling="<<subsampling<<std::endl;
    std::cout<<"GuidedFilterProc::render: ireg.left="<<ireg[0]->valid.left
        <<"  shiftx="<<shiftx<<std::endl;
    std::cout<<"GuidedFilterProc::render: ireg.top="<<ireg[0]->valid.top
        <<"  shifty="<<shifty<<std::endl;
    std::cout<<"GuidedFilterProc::render: ireg.width="<<ireg[0]->valid.width
        <<"  rw="<<rw<<std::endl;
    std::cout<<"GuidedFilterProc::render: ireg.height="<<ireg[0]->valid.height
        <<"  rh="<<rh<<std::endl;
    */
    if( ireg[0]->im->Bands == 1 ) {
      PixelMatrix<float> Lin(rw, rh, offsy, offsx);
      PixelMatrix<float> Lguide(rw, rh, offsy, offsx);
      PixelMatrix<float> Lout(rw, rh, offsy, offsx);

      //std::cout<<"GuidedFilterProc::render: splitting RGB channels"<<std::endl;

      fill_L_matrices( rw, rh, rgbin, Lin, Lguide );

      //std::cout<<"GuidedFilterProc::render: processing RGB channels"<<std::endl;
      //std::cout<<"                          processing R:"<<std::endl;
      guidedFilter(Lguide, Lin, Lout, r, epsilon, subsampling);

      int dx = oreg->valid.left - ireg[0]->valid.left - shiftx;
      int dy = oreg->valid.top - ireg[0]->valid.top - shifty;
      offsx = oreg->valid.left;
      offsy = oreg->valid.top;
      rw = oreg->valid.width;
      rh = oreg->valid.height;
      p = (float*)VIPS_REGION_ADDR( oreg, offsx, offsy );
      rowstride = VIPS_REGION_LSKIP(oreg) / sizeof(float);
      PF::PixelMatrix<float> rgbout(p, rw, rh, rowstride, 0, 0);

      //std::cout<<"GuidedFilterProc::render: r="<<r<<"  epsilon="<<epsilon<<"  subsampling="<<subsampling
      //    <<"  left="<<oreg->valid.left<<"  top="<<oreg->valid.top
      //    <<"  dx="<<dx<<"  dy="<<dy<<"  rw="<<rw<<"  rh="<<rh<<std::endl;

      fill_L_out(rw, rh, dx, dy, rgbout, Lout);
    }

    if( ireg[0]->im->Bands == 3 ) {
      PixelMatrix<float> rin(rw, rh, offsy, offsx);
      PixelMatrix<float> gin(rw, rh, offsy, offsx);
      PixelMatrix<float> bin(rw, rh, offsy, offsx);
      PixelMatrix<float> rguide(rw, rh, offsy, offsx);
      PixelMatrix<float> gguide(rw, rh, offsy, offsx);
      PixelMatrix<float> bguide(rw, rh, offsy, offsx);
      PixelMatrix<float> rout(rw, rh, offsy, offsx);
      PixelMatrix<float> gout(rw, rh, offsy, offsx);
      PixelMatrix<float> bout(rw, rh, offsy, offsx);

      //std::cout<<"GuidedFilterProc::render: splitting RGB channels"<<std::endl;

      fill_RGB_matrices( rw, rh, rgbin, rin, gin, bin, rguide, gguide, bguide );

      //std::cout<<"GuidedFilterProc::render: processing RGB channels"<<std::endl;
      //std::cout<<"                          processing R:"<<std::endl;
      guidedFilter(rguide, rin, rout, r, epsilon, subsampling);
      //std::cout<<"                          processing G:"<<std::endl;
      guidedFilter(gguide, gin, gout, r, epsilon, subsampling);
      //std::cout<<"                          processing B:"<<std::endl;
      guidedFilter(bguide, bin, bout, r, epsilon, subsampling);

      int dx = oreg->valid.left - ireg[0]->valid.left - shiftx;
      int dy = oreg->valid.top - ireg[0]->valid.top - shifty;
      offsx = oreg->valid.left;
      offsy = oreg->valid.top;
      rw = oreg->valid.width;
      rh = oreg->valid.height;
      p = (float*)VIPS_REGION_ADDR( oreg, offsx, offsy );
      rowstride = VIPS_REGION_LSKIP(oreg) / sizeof(float);
      PF::PixelMatrix<float> rgbout(p, rw, rh, rowstride, 0, 0);

      //std::cout<<"GuidedFilterProc::render: r="<<r<<"  epsilon="<<epsilon<<"  subsampling="<<subsampling
      //    <<"  dx="<<dx<<"  dy="<<dy<<"  rw="<<rw<<"  rh="<<rh<<std::endl;

      fill_RGB_out(rw, rh, dx, dy, rgbout, rout, gout, bout);
    }


    if( ireg[0]->im->Bands == 4 ) {
      PixelMatrix<float> rin(rw, rh, offsy, offsx);
      PixelMatrix<float> gin(rw, rh, offsy, offsx);
      PixelMatrix<float> bin(rw, rh, offsy, offsx);
      PixelMatrix<float> Lin(rw, rh, offsy, offsx);
      PixelMatrix<float> rguide(rw, rh, offsy, offsx);
      PixelMatrix<float> gguide(rw, rh, offsy, offsx);
      PixelMatrix<float> bguide(rw, rh, offsy, offsx);
      PixelMatrix<float> Lguide(rw, rh, offsy, offsx);
      PixelMatrix<float> rout(rw, rh, offsy, offsx);
      PixelMatrix<float> gout(rw, rh, offsy, offsx);
      PixelMatrix<float> bout(rw, rh, offsy, offsx);
      PixelMatrix<float> Lout(rw, rh, offsy, offsx);

      //std::cout<<"GuidedFilterProc::render: splitting RGB channels"<<std::endl;

      fill_RGBL_matrices( rw, rh, rgbin, rin, gin, bin, Lin, rguide, gguide, bguide, Lguide );

      //std::cout<<"GuidedFilterProc::render: processing RGB channels"<<std::endl;
      //std::cout<<"                          processing R:"<<std::endl;
      guidedFilter(rguide, rin, rout, r, epsilon, subsampling);
      //std::cout<<"                          processing G:"<<std::endl;
      guidedFilter(gguide, gin, gout, r, epsilon, subsampling);
      //std::cout<<"                          processing B:"<<std::endl;
      guidedFilter(bguide, bin, bout, r, epsilon, subsampling);
      //std::cout<<"                          processing L:"<<std::endl;
      guidedFilter(Lguide, Lin, Lout, r, epsilon, subsampling);

      int dx = oreg->valid.left - ireg[0]->valid.left;
      int dy = oreg->valid.top - ireg[0]->valid.top;
      offsx = oreg->valid.left;
      offsy = oreg->valid.top;
      rw = oreg->valid.width;
      rh = oreg->valid.height;
      p = (float*)VIPS_REGION_ADDR( oreg, offsx, offsy );
      rowstride = VIPS_REGION_LSKIP(oreg) / sizeof(float);
      PF::PixelMatrix<float> rgbout(p, rw, rh, rowstride, 0, 0);

      //std::cout<<"GuidedFilterProc::render: r="<<r<<"  epsilon="<<epsilon<<"  subsampling="<<subsampling
      //    <<"  dx="<<dx<<"  dy="<<dy<<"  rw="<<rw<<"  rh="<<rh<<std::endl;

      fill_RGBL_out(rw, rh, dx, dy, rgbout, rout, gout, bout, Lout);
    }

    /*
    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* rr = rin[y];
      float* gr = gin[y];
      float* br = bin[y];
      float* rrg = rguide[y];
      float* grg = gguide[y];
      float* brg = bguide[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          *rr = (row[0]>0) ? powf( row[0], 1./2.4 ) : row[0];
          *gr = (row[1]>0) ? powf( row[1], 1./2.4 ) : row[1];
          *br = (row[2]>0) ? powf( row[2], 1./2.4 ) : row[2];
        } else {
          *rr = row[0];
          *gr = row[1];
          *br = row[2];
        }
        if(false && profile && profile->is_linear()) {
          *rrg = (*rr>0) ? powf( *rr, 1./2.4 ) : *rr;
          *grg = (*gr>0) ? powf( *gr, 1./2.4 ) : *gr;
          *brg = (*br>0) ? powf( *br, 1./2.4 ) : *br;
        } else {
          *rrg = *rr;
          *grg = *gr;
          *brg = *br;
        }
        row += 3;
        rr += 1;
        gr += 1;
        br += 1;
        rrg += 1;
        grg += 1;
        brg += 1;
      }
    }

    //return;

    int dx = oreg->valid.left - ireg[0]->valid.left;
    int dy = oreg->valid.top - ireg[0]->valid.top;
    offsx = oreg->valid.left;
    offsy = oreg->valid.top;
    rw = oreg->valid.width;
    rh = oreg->valid.height;
    p = (float*)VIPS_REGION_ADDR( oreg, offsx, offsy );
    rowstride = VIPS_REGION_LSKIP(oreg) / sizeof(float);
    PF::PixelMatrix<float> rgbout(p, rw, rh, rowstride, 0, 0);

    if( ireg[0]->im.Bands == 3 ) {
    fill_RGB_out(rw, rh, dx, dy, rout, gout, bout, rgbout);
    }

    for(y = 0; y < rh; y++) {
      float* irow = rgbin[y+dy];
      float* rr = rout[y+dy];
      float* gr = gout[y+dy];
      float* br = bout[y+dy];
      float* row = rgbout[y];
      //float* rr = rin[y+dy];
      //float* gr = gin[y+dy];
      //float* br = bin[y+dy];
      rr += dx; gr += dx; br += dx; irow += dx*3;
      for( x = 0; x < rw; x++ ) {
        if( opar->get_convert_to_perceptual() &&
            profile && profile->is_linear() ) {
          row[0] = (*rr>0) ? powf( *rr, 2.4 ) : *rr;
          row[1] = (*gr>0) ? powf( *gr, 2.4 ) : *gr;
          row[2] = (*br>0) ? powf( *br, 2.4 ) : *br;
        } else {
          row[0] = *rr;
          row[1] = *gr;
          row[2] = *br;
        }
        //row[0] = irow[0];
        //row[1] = irow[1];
        //row[2] = irow[2];
        row += 3;
        irow += 3;
        rr += 1;
        gr += 1;
        br += 1;
      }
    }
    */
  }
};


ProcessorBase* new_guided_filter();

}

#endif 


