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


#include "../../base/processor_imp.hh"
#include "gmic.hh"
#include "convolve.hh"



PF::GmicConvolvePar::GmicConvolvePar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_kernel("kernel", this, 0, "Custom", "Custom"),
  prop_boundary("boundary", this, 1, "Neumann", "Neumann"),
  prop_custom_kernel("custom_kernel", this, "0,1,0;1,-4,1;0,1,0"),
  prop_value_range("value_range", this, 0, "Cut", "Cut"),
  prop_kernel_mul("kernel_mul",this,1.0)
{	
  gmic = PF::new_gmic();
  prop_kernel.add_enum_value( 1, "Average_3x3", "Average 3x3" );
  prop_kernel.add_enum_value( 2, "Average_5x5", "Average 5x5" );
  prop_kernel.add_enum_value( 3, "Average_7x7", "Average 7x7" );
  prop_kernel.add_enum_value( 4, "Average_9x9", "Average 9x9" );
  prop_kernel.add_enum_value( 5, "Prewitt_X", "Prewitt-X" );
  prop_kernel.add_enum_value( 6, "Prewitt_Y", "Prewitt-Y" );
  prop_kernel.add_enum_value( 7, "Sobel_X", "Sobel-X" );
  prop_kernel.add_enum_value( 8, "Sobel_Y", "Sobel-Y" );
  prop_kernel.add_enum_value( 9, "Rotinv_X", "Rotinv-X" );
  prop_kernel.add_enum_value( 10, "Rotinv_Y", "Rotinv-Y" );
  prop_kernel.add_enum_value( 11, "Laplacian", "Laplacian" );
  prop_kernel.add_enum_value( 12, "Robert_Cross_1", "Robert Cross 1" );
  prop_kernel.add_enum_value( 13, "Robert_Cross_2", "Robert Cross 2" );
  prop_kernel.add_enum_value( 14, "Impulses_5x5", "Impulses 5x5" );
  prop_kernel.add_enum_value( 15, "Impulses_7x7", "Impulses 7x7" );
  prop_kernel.add_enum_value( 16, "Impulses_9x9", "Impulses 9x9" );
  prop_boundary.add_enum_value( 0, "Dirichlet", "Dirichlet" );
  prop_value_range.add_enum_value( 1, "Normalize", "Normalize" );
  set_type( "gmic_convolve" );
}


int PF::GmicConvolvePar::get_gmic_padding( int level )
{
  return 0;
}


VipsImage* PF::GmicConvolvePar::build(std::vector<VipsImage*>& in, int first, 
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;
  
  if( !(gmic->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
	for( int l = 1; l <= level; l++ )
		scalefac *= 2;

  std::string command = "-gimp_convolve  ";
  command = command + prop_kernel.get_enum_value_str();
  command = command + std::string(",") + prop_boundary.get_enum_value_str();
  command = command + std::string(",") + prop_custom_kernel.get();
  command = command + std::string(",") + prop_value_range.get_enum_value_str();
  command = command + std::string(",") + prop_kernel_mul.get_str();
  command = command + std::string(",0");
  gpar->set_command( command.c_str() );
  gpar->set_iterations( iterations.get() );
  gpar->set_gmic_padding( get_gmic_padding( level ) );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
}


PF::ProcessorBase* PF::new_gmic_convolve()
{
  return( new PF::Processor<PF::GmicConvolvePar,PF::GmicConvolveProc>() );
}
