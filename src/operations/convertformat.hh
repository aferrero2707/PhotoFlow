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

#ifndef CONVERT_FORMAT_H
#define CONVERT_FORMAT_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation.hh"

namespace PF 
{

  class ConvertFormatPar: public OpParBase
  {
  public:
    ConvertFormatPar();
    bool has_imap() { return false; }
    bool has_omap() { return false; }
    bool needs_input() { return true; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ConvertFormatProc
  {
    template<typename Tin, typename Tout>
    void convert(VipsRegion* ir, VipsRegion* oreg)
    {
      Rect *r = &oreg->valid;
      int width = r->width;
      int height = r->height;
      int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      
      Tin* p;    
      Tout* pout;
      int x, y;
      double norm;
      
      for( y = 0; y < height; y++ ) {
	
	p = (Tin*)VIPS_REGION_ADDR( ir, r->left, r->top + y ); 
	pout = (Tout*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	for( x=0; x < line_size; x++) {
	  norm = (double(p[x]) + FormatInfo<Tin>::MIN)/FormatInfo<Tin>::RANGE;
	  pout[x] = (Tout)(norm*FormatInfo<Tout>::RANGE - FormatInfo<Tout>::MIN);
	  std::cout<<"p[x]="<<(int)p[x]<<"  norm="<<norm<<"  pout[x]="<<pout[x]<<std::endl;
	}
      }
    }

  public: 
    void render(VipsRegion** in, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* out, ConvertFormatPar* par);
  };


  template< OP_TEMPLATE_DEF >
  void ConvertFormatProc< OP_TEMPLATE_IMP >::
  render(VipsRegion** ir, int n, int in_first,
	 VipsRegion* imap, VipsRegion* omap, 
	 VipsRegion* oreg, ConvertFormatPar* par)
  {
    //std::cout<<"ConvertFormatProc::render(): input format="<<ir[in_first]->im->BandFmt<<std::endl;

    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 
    
    T* pout;
    int x, y;
    double norm;
      
#ifndef NDEBUG
    if( r->top==0 && r->left==0 ) {
      std::cout<<"ConvertFormatProc::render(): input format="<<ir[in_first]->im->BandFmt<<std::endl;
    }
#endif
    switch( ir[in_first]->im->BandFmt ) {
    case VIPS_FORMAT_UCHAR:
      //convert<unsigned char, T>( ir[in_first], oreg );
      for( y = 0; y < height; y++ ) {
	
	unsigned char* p = (unsigned char*)VIPS_REGION_ADDR( ir[in_first], r->left, r->top + y ); 
	pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	for( x=0; x < line_size; x++) {
	  norm = (double(p[x]) + FormatInfo<unsigned char>::MIN)/FormatInfo<unsigned char>::RANGE;
	  pout[x] = (T)(norm*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
	  //std::cout<<"p[x]="<<(int)p[x]<<"  norm="<<norm<<"  pout[x]="<<pout[x]<<std::endl;
	}
#ifndef NDEBUG
	if( y==0 && r->top==0 && r->left==0 ) {
	  std::cout<<"ConvertFormatProc::render()"<<std::endl;
	  for( int i = 0; i < 12; i++ )
	    std::cout<<(int)p[i]<<" ";
	  std::cout<<std::endl;
	  for( int i = 0; i < 12; i++ )
	    std::cout<<(int)pout[i]<<" ";
	  std::cout<<std::endl;
	}
#endif
      }
      break;
    case VIPS_FORMAT_CHAR:
      //convert<signed char, T>( ir[in_first], oreg );
      break;
    case VIPS_FORMAT_USHORT:
      //convert<unsigned short, T>( ir[in_first], oreg );
      for( y = 0; y < height; y++ ) {
	
	unsigned short* p = (unsigned short*)VIPS_REGION_ADDR( ir[in_first], r->left, r->top + y ); 
	pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	for( x=0; x < line_size; x++) {
	  norm = (double(p[x]) + FormatInfo<unsigned short>::MIN)/FormatInfo<unsigned short>::RANGE;
	  pout[x] = (T)(norm*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
	  //std::cout<<"p[x]="<<(int)p[x]<<"  norm="<<norm<<"  pout[x]="<<pout[x]<<std::endl;
	}
#ifndef NDEBUG
	if( y==0 && r->top==0 && r->left==0 ) {
	  std::cout<<"ConvertFormatProc::render()"<<std::endl;
	  for( int i = 0; i < 12; i++ )
	    std::cout<<(int)p[i]<<" ";
	  std::cout<<std::endl;
	  for( int i = 0; i < 12; i++ )
	    std::cout<<(int)pout[i]<<" ";
	  std::cout<<std::endl;
	}
#endif
      }
      break;
    case VIPS_FORMAT_SHORT:
      //convert<signed short, T>( ir[in_first], oreg );
      break;
    case VIPS_FORMAT_UINT:
      //convert<unsigned int, T>( ir[in_first], oreg );
      break;
    case VIPS_FORMAT_INT:
      //convert<signed int, T>( ir[in_first], oreg );
      break;
    case VIPS_FORMAT_FLOAT:
      //convert<float, T>( ir[in_first], oreg );
      for( y = 0; y < height; y++ ) {	
	float* p = (float*)VIPS_REGION_ADDR( ir[in_first], r->left, r->top + y ); 
	pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	for( x=0; x < line_size; x++) {
	  pout[x] = (T)(p[x]*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
	}
      }
      break;    
    case VIPS_FORMAT_DOUBLE:
      //convert<double, T>( ir[in_first], oreg );
      for( y = 0; y < height; y++ ) {	
	double* p = (double*)VIPS_REGION_ADDR( ir[in_first], r->left, r->top + y ); 
	pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	for( x=0; x < line_size; x++) {
	  pout[x] = (T)(p[x]*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
	}
      }
      break;    
    default:
      break;
    }
  }


  ProcessorBase* new_convert_format();
}

#endif 


