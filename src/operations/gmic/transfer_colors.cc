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
#include "transfer_colors.hh"



PF::GmicTransferColorsPar::GmicTransferColorsPar():
  PF::GmicUntiledOperationPar(),
  prop_regularization("regularization",this,8),
  prop_preserve_lumi("preserve_lumi",this,0.2),
  prop_precision("precision",this,1,"Normal","Normal")
{
  prop_precision.add_enum_value(0,"Low","Low");
  prop_precision.add_enum_value(2,"High","High");
  prop_precision.add_enum_value(3,"Very high","VeryHigh");
  set_cache_files_num(1);
  set_type( "gmic_transfer_colors");
}


std::vector<VipsImage*> PF::GmicTransferColorsPar::build_many(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  std::vector<VipsImage*> outvec;
  if( !srcimg ) return outvec;
  if( in.size() < 2 ) {
    PF_REF( srcimg, "" );
    outvec.push_back( srcimg );
    return outvec;
  }
  VipsImage* refimg = in[1];
  if( !refimg ) {
    PF_REF( srcimg, "" );
    outvec.push_back( srcimg );
    return outvec;
  }

  update_raster_images();
  PF::RasterImage* raster_image = get_raster_image(0);
  //if( !raster_image || (raster_image->get_file_name () != get_cache_file_name()) ) {
  if( !raster_image ) {
    std::string tempfile1 = save_image( srcimg, IM_BANDFMT_FLOAT );
    std::string tempfile2 = save_image( refimg, IM_BANDFMT_FLOAT );

    std::string command = "-verbose + ";
    command = command + "-input " + tempfile1 + " -mul 255 ";
    command = command + "-input " + tempfile2 + " -mul[1] 255 ";
    //command = command + "-split_details 2 ";
    command = command + "-gimp_transfer_rgb " + prop_regularization.get_str() + ",";
    command = command + prop_preserve_lumi.get_str() + ",1,1,0 ";
    //command = command + prop_threshold.get_str();
    //command = command + std::string(",") + prop_gamma.get_str();
    //command = command + std::string(",") + prop_smoothness.get_str();
    //command = command + std::string(",") + prop_iterations.get_str();
    //command = command + std::string(",") + prop_channels.get_enum_value_str();
    command = command + " -div[0] 255 -output[0] " + get_cache_file_name(0) + ",float";
    
    run_gmic( srcimg, command );

    unlink( tempfile1.c_str() );
    unlink( tempfile2.c_str() );
  }
  std::cout<<"GmicTransferColorsPar::build(): calling get_output()"<<std::endl;
  outvec = get_output( level );

  for(std::size_t i = 0; i < outvec.size(); i++ )
    PF::vips_copy_metadata( srcimg, outvec[i] );

  return outvec;
}


PF::ProcessorBase* PF::new_gmic_transfer_colors()
{
  return( new PF::Processor<PF::GmicTransferColorsPar,PF::GmicTransferColorsProc>() );
}
