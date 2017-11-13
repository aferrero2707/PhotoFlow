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

#ifndef GMIC_EMULATE_FILM_H
#define GMIC_EMULATE_FILM_H


#include "../base/processor.hh"


namespace PF 
{

  class GmicEmulateFilmBEPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmBEPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF > 
  class GmicEmulateFilmBEProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {	
    }
  };

  ProcessorBase* new_gmic_emulate_film_bw();



  class GmicEmulateFilmColorslidePar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmColorslidePar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmColorslideProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_colorslide();



  class GmicEmulateFilmInstantConsumerPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmInstantConsumerPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmInstantConsumerProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_instant_consumer();



  class GmicEmulateFilmInstantProPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmInstantProPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmInstantProProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_instant_pro();



  class GmicEmulateFilmNegativeColorPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmNegativeColorPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmNegativeColorProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_negative_color();



  class GmicEmulateFilmNegativeNewPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    PropertyBase prop_effect;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmNegativeNewPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmNegativeNewProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_negative_new();



  class GmicEmulateFilmNegativeOldPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    PropertyBase prop_effect;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmNegativeOldPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmNegativeOldProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_negative_old();



  class GmicEmulateFilmPrintFilmsPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmPrintFilmsPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmPrintFilmsProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_print_films();


  class GmicEmulateFilmVariousPar: public OpParBase
  {
    Property<int> iterations;
    PropertyBase prop_preset;
    Property<float> prop_opacity;
    Property<float> prop_gamma;
    Property<float> prop_contrast;
    Property<float> prop_brightness;
    Property<float> prop_hue;
    Property<float> prop_saturation;
    Property<int> prop_post_normalize;
    ProcessorBase* gmic;

  public:
    GmicEmulateFilmVariousPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_caching() { return false; }


    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  template < OP_TEMPLATE_DEF >
  class GmicEmulateFilmVariousProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_gmic_emulate_film_various();


  /*
    class GmicEmulateFilmUserDefinedPar: public OpParBase
    {
      Property<std::string> prop_filename;
      Property<std::string> prop_haldlutfilename;
      Property<float> prop_opacity;
      Property<float> prop_gamma;
      Property<float> prop_contrast;
      Property<float> prop_brightness;
      Property<float> prop_hue;
      Property<float> prop_saturation;
      Property<int> prop_post_normalize;
      ProcessorBase* gmic_proc;

      std::string cur_lut_filename;
      bool temp_lut_created;

      char* custom_gmic_commands;
      gmic* gmic_instance;

      gmic* new_gmic();

    public:
      GmicEmulateFilmUserDefinedPar();
      ~GmicEmulateFilmUserDefinedPar();

      bool has_intensity() { return false; }
      bool has_opacity() { return true; }
      bool needs_caching() { return false; }


      int get_gmic_padding( int level );

      void pre_build( rendermode_t mode );
      VipsImage* build(std::vector<VipsImage*>& in, int first,
                       VipsImage* imap, VipsImage* omap,
                       unsigned int& level);
    };

    template < OP_TEMPLATE_DEF >
    class GmicEmulateFilmUserDefinedProc
    {
    public:
      void render(VipsRegion** ireg, int n, int in_first,
                  VipsRegion* imap, VipsRegion* omap,
                  VipsRegion* oreg, OpParBase* par)
      {
      }
    };

    ProcessorBase* new_gmic_emulate_film_user_defined();
  */
}

#endif 


