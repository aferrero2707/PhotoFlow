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


#include <exiv2/easyaccess.hpp>
#include <exiv2/xmp.hpp>
#include <exiv2/error.hpp>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>

#include <glib.h>

#include "exif_data.hh"


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
      //g_print("read_exif(): Generic lens found.\n");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.LensModel"))) != exifData.end() && pos->size())
    {
      //g_print("read_exif(): Generic2 lens found.\n");
      dt_strlcpy_to_utf8(data->exif_lens, sizeof(data->exif_lens), pos, exifData);
    }
    //g_print("read_exif(): lens=%s\n",data->exif_lens);

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
      dt_strlcpy_to_utf8(data->exif_maker, sizeof(data->exif_maker), pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.Make")))
              != exifData.end() && pos->size())
    {
      dt_strlcpy_to_utf8(data->exif_maker, sizeof(data->exif_maker), pos, exifData);
    }

    printf( "data->exif_maker before stripping: \"%s\"\n", data->exif_maker );
    size_t slen = strnlen( data->exif_maker, sizeof(data->exif_maker)-2 );
    for(char *c=data->exif_maker+slen; c >= data->exif_maker; c--) {
      std::cout<<"c: \""<<*c<<"\"("<<(int)*c<<")"<<std::endl;
      if(*c != ' ' && *c != '\0') {
        *(c+1) = '\0';
        break;
      }
    }
    printf( "data->exif_maker after stripping: \"%s\"\n", data->exif_maker );

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
    printf( "data->exif_model before stripping: \"%s\"\n", data->exif_model );
    slen = strnlen( data->exif_model, sizeof(data->exif_model)-2 );
    for(char *c=data->exif_model+slen; c >= data->exif_model; c--) {
      if(*c != ' ' && *c != '\0') {
        *(c+1) = '\0';
        break;
      }
    }
    printf( "data->exif_model after stripping: \"%s\"\n", data->exif_model );
    //g_print("read_exif(): model=%s\n",data->exif_model);

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

