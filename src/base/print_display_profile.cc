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

#include <iostream>

#include "print_display_profile.hh"

#include <glibmm.h>
#include <cairo.h>
#ifdef __APPLE___
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#endif


void PF::print_display_profile()
{
#ifdef __APPLE___
  //ColorSyncProfileRef cs_prof = ColorSyncProfileCreateWithDisplayID (0);

  guint8 *buffer = NULL;
  gint buffer_size = 0;

  int monitor = cairo_current_display_id;
  CGColorSpaceRef space = NULL;
  space = CGDisplayCopyColorSpace (monitor);
  if( space ) {
    CFDataRef data = CGColorSpaceCopyICCProfile(space);
    if( data ) {
      UInt8 *tmp_buffer = (UInt8 *)g_malloc(CFDataGetLength(data));
      CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), tmp_buffer);

      buffer = (guint8 *)tmp_buffer;
      buffer_size = CFDataGetLength(data);

      cmsHPROFILE icc_profile = cmsOpenProfileFromMem( buffer, buffer_size );
      char tstr[1024];
      cmsGetProfileInfoASCII(icc_profile, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"Display profile: "<<tstr<<std::endl;

      CFRelease(data);
    }
  }
  return;

  monitor = 0;
  CGDirectDisplayID ids[monitor + 1];
  uint32_t total_ids;
  CMProfileRef prof = NULL;
  if(CGGetOnlineDisplayList(monitor + 1, &ids[0], &total_ids) == kCGErrorSuccess && total_ids == monitor + 1)
    CMGetProfileByAVID(ids[monitor], &prof);
  if(prof != NULL)
  {
    CFDataRef data;
    data = CMProfileCopyICCData(NULL, prof);
    CMCloseProfile(prof);

    UInt8 *tmp_buffer = (UInt8 *)g_malloc(CFDataGetLength(data));
    CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), tmp_buffer);

    buffer = (guint8 *)tmp_buffer;
    buffer_size = CFDataGetLength(data);

    cmsHPROFILE icc_profile = cmsOpenProfileFromMem( buffer, buffer_size );
    char tstr[1024];
    cmsGetProfileInfoASCII(icc_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"Display profile: "<<tstr<<std::endl;

    CFRelease(data);
    return;
  }


  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  if( !colorSpace ) {
    std::cout<<"Cannot get CGColorSpaceCreateDeviceRGB()"<<std::endl;
    return;
  }
  std::cout<<"Display profile: "<<CFStringGetCStringPtr(CGColorSpaceCopyName(colorSpace),kCFStringEncodingASCII)<<std::endl;
  CFDataRef data = CGColorSpaceCopyICCProfile(colorSpace);
  if( !data ) {
    std::cout<<"Cannot get CGColorSpaceCopyICCProfile()"<<std::endl;
    return;
  }
  CFIndex icc_length = CFDataGetLength(data);
  const UInt8* icc_data = CFDataGetBytePtr(data);
  cmsHPROFILE icc_profile = cmsOpenProfileFromMem( icc_data, icc_length );
  char tstr[1024];
  cmsGetProfileInfoASCII(icc_profile, cmsInfoDescription, "en", "US", tstr, 1024);
  std::cout<<"Display profile: "<<tstr<<std::endl;
#endif
}


void* PF::get_display_profile()
{
  //ColorSyncProfileRef cs_prof = ColorSyncProfileCreateWithDisplayID (0);
  void* profile = NULL;

#ifdef __APPLE___
  int monitor = 0;
  CGDirectDisplayID ids[monitor + 1];
  uint32_t total_ids;
  CMProfileRef prof = NULL;
  if(CGGetOnlineDisplayList(monitor + 1, &ids[0], &total_ids) == kCGErrorSuccess && total_ids == monitor + 1)
    CMGetProfileByAVID(ids[monitor], &prof);
  if(prof != NULL) {
    CFDataRef data;
    data = CMProfileCopyICCData(NULL, prof);
    CMCloseProfile(prof);

    if( data ) {
      profile = CGColorSpaceCreateWithICCProfile(data);
      CFRelease(data);
    }
  }
#endif
  return profile;
}


cmsHPROFILE PF::get_display_ICC_profile()
{
  cmsHPROFILE icc_profile = NULL;
  guint8 *buffer = NULL;
  gint buffer_size = 0;

#ifdef __APPLE___
  int monitor = cairo_current_display_id;
  CGColorSpaceRef space = NULL;
  space = CGDisplayCopyColorSpace (monitor);
  if( space ) {
    CFDataRef data = CGColorSpaceCopyICCProfile(space);
    if( data ) {
      UInt8 *tmp_buffer = (UInt8 *)g_malloc(CFDataGetLength(data));
      CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), tmp_buffer);

      buffer = (guint8 *)tmp_buffer;
      buffer_size = CFDataGetLength(data);

      icc_profile = cmsOpenProfileFromMem( buffer, buffer_size );
      char tstr[1024];
      cmsGetProfileInfoASCII(icc_profile, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"Display profile: "<<tstr<<std::endl;

      CFRelease(data);
    }
  }
#endif
  return icc_profile;
}
