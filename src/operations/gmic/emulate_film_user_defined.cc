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

#include "../../base/pf_mkstemp.hh"
#include "../../base/fileutils.hh"

#include "../../base/processor_imp.hh"
#include "gmic.hh"
#include "emulate_film_user_defined.hh"



PF::GmicEmulateFilmUserDefinedPar::GmicEmulateFilmUserDefinedPar():
OpParBase(),
prop_filename("filename", this),
prop_haldlutfilename("haldlutfilename", this),
  prop_opacity("opacity",this,100),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,0),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0),
  temp_lut_created( false )
{	
  gmic_proc = PF::new_gmic();
  gmic_instance = NULL;
  set_type( "gmic_emulate_film_user_defined" );
  set_default_name( _("apply LUT") );
}


PF::GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar()
{
  if( temp_lut_created ) {
    std::cout<<"GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar(): deleting "<<prop_haldlutfilename.get()<<std::endl;
    unlink( prop_haldlutfilename.get().c_str() );
  }

  std::cout<<"GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar()"<<std::endl;
  if( gmic_instance ) delete gmic_instance;
}


gmic* PF::GmicEmulateFilmUserDefinedPar::new_gmic()
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
  //snprintf( fname, 499, "%s/../share/photoflow/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_data_dir().c_str() );
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


void PF::GmicEmulateFilmUserDefinedPar::pre_build( rendermode_t mode )
{
  std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build() called"<<std::endl;

  if( prop_filename.get().empty() ) return;

  std::string ext;
#if defined(WIN32)
  PF::getFileExtension( "\\", prop_filename.get(), ext );
#else
  PF::getFileExtension( "/", prop_filename.get(), ext );
#endif

  if( ext != "cube" ) {
    return;
  }

  if( prop_filename.get() != cur_lut_filename || prop_haldlutfilename.get().empty() ) {

    std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build(): converting CUBE LUT \""<<prop_filename.get()
        <<"\" to HaldCLUT format"<<std::endl;

    if( !new_gmic() )
      return;

    std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build(): gmic_instance="<<gmic_instance<<std::endl;

    if( prop_haldlutfilename.get().empty() ) {
      char fname[500];
      sprintf( fname,"%shaldclut-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
      int temp_fd = pf_mkstemp( fname, 4 );
      if( temp_fd >= 0 ) close( temp_fd );
      prop_haldlutfilename.update( std::string(fname) );
    }

    std::string command = "-verbose +  -input_cube \"";
    command = command + prop_filename.get();
    command = command + std::string("\" -r 64,64,64,3,3 -r 512,512,1,3,-1 ");
    command = command + std::string(" -o ") + prop_haldlutfilename.get();
    std::cout<<"g'mic command: "<<command<<std::endl;
    gmic_instance->run( command.c_str() );

    if( gmic_instance ) {
      delete gmic_instance;
      gmic_instance = NULL;
    }

    temp_lut_created = true;
    cur_lut_filename = prop_filename.get();
  }
}


VipsImage* PF::GmicEmulateFilmUserDefinedPar::build(std::vector<VipsImage*>& in, int first,
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;
  
  if( prop_filename.get().empty() ) {
    PF_REF( out, "" );
    return out;
  }

  if( !(gmic_proc->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic_proc->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
	for( int l = 1; l <= level; l++ )
		scalefac *= 2;

  std::string ext;
#if defined(WIN32)
  PF::getFileExtension( "\\", prop_filename.get(), ext );
#else
  PF::getFileExtension( "/", prop_filename.get(), ext );
#endif

  std::string lutname = prop_filename.get();
  if( ext == "cube" ) {
    lutname = prop_haldlutfilename.get();
  }

	std::cout<<"GmicEmulateFilmUserDefinedPar::build(): prop_haldlutfilename="<<prop_haldlutfilename.get()<<std::endl;
  std::string command = "-verbose - -gimp_emulate_film_userdefined  2,";
  command = command + lutname;
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_brightness.get_str();
  command = command + std::string(",") + prop_contrast.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_hue.get_str();
  command = command + std::string(",") + prop_saturation.get_str();
  command = command + std::string(",") + prop_post_normalize.get_str();
  gpar->set_command( command.c_str() );
  gpar->set_iterations( 1 );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
}


PF::ProcessorBase* PF::new_gmic_emulate_film_user_defined()
{
  return( new PF::Processor<PF::GmicEmulateFilmUserDefinedPar,PF::GmicEmulateFilmUserDefinedProc>() );
}
