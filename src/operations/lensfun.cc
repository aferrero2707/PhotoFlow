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

#include "../base/exif_data.hh"
#include "lensfun.hh"

int vips_lensfun( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, VipsInterpolate* interpolate, ... );


PF::LensFunPar::LensFunPar():
    OpParBase(),
    prop_camera_maker( "camera_maker", this ),
    prop_camera_model( "camera_model", this ),
    prop_lens( "lens", this )
{
#ifdef PF_HAS_LENSFUN
  ldb = lf_db_new();
  ldb->Load ();
#endif
  set_type("lensfun" );

  set_default_name( _("optical corrections") );
}

VipsImage* PF::LensFunPar::build(std::vector<VipsImage*>& in, int first, 
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* outnew = NULL;
std::cout<<"LensFunPar::build() called."<<std::endl;
  if( (in.size() < 1) || (in[0] == NULL) )
    return NULL;

#ifndef PF_HAS_LENSFUN
  PF_REF( in[0], "LensFunPar::build(): no LENSFUN support availabe." );
  return( in[0] );
#endif

  size_t blobsz;
  PF::exif_data_t* exif_data;
  if( vips_image_get_blob( in[0], PF_META_EXIF_NAME,
      (void**)&exif_data,
      &blobsz ) ) {
    std::cout<<"LensFunPar::build() could not extract exif_custom_data."<<std::endl;
    PF_REF( in[0], "LensFunPar::build(): in[0] ref (exif data missing)")
    return in[0];
  }
  if( blobsz != sizeof(PF::exif_data_t) ) {
    std::cout<<"LensFunPar::build() wrong exif_custom_data size."<<std::endl;
    PF_REF( in[0], "LensFunPar::build(): in[0] ref (exif data wrong size)")
    return in[0];
  }

#ifdef PF_HAS_LENSFUN
  const lfCamera** cameras = ldb->FindCameras( exif_data->exif_maker,
      exif_data->exif_model );
  if( !cameras ) {
    g_print ("Cannot find the camera `%s %s' in database\n",
        exif_data->exif_maker, exif_data->exif_model);
  } else {
    g_print("Camera `%s %s' found in database\n",
        exif_data->exif_maker, exif_data->exif_model);
    const lfCamera *camera = cameras[0];
    lf_free (cameras);

    prop_camera_maker.update( camera->Maker );
    prop_camera_model.update( camera->Model );

    const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_data->exif_lens);
    if (!lenses) {
      g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
    } else {
      const lfLens *lens = lenses[0];
      lf_free (lenses);

      prop_lens.update( lens->Model );
    }
  }
#endif

  focal_length = exif_data->exif_focal_length;
  aperture = exif_data->exif_aperture;
  distance = exif_data->exif_focus_distance;

  std::cout<<"Maker: "<<exif_data->exif_maker<<"  model: "<<exif_data->exif_model
      <<"  lens: "<<exif_data->exif_lens<<std::endl;

  VipsInterpolate* interpolate = NULL; //vips_interpolate_new( "nohalo" );
  if( !interpolate )
    interpolate = vips_interpolate_new( "bicubic" );
  if( !interpolate )
    interpolate = vips_interpolate_new( "bilinear" );

  if( vips_lensfun(in[0], &outnew, get_processor(), interpolate, NULL) ) {
    std::cout<<"vips_lensfun() failed."<<std::endl;
    PF_REF( in[0], "LensFunPar::build(): in[0] ref (exif data wrong size)")
    return in[0];
  }

  PF_UNREF( interpolate, "vips_lensfun(): interpolate unref" );

  //outnew = in[0];
  //PF_REF( outnew, "LensFunPar::build(): in[0] ref")

//#ifndef NDEBUG
  std::cout<<"LensFunPar::build(): outnew refcount ("<<(void*)outnew<<") = "<<G_OBJECT(outnew)->ref_count<<std::endl;
//#endif
  return outnew;
}


PF::ProcessorBase* PF::new_lensfun()
{
  return new PF::Processor<PF::LensFunPar,PF::LensFunProc>();
}
