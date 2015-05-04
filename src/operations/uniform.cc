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

#include "../base/processor.hh"
#include "uniform.hh"

PF::UniformPar::UniformPar(): 
  PixelProcessorPar(),
  grey( "grey", this, 0 ),
  R( "R", this, 0 ),
  G( "G", this, 0 ),
  B( "B", this, 0 ),
  L( "L", this, 0 ),
  a( "a", this, 0 ),
  b( "b", this, 0 ),
  C( "C", this, 0 ),
  M( "M", this, 0 ),
  Y( "Y", this, 0 ),
  K( "K", this, 0 )
{
  set_type( "uniform" );
}



VipsImage* PF::UniformPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  std::cout<<"UniformPar::build(): colorspace="<<get_colorspace()<<std::endl;
  grey.set( R.get() );
  return PF::OpParBase::build( in, first, imap, omap, level );
}



PF::ProcessorBase* PF::new_uniform()
{
  return( new PF::Processor<PF::UniformPar,PF::Uniform>() );
}
