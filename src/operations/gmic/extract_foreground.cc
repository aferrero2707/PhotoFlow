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

#include "../raster_image.hh"

#include "../operations/convertformat.hh"
#include "../operations/blender.hh"

#include "gmic.hh"
#include "extract_foreground.hh"

using namespace PF;

class GmicExtractForegroundMaskPar: public PF::OpParBase
{
  PF::GmicExtractForegroundPar* par;

public:
  GmicExtractForegroundMaskPar(): PF::OpParBase(), par( NULL ) {}
  void set_par( PF::GmicExtractForegroundPar* p ) { par = p; }
  PF::GmicExtractForegroundPar* get_par() { return par; }
};


template < OP_TEMPLATE_DEF > 
class GmicExtractForegroundMaskProc
{
public: 
  void render(VipsRegion** ireg, int n, int in_first,
              VipsRegion* imap, VipsRegion* omap, 
              VipsRegion* oreg, GmicExtractForegroundMaskPar* par)
  {	
  }
};



template < OP_TEMPLATE_DEF_CS_SPEC > 
class GmicExtractForegroundMaskProc< OP_TEMPLATE_IMP_CS_SPEC(PF::PF_COLORSPACE_GRAYSCALE) >
{
public: 
  void render(VipsRegion** ireg, int n, int in_first,
              VipsRegion* imap, VipsRegion* omap, 
              VipsRegion* oreg, GmicExtractForegroundMaskPar* par)
  {	
    if( !par ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
    //int width = r->width;
    int height = r->height;

    T* p;
    int y;

    // Fill with zeroes
    for( y = 0; y < height; y++ ) {
      p = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
      memset( p, 0, sizeof(T)*line_size );
    }

    std::list< std::pair<int,int> >::iterator i;
    for(i = par->get_par()->get_fg_points().get().begin(); 
        i != par->get_par()->get_fg_points().get().end(); i++ ) {
      if( !vips_rect_includespoint( r, i->first, i->second ) )
        continue;

      p = (T*)VIPS_REGION_ADDR( oreg, i->first, i->second ); 
      p[0] = 2;
    }
    for(i = par->get_par()->get_bg_points().get().begin(); 
        i != par->get_par()->get_bg_points().get().end(); i++ ) {
      if( !vips_rect_includespoint( r, i->first, i->second ) )
        continue;

      p = (T*)VIPS_REGION_ADDR( oreg, i->first, i->second ); 
      p[0] = 1;
    }
  }
};



PF::GmicExtractForegroundPar::GmicExtractForegroundPar(): 
  OpParBase(),
  fg_points( "fg_points", this ),
  bg_points( "bg_points", this ),
  custom_gmic_commands( NULL ),
  gmic_instance( NULL ),
  do_update( true ),
  raster_image( NULL )
{	
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  convert_format2 = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  blender = PF::new_blender();

  PF::Processor<GmicExtractForegroundMaskPar,GmicExtractForegroundMaskProc>* proc = 
    new PF::Processor<GmicExtractForegroundMaskPar,GmicExtractForegroundMaskProc>();
  proc->get_par()->set_par( this );
  mask_proc = proc;

  set_type( "gmic_extract_foreground" );
}


PF::GmicExtractForegroundPar::~GmicExtractForegroundPar()
{
	std::cout<<"GmicExtractForegroundPar::~GmicExtractForegroundPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"GmicExtractForegroundPar::~GmicExtractForegroundPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"GmicExtractForegroundPar::~GmicExtractForegroundPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
}


gmic* PF::GmicExtractForegroundPar::new_gmic()
{
  if( custom_gmic_commands ) delete [] custom_gmic_commands;
  if( gmic_instance ) delete gmic_instance;

  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
  snprintf( fname, 499, "%s\\gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
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


bool PF::GmicExtractForegroundPar::import_settings( OpParBase* pin )
{
  GmicExtractForegroundPar* par = dynamic_cast<GmicExtractForegroundPar*>( pin );
  if( !par ) return false;
  preview_cache_file_name = par->get_preview_cache_file_name();
  render_cache_file_name = par->get_render_cache_file_name();
  preview_mode = par->get_preview_mode();

  return( PF::OpParBase::import_settings(pin) );
}


int PF::GmicExtractForegroundPar::get_padding( int level )
{
  return( 1 );
}


void PF::GmicExtractForegroundPar::pre_build( rendermode_t mode )
{
  if( mode==PF_RENDER_PREVIEW ) {
    if( !do_update ) return;
    
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicExtractForegroundPar::pre_build(): cache file="<<fname<<std::endl;
    preview_cache_file_name = fname;
  } else {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicExtractForegroundPar::pre_build(): cache file="<<fname<<std::endl;
    render_cache_file_name = fname;
  }

  do_update = false;
}


VipsImage* PF::GmicExtractForegroundPar::build(std::vector<VipsImage*>& in, int first, 
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  /*
  VipsImage* bgdimg = NULL;
  if( in.size() > 0 ) bgdimg = in[0];
  if( !bgdimg ) return NULL;
  */
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  if( !srcimg ) return NULL;

  //PF_REF( srcimg, "GmicExtractForegroundPar::build(): srcimg ref (temporary)" );
  //return srcimg;

  std::string cache_file_name;
  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    std::cout<<"GmicExtractForegroundPar::build() setting cache file name to preview_cache_file_name ("
             <<preview_cache_file_name<<")"<<std::endl;
    cache_file_name = preview_cache_file_name;
  } else { 
    std::cout<<"GmicExtractForegroundPar::build() setting cache file name to render_cache_file_name ("
             <<render_cache_file_name<<")"<<std::endl;
    cache_file_name = render_cache_file_name;
  }
  std::cout<<"GmicExtractForegroundPar::build() render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;

  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i == raster_images.end() ) {

    std::cout<<"GmicExtractForegroundPar::build(): raster_image="<<(void*)raster_image<<std::endl;
    if( raster_image ) {
      raster_image->unref();
      std::cout<<"GmicExtractForegroundPar::build(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
      if( raster_image->get_nref() == 0 ) {
        std::map<Glib::ustring, RasterImage*>::iterator i = 
          raster_images.find( cache_file_name );
        if( i != raster_images.end() ) 
          raster_images.erase( i );
        delete raster_image;
			std::cout<<"GmicExtractForegroundPar::build(): raster_image deleted"<<std::endl;
			raster_image = 0;
      }
    }

    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    std::cout<<"GmicExtractForegroundPar: temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
    if( temp_fd < 0 ) return NULL;

    std::vector<VipsImage*> in2;
    mask_proc->get_par()->grayscale_image( srcimg->Xsize, srcimg->Ysize );
    mask_proc->get_par()->set_format( IM_BANDFMT_UCHAR );
    VipsImage* maskimg = mask_proc->get_par()->build( in2, 0, NULL, NULL, level );
#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
    vips_image_write_to_file( maskimg, fname );
#else
    vips_image_write_to_file( maskimg, fname, NULL );
#endif
    std::cout<<"Mask file "<<fname<<" has been written"<<std::endl;
    PF_UNREF( maskimg, "GmicExtractForegroundPar::build(): after write_to_file" );

    char fname2[500];
    sprintf( fname2,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd2 = pf_mkstemp( fname2, 4 );
    std::cout<<"GmicExtractForegroundPar: temp file: "<<fname2<<"  fd="<<temp_fd2<<std::endl;
    if( temp_fd2 < 0 ) return NULL;

    in2.push_back( srcimg );
    convert_format->get_par()->set_image_hints( srcimg );
    convert_format->get_par()->set_format( IM_BANDFMT_FLOAT );
    VipsImage* floatimg = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
    if( !floatimg ) return NULL;

#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
    vips_image_write_to_file( floatimg, fname2 );
#else
    vips_image_write_to_file( floatimg, fname2, NULL );
#endif
    PF_UNREF( floatimg, "GmicExtractForegroundPar::build(): after write_to_file" );

    gmic_list<float> images;
    gmic_list<char> images_names;

    std::string command = std::string("-verbose + -verbose + -verbose + -input ") + fname2 + " -n 0,255 -input " + fname;
    command = command + " --blur[0] 0.2% -gradient_norm[-1] -fill[-1] '1/(1+i^2)'" + " -watershed[1] [2] -reverse[-2,-1]"
      + " -n[2] 0,1 -output[2] " + cache_file_name + ",float,lzw";
    std::cout<<"foreground extract command: "<<command<<std::endl;

    if( new_gmic() ) {
      gmic_instance->run( command.c_str() );
      //delete gmic_instance;
      //gmic_instance = NULL;
    }
    close( temp_fd );
    close( temp_fd2 );

    raster_image = new RasterImage( cache_file_name );
    raster_images.insert( make_pair(cache_file_name, raster_image) );

    if( gmic_instance ) {
      delete gmic_instance;
      gmic_instance = NULL;
    }
    unlink( fname );
    unlink( fname2 );
  } else {
    std::cout<<"GmicExtractForegroundPar::build(): raster_image found ("<<cache_file_name<<")"<<std::endl;
    raster_image = i->second;
    raster_image->ref();
  }
  if( !raster_image )
    return NULL;

  VipsImage* image = raster_image->get_image( level );
  
  if( !image ) return NULL;

  VipsImage* mask = image;
  if( (get_format() != image->BandFmt) ) {
    std::vector<VipsImage*> in2;
    in2.push_back( image );
    convert_format2->get_par()->set_image_hints( image );
    convert_format2->get_par()->set_format( get_format() );
    mask = convert_format2->get_par()->build( in2, 0, NULL, NULL, level );
    //std::cout<<"ImageReaderPar::build(): out ("<<(void*)out<<") refcount after convert_format: "<<G_OBJECT(out)->ref_count<<std::endl;
    //g_object_unref( image );
    PF_UNREF( image, "ImageReaderPar::build(): image unref after convert_format" );
  }

  if( mask ) {
    //PF_REF( image, "GmicExtractForegroundPar::build()" );
    set_image_hints( mask );
  }

  VipsImage* out = NULL;
  /*
  if( !is_editing() || (preview_mode == EXTRACT_FG_PREVIEW_BLEND) ) {
    blender->get_par()->set_image_hints( bgdimg );
    blender->get_par()->set_format( get_format() );
    std::vector<VipsImage*> in2;
    in2.push_back( bgdimg );
    in2.push_back( srcimg );
    out = blender->get_par()->build( in2, 0, NULL, mask, level );
    PF_UNREF( mask, "GmicExtractForegroundPar::build(): mask unref (EXTRACT_FG_PREVIEW_BLEND)" );
  }
  */
  if( is_editing() && (preview_mode == EXTRACT_FG_PREVIEW_POINTS) ) {
    PF_REF( srcimg, "GmicExtractForegroundPar::build(): srcimg ref (EXTRACT_FG_PREVIEW_POINTS)" );
    PF_UNREF( mask, "GmicExtractForegroundPar::build(): mask unref (EXTRACT_FG_PREVIEW_POINTS)" );
    out = srcimg;
  }
  if( !is_editing() || (preview_mode == EXTRACT_FG_PREVIEW_MASK) ) {
    VipsImage* bandv[10];
    VipsImage* joined;
    for( int bi = 0; bi < srcimg->Bands; bi++ )
      bandv[bi] = mask;
    if( vips_bandjoin( bandv, &joined, srcimg->Bands, NULL ) ) {
      return NULL;
    }
    vips_copy( joined, &out, 
	       "format", srcimg->BandFmt,
	       "bands", srcimg->Bands,
	       "coding", srcimg->Coding,
	       "interpretation", srcimg->Type,
	       NULL );    
    //rgb_image( out->Xsize, out->Ysize );
    set_image_hints( srcimg );
    PF_UNREF( mask, "GmicExtractForegroundPar::build(): mask unref (EXTRACT_FG_PREVIEW_MASK)" );
  }
  std::cout<<"GmicExtractForegroundPar::build(): out="<<out<<std::endl;
  return out;

  /*
  if( !(gmic->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
	for( int l = 1; l <= level; l++ )
		scalefac *= 2;

  std::string command = "-gimp_dreamsmooth  ";
  command = command + prop_interations.get_str();
  command = command + std::string(",") + prop_equalize.get_str();
  command = command + std::string(",") + prop_merging_option.get_enum_value_str();
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_reverse.get_str();
  command = command + std::string(",") + prop_smoothness.get_str() + ",1," + padding.get_str();
  std::cout<<"drawm smooth command: "<<command<<std::endl;
  gpar->set_command( command.c_str() );
  gpar->set_iterations( iterations.get() );
  gpar->set_padding( get_padding( level ) );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
  */
}


PF::ProcessorBase* PF::new_gmic_extract_foreground()
{
  return( new PF::Processor<PF::GmicExtractForegroundPar,PF::GmicExtractForegroundProc>() );
}
