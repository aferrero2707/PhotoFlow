/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RTENGINE_
#define _RTENGINE_

#include <string>
#include <glibmm.h>
#include "../rtexif/rtexif.h"
#include "opthelper.h"
#include "progresslistener.h"
#include "procparams.h"
#include "imageformat.h"
/**
 * @file
 * This file contains the main functionality of the RawTherapee engine.
 *
 */

namespace rtengine
{

/**
  * This class provides functions to obtain exif and IPTC metadata information
  * from any of the sub-frame of an image file
  */
class FramesMetaData
{

public:
    /** @return Returns the number of root Metadata */
    virtual unsigned int getRootCount () const = 0;
    /** @return Returns the number of frame contained in the file based on Metadata */
    virtual unsigned int getFrameCount () const = 0;

    /** Checks the availability of exif metadata tags.
      * @return Returns true if image contains exif metadata tags */
    virtual bool hasExif (unsigned int frame = 0) const = 0;
    /** Returns the directory of exif metadata tags.
      * @param root root number in the metadata tree
      * @return The directory of exif metadata tags */
    virtual rtexif::TagDirectory* getRootExifData (unsigned int root = 0) const = 0;
    /** Returns the directory of exif metadata tags.
      * @param frame frame number in the metadata tree
      * @return The directory of exif metadata tags */
    virtual rtexif::TagDirectory* getFrameExifData (unsigned int frame = 0) const = 0;
    /** Returns the directory of exif metadata tags containing at least the 'Make' tag for the requested frame.
      * If no usable metadata exist in the frame, send back the best TagDirectory describing the frame content.
      * @param imgSource rawimage that we want the metadata from
      * @param rawParams RawParams to select the frame number
      * @return The directory of exif metadata tags containing at least the 'Make' tag */
    //virtual rtexif::TagDirectory* getBestExifData (ImageSource *imgSource, procparams::RAWParams *rawParams) const = 0;
    /** Checks the availability of IPTC tags.
      * @return Returns true if image contains IPTC tags */
    virtual bool hasIPTC (unsigned int frame = 0) const = 0;
    /** Returns the directory of IPTC tags.
      * @return The directory of IPTC tags */
    virtual procparams::IPTCPairs getIPTCData (unsigned int frame = 0) const = 0;
    /** @return a struct containing the date and time of the image */
    virtual tm getDateTime (unsigned int frame = 0) const = 0;
    /** @return a timestamp containing the date and time of the image */
    virtual time_t getDateTimeAsTS(unsigned int frame = 0) const = 0;
    /** @return the ISO of the image */
    virtual int getISOSpeed (unsigned int frame = 0) const = 0;
    /** @return the F number of the image */
    virtual double getFNumber  (unsigned int frame = 0) const = 0;
    /** @return the focal length used at the exposure */
    virtual double getFocalLen (unsigned int frame = 0) const = 0;
    /** @return the focal length in 35mm used at the exposure */
    virtual double getFocalLen35mm (unsigned int frame = 0) const = 0;
    /** @return the focus distance in meters, 0=unknown, 10000=infinity */
    virtual float getFocusDist (unsigned int frame = 0) const = 0;
    /** @return the shutter speed */
    virtual double getShutterSpeed (unsigned int frame = 0) const = 0;
    /** @return the exposure compensation */
    virtual double getExpComp (unsigned int frame = 0) const = 0;
    /** @return the maker of the camera */
    virtual std::string getMake     (unsigned int frame = 0) const = 0;
    /** @return the model of the camera */
    virtual std::string getModel    (unsigned int frame = 0) const = 0;

    std::string getCamera   (unsigned int frame = 0) const
    {
        return getMake(frame) + " " + getModel(frame);
    }

    /** @return the lens on the camera  */
    virtual std::string getLens     (unsigned int frame = 0) const = 0;
    /** @return the orientation of the image */
    virtual std::string getOrientation (unsigned int frame = 0) const = 0;
    /** @return the rating of the image */
    virtual int getRating (unsigned int frame = 0) const = 0;

    /** @return true if the file is a PixelShift shot (Pentax and Sony bodies) */
    virtual bool getPixelShift () const = 0;
    /** @return false: not an HDR file ; true: single or multi-frame HDR file (e.g. Pentax HDR raw file or 32 bit float DNG file or Log compressed) */
    virtual bool getHDR (unsigned int frame = 0) const = 0;

    /** @return false: not an HDR file ; true: single or multi-frame HDR file (e.g. Pentax HDR raw file or 32 bit float DNG file or Log compressed) */
    virtual std::string getImageType (unsigned int frame) const = 0;
    /** @return the sample format based on MetaData */
    virtual IIOSampleFormat getSampleFormat (unsigned int frame = 0) const = 0;

    /** Functions to convert between floating point and string representation of shutter and aperture */
    static std::string apertureToString (double aperture);
    /** Functions to convert between floating point and string representation of shutter and aperture */
    static std::string shutterToString (double shutter);
    /** Functions to convert between floating point and string representation of shutter and aperture */
    static double apertureFromString (std::string shutter);
    /** Functions to convert between floating point and string representation of shutter and aperture */
    static double shutterFromString (std::string shutter);
    /** Functions to convert between floating point and string representation of exposure compensation */
    static std::string expcompToString (double expcomp, bool maskZeroexpcomp);

    virtual ~FramesMetaData () = default;

    /** Reads metadata from file.
      * @param fname is the name of the file
      * @param rml is a struct containing information about metadata location of the first frame.
      * Use it only for raw files. In caseof jpgs and tiffs pass a NULL pointer.
      * @param firstFrameOnly must be true to get the MetaData of the first frame only, e.g. for a PixelShift file.
      * @return The metadata */
    static FramesMetaData* fromFile (const Glib::ustring& fname, std::unique_ptr<RawMetaDataLocation> rml, bool firstFrameOnly = false);
};


}

#endif

