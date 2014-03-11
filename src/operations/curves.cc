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

#include <iostream>

#include "curves.hh"

PF::CurvesPar::CurvesPar(): 
  PF::OpParBase() ,
  grey_curve( "grey_curve", this ), // 0
  RGB_curve( "RGB_curve", this ),   // 1
  R_curve( "R_curve", this ),       // 2
  G_curve( "G_curve", this ),       // 3
  B_curve( "B_curve", this ),       // 4
  L_curve( "L_curve", this ),       // 5
  a_curve( "a_curve", this ),       // 6
  b_curve( "b_curve", this ),       // 7
  C_curve( "C_curve", this ),       // 8
  M_curve( "M_curve", this ),       // 9
  Y_curve( "Y_curve", this ),       // 10
  K_curve( "K_curve", this ),       // 11
  RGB_active_curve( "RGB_active_curve", this, 1, "RGB", "RGB" ),
  Lab_active_curve( "Lab_active_curve", this, 5, "L", "L" ),
  CMYK_active_curve( "CMYK_active_curve", this, 8, "C", "C" )
{
  set_type( "curves" );
  
  RGB_active_curve.add_enum_value( 1, "RGB", "RGB" );
  RGB_active_curve.add_enum_value( 2, "R", "R" );
  RGB_active_curve.add_enum_value( 3, "G", "G" );
  RGB_active_curve.add_enum_value( 4, "B", "B" );
  
  Lab_active_curve.add_enum_value( 5, "L", "L" );
  Lab_active_curve.add_enum_value( 6, "a", "a" );
  Lab_active_curve.add_enum_value( 7, "b", "b" );

  CMYK_active_curve.add_enum_value( 8, "C", "C" );
  CMYK_active_curve.add_enum_value( 9, "M", "M" );
  CMYK_active_curve.add_enum_value( 10, "Y", "Y" );
  CMYK_active_curve.add_enum_value( 11, "K", "K" );

  //RGB_curve.get().add_point( 0.25, 0.10 );
  //RGB_curve.get().add_point( 0.75, 0.90 );

  //L_curve.get().add_point( 0.25, 0.10 );
  //L_curve.get().add_point( 0.75, 0.90 );
}



VipsImage* PF::CurvesPar::build(std::vector<VipsImage*>& in, int first, 
				VipsImage* imap, VipsImage* omap)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap );

  Greyvec.clear();
  grey_curve.get().get_deltas( Greyvec );

  for( int j = 0; j < 4; j++ ) {
    RGBvec[j].clear();
    for( int i = 0; i <= 1000; i++ ) {
      RGBvec[j].push_back( std::make_pair( float(i)/1000, float(0) ) );
    }
  }
  RGB_curve.get().get_deltas( RGBvec[3] );
  R_curve.get().get_deltas( RGBvec[0] );
  G_curve.get().get_deltas( RGBvec[1] );
  B_curve.get().get_deltas( RGBvec[2] );
  for( int j = 0; j < 3; j++ ) {
    for( unsigned int i = 0; i < RGBvec[3].size(); i++ ) {
      RGBvec[j][i].second += RGBvec[3][i].second;
    }
  }

  for( int j = 0; j < 3; j++ ) {
    Labvec[j].clear();
    for( int i = 0; i <= 1000; i++ ) {
      Labvec[j].push_back( std::make_pair( float(i)/1000, float(0) ) );
    }
  }
  L_curve.get().get_deltas( Labvec[0] );
  a_curve.get().get_deltas( Labvec[1] );
  b_curve.get().get_deltas( Labvec[2] );

  if( in[first] ) {
    PF::colorspace_t cs = PF::convert_colorspace( get_interpretation() );
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      cvec = &Greyvec;
      break;
    case PF_COLORSPACE_RGB:
      cvec = RGBvec;
      break;
    case PF_COLORSPACE_LAB:
      cvec = Labvec;
      break;
    case PF_COLORSPACE_CMYK:
      cvec = CMYKvec;
      break;
    default:
      break;
    }
  }

  return out;
}
