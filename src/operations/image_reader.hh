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

#ifndef VIPS_IMAGE_READER_H
#define VIPS_IMAGE_READER_H

#include <string>

#include "../base/processor.hh"
#include "../base/imagepyramid.hh"

#include "../operations/convertformat.hh"
#include "../operations/blender.hh"
#include "../operations/raster_image.hh"

namespace PF 
{

  class ImageReaderPar: public OpParBase
  {
    Property<std::string> file_name;
    // output color profile
    PropertyBase in_profile_mode;
    PropertyBase in_trc_mode;
    PropertyBase out_profile_mode;
    PropertyBase out_trc_mode;

    VipsImage* image;
    PF::ProcessorBase* convert_format;
    PF::Processor<PF::BlenderPar,PF::BlenderProc>* blender;

    std::string current_file;
    VipsBandFormat current_format;

    cmsHTRANSFORM transform;

    RasterImage* raster_image;

    ImagePyramid pyramid;

  public:
    ImageReaderPar(): 
      OpParBase(), 
      file_name("file_name", this),
      //out_profile_mode("profile_mode",this,PF::OUT_PROF_REC2020,"REC2020","Rec.2020"),
      in_profile_mode("in_profile_mode",this,PF::OUT_PROF_EMBEDDED,"EMBEDDED",_("embedded")),
      in_trc_mode("in_trc_mode",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
      out_profile_mode("out_profile_mode",this,PF::OUT_PROF_EMBEDDED,"EMBEDDED",_("same")),
      out_trc_mode("out_trc_mode",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
      image(NULL),
      current_format(VIPS_FORMAT_NOTSET),
      transform( NULL ),
      raster_image( NULL )
    {
      in_profile_mode.add_enum_value(PF::OUT_PROF_NONE,"NONE","NONE");
      in_profile_mode.add_enum_value(PF::OUT_PROF_sRGB,"sRGB","Built-in sRGB");
      in_profile_mode.add_enum_value(PF::OUT_PROF_REC2020,"REC2020","Rec.2020");
      in_profile_mode.add_enum_value(PF::OUT_PROF_ACES,"ACES","ACES");
      //in_profile_mode.add_enum_value(PF::OUT_PROF_ADOBE,"ADOBE","Built-in Adobe RGB 1998");
      //in_profile_mode.add_enum_value(PF::OUT_PROF_PROPHOTO,"PROPHOTO","Built-in ProPhoto RGB");
      //in_profile_mode.add_enum_value(PF::OUT_PROF_LAB,"LAB","Lab");
      //in_profile_mode.add_enum_value(PF::OUT_PROF_CUSTOM,"CUSTOM","Custom");

      out_profile_mode.add_enum_value(PF::OUT_PROF_NONE,"NONE","NONE");
      out_profile_mode.add_enum_value(PF::OUT_PROF_sRGB,"sRGB","Built-in sRGB");
      out_profile_mode.add_enum_value(PF::OUT_PROF_REC2020,"REC2020","Rec.2020");
      out_profile_mode.add_enum_value(PF::OUT_PROF_ACES,"ACES","ACES");
      //out_profile_mode.add_enum_value(PF::OUT_PROF_ADOBE,"ADOBE","Built-in Adobe RGB 1998");
      //out_profile_mode.add_enum_value(PF::OUT_PROF_PROPHOTO,"PROPHOTO","Built-in ProPhoto RGB");
      //out_profile_mode.add_enum_value(PF::OUT_PROF_LAB,"LAB","Lab");
      //out_profile_mode.add_enum_value(PF::OUT_PROF_CUSTOM,"CUSTOM","Custom");

      //in_trc_mode.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR","linear");
      in_trc_mode.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL","perceptual");
      in_trc_mode.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

      //out_trc_mode.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR","linear");
      out_trc_mode.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL","perceptual");
      out_trc_mode.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

      convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
      blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
      set_type("imageread" );

      set_default_name( _("image layer") );
    }
    ~ImageReaderPar();

    cmsHTRANSFORM get_transform() { return transform; }

    std::string get_file_name() { return file_name.get_str(); }
    void set_file_name( const std::string& name ) { file_name.set_str( name ); }
    void set_file_name( const char* name ) { set_file_name( std::string( name ) ); }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ImageReader
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
      ImageReaderPar* opar = dynamic_cast<ImageReaderPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
      int width = r->width;
      int height = r->height;

      T* p;
      T* pin;
      T* pout;
      int x, y;

      for( y = 0; y < height; y++ ) {
        p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        pin = p;
        if(opar->get_transform())
          cmsDoTransform( opar->get_transform(), pin, pout, width );
        else
          memcpy( pout, pin, sizeof(T)*line_size );
      }
    }
  };




  ProcessorBase* new_image_reader();
}

#endif 


