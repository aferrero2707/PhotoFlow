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

#include "../base/processor_imp.hh"
#include "clahe.hh"


PF::CLAHEPar::CLAHEPar():
  OpParBase(),
  width("width",this,20),
  slope("slope",this,3)
{
  set_type( "clahe" );

  set_default_name( _("CLAHE") );
}



VipsImage* PF::CLAHEPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
	if( srcimg == NULL ) return NULL;
	VipsImage* out;
  VipsImage* tmpimg;
  VipsImage* ucharimg;
  VipsImage* claheimg;


  if( vips_linear1(srcimg, &tmpimg, 255, 0, NULL) ) {
    std::cout<<"CLAHEPar::build(): vips_linear1() failed"<<std::endl;
    return NULL;
  }


  if( vips_cast_uchar(tmpimg, &ucharimg, NULL) ) {
    std::cout<<"CLAHEPar::build(): vips_cast_uchar() failed"<<std::endl;
    return NULL;
  }


  if( vips_hist_local( ucharimg, &claheimg, width.get(), width.get(), "max_slope", slope.get(), NULL ) ) {
    std::cout<<"WARNING: CLAHEPar::build(): vips_hist_local() failed."<<std::endl;
    return NULL;
  }


  if( vips_linear1(claheimg, &tmpimg, 1.0f/255.0f, 0, NULL) ) {
    std::cout<<"CLAHEPar::build(): vips_linear1() failed"<<std::endl;
    return NULL;
  }


  if( vips_cast_float(tmpimg, &out, NULL) ) {
    std::cout<<"CLAHEPar::build(): vips_cast_float() failed"<<std::endl;
    return NULL;
  }
  return out;
}



PF::ProcessorBase* PF::new_clahe()
{ return( new PF::Processor<PF::CLAHEPar,PF::CLAHEProc>() ); }
