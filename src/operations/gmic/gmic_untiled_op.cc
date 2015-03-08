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


#include "../base/pf_mkstemp.hh"

#include "gmic.hh"
#include "gmic_untiled_op.hh"



PF::GmicUntiledOperationPar::GmicUntiledOperationPar(): 
  UntiledOperationPar(),
  gmic_instance( NULL )
{	
}


PF::GmicUntiledOperationPar::~GmicUntiledOperationPar()
{
	std::cout<<"GmicUntiledOperationPar::~GmicUntiledOperationPar()"<<std::endl;
}


gmic* PF::GmicUntiledOperationPar::new_gmic()
{
  //if( custom_gmic_commands ) delete [] custom_gmic_commands;
  if( gmic_instance ) delete gmic_instance;

  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
  snprintf( fname, 499, "%s\\gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
  struct stat buffer;   
  int stat_result = stat( fname, &buffer );
  if( stat_result != 0 ) {
    fname[0] = 0;
  }
#elif defined(__APPLE__) && defined (__MACH__)
  snprintf( fname, 499, "%s/../share/photoflow/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
  struct stat buffer;
  int stat_result = stat( fname, &buffer );
  if( stat_result != 0 ) {
    fname[0] = 0;
  }
#else
  if( getenv("HOME") ) {
    //snprintf( fname, 499, "%s/.photoflow/gmic_update.gmic", getenv("HOME") );
    snprintf( fname, 499, "%s/share/photoflow/gmic_def.gmic", INSTALL_PREFIX );
    std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
    struct stat buffer;   
    int stat_result = stat( fname, &buffer );
    if( stat_result != 0 ) {
      //snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
      //stat_result = stat( fname, &buffer );
      //if( stat_result != 0 ) {
      fname[0] = 0;
      //}
    }
  }
#endif
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
  return gmic_instance;
}


bool PF::GmicUntiledOperationPar::run_gmic( std::string command )
{
  if( command.empty() ) 
    return false;
  std::cout<<"g'mic command: "<<command<<std::endl;
  if( !new_gmic() ) 
    return false;
  gmic_instance->run( command.c_str() );
  
  if( gmic_instance ) {
    delete gmic_instance;
    gmic_instance = NULL;
  }
  
  raster_images_attach();
  
  return true;
}


