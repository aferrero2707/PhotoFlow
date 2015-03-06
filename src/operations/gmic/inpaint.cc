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

#include "../blender.hh"
#include "../invert.hh"
#include "../uniform.hh"
#include "inpaint.hh"


template <class T >
std::string convert2string( const T& val )
{
  std::ostringstream str;
  str<<val;
  return str.str();
}


PF::GmicInpaintPar::GmicInpaintPar(): 
  GmicUntiledOperationPar(),
  patch_size( "patch_size", this, 7 ),
  lookup_size( "lookup_size", this, 16 ),
  lookup_factor( "lookup_factor", this, 0.1 ),
  blend_size( "blend_size", this, 1.2 ),
  blend_threshold( "blend_threshold", this, 0 ),
  blend_decay( "blend_decay", this, 0.05),
  blend_scales( "blend_scales", this, 10 ),
  allow_outer_blending( "allow_outer_blending", this, 1 ),
  pen_size( "pen_size", this, 5 ),
  strokes( "strokes", this ),
  display_mode( "display_mode", this, PF::GMIC_INPAINT_DRAW_MASK, "draw_mask", "Draw mask" )
{
  display_mode.add_enum_value( PF::GMIC_INPAINT_DRAW_OUTPUT, "draw_output", "Draw output" );

  draw_op1 = new PF::Processor<PF::DrawPar,PF::DrawProc>();
  draw_op1->get_par()->get_bgd_color().get().r = 0;
  draw_op1->get_par()->get_pen_color().get().r = 1;

  draw_op2 = new PF::Processor<PF::DrawPar,PF::DrawProc>();
  draw_op2->get_par()->get_bgd_color().get().r = 0;
  draw_op2->get_par()->get_pen_color().get().r = 1;

  black = new PF::Processor<PF::UniformPar,PF::Uniform>();
  if( black->get_par() ) {
    black->get_par()->get_R().set( 0 );
    black->get_par()->get_G().set( 0 );
    black->get_par()->get_B().set( 0 );
  }
  uniform = new PF::Processor<PF::UniformPar,PF::Uniform>();
  if( uniform->get_par() ) {
    uniform->get_par()->get_R().set( 1 );
    uniform->get_par()->get_G().set( 0 );
    uniform->get_par()->get_B().set( 0 );
  }
  maskblend = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  invert = new PF::Processor<PF::InvertPar,PF::Invert>();

  if( PF::PhotoFlow::Instance().is_batch() ) {
    display_mode.set_enum_value( GMIC_INPAINT_DRAW_OUTPUT );
  } else {
    display_mode.set_enum_value( GMIC_INPAINT_DRAW_MASK );    
  }

  set_type( "gmic_inpaint" );
}


PF::GmicInpaintPar::~GmicInpaintPar()
{
}


VipsImage* PF::GmicInpaintPar::build(std::vector<VipsImage*>& in, int first, 
															VipsImage* imap, VipsImage* omap, 
															unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) return NULL;
  
  scale_factor = 1;
  for(unsigned int l = 0; l < level; l++ ) {
    scale_factor *= 2;
  }

  draw_op1->get_par()->get_strokes().import( &strokes );
  draw_op2->get_par()->get_strokes().import( &strokes );
  std::vector<VipsImage*> in2;
  if( display_mode.get_enum_value().first == PF::GMIC_INPAINT_DRAW_MASK ) {
    draw_op1->get_par()->set_image_hints( srcimg );
    draw_op1->get_par()->grayscale_image( srcimg->Xsize, srcimg->Ysize );
    draw_op1->get_par()->set_format( get_format() );
    VipsImage* mask = draw_op1->get_par()->build( in2, first, imap, omap, level );

    /*
    invert->get_par()->set_image_hints( mask );
    invert->get_par()->set_format( get_format() );
    in.clear(); in.push_back( mask );
    VipsImage* maskinverted = invert->get_par()->build(in, 0, NULL, NULL, level );
    */

    uniform->get_par()->set_image_hints( srcimg );
    uniform->get_par()->set_format( get_format() );
    in2.clear();
    VipsImage* redimage = uniform->get_par()->build(in2, 0, NULL, NULL, level );

    maskblend->get_par()->set_image_hints( srcimg );
    maskblend->get_par()->set_format( get_format() );
    maskblend->get_par()->set_blend_mode( PF::PF_BLEND_NORMAL );
    maskblend->get_par()->set_opacity( 0.8 );
    in2.clear(); 
    in2.push_back( srcimg );
    in2.push_back( redimage );
    VipsImage* blendimage = maskblend->get_par()->build(in2, 0, NULL, mask, level );
    //PF_UNREF( srcimg, "GmicInpaintPar::build() srcimg unref" );
    PF_UNREF( mask, "GmicInpaintPar::build() mask unref" );
    PF_UNREF( redimage, "GmicInpaintPar::build() redimage unref" );

    return blendimage;
  } else {
    if( strokes.get().size() < 1 ) {
      PF_REF( srcimg, "GmicInpaintPar::build() srcimg ref (strokes.get().size() < 1)" );
      return srcimg;
    }
    update_raster_images();
    PF::RasterImage* raster_image = get_raster_image(0);
    //if( !raster_image || (raster_image->get_file_name () != get_cache_file_name()) ) {
    if( !raster_image ) {
      std::string tempfile = save_image( srcimg, IM_BANDFMT_FLOAT );

      in2.clear();
      black->get_par()->set_image_hints( srcimg );
      black->get_par()->set_format( get_format() );
      VipsImage* blackimage = black->get_par()->build(in2, 0, NULL, NULL, level );

      in2.clear();
      uniform->get_par()->set_image_hints( srcimg );
      uniform->get_par()->set_format( get_format() );
      VipsImage* redimage = uniform->get_par()->build(in2, 0, NULL, NULL, level );

      in2.clear();
      draw_op2->get_par()->set_image_hints( srcimg );
      draw_op2->get_par()->grayscale_image( srcimg->Xsize, srcimg->Ysize );
      draw_op2->get_par()->set_format( get_format() );
      VipsImage* mask = draw_op2->get_par()->build( in2, first, imap, omap, level );

      maskblend->get_par()->set_image_hints( srcimg );
      maskblend->get_par()->set_format( get_format() );
      maskblend->get_par()->set_blend_mode( PF::PF_BLEND_NORMAL );
      maskblend->get_par()->set_opacity( 0.8 );
      in2.clear(); 
      in2.push_back( blackimage );
      in2.push_back( redimage );
      VipsImage* blendimage = maskblend->get_par()->build(in2, 0, NULL, mask, level );
      //PF_UNREF( srcimg, "GmicInpaintPar::build() srcimg unref" );
      PF_UNREF( mask, "GmicInpaintPar::build() mask unref" );
      PF_UNREF( blackimage, "GmicInpaintPar::build() blackimage unref" );
      PF_UNREF( redimage, "GmicInpaintPar::build() redimage unref" );

      std::string tempfile2 = save_image( blendimage, IM_BANDFMT_UCHAR );

      std::string command = "-verbose + ";
      command = command + "-input " + tempfile + " -n 0,255 ";
      command = command + "-input " + tempfile2 + " -inpaint[0] [1],";
      command = command + patch_size.get_str();
      command = command + std::string(",") + convert2string( lookup_size.get()*patch_size.get() );
      command = command + std::string(",") + lookup_factor.get_str() + ",1";
      command = command + std::string(",") + convert2string( blend_size.get()*patch_size.get() );
      command = command + std::string(",") + blend_threshold.get_str();
      command = command + std::string(",") + blend_decay.get_str();
      command = command + std::string(",") + blend_scales.get_str();
      command = command + std::string(",") + allow_outer_blending.get_str();
      command = command + " -n[0] 0,1 -output[0] " + get_cache_file_name(0) + ",float,lzw";
      run_gmic( command );

      unlink( tempfile.c_str() );
      PF_UNREF( blendimage, "GmicInpaintPar::build() blendimage unref after write" );
      unlink( tempfile2.c_str() );
    }
    std::vector<VipsImage*> outvec = get_output( level );
    VipsImage* out = (outvec.size()>0) ? outvec[0] : NULL;

    return out;
  }

  return NULL;
}



void PF::GmicInpaintPar::start_stroke( unsigned int pen_size )
{
  strokes.get().push_back( PF::Stroke<PF::Pencil>() );

  PF::Stroke<PF::Pencil>& stroke = strokes.get().back();

  PF::Pencil& pen = stroke.get_pen();
  pen.set_size( pen_size );
  pen.set_opacity( 1 );

  switch( get_colorspace() ) {
  case PF::PF_COLORSPACE_GRAYSCALE:
    pen.set_channel( 0, 1 );
    break;
  case PF::PF_COLORSPACE_RGB:
    pen.set_channel( 0, 1 );
    pen.set_channel( 1, 1 );
    pen.set_channel( 2, 1 );
    break;
  case PF::PF_COLORSPACE_LAB:
    break;
  case PF::PF_COLORSPACE_CMYK:
    break;
  default:
    break;
  }

  draw_op1->get_par()->start_stroke( pen_size, 1.0 );
  draw_op2->get_par()->start_stroke( pen_size, 1.0 );
}



void PF::GmicInpaintPar::end_stroke()
{
}



void PF::GmicInpaintPar::draw_point( unsigned int x, unsigned int y, VipsRect& update )
{
  PF::Stroke<PF::Pencil>& stroke = strokes.get().back();

  if( !stroke.get_points().empty() ) {
    if( (stroke.get_points().back().first == x ) &&
				(stroke.get_points().back().second == y ) )
      return;
  }

  stroke.get_points().push_back( std::make_pair(x, y) );

  PF::Pencil& pen = stroke.get_pen();

  update.left = x - pen.get_size();
  update.top = y - pen.get_size();
  update.width = update.height = pen.get_size()*2 + 1;

  draw_op1->get_par()->draw_point( x, y, update );
  draw_op2->get_par()->draw_point( x, y, update );  
}



PF::ProcessorBase* PF::new_gmic_inpaint()
{
  return( new PF::Processor<PF::GmicInpaintPar,PF::GmicInpaintProc>() );
}
