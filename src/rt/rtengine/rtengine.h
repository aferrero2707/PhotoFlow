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
#include "opthelper.h"
/**
 * @file
 * This file contains the main functionality of the RawTherapee engine.
 *
 */

namespace rtengine
{

/** This listener interface is used to indicate the progress of time consuming operations */
class ProgressListener
{

public:
    virtual ~ProgressListener() {}
    /** This member function is called when the percentage of the progress has been changed.
      * @param p is a number between 0 and 1 */
    virtual void setProgress (double p) {}
    /** This member function is called when a textual information corresponding to the progress has been changed.
      * @param str is the textual information corresponding to the progress */
    virtual void setProgressStr (Glib::ustring str) {}
    /** This member function is called when the state of the processing has been changed.
      * @param inProcessing =true if the processing has been started, =false if it has been stopped */
    virtual void setProgressState (bool inProcessing) {}
    /** This member function is called when an error occurs during the operation.
      * @param descr is the error message */
    virtual void error (Glib::ustring descr) {}
};

}

#endif

