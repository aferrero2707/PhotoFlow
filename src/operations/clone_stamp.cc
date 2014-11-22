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

#include "clone_stamp.hh"

PF::CloneStampPar::CloneStampPar(): 
  OpParBase(),
  stamp_size( "stamp_size", this, 5 ),
  stamp_opacity( "stamp_opacity", this, 1 ),
  stamp_smoothness( "stamp_smoothness", this, 0 ),
  strokes( "strokes", this )
{
  set_type( "clone_stamp" );
	//diskbuf = new_diskbuffer();
}


PF::CloneStampPar::~CloneStampPar()
{
}


VipsImage* PF::CloneStampPar::build(std::vector<VipsImage*>& in, int first, 
															VipsImage* imap, VipsImage* omap, 
															unsigned int& level)
{
  scale_factor = 1;
  for(unsigned int l = 0; l < level; l++ ) {
    scale_factor *= 2;
  }
  return OpParBase::build( in, first, imap, omap, level );
}



void PF::CloneStampPar::start_stroke( unsigned int pen_size, float opacity, float smoothness )
{
  if( strokes.get().empty() ) return;

  PF::StrokesGroup& group = strokes.get().back();

  group.get_strokes().push_back( PF::Stroke<PF::Stamp>() );
  PF::Stroke<PF::Stamp>& stroke = group.get_strokes().back();

  PF::Stamp& pen = stroke.get_pen();
  pen.set_size( pen_size );
  pen.set_opacity( opacity );
  pen.set_smoothness( smoothness );
}



void PF::CloneStampPar::end_stroke()
{
}



void PF::CloneStampPar::draw_point( unsigned int x, unsigned int y, VipsRect& update )
{
  if( strokes.get().empty() ) return;

  PF::StrokesGroup& group = strokes.get().back();

  group.get_strokes().push_back( PF::Stroke<PF::Stamp>() );
  PF::Stroke<PF::Stamp>& stroke = group.get_strokes().back();

  if( !stroke.get_points().empty() ) {
    if( (stroke.get_points().back().first == x ) &&
				(stroke.get_points().back().second == y ) )
      return;
  }

  stroke.get_points().push_back( std::make_pair(x, y) );

  PF::Stamp& pen = stroke.get_pen();

  update.left = x - pen.get_size();
  update.top = y - pen.get_size();
  update.width = update.height = pen.get_size()*2 + 1;
}



PF::ProcessorBase* PF::new_clone_stamp()
{
  return( new PF::Processor<PF::CloneStampPar,PF::CloneStampProc>() );
}
