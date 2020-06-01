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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <lcms2.h>

#include <iostream>
#include <map>

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfStandardAttributes.h>
#include <OpenEXR/ImfThreading.h>
#include <OpenEXR/ImfOutputFile.h>

#include <vips/vips.h>

#include <ciso646>

#include <memory>

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfStandardAttributes.h>
#include <OpenEXR/ImfTestFile.h>
#include <OpenEXR/ImfTiledInputFile.h>

#ifdef OPENEXR_IMF_INTERNAL_NAMESPACE
#define IMF_NS OPENEXR_IMF_INTERNAL_NAMESPACE
#else
#define IMF_NS Imf
#endif


static bool VIPS_EXR_INITIALIZED = false;

// this stores our exif data as a blob.

template <typename T> struct array_deleter
{
  void operator()(T const *p)
  {
    delete[] p;
  }
};

namespace IMF_NS
{
class Blob
{
public:
  Blob() : size(0), data((uint8_t *)NULL)
  {
  }

  Blob(uint32_t _size, uint8_t *_data) : size(_size)
  {
    uint8_t *tmp_ptr = new uint8_t[_size];
    memcpy(tmp_ptr, _data, _size);
    data.reset(tmp_ptr, array_deleter<uint8_t>());
  }

  uint32_t size;
#if defined(_LIBCPP_VERSION)
  std::shared_ptr<uint8_t> data;
#else
  std::tr1::shared_ptr<uint8_t> data;
#endif
};


typedef IMF_NS::TypedAttribute<IMF_NS::Blob> BlobAttribute;
template <> const char *BlobAttribute::staticTypeName()
{
  return "blob";
}
template <> void BlobAttribute::writeValueTo(OStream &os, int version) const
{
  Xdr::write<StreamIO>(os, _value.size);
  Xdr::write<StreamIO>(os, (char *)(_value.data.get()), _value.size);
}

template <> void BlobAttribute::readValueFrom(IStream &is, int size, int version)
{
  Xdr::read<StreamIO>(is, _value.size);
  _value.data.reset(new uint8_t[_value.size], array_deleter<uint8_t>());
  Xdr::read<StreamIO>(is, (char *)(_value.data.get()), _value.size);
}
}


struct VipsEXRWrite
{
  Imf::OutputFile* file;
  std::map<int, VipsRegion*> strips;
};



static int copy_region(VipsRegion* region, VipsRect *area, VipsEXRWrite* exr_write)
{
  Imf::OutputFile* file = exr_write->file;

  float* p = (float*)VIPS_REGION_ADDR( region, area->left, area->top );

  Imf::FrameBuffer data;

  int width = region->im->Xsize;
  size_t offset = width * area->top * 3;
  const float *in = (const float *)(p - offset);

  data.insert("R", Imf::Slice(Imf::PixelType::FLOAT, (char *)(in + 0), 3 * sizeof(float),
                              3 * sizeof(float) * width));

  data.insert("G", Imf::Slice(Imf::PixelType::FLOAT, (char *)(in + 1), 3 * sizeof(float),
                              3 * sizeof(float) * width));

  data.insert("B", Imf::Slice(Imf::PixelType::FLOAT, (char *)(in + 2), 3 * sizeof(float),
                              3 * sizeof(float) * width));

  file->setFrameBuffer(data);
  file->writePixels(area->height);

  return 0;
}


static int write_strip(VipsRegion *region, VipsRect *area, void *a)
{
//#ifdef DEBUG
  printf( "[write_strip] strip at %d, height %d\n",
    area->top, area->height );
//#endif/*DEBUG*/

  VipsEXRWrite* exr_write = (VipsEXRWrite*)a;
  Imf::OutputFile* file = exr_write->file;

  int sl = file->currentScanLine();
  if(sl > area->top) {
    printf("[vips_exrsave] error: beginning of strip lower than current scanline (%d, %d)\n", area->top, sl);
    return 1;
  }
  if(sl < area->top) {
    // The EXR file has to be filled sequentially.
    // We need to store regions that are generated too early.
    VipsRegion* r = vips_region_new(region->im);
    vips_region_copy(region, r, area, area->left, area->top);
    exr_write->strips.insert(std::make_pair(area->top, r));
    return 0;
  }

  copy_region(region, area, exr_write);

  while(true) {
    sl = file->currentScanLine();
    std::map<int, VipsRegion*>::iterator i = exr_write->strips.find(sl);
    if(i == exr_write->strips.end()) break;

    VipsRegion* r = i->second;
    copy_region(r, &(r->valid), exr_write);
    VIPS_UNREF(r);
    exr_write->strips.erase(i);
  }

  return( 0 );
}


int vips_exrsave(VipsImage *in, const char *filename, int halfFloat, void *exif, int exif_len)
{
  if(!VIPS_EXR_INITIALIZED) {
    Imf::BlobAttribute::registerAttributeType();
    VIPS_EXR_INITIALIZED = true;
  }

  const Imf::Compression ctypes[6] = {
    Imf::NO_COMPRESSION,
    Imf::ZIPS_COMPRESSION,
    Imf::ZIP_COMPRESSION,
    Imf::PIZ_COMPRESSION,
    Imf::RLE_COMPRESSION,
    Imf::B44_COMPRESSION
  };

  Imf::setGlobalThreadCount(1);

  Imf::Header header(in->Xsize, in->Ysize, 1, Imath::V2f(0, 0), 1, Imf::INCREASING_Y,
                     (Imf::Compression)Imf::NO_COMPRESSION);

  char comment[1024];
  snprintf(comment, sizeof(comment), "Developed using PhotoFlow");

  header.insert("comment", Imf::StringAttribute(comment));

  Imf::Blob exif_blob(exif_len, (uint8_t *)exif);
  header.insert("exif", Imf::BlobAttribute(exif_blob));

  void *iccdata;
  size_t iccdata_length;

  if( !vips_image_get_blob(in, VIPS_META_ICC_NAME, (const void**)(&iccdata), &iccdata_length) ) {
    cmsToneCurve *red_curve = NULL,
                 *green_curve = NULL,
                 *blue_curve = NULL;
    cmsCIEXYZ *red_color = NULL,
              *green_color = NULL,
              *blue_color = NULL;
    cmsHPROFILE out_profile = cmsOpenProfileFromMem( iccdata, iccdata_length );
    float r[2], g[2], b[2], w[2];
    float sum;
    Imf::Chromaticities chromaticities;

    if(cmsIsMatrixShaper(out_profile)) {

      red_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigRedTRCTag);
      green_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigGreenTRCTag);
      blue_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigBlueTRCTag);

      red_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigRedColorantTag);
      green_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigGreenColorantTag);
      blue_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigBlueColorantTag);

      //if(!red_curve || !green_curve || !blue_curve || !red_color || !green_color || !blue_color)
      //  goto icc_error;

      //if(!cmsIsToneCurveLinear(red_curve) || !cmsIsToneCurveLinear(green_curve) || !cmsIsToneCurveLinear(blue_curve))
      //  goto icc_error;

      //     printf("r: %f %f %f\n", red_color->X, red_color->Y, red_color->Z);
      //     printf("g: %f %f %f\n", green_color->X, green_color->Y, green_color->Z);
      //     printf("b: %f %f %f\n", blue_color->X, blue_color->Y, blue_color->Z);
      //     printf("w: %f %f %f\n", white_point->X, white_point->Y, white_point->Z);

      sum = red_color->X + red_color->Y + red_color->Z;
      r[0] = red_color->X / sum;
      r[1] = red_color->Y / sum;
      sum = green_color->X + green_color->Y + green_color->Z;
      g[0] = green_color->X / sum;
      g[1] = green_color->Y / sum;
      sum = blue_color->X + blue_color->Y + blue_color->Z;
      b[0] = blue_color->X / sum;
      b[1] = blue_color->Y / sum;

      // hard code the white point to D50 as the primaries from the ICC should be adapted to that
      // calculated from D50 illuminant XYZ values in ICC specs
      w[0] = 0.345702915;
      w[1] = 0.358538597;

      chromaticities.red = Imath::V2f(r[0], r[1]);
      chromaticities.green = Imath::V2f(g[0], g[1]);
      chromaticities.blue = Imath::V2f(b[0], b[1]);
      chromaticities.white = Imath::V2f(w[0], w[1]);

      Imf::addChromaticities(header, chromaticities);
      Imf::addWhiteLuminance(header, 1.0); // just assume 1 here
    }
    cmsCloseProfile(out_profile);
  }

/*  char *xmp_string = dt_exif_xmp_read_string(imgid);
  if(xmp_string)
  {
    header.insert("xmp", Imf::StringAttribute(xmp_string));
    g_free(xmp_string);
  }

  // try to add the chromaticities
  if(imgid > 0)
  {
    cmsToneCurve *red_curve = NULL,
                 *green_curve = NULL,
                 *blue_curve = NULL;
    cmsCIEXYZ *red_color = NULL,
              *green_color = NULL,
              *blue_color = NULL;
    cmsHPROFILE out_profile = dt_colorspaces_get_output_profile(imgid, over_type, over_filename)->profile;
    float r[2], g[2], b[2], w[2];
    float sum;
    Imf::Chromaticities chromaticities;

    if(!cmsIsMatrixShaper(out_profile)) goto icc_error;

    red_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigRedTRCTag);
    green_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigGreenTRCTag);
    blue_curve = (cmsToneCurve *)cmsReadTag(out_profile, cmsSigBlueTRCTag);

    red_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigRedColorantTag);
    green_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigGreenColorantTag);
    blue_color = (cmsCIEXYZ *)cmsReadTag(out_profile, cmsSigBlueColorantTag);

    if(!red_curve || !green_curve || !blue_curve || !red_color || !green_color || !blue_color)
      goto icc_error;

    if(!cmsIsToneCurveLinear(red_curve) || !cmsIsToneCurveLinear(green_curve) || !cmsIsToneCurveLinear(blue_curve))
      goto icc_error;

//     printf("r: %f %f %f\n", red_color->X, red_color->Y, red_color->Z);
//     printf("g: %f %f %f\n", green_color->X, green_color->Y, green_color->Z);
//     printf("b: %f %f %f\n", blue_color->X, blue_color->Y, blue_color->Z);
//     printf("w: %f %f %f\n", white_point->X, white_point->Y, white_point->Z);

    sum = red_color->X + red_color->Y + red_color->Z;
    r[0] = red_color->X / sum;
    r[1] = red_color->Y / sum;
    sum = green_color->X + green_color->Y + green_color->Z;
    g[0] = green_color->X / sum;
    g[1] = green_color->Y / sum;
    sum = blue_color->X + blue_color->Y + blue_color->Z;
    b[0] = blue_color->X / sum;
    b[1] = blue_color->Y / sum;

    // hard code the white point to D50 as the primaries from the ICC should be adapted to that
    // calculated from D50 illuminant XYZ values in ICC specs
    w[0] = 0.345702915;
    w[1] = 0.358538597;

    chromaticities.red = Imath::V2f(r[0], r[1]);
    chromaticities.green = Imath::V2f(g[0], g[1]);
    chromaticities.blue = Imath::V2f(b[0], b[1]);
    chromaticities.white = Imath::V2f(w[0], w[1]);

    Imf::addChromaticities(header, chromaticities);
    Imf::addWhiteLuminance(header, 1.0); // just assume 1 here

    goto icc_end;

icc_error:
    dt_control_log("%s", _("the selected output profile doesn't work well with exr"));
    fprintf(stderr, "[exr export] warning: exporting with anything but linear matrix profiles might lead to wrong results when opening the image\n");
  }
icc_end:
*/

  if(halfFloat == 0) {
    header.channels().insert("R", Imf::Channel(Imf::PixelType::FLOAT));
    header.channels().insert("G", Imf::Channel(Imf::PixelType::FLOAT));
    header.channels().insert("B", Imf::Channel(Imf::PixelType::FLOAT));
  } else {
    header.channels().insert("R", Imf::Channel(Imf::PixelType::HALF));
    header.channels().insert("G", Imf::Channel(Imf::PixelType::HALF));
    header.channels().insert("B", Imf::Channel(Imf::PixelType::HALF));
  }

  Imf::OutputFile file(filename, header);

  VipsEXRWrite exr_write;
  exr_write.file = &file;

  vips_sink_disc(in, write_strip, &exr_write);

  return 0;
}
