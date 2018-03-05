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

#include <algorithm>

#include "file_util.hh"

bool PF::get_file_extension(const std::string & file, std::string & ext)
{
#ifdef WIN32
  const char * dir_separator = "\\";
#else
  const char * dir_separator = "/";
#endif
  std::size_t ext_pos = file.rfind(".");
  std::size_t dir_pos = file.rfind(dir_separator);

  if(ext_pos>dir_pos+1)
  {
    ext.append(file.begin()+ext_pos+1,file.end());
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return true;
  }

  return false;
}


std::string PF::replace_file_extension(std::string file, std::string new_ext)
{
#ifdef WIN32
  const char * dir_separator = "\\";
#else
  const char * dir_separator = "/";
#endif
  std::size_t ext_pos = file.rfind(".");
  std::size_t dir_pos = file.rfind(dir_separator);

  std::string new_file;

  if(ext_pos>dir_pos+1) {
    new_file.append(file.begin(),file.begin()+ext_pos+1);
    new_file += new_ext;
  }

  return new_file;
}


gchar* PF::pf_path_get_basename( const gchar *file_name )
{
  size_t len = strlen( file_name );
  for(size_t i = len-1; i >= 0; i--) {
    if( file_name[i] == '/' || file_name[i] == '\\' ) {
      if( i == 0 ) return g_strdup( &(file_name[i+1]) );
      if( file_name[i-1] != '\\' ) {
        // the slash/backslash is not escaped
        return g_strdup( &(file_name[i+1]) );
      } else {
        i -= 1;
      }
    }
  }
}


std::string PF::pf_escape_xml(const std::string str)
{
  std::string result;
  for(unsigned int i = 0; i < str.size(); i++) {
    if( str[i] == '&' ) {
      result.append( "&amp;" );
    } else if( str[i] == '<' ) {
      result.append( "&lt;" );
    } else {
      result.push_back( str[i] );
    }
  }
  return result;
}
