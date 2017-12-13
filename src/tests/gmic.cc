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
#include <fstream>
#include "../vips/gmic/gmic/src/gmic.h"


int main(int argc, char** argv)
{
  char* custom_gmic_commands;
  gmic* gmic_instance;
  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
  snprintf( fname, 499, "/Users/ferrero/gtk/inst/share/photoflow/gmic_def.gmic" );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
  if( strlen( fname ) > 0 ) {
    std::ifstream t;
    int length;
    t.open(fname);      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    custom_gmic_commands = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(custom_gmic_commands, length);       // read the whole file into the buffer
    t.close();                    // close file handle
    std::cout<<"G'MIC custom commands loaded"<<std::endl;
  }

  /* Make a gmic for this thread.
   */
  gmic_instance = new gmic( 0, custom_gmic_commands, false, 0, 0 );

  std::string command = "-verbose + -verbose + -verbose + -input /Users/ferrero/test.tif ";
  command = command + std::string(" -n 0,255 -gimp_dreamsmooth 1,0,1,0.8,0,0.8,1,0 ");
  command = command + " -n 0,1 -output /Users/ferrero/test2.tif,float,lzw";
  std::cout<<"dream smooth command: "<<command<<std::endl;

  gmic_instance->run( command.c_str() );

  delete gmic_instance;

  return 0;
}
