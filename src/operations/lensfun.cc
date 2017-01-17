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

#include "lensfun.hh"

int vips_lensfun( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, VipsInterpolate* interpolate, ... );


PF::LensFunParStep::LensFunParStep():
    OpParBase(),
    enable_distortion( true ),
    enable_tca( true ),
    enable_vignetting( true )
{
#ifdef PF_HAS_LENSFUN
  ldb = lf_db_new();
#if (BUNDLED_LENSFUN == 1)
  Glib::ustring lfdb = PF::PhotoFlow::Instance().get_lensfun_db_dir();
  ldb->LoadDirectory( lfdb.c_str() );
  std::cout<<"LensFun database loaded from "<<lfdb<<std::endl;
#else
  //char* lfdb_env = getenv("PF_LENSFUN_DATA_DIR");
  //if( lfdb_env ) {
  //  ldb->LoadDirectory( lfdb.c_str() );
  //  std::cout<<"LensFun database loaded from "<<lfdb_env<<std::endl;
  //} else {
    ldb->Load ();
    std::cout<<"LensFun database loaded from default location"<<std::endl;
  //}
#endif //(BUNDLED_LENSFUN == 1)
#endif

  set_type("lensfun_step" );
}


int PF::LensFunParStep::get_flags( VipsImage* img )
{
  int flags = 0;

  if( !img ) return flags;

  size_t blobsz;
  PF::exif_data_t* exif_data;
  if( vips_image_get_blob( img, PF_META_EXIF_NAME, (void**)&exif_data, &blobsz ) ) {
    std::cout<<"LensFunPar::get_flags() could not extract exif_custom_data."<<std::endl;
    return flags;
  }
  if( blobsz != sizeof(PF::exif_data_t) ) {
    std::cout<<"LensFunPar::get_flags() wrong exif_custom_data size."<<std::endl;
    return flags;
  }

  float focal_length = exif_data->exif_focal_length;
  float aperture = exif_data->exif_aperture;
  float distance = exif_data->exif_focus_distance;

#ifdef PF_HAS_LENSFUN
  const lfCamera** cameras = ldb->FindCameras( exif_data->exif_maker,
      exif_data->exif_model );
  if( !cameras ) {
    g_print ("Cannot find the camera `%s %s' in database\n",
        exif_data->exif_maker, exif_data->exif_model);
    return flags;
  } else {
    g_print("Camera `%s %s' found in database\n",
        exif_data->exif_maker, exif_data->exif_model);
    const lfCamera *camera = cameras[0];
    lf_free (cameras);

    prop_camera_maker = camera->Maker;
    prop_camera_model = camera->Model;

    const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_data->exif_lens);
    if (!lenses) {
      g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
      return flags;
    } else {
      const lfLens *lens = lenses[0];
      lf_free (lenses);

      lfModifier* modifier = lfModifier::Create( lens, lens->CropFactor,
          img->Xsize, img->Ysize );
      flags = modifier->Initialize(
          lens, LF_PF_F32, focal_length, aperture, distance, 1.0, lens->Type,
          LF_MODIFY_ALL, false );
      lf_free (modifier);
    }
  }
#endif

  return flags;
}


VipsImage* PF::LensFunParStep::build(std::vector<VipsImage*>& in, int first,
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
    PF_REF( in[0], "LensFunPar::build(): camera not found in lensfun database." );
    return( in[0] );
  } else {
    g_print("Camera `%s %s' found in database\n",
        exif_data->exif_maker, exif_data->exif_model);
    const lfCamera *camera = cameras[0];
    lf_free (cameras);

    prop_camera_maker = camera->Maker;
    prop_camera_model = camera->Model;

    const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_data->exif_lens);
    if (!lenses) {
      g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
      PF_REF( in[0], "LensFunPar::build(): lens not found in lensfun database." );
      return( in[0] );
    } else {
      const lfLens *lens = lenses[0];
      lf_free (lenses);

      prop_lens = lens->Model;
    }
  }
#endif

  focal_length = exif_data->exif_focal_length;
  aperture = exif_data->exif_aperture;
  distance = exif_data->exif_focus_distance;

  std::cout<<"Maker: "<<exif_data->exif_maker<<"  model: "<<exif_data->exif_model
      <<"  lens: "<<prop_lens<<std::endl;

  if( !enable_distortion && !enable_tca && !enable_vignetting ) {
    PF_REF( in[0], "LensFunPar::build(): no LENSFUN corrections requested." );
    return( in[0] );
  }

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


PF::LensFunPar::LensFunPar():
    OpParBase(),
    prop_camera_maker( "camera_maker", this ),
    prop_camera_model( "camera_model", this ),
    prop_lens( "lens", this ),
    enable_distortion( "enable_distortion", this, true ),
    enable_tca( "enable_tca", this, true ),
    enable_vignetting( "enable_vignetting", this, false ),
    enable_all( "enable_all", this, false )
{
  set_type("lensfun" );

  set_default_name( _("optical corrections") );
}


VipsImage* PF::LensFunPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;

  std::vector<VipsImage*> in2;
  step1.get_par()->set_image_hints( in[0] );
  step1.get_par()->set_format( get_format() );
  step1.get_par()->set_vignetting_enabled( enable_vignetting.get() );
  step1.get_par()->set_distortion_enabled( false );
  step1.get_par()->set_tca_enabled( false );
  in2.clear(); in2.push_back( in[0] );
  VipsImage* out1 = step1.get_par()->build( in2, 0, NULL, NULL, level );

  prop_camera_maker.update( step1.get_par()->camera_maker() );
  prop_camera_model.update( step1.get_par()->camera_model() );
  prop_lens.update( step1.get_par()->lens() );

  step2.get_par()->set_image_hints( out1 );
  step2.get_par()->set_format( get_format() );
  step2.get_par()->set_vignetting_enabled( false );
  step2.get_par()->set_distortion_enabled( enable_distortion.get() );
  step2.get_par()->set_tca_enabled( enable_tca.get() );
  in2.clear(); in2.push_back( out1 );
  out = step2.get_par()->build( in2, 0, NULL, NULL, level );
  g_object_unref( out1 );

  return out;
}



PF::ProcessorBase* PF::new_lensfun()
{
  return new PF::Processor<PF::LensFunPar,PF::LensFunProc>();
}
