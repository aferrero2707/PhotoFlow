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
      //std::cout<<"Deleting "<<(void*)op_par_base<<std::endl;
      //delete op_par_base;
    }


    OpParBase* get_par() { return op_par_base; }

    virtual void process(VipsRegion** in, int n, int in_first,
			 VipsRegion* imap, VipsRegion* omap, 
			 VipsRegion* out) = 0;  
  };

  template< template < OP_TEMPLATE_DEF > class OP, class OPPAR >
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



#define MAPFLAG_SWITCH( TYPE, CS, BLENDER ) {	\
        switch(mapflag) { \
        case 0:								\
	  if(PF::PhotoFlow::Instance().get_render_mode() == PF_RENDER_NORMAL) { \
	    OP< TYPE, BLENDER< TYPE, CS, false >, CS, false, false, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, BLENDER< TYPE, CS, false >, CS, false, false, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 1:								\
	  if(PF::PhotoFlow::Instance().get_render_mode() == PF_RENDER_NORMAL) { \
	    OP< TYPE, BLENDER< TYPE, CS, false >, CS, true, false, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, BLENDER< TYPE, CS, false >, CS, true, false, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 2:								\
	  if(PF::PhotoFlow::Instance().get_render_mode() == PF_RENDER_NORMAL) { \
	    OP< TYPE, BLENDER< TYPE, CS, true >, CS, false, true, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, BLENDER< TYPE, CS, true >, CS, false, true, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        case 3:								\
	  if(PF::PhotoFlow::Instance().get_render_mode() == PF_RENDER_NORMAL) { \
	    OP< TYPE, BLENDER< TYPE, CS, true >, CS, true, true, false > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  } else {							\
	    OP< TYPE, BLENDER< TYPE, CS, true >, CS, true, true, true > op; \
	    op.render(in,n,in_first,imap,omap,out,&op_par);		\
	  }								\
	  break; \
        }						\
}					


#define BLENDER_SWITCH( TYPE, CS ) {			\
        switch(op_par.get_blend_mode()) {		\
        case PF_BLEND_PASSTHROUGH:				\
          MAPFLAG_SWITCH( TYPE, CS, BlendPassthrough );		\
          break;							\
        case PF_BLEND_NORMAL:				\
          MAPFLAG_SWITCH( TYPE, CS, BlendNormal );		\
          break;							\
        case PF_BLEND_UNKNOWN:					\
          break;						\
	}							\
}


#define CS_SWITCH( TYPE ) {			\
        switch(colorspace) {				\
        case PF_COLORSPACE_GRAYSCALE:					\
          BLENDER_SWITCH( TYPE, PF_COLORSPACE_GRAYSCALE );		\
          break;								\
        case PF_COLORSPACE_RGB:					\
          BLENDER_SWITCH( TYPE, PF_COLORSPACE_RGB );			\
          break;								\
        case PF_COLORSPACE_RAW:					\
        case PF_COLORSPACE_LAB:					\
          BLENDER_SWITCH( TYPE, PF_COLORSPACE_LAB );			\
          break;								\
        case PF_COLORSPACE_CMYK:					\
          BLENDER_SWITCH( TYPE, PF_COLORSPACE_CMYK );			\
          break;								\
        case PF_COLORSPACE_UNKNOWN:					\
          break;							\
        }								\
}


  template< template < OP_TEMPLATE_DEF > class OP, class OPPAR >
  void Processor<OP,OPPAR>::process(VipsRegion** in, int n, int in_first,
			      VipsRegion* imap, VipsRegion* omap, 
			      VipsRegion* out)
  {
    BandFormat fmt = PF_BANDFMT_UNKNOWN;
    colorspace_t colorspace = PF_COLORSPACE_UNKNOWN;
    
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

    int mapflag = 0;
    if(imap) mapflag += 1;
    if(omap) mapflag += 2;

    /*
    std::cout<<"Processor::process(): "<<std::endl
	     <<"  fmt = "<<fmt<<std::endl
	     <<"  colorspace = "<<colorspace<<std::endl
	     <<"  blend mode = "<<op_params.get_blend_mode()<<std::endl
	     <<"  imap = "<<imap<<"  omap = "<<omap<<std::endl
	     <<"  mapflag = "<<mapflag<<std::endl;
    */

    switch(fmt) {
    case PF_BANDFMT_UCHAR:
      CS_SWITCH( uint8_t );
      break;
    case PF_BANDFMT_CHAR:
    case PF_BANDFMT_USHORT:
    case PF_BANDFMT_SHORT:
    case PF_BANDFMT_UINT:
    case PF_BANDFMT_INT:
    case PF_BANDFMT_FLOAT:
    case PF_BANDFMT_DOUBLE:
    case PF_BANDFMT_UNKNOWN:
      break;
    }
  }


}


#endif /*VIPS_PARITHMETIC_H*/


