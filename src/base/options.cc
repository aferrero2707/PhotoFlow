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
  display_profile_type = PF::PF_DISPLAY_PROF_sRGB;
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

  try {
    if (keyFile.load_from_file (fname)) {

      // --------------------------------------------------------------------------------------------------------

      if (keyFile.has_group ("Color Management")) {
        if (keyFile.has_key ("Color Management", "DisplayProfileType")) {
          int keyval = keyFile.get_integer ("Color Management", "DisplayProfileType");
          if( keyval>0 && keyval<PF::PF_DISPLAY_PROF_MAX)
            display_profile_type = (display_profile_t)keyval;
        }
        if (keyFile.has_key ("Color Management", "CustomDisplayProfileName")) {
          custom_display_profile_name = keyFile.get_string ("Color Management", "CustomDisplayProfileName");
        }
      }
    }
  } catch (Glib::Error &err) {
    printf("Options::readFromFile / Error code %d while reading values from \"%s\":\n%s\n", err.code(), fname.c_str(), err.what().c_str());
  } catch (...) {
    printf("Options::readFromFile / Unknown exception while trying to load \"%s\"!\n", fname.c_str());
  }
}


void PF::Options::save()
{
  setlocale(LC_NUMERIC, "C"); // to set decimal point to "."
  rtengine::SafeKeyFile keyFile;

  Glib::ustring fname =
      Glib::build_filename( Glib::ustring(PF::PhotoFlow::Instance().get_config_dir()), "options" );

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
