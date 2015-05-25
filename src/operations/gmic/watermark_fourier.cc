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


#include "gmic.hh"
#include "watermark_fourier.hh"



PF::GmicWatermarkFourierPar::GmicWatermarkFourierPar():
  PF::GmicUntiledOperationPar(),
  prop_text("text",this,""),
  prop_text_size("text_size",this,33)
{
  set_cache_files_num(1);
  set_type( "gmic_watermark_fourier");
}


std::vector<VipsImage*> PF::GmicWatermarkFourierPar::build_many(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  std::vector<VipsImage*> outvec;
  if( !srcimg ) return outvec;

  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    PF_REF( srcimg, "GmicWatermarkFourierPar::build_many(): srcimg ref in preview render mode" );
    outvec.push_back( srcimg );
    return outvec;
  }

  if( prop_text.get().empty() ) {
    PF_REF( srcimg, "GmicWatermarkFourierPar::build_many(): srcimg ref with empty watermark" );
    outvec.push_back( srcimg );
    return outvec;
  }

  update_raster_images();
  PF::RasterImage* raster_image = get_raster_image(0);
  //if( !raster_image || (raster_image->get_file_name () != get_cache_file_name()) ) {
  if( !raster_image ) {
    std::string tempfile1 = save_image( srcimg, IM_BANDFMT_FLOAT );

    std::string command = "-verbose + ";
    command = command + "-input " + tempfile1 + " -mul 255 ";
    command = command + "-watermark_fourier \"" + prop_text.get() + "\"," + prop_text_size.get_str();
    command = command + " -c 0,255 -div 255 -output " + get_cache_file_name(0) + ",float";
    
    run_gmic( srcimg, command );

    //unlink( tempfile1.c_str() );
  }
  std::cout<<"GmicWatermarkFourierPar::build(): calling get_output()"<<std::endl;
  outvec = get_output( level );

  for(std::size_t i = 0; i < outvec.size(); i++ )
    PF::vips_copy_metadata( srcimg, outvec[i] );

  return outvec;
}


PF::ProcessorBase* PF::new_gmic_watermark_fourier()
{
  return( new PF::Processor<PF::GmicWatermarkFourierPar,PF::GmicWatermarkFourierProc>() );
}
