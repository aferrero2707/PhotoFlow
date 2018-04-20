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

#include <string.h>
#include "lensfun.hh"

int vips_lensfun( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, VipsInterpolate* interpolate, ... );


PF::LensFunParStep::LensFunParStep():
            OpParBase(),
            auto_matching( true ),
            auto_crop( false ),
            enable_distortion( true ),
            enable_tca( true ),
            enable_vignetting( true )
{
  set_type("lensfun_step" );
}


const lfLens* PF::lf_get_lens( VipsImage* img, lfDatabase* ldb )
{
  /*
  if( !img ) return NULL;

  size_t blobsz;
  PF::exif_data_t* exif_data;
  if( vips_image_get_blob( img, PF_META_EXIF_NAME, (void**)&exif_data, &blobsz ) ) {
    std::cout<<"LensFunPar::get_flags() could not extract exif_custom_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(PF::exif_data_t) ) {
    std::cout<<"LensFunPar::get_flags() wrong exif_custom_data size."<<std::endl;
    return NULL;
  }

  float focal_length = exif_data->exif_focal_length;
  float aperture = exif_data->exif_aperture;
  float distance = exif_data->exif_focus_distance;

  const lfCamera** cameras = ldb->FindCameras( exif_data->exif_maker,
      exif_data->exif_model );
  if( !cameras ) {
#ifndef NDEBUG
    g_print ("Cannot find the camera `%s %s' in database\n",
        exif_data->exif_maker, exif_data->exif_model);
#endif
    return NULL;
  }
#ifndef NDEBUG
  g_print("Camera `%s %s' found in database\n",
      exif_data->exif_maker, exif_data->exif_model);
#endif
  const lfCamera *camera = cameras[0];
  lf_free (cameras);

  char exif_lens[128];
  g_strlcpy(exif_lens, exif_data->exif_lens, sizeof(exif_lens));

  const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_lens);
  if (!lenses) {
    if(islower(camera->Mount[0])) {
      // This is a fixed-lens camera, and LF returned no lens.
      // Let's unset lens name and re-run lens query
      g_print("Camera `%s %s' is fixed-lens\n",
          exif_data->exif_maker, exif_data->exif_model);
      g_strlcpy(exif_lens, "", sizeof(exif_lens));
      lenses = ldb->FindLenses (camera, NULL, exif_lens);
    }
  }

  if (!lenses) {
#ifndef NDEBUG
    g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
#endif
    return NULL;
  }

  int lens_i = 0;
  if(islower(camera->Mount[0]) && !exif_lens[0]) {
    // no lens info in EXIF, and this is fixed-lens camera,
    // let's find shortest lens model in the list of possible lenses
    g_print("Camera `%s %s' is fixed-lens, searching for lens with shortest lens model\n",
        exif_data->exif_maker, exif_data->exif_model);
    size_t min_model_len = SIZE_MAX;
    for(int i = 0; lenses[i]; i++) {
      if(strlen(lenses[i]->Model) < min_model_len) {
        min_model_len = strlen(lenses[i]->Model);
        g_print("  Lens with shortest model name: `%d %s'\n",
            i, lenses[i]->Model);
        lens_i = i;
      }
    }
  }

  const lfLens *lens = lenses[lens_i];
  lf_free (lenses);
#ifndef NDEBUG
  g_print("Found lens: `%d %s'\n", lens_i, lens->Model);
#endif
  return lens;
  */
  return NULL;
}




int PF::LensFunParStep::get_flags( VipsImage* img )
{
  int flags = 0;
  std::cout<<"LensFunParStep::get_flags(): img="<<img<<std::endl;
  if( !img ) return flags;

  PF::exif_data_t* exif_data = PF::get_exif_data( img );
  if( !exif_data ) {
    return flags;
  }
  std::cout<<"LensFunParStep::get_flags(): exif_data="<<exif_data<<std::endl;


  float focal_length = exif_data->exif_focal_length;
  float aperture = exif_data->exif_aperture;
  float distance = exif_data->exif_focus_distance;

  Glib::ustring cam_make, cam_model, lens_model;
  if( auto_matching ) {
    cam_make = exif_data->exif_maker;
    cam_model = exif_data->exif_model;
    lens_model = exif_data->exif_lens;
  } else {
    cam_make = prop_camera_maker;
    cam_model = prop_camera_model;
    lens_model = prop_lens;
  }

  std::cout<<"LensFunParStep::get_flags(): auto_matching="<<auto_matching
      <<" cam_make="<<cam_make<<" cam_model="<<cam_model<<" lens_model="<<lens_model<<std::endl;
  rtengine::LFCamera lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_make, cam_model);
  rtengine::LFLens lflens = rtengine::LFLens();
  if( lfcamera ) {
    std::cout<<"LensFunParStep::get_flags(): camera "<<lfcamera.getMake()<<" / "<<lfcamera.getModel()<<" found in database"<<std::endl;
    lflens  = rtengine::LFDatabase::getInstance()->findLens(lfcamera, lens_model, !auto_matching);
    if( lflens ) {
      std::cout<<"LensFunParStep::get_flags(): lens "<<lflens.getMake()<<" / "<<lflens.getLens()<<" ("<<exif_data->exif_lens<<") found in database"<<std::endl;
    } else {
      return flags;
    }
  } else {
    return flags;
  }


  const lfCamera *camera = lfcamera.data_;
  std::cout<<"LensFunParStep::get_flags(): camera="<<camera<<std::endl;
  const lfLens *lens = lflens.data_;
  std::cout<<"LensFunParStep::get_flags(): lens="<<lens<<std::endl;
  lfModifier* modifier = new lfModifier( lens, camera->CropFactor,
      img->Xsize, img->Ysize );
  flags = modifier->Initialize( lens, LF_PF_F32,
      focal_length, aperture, distance, 1.0, lens->Type,
      LF_MODIFY_ALL, false );
  delete modifier;


  /*

  const lfCamera** cameras = ldb->FindCameras( exif_data->exif_maker,
      exif_data->exif_model );
  if( !cameras ) {
#ifndef NDEBUG
    g_print ("Cannot find the camera `%s %s' in database\n",
        exif_data->exif_maker, exif_data->exif_model);
#endif
    return flags;
  } else {
#ifndef NDEBUG
    g_print("Camera `%s %s' found in database\n",
        exif_data->exif_maker, exif_data->exif_model);
#endif
    const lfCamera *camera = cameras[0];
    lf_free (cameras);

    prop_camera_maker = camera->Maker;
    prop_camera_model = camera->Model;

    const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_data->exif_lens);
    if (!lenses) {
#ifndef NDEBUG
      g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
#endif
      return flags;
    } else {
      const lfLens *lens = lenses[0];
      lf_free (lenses);

      lfModifier* modifier = new lfModifier( lens, camera->CropFactor,
          img->Xsize, img->Ysize );
      flags = modifier->Initialize(
          lens, LF_PF_F32, focal_length, aperture, distance, 1.0, lens->Type,
          LF_MODIFY_ALL, false );
      delete modifier;
    }
  }
  */
  return flags;
}


VipsImage* PF::LensFunParStep::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* outnew = NULL;
#ifndef NDEBUG
  std::cout<<"LensFunParStep::build() called."<<std::endl;
#endif
  if( (in.size() < 1) || (in[0] == NULL) )
    return NULL;

  PF::exif_data_t* exif_data = get_exif_data( in[0] );
  if( !exif_data && auto_matching ) {
    PF_REF( in[0], "LensFunParStep::build(): in[0] ref (exif data missing)")
    return in[0];
  }

  focal_length = exif_data->exif_focal_length;
  aperture = exif_data->exif_aperture;
  distance = exif_data->exif_focus_distance;

#ifndef NDEBUG
  std::cout<<"Maker: "<<exif_data->exif_maker<<"  model: "<<exif_data->exif_model
      <<"  lens: "<<prop_lens<<std::endl;
#endif

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


  g_print("LensFunPar::build(): distortion_enabled()=%d  auto_crop=%d\n", distortion_enabled(), auto_crop);

  if( distortion_enabled() && auto_crop ) {
    int flags = 0;
    flags |= LF_MODIFY_DISTORTION;
    lfModifier* modifier = new lfModifier( get_lens(), get_camera()->CropFactor,
        outnew->Xsize, outnew->Ysize );
    int modflags = modifier->Initialize(
        get_lens(), LF_PF_F32, get_focal_length(),
        get_aperture(), get_distance(), 1.0, get_lens()->Type,
        flags, false );

    g_print("LensFunPar::build(): modflags: %d\n", modflags);

    if( modflags & LF_MODIFY_DISTORTION ) {
      float scale = 1.f/modifier->GetAutoScale(false);
      g_print("LensFunPar::build(): scale: %f\n", scale);
      if( scale < 1 ) {
        int cw = scale * outnew->Xsize - 1;
        int ch = scale * outnew->Ysize - 1;
        int cleft = (outnew->Xsize - cw)/2;
        int ctop = (outnew->Ysize - ch)/2;
        VipsImage* timg = NULL;
        if( vips_crop( outnew, &timg, cleft, ctop, cw, ch, NULL ) ) {
          std::cout<<"WARNIG: LensFunPar::build(): vips_crop() failed."<<std::endl;
          std::cout<<"outnew->Xsize="<<outnew->Xsize<<"  outnew->Ysize="<<outnew->Ysize<<std::endl;
          std::cout<<"vips_crop( outnew, &timg, "<<cleft<<", "<<ctop<<", "
              <<cw<<", "<<ch<<", NULL )"<<std::endl;
        } else {
          PF_UNREF( outnew, "vips_lensfun(): outnew unref" );
          outnew = timg;
        }
      }
    }
    delete modifier;
  }


  //outnew = in[0];
  //PF_REF( outnew, "LensFunPar::build(): in[0] ref")

#ifndef NDEBUG
  std::cout<<"LensFunPar::build(): outnew refcount ("<<(void*)outnew<<") = "<<G_OBJECT(outnew)->ref_count<<std::endl;
#endif
  return outnew;
}


PF::LensFunPar::LensFunPar():
            OpParBase(),
            prop_camera_maker( "camera_maker", this ),
            prop_camera_model( "camera_model", this ),
            prop_lens( "lens", this ),
            auto_matching( "auto_matching", this, true ),
            auto_crop( "auto_crop", this, false ),
            enable_distortion( "enable_distortion", this, true ),
            enable_tca( "enable_tca", this, true ),
            enable_vignetting( "enable_vignetting", this, false ),
            enable_all( "enable_all", this, false )
{
  step1 = new_lensfun_step();
  step2 = new_lensfun_step();

  set_type("lensfun" );

  set_default_name( _("optical corrections") );
}


float PF::LensFunPar::get_focal_length()
{
  PF::LensFunParStep* step1_par = dynamic_cast<PF::LensFunParStep*>( step1->get_par() );
  if( !step1_par ) return 0;
  return( step1_par->get_focal_length() );
}


float PF::LensFunPar::get_aperture()
{
  PF::LensFunParStep* step1_par = dynamic_cast<PF::LensFunParStep*>( step1->get_par() );
  if( !step1_par ) return 0;
  return( step1_par->get_aperture() );
}


float PF::LensFunPar::get_distance()
{
  PF::LensFunParStep* step1_par = dynamic_cast<PF::LensFunParStep*>( step1->get_par() );
  if( !step1_par ) return 0;
  return( step1_par->get_distance() );
}


int PF::LensFunPar::get_flags( VipsImage* img )
{
  std::cout<<"LensFunPar::get_flags() called"<<std::endl;

  int flags = 0;
  std::cout<<"LensFunPar::get_flags(): img="<<img<<std::endl;
  if( !img ) return flags;

  PF::exif_data_t* exif_data = PF::get_exif_data( img );
  if( !exif_data ) {
    return flags;
  }
  std::cout<<"LensFunPar::get_flags(): exif_data="<<exif_data<<std::endl;


  float focal_length = exif_data->exif_focal_length;
  float aperture = exif_data->exif_aperture;
  float distance = exif_data->exif_focus_distance;

  Glib::ustring cam_make, cam_model, lens_model;
  if( auto_matching.get() ) {
    cam_make = exif_data->exif_maker;
    cam_model = exif_data->exif_model;
    lens_model = exif_data->exif_lens;
  } else {
    cam_make = prop_camera_maker.get();
    cam_model = prop_camera_model.get();
    lens_model = prop_lens.get();
    if(cam_make.empty()) cam_make = exif_data->exif_maker;
    if(cam_model.empty()) cam_model = exif_data->exif_model;
    if(lens_model.empty()) lens_model = exif_data->exif_lens;
  }

  std::cout<<"LensFunPar::get_flags(): auto_matching="<<auto_matching.get()
      <<" cam_make="<<cam_make<<" cam_model="<<cam_model<<" lens_model="<<lens_model<<std::endl;
  rtengine::LFCamera lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_make, cam_model);
  rtengine::LFLens lflens = rtengine::LFLens();
  if( lfcamera ) {
    std::cout<<"LensFunPar::get_flags(): camera "<<lfcamera.getMake()<<" / "<<lfcamera.getModel()<<" found in database"<<std::endl;
    lflens  = rtengine::LFDatabase::getInstance()->findLens(lfcamera, lens_model, !(auto_matching.get()));
    if( lflens ) {
      std::cout<<"LensFunParStep::get_flags(): lens "<<lflens.getMake()<<" / "<<lflens.getLens()<<" ("<<exif_data->exif_lens<<") found in database"<<std::endl;
    } else {
      g_print ("LensFunPar::get_flags(): cannot find the lens `%s' in database\n", lens_model.c_str());
      return flags;
    }
  } else {
    g_print ("LensFunPar::get_flags(): cannot find the camera `%s %s' in database\n",
        cam_make.c_str(), cam_model.c_str());
    return flags;
  }


  const lfCamera *camera = lfcamera.data_;
  std::cout<<"LensFunPar::get_flags(): camera="<<camera<<std::endl;
  const lfLens *lens = lflens.data_;
  std::cout<<"LensFunPar::get_flags(): lens="<<lens<<std::endl;
  lfModifier* modifier = new lfModifier( lens, camera->CropFactor,
      img->Xsize, img->Ysize );
  flags = modifier->Initialize( lens, LF_PF_F32,
      focal_length, aperture, distance, 1.0, lens->Type,
      LF_MODIFY_ALL, false );
  delete modifier;

  return flags;

  //PF::LensFunParStep* step1_par = dynamic_cast<PF::LensFunParStep*>( step1->get_par() );
  //if( !step1_par ) return 0;
  //return( step1_par->get_flags(img) );
}


VipsImage* PF::LensFunPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;

  std::cout<<"LensFunPar::build(): called"<<std::endl;

  VipsImage* outnew = NULL;
#ifndef NDEBUG
  std::cout<<"LensFunPar::build() called."<<std::endl;
#endif
  if( (in.size() < 1) || (in[0] == NULL) )
    return NULL;

  PF::exif_data_t* exif_data = get_exif_data( in[0] );
  if( !exif_data && auto_matching.get() ) {
    PF_REF( in[0], "LensFunPar::build(): in[0] ref (exif data missing)")
    return in[0];
  }


  Glib::ustring cam_make, cam_model, lens_model;
  if( auto_matching.get() ) {
    cam_make = exif_data->exif_maker;
    cam_model = exif_data->exif_model;
    lens_model = exif_data->exif_lens;
  } else {
    cam_make = prop_camera_maker.get();
    cam_model = prop_camera_model.get();
    lens_model = prop_lens.get();
    if(cam_make.empty()) cam_make = exif_data->exif_maker;
    if(cam_model.empty()) cam_model = exif_data->exif_model;
    if(lens_model.empty()) lens_model = exif_data->exif_lens;
  }
  //lens_model = "Sigma 30mm f/1.4 EX DC HSM";

  std::cout<<"LensFunPar::build(): auto_matching="<<auto_matching.get()
      <<" cam_make="<<cam_make<<" cam_model="<<cam_model<<" lens_model="<<lens_model<<std::endl;
  lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_make, cam_model);
  lflens = rtengine::LFLens();
  if( lfcamera ) {
    std::cout<<"LensFunPar::build(): camera "<<lfcamera.getMake()<<" / "<<lfcamera.getModel()<<" found in database"<<std::endl;
    lflens  = rtengine::LFDatabase::getInstance()->findLens(lfcamera, lens_model, !(auto_matching.get()));
    if( lflens ) {
      std::cout<<"LensFunPar::build(): lens "<<lflens.getMake()<<" / "<<lflens.getLens()<<" ("<<exif_data->exif_lens<<") found in database"<<std::endl;
    } else {
      g_print ("LensFunPar::build(): cannot find the lens `%s' in database\n", lens_model.c_str());
      PF_REF( in[0], "LensFunPar::build(): lens not found in lensfun database." );
      return( in[0] );
    }
  } else {
    g_print ("LensFunPar::build(): cannot find the camera `%s %s' in database\n",
        cam_make.c_str(), cam_model.c_str());
    PF_REF( in[0], "LensFunPar::build(): camera not found in lensfun database." );
    return( in[0] );
  }

#ifndef NDEBUG
  std::cout<<"Maker: "<<exif_data->exif_maker<<"  model: "<<exif_data->exif_model
      <<"  lens: "<<prop_lens<<std::endl;
#endif

  std::vector<VipsImage*> in2;
  PF::LensFunParStep* step1_par = dynamic_cast<PF::LensFunParStep*>( step1->get_par() );
  if( !step1_par ) {
    std::cerr<<"LensFunPar::build(): NULL step1_par"<<std::endl;
    PF_REF(in[0], "LensFunPar::build(): in[0] ref after NULL step1_par");
    return(in[0]);
  }
  step1_par->set_image_hints( in[0] );
  step1_par->set_format( get_format() );
  step1_par->set_camera_maker( cam_make );
  step1_par->set_camera_model( cam_model );
  step1_par->set_lens( lens_model );
  step1_par->set_lfcamera( lfcamera );
  step1_par->set_lflens( lflens );
  step1_par->set_auto_matching_enabled( auto_matching.get() );
  step1_par->set_auto_crop_enabled( false );
  step1_par->set_vignetting_enabled( enable_vignetting.get() );
  step1_par->set_distortion_enabled( false );
  step1_par->set_tca_enabled( false );
  in2.clear(); in2.push_back( in[0] );
  VipsImage* out1 = step1_par->build( in2, 0, NULL, NULL, level );

  //prop_camera_maker.update( step1_par->camera_maker() );
  //prop_camera_model.update( step1_par->camera_model() );
  //prop_lens.update( step1_par->lens() );

  PF::LensFunParStep* step2_par = dynamic_cast<PF::LensFunParStep*>( step2->get_par() );
  if( !step2_par ) {
    std::cerr<<"LensFunPar::build(): NULL step2_par"<<std::endl;
    return(out1);
  }
  std::cerr<<"LensFunPar::build(): building step2_par, auto_crop="<<auto_crop.get()<<std::endl;
  step2_par->set_image_hints( out1 );
  step2_par->set_format( get_format() );
  step2_par->set_camera_maker( cam_make );
  step2_par->set_camera_model( cam_model );
  step2_par->set_lens( lens_model );
  step2_par->set_lfcamera( lfcamera );
  step2_par->set_lflens( lflens );
  step2_par->set_auto_matching_enabled( auto_matching.get() );
  step2_par->set_auto_crop_enabled( auto_crop.get() );
  step2_par->set_vignetting_enabled( false );
  step2_par->set_distortion_enabled( enable_distortion.get() );
  step2_par->set_tca_enabled( enable_tca.get() );
  in2.clear(); in2.push_back( out1 );
  out = step2_par->build( in2, 0, NULL, NULL, level );
  g_object_unref( out1 );

  return out;
}
