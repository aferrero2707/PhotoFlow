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

#include <glibmm/iochannel.h>

#include "photoflow.hh"
#include "options.hh"
#include "../rt/rtengine/safekeyfile.h"

PF::Options::Options()
{
  working_profile_type = PF::OUT_PROF_REC2020;
  working_trc_type = PF_TRC_LINEAR;
  display_profile_type = PF::PF_DISPLAY_PROF_sRGB;
}

void PF::Options::set_working_profile_type(int t)
{
  working_profile_type = (profile_type_t)t;
}


void PF::Options::set_working_trc_type(int t)
{
  working_trc_type = (TRC_type)t;
}


void PF::Options::set_display_profile_type(int t)
{
  if( t>= 0 && t < PF::PF_DISPLAY_PROF_MAX)
    display_profile_type = (display_profile_t)t;
}


void PF::Options::load()
{
  setlocale(LC_NUMERIC, "C"); // to set decimal point to "."
  rtengine::SafeKeyFile keyFile;

  Glib::ustring fname =
      Glib::build_filename( Glib::ustring(PF::PhotoFlow::Instance().get_config_dir()), "options" );
  std::cout<<"Loading custom settings..."<<std::endl;
  try {
    if (keyFile.load_from_file (fname)) {

      // --------------------------------------------------------------------------------------------------------

      if (keyFile.has_group ("Folders")) {
        if (keyFile.has_key ("Folders", "last_visited_image_folder")) {
          last_visited_image_folder = keyFile.get_string ("Folders", "last_visited_image_folder");
        }
        if (keyFile.has_key ("Folders", "last_visited_preset_folder")) {
          last_visited_preset_folder = keyFile.get_string ("Folders", "last_visited_preset_folder");
        }
        if (keyFile.has_key ("Folders", "last_visited_icc_folder")) {
          last_visited_icc_folder = keyFile.get_string ("Folders", "last_visited_icc_folder");
        }
      }

      if (keyFile.has_group ("Color Management")) {
        if (keyFile.has_key ("Color Management", "WorkingProfileType")) {
          std::string keyval = keyFile.get_string ("Color Management", "WorkingProfileType");
          if( keyval == "sRGB" )
            working_profile_type = PF::OUT_PROF_sRGB;
          else if( keyval == "ADOBE" )
            working_profile_type = PF::OUT_PROF_ADOBE;
          else if( keyval == "REC2020" )
            working_profile_type = PF::OUT_PROF_REC2020;
          else if( keyval == "PROPHOTO" )
            working_profile_type = PF::OUT_PROF_PROPHOTO;
          else if( keyval == "ACEScg" )
            working_profile_type = PF::OUT_PROF_ACEScg;
          else if( keyval == "ACES" )
            working_profile_type = PF::OUT_PROF_ACES;
          else if( keyval == "CUSTOM" )
            working_profile_type = PF::OUT_PROF_CUSTOM;
          std::cout<<"working_profile_type="<<working_profile_type<<std::endl;
        }
        if (keyFile.has_key ("Color Management", "WorkingTRCType")) {
          std::string keyval = keyFile.get_string ("Color Management", "WorkingTRCType");
          if( keyval == "TRC_STANDARD" )
            working_trc_type = PF::PF_TRC_STANDARD;
          else if( keyval == "TRC_PERCEPTUAL" )
            working_trc_type = PF::PF_TRC_PERCEPTUAL;
          else if( keyval == "TRC_LINEAR" )
            working_trc_type = PF::PF_TRC_LINEAR;
          else if( keyval == "TRC_sRGB" )
            working_trc_type = PF::PF_TRC_sRGB;
          std::cout<<"working_trc_type="<<working_trc_type<<std::endl;
        }
        if (keyFile.has_key ("Color Management", "CustomWorkingProfileName")) {
          custom_working_profile_name = keyFile.get_string ("Color Management", "CustomWorkingProfileName");
          std::cout<<"custom_working_profile_name="<<custom_working_profile_name<<std::endl;
        }

        if (keyFile.has_key ("Color Management", "DisplayProfileType")) {
          int keyval = keyFile.get_integer ("Color Management", "DisplayProfileType");
          if( keyval>0 && keyval<PF::PF_DISPLAY_PROF_MAX)
            display_profile_type = (display_profile_t)keyval;
          std::cout<<"display_profile_type="<<display_profile_type<<std::endl;
        }
        if (keyFile.has_key ("Color Management", "CustomDisplayProfileName")) {
          custom_display_profile_name = keyFile.get_string ("Color Management", "CustomDisplayProfileName");
          std::cout<<"custom_display_profile_name="<<custom_display_profile_name<<std::endl;
        }
      }
    }
  } catch (Glib::Error &err) {
    printf("Options::readFromFile / Error code %d while reading values from \"%s\":\n%s\n", err.code(), fname.c_str(), err.what().c_str());
  } catch (...) {
    printf("Options::readFromFile / Unknown exception while trying to load \"%s\"!\n", fname.c_str());
  }
  std::cout<<"... custom settings loaded."<<std::endl; //getchar();
}


void PF::Options::save()
{
  setlocale(LC_NUMERIC, "C"); // to set decimal point to "."
  rtengine::SafeKeyFile keyFile;

  Glib::ustring fname =
      Glib::build_filename( Glib::ustring(PF::PhotoFlow::Instance().get_config_dir()), "options" );

  keyFile.set_string ("Folders", "last_visited_image_folder", last_visited_image_folder);
  keyFile.set_string ("Folders", "last_visited_preset_folder", last_visited_preset_folder);
  keyFile.set_string ("Folders", "last_visited_icc_folder", last_visited_icc_folder);

  switch( working_profile_type ) {
  case OUT_PROF_sRGB:
    keyFile.set_string ("Color Management", "WorkingProfileType", "sRGB");
    break;
  case OUT_PROF_REC2020:
    keyFile.set_string ("Color Management", "WorkingProfileType", "REC2020");
    break;
  case OUT_PROF_ADOBE:
    keyFile.set_string ("Color Management", "WorkingProfileType", "ADOBE");
    break;
  case OUT_PROF_PROPHOTO:
    keyFile.set_string ("Color Management", "WorkingProfileType", "PROPHOTO");
    break;
  case OUT_PROF_ACEScg:
    keyFile.set_string ("Color Management", "WorkingProfileType", "ACEScg");
    break;
  case OUT_PROF_ACES:
    keyFile.set_string ("Color Management", "WorkingProfileType", "ACES");
    break;
  case OUT_PROF_CUSTOM:
    keyFile.set_string ("Color Management", "WorkingProfileType", "CUSTOM");
    break;
  default: break;
  }
  switch( working_trc_type ) {
  case PF_TRC_STANDARD:
    keyFile.set_string ("Color Management", "WorkingTRCType", "TRC_STANDARD");
    break;
  case PF_TRC_PERCEPTUAL:
    keyFile.set_string ("Color Management", "WorkingTRCType", "TRC_PERCEPTUAL");
    break;
  case PF_TRC_LINEAR:
    keyFile.set_string ("Color Management", "WorkingTRCType", "TRC_LINEAR");
    break;
  case PF_TRC_sRGB:
    keyFile.set_string ("Color Management", "WorkingTRCType", "TRC_sRGB");
    break;
  default: break;
  }
  keyFile.set_string ("Color Management", "CustomWorkingProfileName", custom_display_profile_name);

  keyFile.set_integer ("Color Management", "DisplayProfileType", (int)display_profile_type);
  keyFile.set_string ("Color Management", "CustomDisplayProfileName", custom_display_profile_name);

  try {
    //keyFile.save_to_file( fname );
    Glib::ustring opt_str = keyFile.to_data();
    //std::string fnames = fname.get_raw();
    std::cout<<"Saving options..."<<std::endl;
    std::cout<<opt_str<<std::endl;
    std::string mode = "w";
    Glib::RefPtr< Glib::IOChannel > ioch = Glib::IOChannel::create_from_file( fname.raw(), mode );
    ioch->write( opt_str );
    ioch->close();
    std::cout<<"... options saved."<<std::endl;
  } catch (Glib::Error &err) {
    printf("Options::readFromFile / Error code %d while saving values to \"%s\":\n%s\n", err.code(), fname.c_str(), err.what().c_str());
  } catch (...) {
    printf("Options::readFromFile / Unknown exception while trying to save \"%s\"!\n", fname.c_str());
  }
}
