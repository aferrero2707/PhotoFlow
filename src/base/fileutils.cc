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
#include <glibmm.h>
#include "fileutils.hh"

bool PF::getFileExtension(const std::string dir_separator, const std::string file, std::string & ext)
{
  std::size_t ext_pos = file.rfind(".");
  //std::size_t dir_pos = (dir_separator.empty()) ? 0 : file.rfind(dir_separator);

  //if(ext_pos>dir_pos+1) {
  if(ext_pos>0) {
    ext.append(file.begin()+ext_pos+1,file.end());
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return true;
  }

  return false;
}



std::string PF::replaceFileExtension(const std::string file, std::string ext)
{
  std::string result;
  std::size_t ext_pos = file.rfind(".");

  if(ext_pos>0) {
    result.append(file.begin(), file.begin()+ext_pos+1);
    result += ext;
  }

  return result;
}



bool PF::getFileExtensionLowcase(const std::string dir_separator, const std::string file, std::string & ext)
{
  if(!getFileExtension(dir_separator, file, ext)) return false;
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return true;
}



bool PF::getFileName(const std::string dir_separator, const std::string file, std::string & name)
{
  gchar* basename = g_path_get_basename( file.c_str() );
  std::string tname = basename;
  g_free( basename );
  std::size_t ext_pos = tname.rfind(".");
  //std::size_t dir_pos = (dir_separator.empty()) ? 0 : file.rfind(dir_separator);

  //if(ext_pos>dir_pos+1) {
  //  name.append(file.begin()+dir_pos, file.begin()+ext_pos);
  if(ext_pos>0) {
    name.append(tname.begin(), tname.begin()+ext_pos);
    //std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return true;
  }

  return false;
}



bool PF::getDirName(const std::string dir_separator, const std::string file, std::string & name)
{
  gchar* dirname = g_path_get_dirname( file.c_str() );
  name = dirname;
  g_free( dirname );
  return true;
}

