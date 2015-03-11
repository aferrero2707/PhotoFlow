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
#include "split_details.hh"



PF::GmicSplitDetailsPar::GmicSplitDetailsPar():
  GmicUntiledOperationPar(),
  prop_nscales("nscales",this,4),
  prop_base_scale("base_scale",this,1),
  prop_detail_scale("detail_scale",this,0.01)
{	
  set_cache_files_num(5);
  set_type( "gmic_split_details");
}


std::vector<VipsImage*> PF::GmicSplitDetailsPar::build_many(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  std::vector<VipsImage*> outvec;

  if( !srcimg ) return outvec;
  
  update_raster_images();
  PF::RasterImage* raster_image = get_raster_image(0);
  //if( !raster_image || (raster_image->get_file_name () != get_cache_file_name()) ) {
  if( !raster_image ) {
    std::string tempfile = save_image( srcimg, IM_BANDFMT_FLOAT );

    std::string command = "-verbose + ";
    command = command + "-input " + tempfile + " -mul 255 ";
    //command = command + "-split_details 2 ";
    command = command + "-split_details 5,"+prop_base_scale.get_str()+"%,"+prop_detail_scale.get_str()+"% ";
    //command = command + prop_threshold.get_str();
    //command = command + std::string(",") + prop_gamma.get_str();
    //command = command + std::string(",") + prop_smoothness.get_str();
    //command = command + std::string(",") + prop_iterations.get_str();
    //command = command + std::string(",") + prop_channels.get_enum_value_str();
    command = command + " -div[0] 255 -output[0] " + get_cache_file_name(0) + ",float,lzw";
    for( int i = 1; i < get_cache_files_num(); i++ ) {
      std::ostringstream str;
      str<<i;
      std::string id = str.str();
      command = command + " -add["+id+"] 127 -c["+id+"] 0,255 -div["+id+"] 255 -output["+id+"] " + get_cache_file_name(i) + ",float,lzw";
    }
    
    run_gmic( srcimg, command );

    unlink( tempfile.c_str() );
  }
  outvec = get_output( level );

  return outvec;
}


PF::ProcessorBase* PF::new_gmic_split_details()
{
  return( new PF::Processor<PF::GmicSplitDetailsPar,PF::GmicSplitDetailsProc>() );
}
