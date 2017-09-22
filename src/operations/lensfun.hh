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

/*
#ifdef PF_LENSFUN_HH
#warning "PF_LENSFUN_HH already defined!!!"
#else
#warning "PF_LENSFUN_HH not defined."
#endif
*/

#ifndef PF_LENSFUNOP_HH
#define PF_LENSFUNOP_HH

#include <string>

#if (BUNDLED_LENSFUN == 1)
#include <lensfun/lensfun.h>
#else
#include <lensfun.h>
#endif

#include "../base/exif_data.hh"
#include "../base/property.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"


namespace PF
{

#ifdef PF_HAS_LENSFUN
const lfLens* lf_get_lens( VipsImage* img, lfDatabase* ldb );
#endif

class LensFunParStep: public OpParBase
{
  std::string prop_camera_maker;
  std::string prop_camera_model;
  std::string prop_lens;

  bool enable_distortion, enable_tca, enable_vignetting;
#ifdef PF_HAS_LENSFUN
  lfDatabase* ldb;
#endif
  float focal_length, aperture, distance;

public:
  LensFunParStep();

  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_input() { return true; }
  bool needs_caching() { return false; }

  std::string camera_maker() { return prop_camera_maker; }
  std::string camera_model() { return prop_camera_model; }
  std::string lens() { return prop_lens; }

  float get_focal_length() { return( focal_length ); }
  float get_aperture() { return( aperture ); }
  float get_distance() { return( distance ); }

  int get_flags( VipsImage* img );

  bool distortion_enabled() { return enable_distortion; }
  bool tca_enabled() { return enable_tca; }
  bool vignetting_enabled() { return enable_vignetting; }

  void set_distortion_enabled(bool flag) { enable_distortion = flag; }
  void set_tca_enabled(bool flag) { enable_tca = flag; }
  void set_vignetting_enabled(bool flag) { enable_vignetting = flag; }

  virtual VipsImage* build(std::vector<VipsImage*>& in, int first,
         VipsImage* imap, VipsImage* omap, unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class LensFunProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};


class LensFunPar: public OpParBase
{
  Property<std::string> prop_camera_maker;
  Property<std::string> prop_camera_model;
  Property<std::string> prop_lens;

  Property<bool> enable_distortion, enable_tca, enable_vignetting, enable_all;

  Processor<LensFunParStep,PF::LensFunProc> step1;
  Processor<LensFunParStep,PF::LensFunProc> step2;

public:
  LensFunPar();

  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_input() { return true; }
  bool needs_caching() { return false; }

  std::string camera_maker() { return prop_camera_maker.get(); }
  std::string camera_model() { return prop_camera_model.get(); }
  std::string lens() { return prop_lens.get(); }

  float get_focal_length() { return( step1.get_par()->get_focal_length() ); }
  float get_aperture() { return( step1.get_par()->get_aperture() ); }
  float get_distance() { return( step1.get_par()->get_distance() ); }

  int get_flags( VipsImage* img ) { return( step1.get_par()->get_flags(img) ); }

  bool distortion_enabled() { return enable_distortion.get(); }
  bool tca_enabled() { return enable_tca.get(); }
  bool vignetting_enabled() { return enable_vignetting.get(); }

  void set_distortion_enabled(bool flag) { enable_distortion.update(flag); }
  void set_tca_enabled(bool flag) { enable_tca.update(flag); }
  void set_vignetting_enabled(bool flag) { enable_vignetting.update(flag); }

  virtual VipsImage* build(std::vector<VipsImage*>& in, int first, 
         VipsImage* imap, VipsImage* omap, unsigned int& level);
};



ProcessorBase* new_lensfun();
}

#endif
