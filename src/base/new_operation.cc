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


#include "new_operation.hh"

#include "../operations/vips_operation.hh"
#include "../operations/image_reader.hh"
#include "../operations/brightness_contrast.hh"
#include "../operations/invert.hh"
#include "../operations/gradient.hh"
#include "../operations/convert2lab.hh"
#include "../operations/clone.hh"
#include "../operations/curves.hh"


PF::ProcessorBase* PF::new_operation( std::string op_type, PF::Layer* current_layer )
{
  PF::ProcessorBase* processor = NULL;

  if( op_type == "imageread" ) { 

    processor = new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();

  } else if( op_type == "blender" ) {

    processor = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();

  } else if( op_type == "clone" ) {

    processor = new PF::Processor<PF::ClonePar,PF::CloneProc>();

  } else if( op_type == "invert" ) {

    processor = new PF::Processor<PF::InvertPar,PF::Invert>();

  } else if( op_type == "brightness_contrast" ) {

    processor = new PF::Processor<PF::BrightnessContrastPar,PF::BrightnessContrast>();

  } else if( op_type == "curves" ) {
      
    processor = new PF::Processor<PF::CurvesPar,PF::Curves>();

  } else if( op_type == "convert2lab" ) {

    processor = new PF::Processor<PF::Convert2LabPar,PF::Convert2LabProc>();

  } else { // it must be a VIPS operation...

    int pos = op_type.find( "vips-" );
    if( pos != 0 ) return NULL;
    std::string vips_op_type;
    vips_op_type.append(op_type.begin()+5,op_type.end());

    PF::Processor<PF::VipsOperationPar,PF::VipsOperationProc>* vips_op = 
      new PF::Processor<PF::VipsOperationPar,PF::VipsOperationProc>();
    vips_op->get_par()->set_op( vips_op_type.c_str() );
    processor = vips_op;
  }

  if( processor && current_layer )
    current_layer->set_processor( processor );

  return processor;
}
