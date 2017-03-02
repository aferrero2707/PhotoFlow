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

int
vips_clone_stamp( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, int group_num, int stroke_num, ...);



PF::CloneStampPar::CloneStampPar(): 
  OpParBase(),
  stamp_size( "stamp_size", this, 5 ),
  stamp_opacity( "stamp_opacity", this, 1 ),
  stamp_smoothness( "stamp_smoothness", this, 1 ),
  strokes( "strokes", this )
{
  mutex = vips_g_mutex_new();
  set_type( "clone_stamp" );

  set_default_name( _("clone stamp") );
	//diskbuf = new_diskbuffer();
}


PF::CloneStampPar::~CloneStampPar()
{
}


VipsImage* PF::CloneStampPar::build(std::vector<VipsImage*>& in, int first, 
				VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  scale_factor = 1;
  for(unsigned int l = 0; l < level; l++ ) {
    scale_factor *= 2;
  }

  //std::cout<<"CloneStampPar::build(): stamp_size="<<stamp_size.get()<<std::endl;

  if( (in.size() < 1) || (in[0] == NULL) )
    return NULL;

  VipsImage* outnew = in[0];
  PF_REF( outnew, "CloneStampPar::build(): initial outnew ref" );

  lock();
  for( unsigned int i = 0; i < strokes.get().size(); i++) {
    PF::StrokesGroup& group = strokes.get()[i];
    unsigned int j = 0;
    //for( unsigned int j = 0; j < group.get_strokes().size(); j++ ) {
      //PF::Stroke<PF::Stamp>& stroke = group.get_strokes()[j];
      VipsImage* tempimg = outnew;
      if( vips_clone_stamp( tempimg, &outnew, get_processor(), i, j,NULL ) )
        return NULL;
      PF_UNREF( tempimg, "CloneStampPar::build(): tempimg unref" );
      //std::cout<<"CloneStampPar::build(): stroke "<<i<<","<<j<<" built"<<std::endl;
    //}
  }
  unlock();

  /*
#ifndef NDEBUG    
  std::cout<<"OpParBase::build(): type="<<type<<"  format="<<get_format()<<std::endl
	   <<"input images:"<<std::endl;
  for(int i = 0; i < n; i++) {
    std::cout<<"  "<<(void*)invec[i]<<"   ref_count="<<G_OBJECT( invec[i] )->ref_count<<std::endl;
  }
  std::cout<<"imap: "<<(void*)imap<<std::endl<<"omap: "<<(void*)omap<<std::endl;
  std::cout<<"out: "<<(void*)outnew<<std::endl<<std::endl;
#endif
  */

  //set_image( outnew );
#ifndef NDEBUG    
  std::cout<<"OpParBase::build(): outnew refcount ("<<(void*)outnew<<") = "<<G_OBJECT(outnew)->ref_count<<std::endl;
#endif
  return outnew;
}




void PF::CloneStampPar::new_group( int drow, int dcol )
{
  strokes.get().push_back( PF::StrokesGroup() );
  PF::StrokesGroup& group = strokes.get().back();

  group.set_delta_row( drow );
  group.set_delta_col( dcol );
  std::cout<<"CloneStampPar::new_group(): Drow="<<drow<<"  Dcol="<<dcol<<std::endl;
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

  //std::cout<<"CloneStampPar::start_stroke(): pen size="<<pen.get_size()<<"  opacity="<<pen.get_opacity()
  //         <<"  smoothness="<<pen.get_smoothness()<<std::endl;
}



void PF::CloneStampPar::end_stroke()
{
}



void PF::CloneStampPar::draw_point( unsigned int x, unsigned int y, VipsRect& update )
{
  update.width = update.height = update.left = update.top = 0;

  if( strokes.get().empty() ) return;

  PF::StrokesGroup& group = strokes.get().back();
  if( group.get_strokes().empty() ) return;

  //group.get_strokes().push_back( PF::Stroke<PF::Stamp>() );
  PF::Stroke<PF::Stamp>& stroke = group.get_strokes().back();

  if( !stroke.get_points().empty() ) {
    if( (stroke.get_points().back().first == (int)x ) &&
				(stroke.get_points().back().second == (int)y ) )
      return;
  }

  stroke.get_points().push_back( std::make_pair(x, y) );

  strokes.modified();

  PF::Stamp& pen = stroke.get_pen();

  update.left = x - pen.get_size();
  update.top = y - pen.get_size();
  update.width = update.height = pen.get_size()*2 + 1;
}



PF::ProcessorBase* PF::new_clone_stamp()
{
  return( new PF::Processor<PF::CloneStampPar,PF::CloneStampProc>() );
}
