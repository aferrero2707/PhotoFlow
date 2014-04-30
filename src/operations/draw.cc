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

#include "draw.hh"

PF::DrawPar::DrawPar(): 
  BlenderPar(),
  pen_grey( "pen_grey", this, 0 ),
  pen_R( "pen_R", this, 1 ),
  pen_G( "pen_G", this, 1 ),
  pen_B( "pen_B", this, 1 ),
  pen_L( "pen_L", this, 1 ),
  pen_a( "pen_a", this, 1 ),
  pen_b( "pen_b", this, 1 ),
  pen_C( "pen_C", this, 1 ),
  pen_M( "pen_M", this, 1 ),
  pen_Y( "pen_Y", this, 1 ),
  pen_K( "pen_K", this, 1 ),
  bgd_grey( "bgd_grey", this, 0 ),
  bgd_R( "bgd_R", this, 0 ),
  bgd_G( "bgd_G", this, 0 ),
  bgd_B( "bgd_B", this, 0 ),
  bgd_L( "bgd_L", this, 0 ),
  bgd_a( "bgd_a", this, 0 ),
  bgd_b( "bgd_b", this, 0 ),
  bgd_C( "bgd_C", this, 0 ),
  bgd_M( "bgd_M", this, 0 ),
  bgd_Y( "bgd_Y", this, 0 ),
  bgd_K( "bgd_K", this, 0 ),
  pen_color( "pen_color", this, RGBColor(1,1,1) ),
  bgd_color( "bgd_color", this ),
  pen_size( "pen_size", this, 5 ),
  pen_opacity( "pen_opacity", this, 1 ),
  strokes( "strokes", this ),
  rawbuf(NULL)
{
  set_type( "draw" );
}


PF::DrawPar::~DrawPar()
{
  if( rawbuf )
    delete( rawbuf );
}


void PF::DrawPar::init_buffer( unsigned int level )
{
  if( !rawbuf ) return;

  std::vector<float> bgdcol;
  switch( get_colorspace() ) {
  case PF::PF_COLORSPACE_GRAYSCALE:
    bgdcol.push_back( bgd_color.get().r );
    break;
  case PF::PF_COLORSPACE_RGB:
    bgdcol.push_back( bgd_color.get().r );
    bgdcol.push_back( bgd_color.get().g );
    bgdcol.push_back( bgd_color.get().b );
    break;
  case PF::PF_COLORSPACE_LAB:
    bgdcol.push_back( bgd_L.get() );
    bgdcol.push_back( bgd_a.get() );
    bgdcol.push_back( bgd_b.get() );
    break;
  case PF::PF_COLORSPACE_CMYK:
    bgdcol.push_back( bgd_C.get() );
    bgdcol.push_back( bgd_M.get() );
    bgdcol.push_back( bgd_Y.get() );
    bgdcol.push_back( bgd_K.get() );
    break;
  default:
    break;
  }

  bool bgd_match = (rawbuf->get_bgd_color().size() == bgdcol.size()) ? true : false;
  if( bgd_match ) {
    for( unsigned int i = 0; i < bgdcol.size(); i++ ) {
      if( rawbuf->get_bgd_color()[i] != bgdcol[i] ) {
	bgd_match = false;
	break;
      }
    }
  }

  unsigned int cur_xsize = rawbuf->get_xsize();
  unsigned int cur_ysize = rawbuf->get_ysize();
  unsigned int new_xsize = get_xsize();
  unsigned int new_ysize = get_ysize();
  for( unsigned int l = 0; l < level; l++ ) {
    cur_xsize /= 2;
    cur_ysize /= 2;
    new_xsize *= 2;
    new_ysize *= 2;
  }

  if( (bgd_match == false) || 
      (cur_xsize != get_xsize()) ||
      (cur_ysize != get_ysize()) ||
      (rawbuf->get_nbands() != get_nbands()) ||
      (rawbuf->get_format() != get_format()) ) {
    rawbuf->set_xsize( new_xsize );
    rawbuf->set_ysize( new_ysize );
    rawbuf->set_nbands( get_nbands() );
    rawbuf->set_format( get_format() );
    rawbuf->set_interpretation( get_interpretation() );

    rawbuf->init( bgdcol );

    VipsRect update;
    std::list<Stroke>::iterator si;
    for( si = strokes.get().begin(); si != strokes.get().end(); si++ ) {
      Stroke& stroke = *si;
      Pen& pen = stroke.get_pen();
      rawbuf->start_stroke();
      std::list< std::pair<unsigned int, unsigned int> >::iterator pi;
      for( pi = stroke.get_points().begin(); pi != stroke.get_points().end(); pi++ ) {
	rawbuf->draw_point( pen, pi->first, pi->second, update, false );
      }
      rawbuf->end_stroke();
    }

    //rawbuf->get_pyramid().reset();
  }
}



VipsImage* PF::DrawPar::build(std::vector<VipsImage*>& in, int first, 
			      VipsImage* imap, VipsImage* omap, 
			      unsigned int& level)
{
  if( !rawbuf ) {
    char* fname = tempnam( NULL, "pfraw" );
    if( fname ) {
      rawbuf = new PF::RawBuffer( fname );
    }
  }
  if( !rawbuf ) 
    return NULL;

  init_buffer( level );

  PF::PyramidLevel* l = rawbuf->get_pyramid().get_level( level );
  if( l ) {
    std::vector<VipsImage*> in2;
    if( in.size() > 0 ) {
      in2.push_back( in[0] );
      in2.push_back( l->image );
    } else {
      in2.push_back( NULL );
      in2.push_back( l->image );
    }
    VipsImage* out = PF::BlenderPar::build( in2, 0, NULL, omap, level );
    g_object_unref( l->image );
    return out;
  }
  return NULL;
}



void PF::DrawPar::start_stroke( unsigned int pen_size, float opacity )
{
  strokes.get().push_back( PF::Stroke() );

  PF::Stroke& stroke = strokes.get().back();

  PF::Pen& pen = stroke.get_pen();
  pen.set_size( pen_size );
  pen.set_opacity( opacity );

  switch( get_colorspace() ) {
  case PF::PF_COLORSPACE_GRAYSCALE:
    pen.set_channel( 0, pen_color.get().r );
    break;
  case PF::PF_COLORSPACE_RGB:
    pen.set_channel( 0, pen_color.get().r );
    pen.set_channel( 1, pen_color.get().g );
    pen.set_channel( 2, pen_color.get().b );
    break;
  case PF::PF_COLORSPACE_LAB:
    pen.set_channel( 0, pen_L.get() );
    pen.set_channel( 1, pen_a.get() );
    pen.set_channel( 2, pen_b.get() );
    break;
  case PF::PF_COLORSPACE_CMYK:
    pen.set_channel( 0, pen_C.get() );
    pen.set_channel( 1, pen_M.get() );
    pen.set_channel( 2, pen_Y.get() );
    pen.set_channel( 3, pen_K.get() );
    break;
  default:
    break;
  }

  if( rawbuf )
    rawbuf->start_stroke();
}



void PF::DrawPar::end_stroke()
{
}



void PF::DrawPar::draw_point( unsigned int x, unsigned int y, VipsRect& update )
{
  PF::Stroke& stroke = strokes.get().back();

  if( !stroke.get_points().empty() ) {
    if( (stroke.get_points().back().first == x ) &&
	(stroke.get_points().back().second == y ) )
      return;
  }

  stroke.get_points().push_back( std::make_pair(x, y) );

  PF::Pen& pen = stroke.get_pen();

  if( rawbuf )
    rawbuf->draw_point( pen, x, y, update, true );
}



PF::ProcessorBase* PF::new_draw()
{
  return( new PF::Processor<PF::DrawPar,PF::DrawProc>() );
}
