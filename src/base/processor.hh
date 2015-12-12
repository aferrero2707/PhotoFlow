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

#ifndef VIPS_PROCESSOR_H
#define VIPS_PROCESSOR_H


#include "operation.hh"
#include "blender.hh"
#include "photoflow.hh"


namespace PF
{


  class ProcessorBase
  {
    OpParBase* op_par_base;
  public:
    ProcessorBase(OpParBase* p): op_par_base(p) {}
    virtual ~ProcessorBase() 
    {
      std::cout<<"~ProcessorBase(): deleting "<<(void*)op_par_base<<std::endl;
      //delete op_par_base;
    }


    OpParBase* get_par() { return op_par_base; }

    virtual void process(VipsRegion** in, int n, int in_first,
			 VipsRegion* imap, VipsRegion* omap, 
			 VipsRegion* out) = 0;  
  };

  template< class OPPAR, template < OP_TEMPLATE_DEF > class OP >
  class Processor: public ProcessorBase
  {
    OPPAR op_par;

  public:
    Processor(): ProcessorBase(&op_par) { op_par.set_processor(this); }
    OPPAR* get_par() { return &op_par; }
    //virtual OpParBase* get_params() { return &op_params; };
    virtual void process(VipsRegion** in, int n, int in_first,
			 VipsRegion* imap, VipsRegion* omap, 
			 VipsRegion* out);  
  };



#define MAPFLAG_SWITCH( TYPE, CS, CHMIN, CHMAX ) {	\
        switch(mapflag) { \
        case 0:								\
					if(get_par()->get_render_mode() == PF_RENDER_NORMAL) {					\
						OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, false >, CS, CHMIN, CHMAX, false, false, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, false >, CS, CHMIN, CHMAX, false, false, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 1:								\
					if(get_par()->get_render_mode() == PF_RENDER_NORMAL) {					\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, false >, CS, CHMIN, CHMAX, true, false, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, false >, CS, CHMIN, CHMAX, true, false, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 2:								\
					if(get_par()->get_render_mode() == PF_RENDER_NORMAL) {					\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, true >, CS, CHMIN, CHMAX, false, true, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, true >, CS, CHMIN, CHMAX, false, true, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 3:								\
					if(get_par()->get_render_mode() == PF_RENDER_NORMAL) {					\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, true >, CS, CHMIN, CHMAX, true, true, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, Blender< TYPE, CS, CHMIN, CHMAX, true >, CS, CHMIN, CHMAX, true, true, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        }						\
}					


#define CS_SWITCH( TYPE ) {			\
  switch(colorspace) {							\
  case PF_COLORSPACE_RAW:						\
  case PF_COLORSPACE_GRAYSCALE:						\
    MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_GRAYSCALE, 0, 0 );	\
    break;								\
  case PF_COLORSPACE_RGB:						\
    switch( op_par.get_rgb_target_channel() ) {				\
    case 0:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_RGB, 0, 0 );	\
      break;								\
    case 1:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_RGB, 1, 1 );	\
      break;								\
    case 2:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_RGB, 2, 2 );	\
      break;								\
    default:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_RGB, 0, 2 );	\
      break;								\
    }									\
    break;								\
  case PF_COLORSPACE_LAB:						\
    switch( op_par.get_lab_target_channel() ) {				\
    case 0:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_LAB, 0, 0 );	\
      break;								\
    case 1:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_LAB, 1, 1 );	\
      break;								\
    case 2:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_LAB, 2, 2 );	\
      break;								\
    default:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_LAB, 0, 2 );	\
      break;								\
    }									\
    break;								\
  case PF_COLORSPACE_CMYK:						\
    switch( op_par.get_cmyk_target_channel() ) {			\
    case 0:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_CMYK, 0, 0 );	\
      break;								\
    case 1:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_CMYK, 1, 1 );	\
      break;								\
    case 2:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_CMYK, 2, 2 );	\
      break;								\
    case 3:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_CMYK, 3, 3 );	\
      break;								\
    default:								\
      MAPFLAG_SWITCH( TYPE, PF_COLORSPACE_CMYK, 0, 3 );	\
      break;								\
    }									\
    break;								\
  case PF_COLORSPACE_UNKNOWN:						\
    break;								\
  }									\
}


  template< class OPPAR, template < OP_TEMPLATE_DEF > class OP >
  void Processor<OPPAR,OP>::process(VipsRegion** in, int n, int in_first,
				    VipsRegion* imap, VipsRegion* omap, 
				    VipsRegion* out)
  {
    BandFormat fmt = PF_BANDFMT_UNKNOWN;
    colorspace_t colorspace = PF_COLORSPACE_UNKNOWN;

    fmt = (PF::BandFormat)op_par.get_format();
    //std::cout<<"Processor<OPPAR,OP>::process(): format = "<<fmt<<std::endl;
    colorspace = PF::convert_colorspace( op_par.get_interpretation() );

    /*    
    if(out) {
      fmt = (BandFormat)out->im->BandFmt;
      colorspace = PF::convert_colorspace(out->im->Type);
    } else {
      if(n > 0) {
	fmt = (BandFormat)in[0]->im->BandFmt;
	colorspace = PF::convert_colorspace(in[0]->im->Type);
	//interpret = in[0]->image->Type;
      }
    }
    */

    int mapflag = 0;
    if(imap) mapflag += 1;
    if(omap) mapflag += 2;

    /*
    std::cout<<"Processor::process(): "<<std::endl
	     <<"  fmt = "<<fmt<<std::endl
	     <<"  colorspace = "<<colorspace<<std::endl
	     //<<"  blend mode = "<<op_params.get_blend_mode()<<std::endl
	     <<"  imap = "<<imap<<"  omap = "<<omap<<std::endl
	     <<"  mapflag = "<<mapflag<<std::endl;
    */

    switch(fmt) {
    case PF_BANDFMT_UCHAR:
      CS_SWITCH( uint8_t );
      break;
    case PF_BANDFMT_USHORT:
      CS_SWITCH( unsigned short int );
      break;
    case PF_BANDFMT_FLOAT:
      CS_SWITCH( float );
      break;
    case PF_BANDFMT_CHAR:
    case PF_BANDFMT_SHORT:
    case PF_BANDFMT_UINT:
    case PF_BANDFMT_INT:
    case PF_BANDFMT_DOUBLE:
    case PF_BANDFMT_UNKNOWN:
      std::cout<<"Processor::process(): unhandled band format"<<std::endl;
      break;
    }
  }

}


#endif /*VIPS_PARITHMETIC_H*/


