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

//#include <arpa/inet.h>

#include "hsl_mask.hh"



PF::HSLMaskPar::HSLMaskPar():
  OpParBase(),
  invert( "invert", this, false ),
  H_curve( "H_curve", this ),
  S_curve( "S_curve", this ),
  L_curve( "L_curve", this ),
  H_curve_enabled( "H_curve_enabled", this, false ),
  S_curve_enabled( "S_curve_enabled", this, false ),
  L_curve_enabled( "L_curve_enabled", this, false )
{
  set_type("hsl_mask" );

  set_default_name( _("HSL mask") );

  int id = 0;
  eq_vec[id++] = &H_curve;
  eq_vec[id++] = &S_curve;
  eq_vec[id++] = &L_curve;

  H_curve.get().set_circular( true );

  float x1 = 0, y1 = 1., x2 = 1, y2 = 1.;
  for( id = 0; id < 3; id++ ) {
    eq_vec[id]->get().set_point( 0, x1, y1 );
    eq_vec[id]->get().set_point( 1, x2, y2 );
    eq_vec[id]->store_default();
  }
}



void PF::HSLMaskPar::update_curve( PF::Property<PF::SplineCurve>* curve, float* vec )
{
  curve->get().lock();
  //std::cout<<"HSLMaskPar::update_curve() called. # of points="<<curve->get().get_npoints()<<std::endl;
  for(int i = 0; i <= 65535; i++) {
    float x = ((float)i)/65535;
    float y = curve->get().get_value( x );
    vec[i] = y;
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec[i]="<<vec[i]<<std::endl;
  }
  curve->get().unlock();
}



VipsImage* PF::HSLMaskPar::build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );

  for( int id = 0; id < 3; id++ ) {
    if( eq_vec[id]->is_modified() ) {
#ifndef NDEBUG
      std::cout<<"HSLMaskPar::build(): updating curve #"<<id<<std::endl;
#endif
      update_curve( eq_vec[id], vec[id] );
    }
    eq_enabled[id] = false;
    //std::cout<<"eq_vec["<<id<<"]->get().get_npoints()="<<eq_vec[id]->get().get_npoints()<<std::endl;
    //for( size_t pi = 0; pi < eq_vec[id]->get().get_npoints(); pi++ ) {
      //std::cout<<"  get_point("<<pi<<").second="<<eq_vec[id]->get().get_point(pi).second<<std::endl;
      //if( fabs(eq_vec[id]->get().get_point(pi).second) > 0.001 ) {
        //eq_enabled[id] = true;
        //break;
      //}
    //}
  }
  eq_enabled[0] = H_curve_enabled.get();
  eq_enabled[1] = S_curve_enabled.get();
  eq_enabled[2] = L_curve_enabled.get();

  return out;
}
