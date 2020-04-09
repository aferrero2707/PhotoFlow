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

#ifndef ICC_TRANSFORM_H
#define ICC_TRANSFORM_H

#include <cstring>
#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../base/processor.hh"


namespace PF 
{

  class ICCTransformPar: public OpParBase
  {
    std::string in_profile_name;

    ICCProfile* in_profile;
    ICCProfile* out_profile;
    cmsUInt32Number intent;
    bool bpc;
    float adaptation_state;
    PF::ICCTransform transform;
    bool do_LCh, do_LSh, do_Lab;

    cmsColorSpaceSignature input_cs_type;
    cmsColorSpaceSignature output_cs_type;

    bool clip_negative, clip_overflow, gamut_mapping;
    float saturation_intent;

  public:

    ICCTransformPar();

    cmsUInt32Number get_intent() { return intent; }
    void set_intent( cmsUInt32Number i ) { intent = i; }

    void set_Lab_format() { do_Lab = true; do_LCh = do_LSh = false; }
    void set_LCh_format() { do_LCh = true; do_Lab = do_LSh = false; }
    void set_LSh_format() { do_LSh = true; do_Lab = do_LCh = false; }

    bool get_Lab_format() { return do_Lab; }
    bool get_LCh_format() { return do_LCh; }
    bool get_LSh_format() { return do_LSh; }

    bool get_bpc() { return bpc; }
    void set_bpc( bool flag ) { bpc = flag; }
    void set_adaptation_state( float s ) { adaptation_state = s; }

    PF::ICCTransform& get_transform() { return transform; }

    ICCProfile* get_in_profile() { return in_profile; }
    void set_out_profile( ICCProfile* p ) { out_profile = p; }
    ICCProfile* get_out_profile() { return out_profile; }

    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      //rgb_image( get_xsize(), get_ysize() );
    }

    cmsColorSpaceSignature get_input_cs_type() { return input_cs_type; }
    cmsColorSpaceSignature get_output_cs_type() { return output_cs_type; }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
       creation of an intensity map is not allowed
       2. the operation can work without an input image;
       the blending will be set in this case to "passthrough" and the image
       data will be simply linked to the output
    */
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    bool get_clip_negative() { return clip_negative; }
    bool get_clip_overflow() { return clip_overflow; }
    void set_clip_negative( bool flag ) { clip_negative = flag; }
    void set_clip_overflow( bool flag ) { clip_overflow = flag; }

    bool get_gamut_mapping() { return gamut_mapping; }
    void set_gamut_mapping( bool flag ) { gamut_mapping = flag; }
    float get_saturation_intent() { return saturation_intent; }
    void set_saturation_intent( float s ) { saturation_intent = s; }

    //cmsHPROFILE create_profile_from_matrix (const double matrix[3][3], bool gamma, Glib::ustring name);

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };



  ProcessorBase* new_icc_transform();
}

#endif 


