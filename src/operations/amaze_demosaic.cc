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

#include <stdlib.h>

#include "amaze_demosaic.hh"
#include "../base/processor.hh"

//#define RT_EMU 1


PF::AmazeDemosaicPar::AmazeDemosaicPar(): 
  OpParBase()
{
  set_demand_hint( VIPS_DEMAND_STYLE_ANY );
  set_type( "amaze_demosaic" );
}


VipsImage* PF::AmazeDemosaicPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  VipsImage* img = OpParBase::build( in, first, NULL, NULL, level );

  VipsImage* out;
  int bands = 3;
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_RGB;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  vips_copy( img, &out, 
	     "format", format,
	     "bands", bands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  //sprintf(tifname,"/tmp/level_%d-2.tif",(int)levels.size());
  //vips_image_write_to_file( out, tifname );
  //g_object_unref( img );
  PF_UNREF( img, "PF::AmazeDemosaicPar::build(): img unref" );

  return out;
}



PF::ProcessorBase* PF::new_amaze_demosaic()
{
  return( new PF::Processor<PF::AmazeDemosaicPar,PF::AmazeDemosaicProc>() );
}

