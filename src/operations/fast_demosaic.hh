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

#ifndef FAST_DEMOSAIC_H
#define FAST_DEMOSAIC_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../base/rawmatrix.hh"


// bit representations of flags
#define PF_LUT_CLIP_BELOW 1
#define PF_LUT_CLIP_ABOVE 2

#define PF_LUTf PF::LUT<float>
#define PF_LUTi PF::LUT<int>
#define PF_LUTu PF::LUT<unsigned int>
#define PF_LUTd PF::LUT<double>

#include <cstring>

namespace PF
{

template<typename T>
class LUT {
private:
  // list of variables ordered to improve cache speed
  unsigned int maxs;
  T * data;
  unsigned int clip, size, owner;
public:
  LUT(int s, int flags = 0xfffffff) {
    clip = flags;
    data = new T[s];
    owner = 1;
    size = s;
    maxs=size-2;
  }
  void operator ()(int s, int flags = 0xfffffff) {
    if (owner&&data)
      delete[] data;
    clip = flags;
    data = new T[s];
    owner = 1;
    size = s;
    maxs=size-2;
  }

  LUT(int s, T * source, int flags = 0xfffffff) {
    clip = flags;
    data = new T[s];
    owner = 1;
    size = s;
    maxs=size-2;
    for (int i = 0; i < s; i++) {
      data[i] = source[i];
    }
  }

  LUT(void) {
    data = NULL;
    reset();
  }

  ~LUT() {
    if (owner)
      delete[] data;
  }

  void setClip(int flags) {
    clip = flags;
  }

  LUT<T> & operator=(const LUT<T> &rhs) {
    if (this != &rhs) {
      if (rhs.size>this->size)
      {
        delete [] this->data;
        this->data=NULL;
      }
      if (this->data==NULL) this->data=new T[rhs.size];
      this->clip=rhs.clip;
      this->owner=1;
      memcpy(this->data,rhs.data,rhs.size*sizeof(T));
      this->size=rhs.size;
      this->maxs=this->size-2;
    }

    return *this;
  }
  // use with integer indices
  T& operator[](int index) const {
    if (((unsigned int)index)<size) return data[index];
    else
    {
      if (index < 0)
        return data[0];
      else
        return data[size - 1];
    }

  }
  // use with float indices
  T operator[](float index) const {
    int idx = (int)index;  // don't use floor! The difference in negative space is no problems here
    if (((unsigned int)idx) > maxs) {
      if (idx<0)
      {
        if (clip & PF_LUT_CLIP_BELOW)
          return data[0];
        idx=0;
      }
      else
      {
        if (clip & PF_LUT_CLIP_ABOVE)
          return data[size - 1];
        idx =maxs;
      }
    }
    float diff = index - (float) idx;
    T p1 = data[idx];
    T p2 = data[idx + 1]-p1;
    return (p1 + p2*diff);
  }


  operator bool (void) const
        {
    return size>0;
        }

  void clear(void) {
    memset(data, 0, size * sizeof(T));
  }

  void reset(void) {
    delete[] data;
    data = NULL;
    owner = 1;
    size = 0;
    maxs=0;
  }
};


static const int MAXVAL = 0xffff;
static const float MAXVALF = float(MAXVAL);  // float version of MAXVAL
static const double MAXVALD = double(MAXVAL); // double version of MAXVAL

template <typename _Tp>
inline const _Tp SQR (_Tp x) {
  //		return std::pow(x,2); Slower than:
  return (x*x);
}

template<typename _Tp>
inline const _Tp& min(const _Tp& a, const _Tp& b) {
  return std::min(a,b);
}

template<typename _Tp>
inline const _Tp& max(const _Tp& a, const _Tp& b) {
  return std::max(a,b);
}


template<typename _Tp>
inline const _Tp LIM(const _Tp& a, const _Tp& b, const _Tp& c) {
  return std::max(b,std::min(a,c));
}

template<typename _Tp>
inline const _Tp LIM01(const _Tp& a) {
  return std::max(_Tp(1),std::min(a,_Tp(0)));
}

template<typename _Tp>
inline const _Tp ULIM(const _Tp& a, const _Tp& b, const _Tp& c) {
  return ((b < c) ? LIM(a,b,c) : LIM(a,c,b));
}

template<typename _Tp>
inline const _Tp CLIP(const _Tp& a) {
  return LIM(a, static_cast<_Tp>(0), static_cast<_Tp>(MAXVAL));
}


template<typename _Tp>
inline const _Tp& min(const _Tp& a, const _Tp& b, const _Tp& c) {
  return std::min(c,std::min(a,b));
}

template<typename _Tp>
inline const _Tp& max(const _Tp& a, const _Tp& b, const _Tp& c) {
  return std::max(c,std::max(a,b));
}

template<typename _Tp>
inline const _Tp& min(const _Tp& a, const _Tp& b, const _Tp& c, const _Tp& d) {
  return std::min(d,std::min(c,std::min(a,b)));
}

template<typename _Tp>
inline const _Tp& max(const _Tp& a, const _Tp& b, const _Tp& c, const _Tp& d) {
  return std::max(d,std::max(c,std::max(a,b)));
}




class FastDemosaicPar: public OpParBase
{
  PF_LUTf invGrad;

public:
  FastDemosaicPar();
  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_input() { return true; }

  PF_LUTf& get_inv_grad() { return invGrad; }

  void set_image_hints( VipsImage* img )
  {
    if( !img ) return;
    OpParBase::set_image_hints( img );
    rgb_image( get_xsize(), get_ysize() );
    set_demand_hint( VIPS_DEMAND_STYLE_FATSTRIP );
  }

  /* Function to derive the output area from the input area
   */
   virtual void transform(const VipsRect* rin, VipsRect* rout, int /*id*/)
   {
     rout->left = rin->left+5;
     rout->top = rin->top+5;
     rout->width = rin->width-10;
     rout->height = rin->height-10;
   }

   /* Function to derive the area to be read from input images,
       based on the requested output area
    */
   virtual void transform_inv(const VipsRect* rout, VipsRect* rin, int /*id*/)
   {
     rin->left = rout->left-5;
     rin->top = rout->top-5;
     rin->width = rout->width+10;
     rin->height = rout->height+10;
   }



   VipsImage* build(std::vector<VipsImage*>& in, int first,
       VipsImage* imap, VipsImage* omap,
       unsigned int& level);
};



void fast_demosaic(VipsRegion** ir, int n, int in_first,
    VipsRegion* imap, VipsRegion* omap,
    VipsRegion* oreg, FastDemosaicPar* par);


template < OP_TEMPLATE_DEF >
class FastDemosaicProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, FastDemosaicPar* par)
  {
    //fast_demosaic( in, n, in_first,
    //	     imap, omap, out, par );
  }
};


template < OP_TEMPLATE_DEF_TYPE_SPEC >
class FastDemosaicProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, FastDemosaicPar* par)
  {
    fast_demosaic( in, n, in_first,
        imap, omap, out, par );
  }
};


ProcessorBase* new_fast_demosaic();
}


#endif
