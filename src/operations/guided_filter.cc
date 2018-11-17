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
#include "guided_filter.hh"


PF::GuidedFilterPar::GuidedFilterPar():
PaddedOpPar(),
radius("radius",this,4.0),
threshold("threshold",this,20.0)
{
  set_type("guided_filter" );

  set_default_name( _("guided filter") );
}


bool PF::GuidedFilterPar::needs_caching()
{
  return false;
}




void PF::GuidedFilterPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ )
    radius2 /= 2;

  int padding = radius2 * 2 + 1;

  set_padding( padding, id );
}


VipsImage* PF::GuidedFilterPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  return PF::PaddedOpPar::build( in, first, imap, omap, level );
}


PF::ProcessorBase* PF::new_guided_filter()
{ return( new PF::Processor<PF::GuidedFilterPar,PF::GuidedFilterProc>() ); }

