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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/processor_imp.hh"
#include "median_filter.hh"



PF::MedianFilterPar::MedianFilterPar():
PaddedOpPar(),
radius("radius",this,40.0),
threshold("threshold",this,0.1),
fast_approx("fast_approx",this,true),
convert_to_perceptual(true)
{
  set_type("median_filter" );

  set_default_name( _("median filter") );
}


bool PF::MedianFilterPar::needs_caching()
{
  return true;
}




void PF::MedianFilterPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ )
    radius2 /= 2;

  int padding = radius2;

  set_padding( padding, id );
}


VipsImage* PF::MedianFilterPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  icc_data = NULL;
  if(in.size()>0 && in[0]) icc_data = PF::get_icc_profile( in[0] );

  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ )
    radius2 /= 2;

  radius_real = radius2;

  std::cout<<"MedianFilterPar::build: radius="<<radius_real<<"  threshold="<<threshold.get()<<std::endl;
  VipsImage* out = PF::PaddedOpPar::build( in, first, imap, omap, level );
  //std::cout<<"MedianFilterPar::build: out="<<out<<std::endl;
  return out;
}


PF::ProcessorBase* PF::new_median_filter()
{ return( new PF::Processor<PF::MedianFilterPar,PF::MedianFilterProc>() ); }

