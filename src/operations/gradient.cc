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
#include "gradient.hh"


PF::GradientPar::GradientPar():
OpParBase(),
gradient_type("gradient_type",this,GRADIENT_VERTICAL,"vertical",_("Vertical")),
invert("invert",this,false),
perceptual("perceptual",this,true),
gradient_center_x("gradient_center_x",this,0.5),
gradient_center_y("gradient_center_y",this,0.5),
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
CMYK_active_curve( "CMYK_active_curve", this, 8, "C", "C" ),
hmod( "hmod", this ),
vmod( "vmod", this ),
modvec( NULL )
{
  //gradient_type.add_enum_value(GRADIENT_VERTICAL,"vertical","Vertical");
  gradient_type.add_enum_value(GRADIENT_HORIZONTAL,"horizontal",_("Horizontal"));
  gradient_type.add_enum_value(GRADIENT_RADIAL,"radial",_("Radial"));

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

  float x1 = 0, y1 = 0.5, x2 = 1, y2 = 0.5;
  hmod.get().set_point( 0, x1, y1 );
  hmod.get().set_point( 1, x2, y2 );
  vmod.get().set_point( 0, x1, y1 );
  vmod.get().set_point( 1, x2, y2 );

  curve = new_curves();

  set_type( "gradient" );
  set_default_name( _("gradient") );
}



VipsImage* PF::GradientPar::build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );
  VipsImage* out2 = out;

  //PF::ICCProfileData* data;
  icc_data = PF::get_icc_profile( out );

  int modlen = 0, modh = 0;
  switch( get_gradient_type() ) {
  case GRADIENT_VERTICAL:
    modlen = out->Xsize;
    modh = out->Ysize;
    break;
  case GRADIENT_HORIZONTAL:
    modlen = out->Ysize;
    modh = out->Xsize;
    break;
  default:
    break;
  }
  if( modlen > 0 ) {
    modvec = (float*)realloc( modvec, sizeof(float)*modlen);
    //modulation.get().lock();
    for(int i = 0; i < modlen; i++) {
      float x = ((float)i)/(modlen-1);
      if( get_gradient_type() == GRADIENT_VERTICAL )
        modvec[i] = vmod.get().get_value( x );
      if( get_gradient_type() == GRADIENT_HORIZONTAL )
        modvec[i] = 1.0f-hmod.get().get_value( x );
    }
    //modulation.get().unlock();
  }

  //std::cout<<"GradientPar::build(): is_map()="<<is_map()<<std::endl;
  for( int i = 0; i < 65536; i++ ) {
    float fval = i;
    fval /= 65535;
    if( perceptual.get() ) {
      if( !is_map() && icc_data && icc_data->is_linear() ) {
        float lval = cmsEvalToneCurveFloat( PF::ICCStore::Instance().get_Lstar_trc(), fval );
        //std::cout<<"modvec["<<i<<"]: perceptual="<<fval<<"  linear="<<lval<<std::endl;
        fval = lval;
      }
    } else {
      if( is_map() || !icc_data || !icc_data->is_linear() ) {
            float lval = cmsEvalToneCurveFloat( PF::ICCStore::Instance().get_iLstar_trc(), fval );
            //std::cout<<"modvec["<<i<<"]: perceptual="<<fval<<"  linear="<<lval<<std::endl;
            fval = lval;
          }
    }
    //std::cout<<"trc_vec["<<i<<"]: "<<fval<<std::endl;
    trc_vec[i] = fval;
  }

  CurvesPar* curvepar = dynamic_cast<CurvesPar*>( curve->get_par() );
  if( curvepar ) {
    curvepar->set_grey_curve( grey_curve );
    curvepar->set_RGB_curve( RGB_curve );
    curvepar->set_R_curve( R_curve );
    curvepar->set_G_curve( G_curve );
    curvepar->set_B_curve( B_curve );
    curvepar->set_L_curve( L_curve );
    curvepar->set_a_curve( a_curve );
    curvepar->set_b_curve( b_curve );
    curvepar->set_C_curve( C_curve );
    curvepar->set_M_curve( M_curve );
    curvepar->set_Y_curve( Y_curve );
    curvepar->set_K_curve( K_curve );
    curvepar->set_image_hints( out );
    curvepar->set_format( get_format() );
    std::vector<VipsImage*> in2; in2.push_back(out);
    out2 = curvepar->build( in2, 0, imap, omap, level );

    PF_UNREF( out, "GradientPar::build(): out unref" );
  }

  return out2;
}
