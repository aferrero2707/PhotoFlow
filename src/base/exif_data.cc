/* Exif metadata utilities.
 * Derived from Darktable (http://www.darktable.org/)
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
#include <assert.h>
#include <math.h>
#include <iostream>


#include <glib.h>

#include "../rt/rtengine/imagedata.h"
#include "../rt/rtengine/rawimage.h"
#include "exif_data.hh"
#include "photoflow.hh"


// inspired by ufraw_exiv2.cc:

static void dt_strlcpy_to_utf8(char *dest, size_t dest_max,
                               Exiv2::ExifData::const_iterator &pos, Exiv2::ExifData& exifData)
{
  std::string str = pos->print(&exifData);
  //g_print("dt_strlcpy_to_utf8(): str=%s\n",str.c_str());

  char *s = g_locale_to_utf8(str.c_str(), str.length(),
                             NULL, NULL, NULL);
  if ( s!=NULL )
  {
    g_strlcpy(dest, s, dest_max);
    g_free(s);
  }
  else
  {
    g_strlcpy(dest, str.c_str(), dest_max);
  }
  //g_print("dt_strlcpy_to_utf8(): dest=%s\n",dest);
}

PF::exif_data_t::exif_data_t(): exif_exposure(0),
    exif_aperture(0),
    exif_iso(0),
    exif_focal_length(0),
    exif_focus_distance(0),
    exif_crop(0)
{
  exif_maker[0] = '\0';
  exif_model[0] = '\0';
  exif_lens[0] = '\0';
  exif_datetime_taken[0] = '\0';
  camera_maker[0] = '\0';
  camera_model[0] = '\0';
  camera_alias[0] = '\0';
  camera_makermodel[0] = '\0';
  camera_legacy_makermodel[0] = '\0';
}


bool PF::exif_read(exif_data_t* data, const char* path)
{
  try
  {
    Exiv2::Image::AutoPtr image;
    image = Exiv2::ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();
    bool res = true;

    // EXIF metadata
    Exiv2::ExifData &exifData = image->exifData();
    if(exifData.empty())
      return false;

    /* List of tag names taken from exiv2's printSummary() in actions.cpp */
    Exiv2::ExifData::const_iterator pos;
    /* Read shutter time */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime")))
         != exifData.end() && pos->size())
    {
      // dt_strlcpy_to_utf8(uf->conf->shutterText, max_name, pos, exifData);
      data->exif_exposure = pos->toFloat ();
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue")))
              != exifData.end() && pos->size())
    {
      // uf_strlcpy_to_utf8(uf->conf->shutterText, max_name, pos, exifData);
      data->exif_exposure = 1.0/pos->toFloat ();
    }
    /* Read aperture */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.FNumber")))
         != exifData.end() && pos->size())
    {
      data->exif_aperture = pos->toFloat ();
    }
    else if ( (pos=exifData.findKey(
                     Exiv2::ExifKey("Exif.Photo.ApertureValue")))
              != exifData.end() && pos->size())
    {
      data->exif_aperture = pos->toFloat ();
    }
    /* Read ISO speed - Nikon happens to return a pair for Lo and Hi modes */
    if ( (pos=Exiv2::isoSpeed(exifData) )
         != exifData.end() && pos->size())
    {
      // if standard exif iso tag, use the old way of interpreting the return value to be more regression-save
      if (strcmp(pos->key().c_str(), "Exif.Photo.ISOSpeedRatings") == 0)
      {
        int isofield = pos->count () > 1  ? 1 : 0;
        data->exif_iso = pos->toFloat (isofield);
      }
      else
      {
        std::ostringstream os;
        pos->write(os, &exifData);
        std::string os_str = os.str();
        const char * exifstr = os_str.c_str();
        data->exif_iso = (float) std::atof( exifstr );
        // beware the following does not result in the same!:
        //data->exif_iso = (float) std::atof( pos->toString().c_str() );
      }
    }
#if EXIV2_MINOR_VERSION>19
    /* Read focal length  */
    if ( (pos=Exiv2::focalLength(exifData))
         != exifData.end() &&  pos->size())
    {
      data->exif_focal_length = pos->toFloat ();
    }

    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.NikonLd2.FocusDistance")))
         != exifData.end() && pos->size())
    {
      float value = pos->toFloat();
      data->exif_focus_distance = (0.01 * pow(10, value/40));
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.NikonLd3.FocusDistance")))
              != exifData.end() && pos->size())
    {
      float value = pos->toFloat();
      data->exif_focus_distance = (0.01 * pow(10, value/40));
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.OlympusFi.FocusDistance")))
              != exifData.end() && pos->size())
    {
      /* the distance is stored as a rational (fraction). according to http://www.dpreview.com/forums/thread/1173960?page=4
       * some Olympus cameras have a wrong denominator of 10 in there while the nominator is always in mm. thus we ignore the denominator
       * and divide with 1000.
       * "I've checked a number of E-1 and E-300 images, and I agree that the FocusDistance looks like it is in mm for the E-1. However,
       * it looks more like cm for the E-300.
       * For both cameras, this value is stored as a rational. With the E-1, the denominator is always 1, while for the E-300 it is 10.
       * Therefore, it looks like the numerator in both cases is in mm (which makes a bit of sense, in an odd sort of way). So I think
       * what I will do in ExifTool is to take the numerator and divide by 1000 to display the focus distance in meters."
       *   -- Boardhead, dpreview forums in 2005
       */
      int nominator = pos->toRational(0).first;
      data->exif_focus_distance = fmax(0.0, (0.001 * nominator));
    }
#if EXIV2_MINOR_VERSION>24
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.CanonFi.FocusDistanceUpper")))
              != exifData.end() && pos->size())
    {
      float FocusDistanceUpper = pos->toFloat ();
      if(FocusDistanceUpper <= 0.0f || FocusDistanceUpper >= 0xffff)
      {
        data->exif_focus_distance = 0.0f;
      }
      else if ((pos=exifData.findKey(Exiv2::ExifKey("Exif.CanonFi.FocusDistanceLower")))
        != exifData.end() && pos->size())
      {
        float FocusDistanceLower = pos->toFloat ();
        data->exif_focus_distance = (FocusDistanceLower+FocusDistanceUpper)/200;
      }
    }
#endif
    else if ( (pos=Exiv2::subjectDistance(exifData))
              != exifData.end() && pos->size())
    {
      data->exif_focus_distance = pos->toFloat ();
    }
#endif

    /* Read lens name */
    if ((((pos = exifData.findKey(Exiv2::ExifKey("Exif.CanonCs.LensType"))) != exifData.end()) ||
         ((pos = exifData.findKey(Exiv2::ExifKey("Exif.Canon.0x0095")))     != exifData.end())
        ) && pos->size())
    {
      //g_print("read_exif(): Canon lens found.\n");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Panasonic.LensType"))) != exifData.end() && pos->size())
    {
      //g_print("read_exif(): Pnasonic lens found.");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
#if LF_VERSION>((0 << 24) | (2 << 16) | (8 << 8) | 0)
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.OlympusEq.LensType"))) != exifData.end() && pos->size())
    {
      //g_print("read_exif(): Olympus lens found.\n");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
#endif
#if EXIV2_MINOR_VERSION>20
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.OlympusEq.LensModel"))) != exifData.end() && pos->size())
    {
      //g_print("read_exif(): Olympus lens found.\n");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
#endif
    else if ( (pos=Exiv2::lensName(exifData)) != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
      //g_print("read_exif(): Generic lens found: \"%s\"\n", data->exif_lens);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.LensModel"))) != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
      //g_print("read_exif(): Generic2 lens found: \"%s\"\n", data->exif_lens);
    }
    g_print("read_exif(): lens=%s\n",data->exif_lens);
    {
      Glib::ustring fname = path;
      rtengine::FramesData* fd = nullptr;

      rtengine::RawImage* ri = new rtengine::RawImage(fname);
      int errCode = ri->loadRaw (false, 0, false);
      g_printf("read_exif(): errCode=%d\n", errCode);
      if( !errCode) {
        std::unique_ptr<rtengine::RawMetaDataLocation> rml(new rtengine::RawMetaDataLocation (ri->get_exifBase(), ri->get_ciffBase(), ri->get_ciffLen()));
        fd = new rtengine::FramesData(fname, std::move(rml), true);
      }

      data->exif_lens_alt[0] = '\0';
      std::string rt_lens;
      if( fd ) rt_lens = fd->getLens(0);
      g_print("read_exif(): rt_lens=%s\n",rt_lens.c_str());
      if( !rt_lens.empty() ) {
        char *s = g_locale_to_utf8(rt_lens.c_str(), rt_lens.length(),
                                   NULL, NULL, NULL);
        if ( s!=NULL )
        {
          g_strlcpy(data->exif_lens_alt, s, sizeof(data->exif_lens_alt));
          g_free(s);
        }
        else
        {
          g_strlcpy(data->exif_lens_alt, rt_lens.c_str(), sizeof(data->exif_lens_alt));
        }
      }

      if( fd ) delete fd;
      if( ri ) delete ri;
    }

#if 0
    /* Read flash mode */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.Flash")))
         != exifData.end() && pos->size())
    {
      uf_strlcpy_to_utf8(uf->conf->flashText, max_name, pos, exifData);
    }
    /* Read White Balance Setting */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.WhiteBalance")))
         != exifData.end() && pos->size())
    {
      uf_strlcpy_to_utf8(uf->conf->whiteBalanceText, max_name, pos, exifData);
    }
#endif

    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.Make")))
         != exifData.end() && pos->size())
    {
      std::string str = pos->print(&exifData);
      dt_strlcpy_to_utf8(data->exif_maker, sizeof(data->exif_maker), pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.Make")))
              != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_maker, sizeof(data->exif_maker), pos, exifData);
    }

#ifndef NDEBUG
    printf( "data->exif_maker before stripping: \"%s\"\n", data->exif_maker );
#endif
    size_t slen = strnlen( data->exif_maker, sizeof(data->exif_maker)-2 );
    for(char *c=data->exif_maker+slen; c >= data->exif_maker; c--) {
#ifndef NDEBUG
      std::cout<<"c: \""<<*c<<"\"("<<(int)*c<<")"<<std::endl;
#endif
      if(*c != ' ' && *c != '\0') {
        *(c+1) = '\0';
        break;
      }
    }
#ifndef NDEBUG
    printf( "data->exif_maker after stripping: \"%s\"\n", data->exif_maker );
#endif

    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.Model")))
         != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_model, sizeof(data->exif_model), pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.Model")))
              != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_model, sizeof(data->exif_model), pos, exifData);
    }

    //g_print("read_exif(): model=%s\n",data->exif_model);
#ifndef NDEBUG
    printf( "data->exif_model before stripping: \"%s\"\n", data->exif_model );
#endif
    slen = strnlen( data->exif_model, sizeof(data->exif_model)-2 );
    for(char *c=data->exif_model+slen; c >= data->exif_model; c--) {
      if(*c != ' ' && *c != '\0') {
        *(c+1) = '\0';
        break;
      }
    }
#ifndef NDEBUG
    printf( "data->exif_model after stripping: \"%s\"\n", data->exif_model );
    //g_print("read_exif(): model=%s\n",data->exif_model);
#endif

    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTimeOriginal")))
         != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_datetime_taken, 20, pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal")))
         != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_datetime_taken, 20, pos, exifData);
    }


    //g_print("read_exif(): lens=%s\n",data->exif_lens);
#if EXIV2_MINOR_VERSION<23
    // workaround for an exiv2 bug writing random garbage into exif_lens for this camera:
    // http://dev.exiv2.org/issues/779
    if(!strcmp(data->exif_model, "DMC-GH2")) snprintf(data->exif_lens, sizeof(data->exif_lens), "(unknown)");
#endif

    // Workaround for an issue on newer Sony NEX cams.
    // The default EXIF field is not used by Sony to store lens data
    // http://dev.exiv2.org/issues/883
    // http://darktable.org/redmine/issues/8813
    // FIXME: This is still a workaround
    if((!strncmp(data->exif_model, "NEX", 3)) || (!strncmp(data->exif_model, "ILCE", 4)))
    {
      snprintf(data->exif_lens, sizeof(data->exif_lens), "(unknown)");
      if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.LensModel"))) != exifData.end() && pos->size())
      {
        std::string str = pos->print(&exifData);
        snprintf(data->exif_lens, sizeof(data->exif_lens), "%s", str.c_str());
      }
    };
    //g_print("read_exif(): lens=%s\n",data->exif_lens);

    return true;
  }
  catch (Exiv2::AnyError& e)
  {
    std::string s(e.what());
    std::cerr << "[exiv2] " << s << std::endl;
    return false;
  }
}


void PF::exif_free (gpointer mem)
{
#ifndef NDEBUG
  std::cout<<"Freeing exif data structure"<<std::endl;
#endif
  g_free( mem );
}


void PF::exiv2_free (gpointer mem)
{
#ifndef NDEBUG
  std::cout<<"Freeing exiv2 data structure"<<std::endl;
#endif
  if( mem ) {
    exiv2_data_t* ptr = (exiv2_data_t*)mem;
    if(ptr->image.get() != NULL)
      ptr->image.reset();
    delete( ptr );
  }
}


PF::exif_data_t* PF::get_exif_data( VipsImage* img )
{
  size_t blobsz;
  PF::exif_data_t* exif_data = NULL;
  if( PF_VIPS_IMAGE_GET_BLOB( img, PF_META_EXIF_NAME, &exif_data, &blobsz ) ) {
    std::cout<<"get_exif_data: could not extract exif_custom_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(PF::exif_data_t) ) {
    std::cout<<"get_exif_data: wrong exif_custom_data size."<<std::endl;
    return NULL;
  }
  return exif_data;
}


// Darktable code starts here
//_________________________________

static void dt_remove_exif_keys(Exiv2::ExifData &exif, const char *keys[], unsigned int n_keys)
{
  for(unsigned int i = 0; i < n_keys; i++)
  {
    try
    {
      Exiv2::ExifData::iterator pos;
      while((pos = exif.findKey(Exiv2::ExifKey(keys[i]))) != exif.end())
        exif.erase(pos);
    }
    catch(Exiv2::AnyError &e)
    {
      // the only exception we may get is "invalid" tag, which is not
      // important enough to either stop the function, or even display
      // a message (it's probably the tag is not implemented in the
      // exiv2 version used)
    }
  }
}



#if defined(_WIN32) && defined(EXV_UNICODE_PATH)
  #define WIDEN(s) pugi::as_wide(s)
#else
  #define WIDEN(s) (s)
#endif



int PF::dt_exif_write_blob(uint8_t *blob, uint32_t size, const char *path, const int sRGB, const int out_width, const int out_height)
{
  try
  {
    std::unique_ptr<Exiv2::Image> image(Exiv2::ImageFactory::open(WIDEN(path)));
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &imgExifData = image->exifData();
    Exiv2::ExifData blobExifData;
    Exiv2::ExifParser::decode(blobExifData, blob + 6, size);
    Exiv2::ExifData::const_iterator end = blobExifData.end();
    Exiv2::ExifData::iterator it;
    for(Exiv2::ExifData::const_iterator i = blobExifData.begin(); i != end; ++i)
    {
      // add() does not override! we need to delete existing key first.
      Exiv2::ExifKey key(i->key());
      if((it = imgExifData.findKey(key)) != imgExifData.end()) imgExifData.erase(it);

      imgExifData.add(Exiv2::ExifKey(i->key()), &i->value());
    }

    {
      // Remove thumbnail
      static const char *keys[] = {
        "Exif.Thumbnail.Compression",
        "Exif.Thumbnail.XResolution",
        "Exif.Thumbnail.YResolution",
        "Exif.Thumbnail.ResolutionUnit",
        "Exif.Thumbnail.JPEGInterchangeFormat",
        "Exif.Thumbnail.JPEGInterchangeFormatLength"
      };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(imgExifData, keys, n_keys);
    }

    if(out_width > 0 && out_height > 0) {
      imgExifData["Exif.Photo.PixelXDimension"] = (uint32_t)out_width;
      imgExifData["Exif.Photo.PixelYDimension"] = (uint32_t)out_height;
    } else {
      static const char *keys[] = {
        "Exif.Photo.PixelXDimension",
        "Exif.Photo.PixelYDimension"
      };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(imgExifData, keys, n_keys);
    }

    /* Write appropriate color space tag if using sRGB output */
    if(sRGB)
      imgExifData["Exif.Photo.ColorSpace"] = uint16_t(1); /* sRGB */
    else
      imgExifData["Exif.Photo.ColorSpace"] = uint16_t(0xFFFF); /* Uncalibrated */

    imgExifData.sortByTag();
    image->writeMetadata();
  }
  catch(Exiv2::AnyError &e)
  {
    std::string s(e.what());
    std::cerr << "[exiv2 dt_exif_write_blob] " << path << ": " << s << std::endl;
    return 0;
  }
  return 1;
}



int PF::dt_exif_read_blob(uint8_t **buf, const char *path, const int imgid, const int out_width,
                      const int out_height, const int dng_mode)
{
  *buf = NULL;
  try
  {
    std::unique_ptr<Exiv2::Image> image(Exiv2::ImageFactory::open(WIDEN(path)));
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    // get rid of thumbnails
    Exiv2::ExifThumb(exifData).erase();
    Exiv2::ExifData::const_iterator pos;

    {
      static const char *keys[] = {
        "Exif.Image.ImageWidth",
        "Exif.Image.ImageLength",
        "Exif.Image.BitsPerSample",
        "Exif.Image.Compression",
        "Exif.Image.PhotometricInterpretation",
        "Exif.Image.FillOrder",
        "Exif.Image.SamplesPerPixel",
        "Exif.Image.StripOffsets",
        "Exif.Image.RowsPerStrip",
        "Exif.Image.StripByteCounts",
        "Exif.Image.PlanarConfiguration",
        "Exif.Image.DNGVersion",
        "Exif.Image.DNGBackwardVersion"
      };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(exifData, keys, n_keys);
    }

      /* Many tags should be removed in all cases as they are simply wrong also for dng files */

      // remove subimage* trees, related to thumbnails or HDR usually; also UserCrop
    for(Exiv2::ExifData::iterator i = exifData.begin(); i != exifData.end();)
    {
      static const std::string needle = "Exif.SubImage";
      if(i->key().compare(0, needle.length(), needle) == 0)
        i = exifData.erase(i);
      else
        ++i;
    }

    {
      static const char *keys[] = {
        // Canon color space info
        "Exif.Canon.ColorSpace",
        "Exif.Canon.ColorData",

        // Nikon thumbnail data
        "Exif.Nikon3.Preview",
        "Exif.NikonPreview.JPEGInterchangeFormat",

        // DNG stuff that is irrelevant or misleading
        "Exif.Image.DNGPrivateData",
        "Exif.Image.DefaultBlackRender",
        "Exif.Image.DefaultCropOrigin",
        "Exif.Image.DefaultCropSize",
        "Exif.Image.RawDataUniqueID",
        "Exif.Image.OriginalRawFileName",
        "Exif.Image.OriginalRawFileData",
        "Exif.Image.ActiveArea",
        "Exif.Image.MaskedAreas",
        "Exif.Image.AsShotICCProfile",
        "Exif.Image.OpcodeList1",
        "Exif.Image.OpcodeList2",
        "Exif.Image.OpcodeList3",
        "Exif.Photo.MakerNote",

        // Pentax thumbnail data
        "Exif.Pentax.PreviewResolution",
        "Exif.Pentax.PreviewLength",
        "Exif.Pentax.PreviewOffset",
        "Exif.PentaxDng.PreviewResolution",
        "Exif.PentaxDng.PreviewLength",
        "Exif.PentaxDng.PreviewOffset",
        // Pentax color info
        "Exif.PentaxDng.ColorInfo",

        // Minolta thumbnail data
        "Exif.Minolta.Thumbnail",
        "Exif.Minolta.ThumbnailOffset",
        "Exif.Minolta.ThumbnailLength",

        // Sony thumbnail data
        "Exif.SonyMinolta.ThumbnailOffset",
        "Exif.SonyMinolta.ThumbnailLength",

        // Olympus thumbnail data
        "Exif.Olympus.Thumbnail",
        "Exif.Olympus.ThumbnailOffset",
        "Exif.Olympus.ThumbnailLength"

        "Exif.Image.BaselineExposureOffset",
        };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(exifData, keys, n_keys);
    }
#if EXIV2_MINOR_VERSION >= 23
    {
      // Exiv2 versions older than 0.23 drop all EXIF if the code below is executed
      // Samsung makernote cleanup, the entries below have no relevance for exported images
      static const char *keys[] = {
        "Exif.Samsung2.SensorAreas",
        "Exif.Samsung2.ColorSpace",
        "Exif.Samsung2.EncryptionKey",
        "Exif.Samsung2.WB_RGGBLevelsUncorrected",
        "Exif.Samsung2.WB_RGGBLevelsAuto",
        "Exif.Samsung2.WB_RGGBLevelsIlluminator1",
        "Exif.Samsung2.WB_RGGBLevelsIlluminator2",
        "Exif.Samsung2.WB_RGGBLevelsBlack",
        "Exif.Samsung2.ColorMatrix",
        "Exif.Samsung2.ColorMatrixSRGB",
        "Exif.Samsung2.ColorMatrixAdobeRGB",
        "Exif.Samsung2.ToneCurve1",
        "Exif.Samsung2.ToneCurve2",
        "Exif.Samsung2.ToneCurve3",
        "Exif.Samsung2.ToneCurve4"
      };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(exifData, keys, n_keys);
    }
#endif

      static const char *dngkeys[] = {
        // Embedded color profile info
        "Exif.Image.CalibrationIlluminant1",
        "Exif.Image.CalibrationIlluminant2",
        "Exif.Image.ColorMatrix1",
        "Exif.Image.ColorMatrix2",
        "Exif.Image.ForwardMatrix1",
        "Exif.Image.ForwardMatrix2",
        "Exif.Image.ProfileCalibrationSignature",
        "Exif.Image.ProfileCopyright",
        "Exif.Image.ProfileEmbedPolicy",
        "Exif.Image.ProfileHueSatMapData1",
        "Exif.Image.ProfileHueSatMapData2",
        "Exif.Image.ProfileHueSatMapDims",
        "Exif.Image.ProfileHueSatMapEncoding",
        "Exif.Image.ProfileLookTableData",
        "Exif.Image.ProfileLookTableDims",
        "Exif.Image.ProfileLookTableEncoding",
        "Exif.Image.ProfileName",
        "Exif.Image.ProfileToneCurve",
        "Exif.Image.ReductionMatrix1",
        "Exif.Image.ReductionMatrix2"
        };
      static const guint n_dngkeys = G_N_ELEMENTS(dngkeys);
    dt_remove_exif_keys(exifData, dngkeys, n_dngkeys);

    // we don't write the orientation here for dng as it is set in dt_imageio_dng_write_tiff_header
    // or might be defined in this blob.
    if(!dng_mode) exifData["Exif.Image.Orientation"] = uint16_t(1);

    /* Replace RAW dimension with output dimensions (for example after crop/scale, or orientation for dng
     * mode) */
    if(out_width > 0) exifData["Exif.Photo.PixelXDimension"] = (uint32_t)out_width;
    if(out_height > 0) exifData["Exif.Photo.PixelYDimension"] = (uint32_t)out_height;

    int resolution = 0; //dt_conf_get_int("metadata/resolution");
    if(resolution > 0)
    {
      exifData["Exif.Image.XResolution"] = Exiv2::Rational(resolution, 1);
      exifData["Exif.Image.YResolution"] = Exiv2::Rational(resolution, 1);
      exifData["Exif.Image.ResolutionUnit"] = uint16_t(2); /* inches */
    }
    else
    {
      static const char *keys[] = {
        "Exif.Image.XResolution",
        "Exif.Image.YResolution",
        "Exif.Image.ResolutionUnit"
      };
      static const guint n_keys = G_N_ELEMENTS(keys);
      dt_remove_exif_keys(exifData, keys, n_keys);
    }

    exifData["Exif.Image.Software"] = "PhotoFlow Image Editor";

    Exiv2::Blob blob;
    Exiv2::ExifParser::encode(blob, Exiv2::bigEndian, exifData);
    const int length = blob.size();
    *buf = (uint8_t *)malloc(length+6);
    if (!*buf)
    {
      return 0;
    }
    memcpy(*buf, "Exif\000\000", 6);
    memcpy(*buf + 6, &(blob[0]), length);
    return length + 6;
  }
  catch(Exiv2::AnyError &e)
  {
    // std::cerr.rdbuf(savecerr);
    std::string s(e.what());
    std::cerr << "[exiv2 dt_exif_read_blob] " << path << ": " << s << std::endl;
    free(*buf);
    *buf = NULL;
    return 0;
  }
}
