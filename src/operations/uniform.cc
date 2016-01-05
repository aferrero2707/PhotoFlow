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

  set_default_name( _("uniform fill") );
}



VipsImage* PF::UniformPar::build(std::vector<VipsImage*>& in, int first, 
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  std::cout<<"UniformPar::build(): colorspace="<<get_colorspace()<<std::endl;
  grey.set( R.get() );

  if( get_colorspace() == PF::PF_COLORSPACE_RGB ) {
    Rconv = R.get();
    Gconv = G.get();
    Bconv = B.get();

    void *data;
    size_t data_length;
    if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
        &data, &data_length ) ) {
      cmsHPROFILE wprofile = cmsOpenProfileFromMem( data, data_length );
      if( wprofile ) {
        cmsHTRANSFORM transform = cmsCreateTransform( PF::ICCStore::Instance().get_srgb_profile(PF::PF_TRC_STANDARD),
            TYPE_RGB_FLT, wprofile, TYPE_RGB_FLT,
            INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
        if( transform ) {
          float rgb_in[3], rgb_out[3];
          rgb_in[0] = R.get(); rgb_in[1] = G.get(); rgb_in[2] = B.get();
          cmsDoTransform( transform, rgb_in, rgb_out, 1 );
          cmsDeleteTransform( transform );

          Rconv = rgb_out[0];
          Bconv = rgb_out[1];
          Bconv = rgb_out[2];
        }
        cmsCloseProfile( wprofile );
      }
    }
  }

  return PF::OpParBase::build( in, first, imap, omap, level );
}



PF::ProcessorBase* PF::new_uniform()
{
  return( new PF::Processor<PF::UniformPar,PF::Uniform>() );
}
