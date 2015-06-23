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

#include "hue_saturation.hh"



PF::HueSaturationPar::HueSaturationPar(): 
  OpParBase(),
  hue("hue",this,0),
  hue_eq("hue_eq",this,0),
  saturation("saturation",this,0),
  saturation_eq("saturation_eq",this,0),
  contrast("contrast",this,0),
  contrast_eq("contrast_eq",this,0),
  brightness("brightness",this,0),
  brightness_eq("brightness_eq",this,0),
  brightness_is_gamma("brightness_is_gamma",this,false),
  hue_H_equalizer( "hue_H_equalizer", this ),
  hue_S_equalizer( "hue_S_equalizer", this ),
  hue_L_equalizer( "hue_L_equalizer", this ),
  hue_H_equalizer_enabled( "hue_H_equalizer_enabled", this, false ),
  hue_S_equalizer_enabled( "hue_S_equalizer_enabled", this, false ),
  hue_L_equalizer_enabled( "hue_L_equalizer_enabled", this, false ),
  saturation_H_equalizer( "saturation_H_equalizer", this ),
  saturation_S_equalizer( "saturation_S_equalizer", this ),
  saturation_L_equalizer( "saturation_L_equalizer", this ),
  contrast_H_equalizer( "contrast_H_equalizer", this ),
  contrast_S_equalizer( "contrast_S_equalizer", this ),
  contrast_L_equalizer( "contrast_L_equalizer", this ),
  brightness_H_equalizer( "brightness_H_equalizer", this ),
  brightness_S_equalizer( "brightness_S_equalizer", this ),
  brightness_L_equalizer( "brightness_L_equalizer", this )
{
  set_type("hue_saturation" );

  int id = 0;
  eq_vec[id++] = &hue_H_equalizer;
  eq_vec[id++] = &hue_S_equalizer;
  eq_vec[id++] = &hue_L_equalizer;
/*
  eq_vec[id++] = &saturation_H_equalizer;
  eq_vec[id++] = &saturation_S_equalizer;
  eq_vec[id++] = &saturation_L_equalizer;
  eq_vec[id++] = &contrast_H_equalizer;
  eq_vec[id++] = &contrast_S_equalizer;
  eq_vec[id++] = &contrast_L_equalizer;
  eq_vec[id++] = &brightness_H_equalizer;
  eq_vec[id++] = &brightness_S_equalizer;
  eq_vec[id++] = &brightness_L_equalizer;
*/

  hue_H_equalizer.get().set_circular( true );

  float x1 = 0, y1 = 0., x2 = 1, y2 = 0.;
  for( id = 0; id < 3; id++ ) {
    eq_vec[id]->get().set_point( 0, x1, y1 );
    eq_vec[id]->get().set_point( 1, x2, y2 );
  }
  for( id = 0; id < 3; id+=3 ) {
    float x = 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
    eq_vec[id]->get().add_point( x/360, 0. ); x += 40;
  }

  x1 = 0; y1 = 0.5;
  //eq_vec[0]->get().set_point( 0, x1, y1 );
}



void PF::HueSaturationPar::update_curve( PF::Property<PF::SplineCurve>* curve, float* vec )
{
  curve->get().lock();
  //std::cout<<"CurvesPar::update_curve() called. # of points="<<curve.get().get_npoints()<<std::endl;std::cout.flush();
  for(int i = 0; i <= 65535; i++) {
    float x = ((float)i)/65535;
    float y = curve->get().get_value( x );
    vec[i] = y;
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec8[i]="<<vec8[i]<<std::endl;
  }
  curve->get().unlock();
}



VipsImage* PF::HueSaturationPar::build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );

  for( int id = 0; id < 3; id++ ) {
    if( eq_vec[id]->is_modified() )
      update_curve( eq_vec[id], vec[id] );
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
  eq_enabled[0] = hue_H_equalizer_enabled.get();
  eq_enabled[1] = hue_S_equalizer_enabled.get();
  eq_enabled[2] = hue_L_equalizer_enabled.get();

  return out;
}


PF::ProcessorBase* PF::new_hue_saturation()
{
  return new PF::Processor<PF::HueSaturationPar,PF::HueSaturation>();
}
