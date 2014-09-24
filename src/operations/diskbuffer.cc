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
#include <string.h>

#include <stdlib.h>

#include <iostream>

#include "../base/processor.hh"
#include "diskbuffer.hh"


PF::DiskBufferPar::DiskBufferPar():
	OpParBase()
{
}


VipsImage* PF::DiskBufferPar::build(std::vector<VipsImage*>& in, int first, 
																		VipsImage* imap, VipsImage* omap, 
																		unsigned int& level)
{
  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  return out;
}



PF::ProcessorBase* PF::new_diskbuffer()
{
  return( new PF::Processor<PF::DiskBufferPar,PF::DiskBufferProc>() );
}
