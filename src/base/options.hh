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

#ifndef PF_OPTIONS_H
#define PF_OPTIONS_H

#include <string>
#include <glibmm.h>

namespace PF
{

  enum display_profile_t
  {
    PF_DISPLAY_PROF_sRGB = 0,
    PF_DISPLAY_PROF_SYSTEM = 1,
    PF_DISPLAY_PROF_CUSTOM = 2,
    PF_DISPLAY_PROF_MAX
  };

  class Options
  {

    display_profile_t display_profile_type;
    Glib::ustring custom_display_profile_name;

    Glib::ustring last_visited_image_folder;
    Glib::ustring last_visited_preset_folder;
    Glib::ustring last_visited_icc_folder;

    int save_sidecar_files;

  public:
    Options();

    void set_display_profile_type(int t);
    display_profile_t get_display_profile_type() { return display_profile_type; }

    void set_custom_display_profile_name( std::string n )
    {
      custom_display_profile_name = n;
    }
    std::string get_custom_display_profile_name() { return custom_display_profile_name; }

    // last visited folders
    void set_last_visited_image_folder( std::string f ) { last_visited_image_folder = f; }
    std::string get_last_visited_image_folder() { return last_visited_image_folder; }
    void set_last_visited_preset_folder( std::string f ) { last_visited_preset_folder = f; }
    std::string get_last_visited_preset_folder() { return last_visited_preset_folder; }
    void set_last_visited_icc_folder( std::string f ) { last_visited_icc_folder = f; }
    std::string get_last_visited_icc_folder() { return last_visited_icc_folder; }

    void set_save_sidecar_files(int val) { save_sidecar_files = val; }
    int get_save_sidecar_files() { return save_sidecar_files; }

    void load();
    void save();
  };

}


#endif 


