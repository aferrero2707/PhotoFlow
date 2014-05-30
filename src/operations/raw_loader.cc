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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/rawmatrix.hh"

#include "raw_loader.hh"


PF::RawImage::RawImage( std::string f ):
  nref(1), file_name( f ),
  image( NULL ), demo_image( NULL )
{
  LibRaw* raw_loader = new LibRaw();
  int result = raw_loader->open_file( file_name.c_str() );
  if( result != 0 ) return;
  if( (raw_loader->imgdata.idata.cdesc[0] != 'R') ||
      (raw_loader->imgdata.idata.cdesc[1] != 'G') ||
      (raw_loader->imgdata.idata.cdesc[2] != 'B') ||
      (raw_loader->imgdata.idata.cdesc[3] != 'G') )
    return;

  raw_loader->imgdata.params.no_auto_bright = 1;
  result = raw_loader->unpack();
  if( result != 0 ) 
    return;

  raw_loader->raw2image();
#warning "TODO: add a custom subtract_black() function that works in unbounded mode"
  raw_loader->subtract_black();

  
  //==================================================================
  // Save decoded data to cache file on disk
  // The pixel values are normalized to the [0..1] range
  char fname[500];
  if( getenv("HOME") )
    sprintf( fname,"%s/.photoflow/cache/raw-XXXXXX", getenv("HOME") );
  else
    sprintf( fname,"/tmp/pfraw-XXXXXX" );
  int temp_fd = mkstemp( fname );
  if( temp_fd < 0 ) return;
  
#ifndef NDEBUG
  std::cout<<"Saving raw data to buffer..."<<std::endl;
#endif
  int row, col, col2;
  //size_t pxsize = sizeof(PF::RawPixel);
  //size_t pxsize2 = sizeof(float)+sizeof(unsigned char);
  PF::RawPixel* rowbuf = new PF::RawPixel[raw_loader->imgdata.sizes.iwidth*2];
  for(row=0;row<raw_loader->imgdata.sizes.iheight;row++) {
    unsigned int row_offset = row*raw_loader->imgdata.sizes.iwidth;
    for(col=0; col<raw_loader->imgdata.sizes.iwidth; col++) {
      unsigned char color = (unsigned char)raw_loader->COLOR(row,col);
      float val = raw_loader->imgdata.image[row_offset+col][color];
      rowbuf[col].data = val/raw_loader->imgdata.color.maximum;
      rowbuf[col].color = color;
    }
    write( temp_fd, rowbuf, sizeof(PF::RawPixel)*raw_loader->imgdata.sizes.iwidth );
#ifndef NDEBUG
    if( (row%100) == 0 ) std::cout<<"  row "<<row<<" saved."<<std::endl;
#endif
  }
  delete rowbuf;

  //==================================================================
  // Load the raw file into a vips image
  VipsImage* in;
  if( vips_rawload( fname, &in, raw_loader->imgdata.sizes.iwidth, 
		    raw_loader->imgdata.sizes.iheight, sizeof(PF::RawPixel), NULL ) )
    return;
  unlink( fname );
  
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_MULTIBAND;
  VipsBandFormat format = VIPS_FORMAT_UCHAR;
  int nbands = 2;
  vips_copy( in, &image, 
	     "format", format,
	     //"bands", nbands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  g_object_unref( in );
  
  
  //==================================================================
  // Save the LibRaw parameters into the image
  void* buf = malloc( sizeof(raw_loader->imgdata) );
  if( !buf ) return;
  memcpy( buf, &(raw_loader->imgdata), sizeof(libraw_data_t) );
  vips_image_set_blob( image, "raw_image_data",
		       (VipsCallbackFn) g_free, buf, 
		       sizeof(libraw_data_t) );
  delete raw_loader;
  //return;

  
  int width = image->Xsize;
  int height = image->Ysize;
  PF::ProcessorBase* fast_demosaic = PF::new_fast_demosaic();
  fast_demosaic->get_par()->set_image_hints( image );
  fast_demosaic->get_par()->set_format( VIPS_FORMAT_FLOAT );
  fast_demosaic->get_par()->set_demand_hint( VIPS_DEMAND_STYLE_FATSTRIP );
  std::vector<VipsImage*> in2;
  in2.push_back( image );
  unsigned int level = 0;
  VipsImage* out_demo = fast_demosaic->get_par()->build( in2, 0, NULL, NULL, level );
  //g_object_unref( image );
  
  if( getenv("HOME") )
    sprintf( fname,"%s/.photoflow/cache/raw-XXXXXX", getenv("HOME") );
  else
    sprintf( fname,"/tmp/pfraw-XXXXXX", getenv("HOME") );
  int fd = mkstemp( fname );
  if( fd < 0 )
    return;
  
  vips_rawsave_fd( out_demo, fd, NULL );
  g_object_unref( out_demo );
  delete fast_demosaic;
  
  if( vips_rawload( fname, &out_demo, width, 
		    height, sizeof(float)*3, NULL ) ) {
    unlink( fname );
    return;
  }
  unlink( fname );

  vips_copy( out_demo, &demo_image, 
	     "format", VIPS_FORMAT_FLOAT,
	     "bands", (int)3,
	     "coding", VIPS_CODING_NONE,
	     "interpretation", VIPS_INTERPRETATION_RGB,
	     NULL );
  g_object_unref( out_demo );
  
  //==================================================================
  // Save the LibRaw parameters into the image
  void* buf2 = malloc( sizeof(libraw_data_t) );
  if( !buf2 ) return;
  memcpy( buf2, buf, sizeof(libraw_data_t) );
  vips_image_set_blob( demo_image, "raw_image_data",
		       (VipsCallbackFn) g_free, buf2, 
		       sizeof(libraw_data_t) );
  
  pyramid.init( demo_image );
}


PF::RawImage::~RawImage()
{
  if( image ) g_object_unref( image );
  if( demo_image ) g_object_unref( demo_image );
}


std::map<std::string, PF::RawImage*> raw_images;


PF::RawLoaderPar::RawLoaderPar(): 
  OpParBase(), 
  file_name("file_name", this),
  raw_image( NULL ),
  image(NULL),
  demo_image(NULL),
  raw_loader(NULL),
  current_format(VIPS_FORMAT_NOTSET)
{
  set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
  //convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  //blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  fast_demosaic = new_fast_demosaic();
  set_type("raw_loader" );
}


PF::RawLoaderPar::~RawLoaderPar()
{
  if( raw_image ) {
    raw_image->unref();
    if( raw_image->get_nref() == 0 ) {
      std::map<std::string, RawImage*>::iterator i = 
	raw_images.find( file_name.get() );
      if( i != raw_images.end() ) 
	raw_images.erase( i );
      delete raw_image;
    }
  }
}


VipsImage* PF::RawLoaderPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  bool modified = false;

  if( file_name.get().empty() )
    return NULL;

  std::map<std::string, RawImage*>::iterator i = 
    raw_images.find( file_name.get() );
  if( i == raw_images.end() ) {
    raw_image = new RawImage( file_name.get() );
    raw_images.insert( make_pair(file_name.get(), raw_image) );
  } else {
    raw_image = i->second;
    raw_image->ref();
  }
  if( !raw_image )
    return NULL;

  VipsImage* image = raw_image->get_image( level );
  
#ifndef NDEBUG
  std::cout<<"RawLoaderPar::build(): "<<std::endl;
  std::cout<<"image->Interpretation: "<<image->Type<<std::endl;
#endif


  if( image ) {
    g_object_ref( image );
    set_image_hints( image );
  }
  return image;
}


PF::ProcessorBase* PF::new_raw_loader()
{
  return new PF::Processor<PF::RawLoaderPar,PF::RawLoader>();
}
