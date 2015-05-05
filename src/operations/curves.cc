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

#include "../base/processor.hh"
#include "curves.hh"

PF::CurvesPar::CurvesPar(): 
  PF::PixelProcessorPar() ,
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


void PF::CurvesPar::update_curve( PF::Property<PF::SplineCurve>& curve,
                                  short int* vec8, int* vec16 )
{
  /*
    Greyvec.clear();
    for( int i = 0; i <= 1000; i++ ) {
    Greyvec.push_back( std::make_pair( float(i)/1000, float(0) ) );
    }
    grey_curve.get().get_deltas( Greyvec );
  */
  curve.get().lock();
  //std::cout<<"CurvesPar::update_curve() called. # of points="<<curve.get().get_npoints()<<std::endl;std::cout.flush();
  for(int i = 0; i <= FormatInfo<unsigned char>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned char>::RANGE;
    float y = curve.get().get_delta( x );
    vec8[i] = (short int)(y*FormatInfo<unsigned char>::RANGE);
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec8[i]="<<vec8[i]<<std::endl;
  }
  for(int i = 0; i <= FormatInfo<unsigned short int>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned short int>::RANGE;
    float y = curve.get().get_delta( x );
    vec16[i] = (int)(y*FormatInfo<unsigned short int>::RANGE);
   //if(i%1000 == 0) 
    //if(curve.get().get_points().size()>100) 
   // 	std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec16[i]="<<vec16[i]<<"  points="<<curve.get().get_points().size()<<std::endl;
  }
  curve.get().unlock();
}



VipsImage* PF::CurvesPar::build(std::vector<VipsImage*>& in, int first, 
				VipsImage* imap, VipsImage* omap, 
				unsigned int& level)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );

  if( grey_curve.is_modified() ) {
	  //std::cout<<"update_curve( grey_curve, Greyvec8, Greyvec16 );"<<std::endl;std::cout.flush();
    update_curve( grey_curve, Greyvec8, Greyvec16 );
  }

  if( R_curve.is_modified() || G_curve.is_modified() || 
      B_curve.is_modified() || RGB_curve.is_modified() ) {
	    //std::cout<<"update_curve( R_curve, RGBvec8[0], RGBvec16[0] );"<<std::endl;std::cout.flush();
	    update_curve( R_curve, RGBvec8[0], RGBvec16[0] );
	    //std::cout<<"update_curve( G_curve, RGBvec8[1], RGBvec16[1] );"<<std::endl;std::cout.flush();
	    update_curve( G_curve, RGBvec8[1], RGBvec16[1] );
	    //std::cout<<"update_curve( B_curve, RGBvec8[2], RGBvec16[2] );"<<std::endl;std::cout.flush();
	    update_curve( B_curve, RGBvec8[2], RGBvec16[2] );
	    //std::cout<<"update_curve( RGB_curve, RGBvec8[3], RGBvec16[3] );"<<std::endl;std::cout.flush();
	    update_curve( RGB_curve, RGBvec8[3], RGBvec16[3] );
    for(int i = 0; i <= FormatInfo<unsigned char>::RANGE; i++) {
      for(int j = 0; j < 3; j++) RGBvec8[j][i] += RGBvec8[3][i];
    }
    for(int i = 0; i <= FormatInfo<unsigned short int>::RANGE; i++) {
      for(int j = 0; j < 3; j++) {
        RGBvec16[j][i] += RGBvec16[3][i];
        //if(i%1000 == 0) std::cout<<"i="<<i<<"  RGBvec16["<<j<<"][i]="<<RGBvec16[j][i]<<std::endl;
      }
    }
  }

  if( L_curve.is_modified() )
    update_curve( L_curve, Labvec8[0], Labvec16[0] );
  if( a_curve.is_modified() )
    update_curve( a_curve, Labvec8[1], Labvec16[1] );
  if( b_curve.is_modified() )
    update_curve( b_curve, Labvec8[2], Labvec16[2] );

  if( C_curve.is_modified() )
    update_curve( C_curve, CMYKvec8[0], CMYKvec16[0] );
  if( M_curve.is_modified() )
    update_curve( M_curve, CMYKvec8[1], CMYKvec16[1] );
  if( Y_curve.is_modified() )
    update_curve( Y_curve, CMYKvec8[2], CMYKvec16[2] );
  if( K_curve.is_modified() )
    update_curve( K_curve, CMYKvec8[3], CMYKvec16[3] );

  /*
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
  */

  if( in[first] ) {
    PF::colorspace_t cs = PF::convert_colorspace( get_interpretation() );
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      scvec[0] = &grey_curve;
      cvec = &Greyvec;
      cvec8[0] = Greyvec8;
      cvec16[0] = Greyvec16;
      break;
    case PF_COLORSPACE_RGB:
      scvec[0] = &R_curve;
      scvec[1] = &G_curve;
      scvec[2] = &B_curve;
      scvec[3] = &RGB_curve;
      cvec = RGBvec;
      for(int i=0; i<4; i++ ) {cvec8[i] = RGBvec8[i];cvec16[i] = RGBvec16[i];}
      break;
    case PF_COLORSPACE_LAB:
      scvec[0] = &L_curve;
      scvec[1] = &a_curve;
      scvec[2] = &b_curve;
      cvec = Labvec;
      for(int i=0; i<3; i++ ) {cvec8[i] = Labvec8[i];cvec16[i] = Labvec16[i];}
      break;
    case PF_COLORSPACE_CMYK:
      scvec[0] = &C_curve;
      scvec[1] = &M_curve;
      scvec[2] = &Y_curve;
      scvec[3] = &K_curve;
      cvec = CMYKvec;
      for(int i=0; i<4; i++ ) {cvec8[i] = CMYKvec8[i];cvec16[i] = CMYKvec16[i];}
      break;
    default:
      break;
    }
  }

  return out;
}


PF::ProcessorBase* PF::new_curves()
{
  return( new PF::Processor<PF::CurvesPar,PF::Curves>() );
}
