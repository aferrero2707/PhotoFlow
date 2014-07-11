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
#include "../base/rawmatrix.hh"

#include "../rt/rtengine/camconst.h"
#include "raw_image.hh"



PF::RawImage::RawImage( const Glib::ustring f ):
#ifdef PF_USE_DCRAW_RT
  rtengine::RawImage(f), 
#endif
	nref(1), file_name( f ),
  image( NULL ), demo_image( NULL )
{
	dcraw_data_t* pdata;
#ifdef PF_USE_LIBRAW
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

	int iwidth = raw_loader->imgdata.sizes.iwidth;
	int iheight = raw_loader->imgdata.sizes.iheight;
	pdata = &(raw_loader->imgdata);
#endif
  
#ifdef PF_USE_DCRAW_RT
	if( !rtengine::CameraConstantsStore::getInstance() )
		rtengine::CameraConstantsStore::initCameraConstants(PF::PhotoFlow::Instance().get_base_dir(),"");
	int result = loadRaw( true, true, NULL, 1 );
	if( result != 0 ) {
		std::cout<<"RawImage::RawImage("<<f <<"): loadRaw() result="<<result<<std::endl;
		return;
	}
	if( iwidth == 0 || iheight == 0 )
		return;

	compress_image();
	if ((this->get_cblack(4)+1)/2 == 1 && (this->get_cblack(5)+1)/2 == 1) {
		for (int c = 0; c < 4; c++){
			c_black[FC(c/2,c%2)] = this->get_cblack(6 + c/2 % this->get_cblack(4) * this->get_cblack(5) + c%2 % this->get_cblack(5));
		}
	} else {
		for (int c = 0; c < 4; c++){
			c_black[c] = (float) this->get_cblack(c);
		}
	}
	dcraw_data_t dcraw_data;
	dcraw_data.color.cam_mul[0] = get_cam_mul(0);
	dcraw_data.color.cam_mul[1] = get_cam_mul(1);
	dcraw_data.color.cam_mul[2] = get_cam_mul(2);
	dcraw_data.color.cam_mul[3] = get_cam_mul(3);
	if(dcraw_data.color.cam_mul[3] < 0.00000001) 
		dcraw_data.color.cam_mul[3] = dcraw_data.color.cam_mul[1];
	std::cout<<"Camera WB multipliers: "<<get_cam_mul(0)<<" "<<get_cam_mul(1)
					 <<" "<<get_cam_mul(2)<<" "<<get_cam_mul(3)<<std::endl;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 3; j++)
			dcraw_data.color.cam_xyz[i][j] = get_cam_xyz(i,j);
	pdata = &dcraw_data;
#endif
  //==================================================================
  // Save decoded data to cache file on disk.
  // The pixel values are normalized to the [0..65535] range 
	// using the default black and white levels.
  char fname[500];
  sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int temp_fd = pf_mkstemp( fname );
  if( temp_fd < 0 ) return;
  std::cout<<"RawLoader: cache file: "<<fname<<std::endl;
	cache_file_name = fname;
  
#ifndef NDEBUG
  std::cout<<"Saving raw data to buffer..."<<std::endl;
#endif
  int row, col, col2;
  //size_t pxsize = sizeof(PF::RawPixel);
  size_t pxsize = sizeof(float)+sizeof(guint8);
  guint8* rowbuf = (guint8*)malloc( iwidth*pxsize );
#ifndef NDEBUG
  std::cout<<"Row buffer allocated: "<<(void*)rowbuf<<std::endl;
#endif
	guint8* ptr;
	float* fptr;
  for(row=0;row<iheight;row++) {
    unsigned int row_offset = row*iwidth;
		ptr = rowbuf;
		/**/
    for(col=0; col<iwidth; col++) {
#ifdef PF_USE_LIBRAW
      unsigned char color = (unsigned char)raw_loader->COLOR(row,col);
      float val = raw_loader->imgdata.image[row_offset+col][color];
			val /= raw_loader->imgdata.color.maximum;
			val *= 65535;
			if( row==10 && col==11 ) {
				//std::cout<<"raw pixel @ (0,0): val="<<*fptr<<"  c="<<(int)ptr[sizeof(float)]<<std::endl;
				std::cout<<"raw pixel @ ("<<row<<","<<col<<"): val="<<val<<"  c="<<(int)color<<std::endl;
			}
#endif
#ifdef PF_USE_DCRAW_RT
      unsigned char color = (unsigned char)FC(row,col);
      float val = (data[row][col]-c_black[color])*65535/(this->get_white(color)-c_black[color]);
#endif
			fptr = (float*)ptr;
			*fptr = val;
			ptr[sizeof(float)] = color;
			ptr += pxsize;
    }
		/**/
    write( temp_fd, rowbuf, pxsize*iwidth );
#ifndef NDEBUG
    if( (row%100) == 0 ) std::cout<<"  row "<<row<<" saved."<<std::endl;
#ifdef PF_USE_DCRAW_RT
		if( row==0 ) {
			for(col=0; col<10; col++) {
				std::cout<<"  val="<<data[row][col]<<"  c="<<(unsigned int)FC(row,col)<<std::endl;
			}
		}
#endif
#endif
  }
#ifndef NDEBUG
  std::cout<<"Deleting row buffer: "<<(void*)rowbuf<<std::endl;
#endif
  free( rowbuf );
#ifndef NDEBUG
  std::cout<<"Row buffer deleted"<<std::endl;
#endif

  std::cout<<"iwidth="<<iwidth<<std::endl
	   <<"iheight="<<iheight<<std::endl
	   <<"row="<<row<<"  sizeof(PF::raw_pixel_t)="<<sizeof(PF::raw_pixel_t)<<std::endl;
  //==================================================================
  // Load the raw file into a vips image
  VipsImage* in;
  if( vips_rawload( fname, &in, iwidth, 
										iheight, pxsize, NULL ) )
    return;
  //unlink( fname );
  
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
  void* buf = malloc( sizeof(dcraw_data_t) );
  if( !buf ) return;
  memcpy( buf, pdata, sizeof(dcraw_data_t) );
  vips_image_set_blob( image, "raw_image_data",
		       (VipsCallbackFn) g_free, buf, 
		       sizeof(dcraw_data_t) );
#ifdef PF_USE_LIBRAW
  delete raw_loader;
  //return;
#endif

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
  
  sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int fd = pf_mkstemp( fname );
  if( fd < 0 )
    return;
  std::cout<<"RawLoader: cache file: "<<fname<<std::endl;
	cache_file_name2 = fname;
  
  vips_rawsave_fd( out_demo, fd, NULL );
  g_object_unref( out_demo );
  delete fast_demosaic;
  
  if( vips_rawload( fname, &out_demo, width, 
		    height, sizeof(float)*3, NULL ) ) {
    unlink( fname );
    return;
  }
  //unlink( fname );

  vips_copy( out_demo, &demo_image, 
	     "format", VIPS_FORMAT_FLOAT,
	     "bands", (int)3,
	     "coding", VIPS_CODING_NONE,
	     "interpretation", VIPS_INTERPRETATION_RGB,
	     NULL );
  g_object_unref( out_demo );
  
  //==================================================================
  // Save the LibRaw parameters into the image
  void* buf2 = malloc( sizeof(dcraw_data_t) );
  if( !buf2 ) return;
  memcpy( buf2, buf, sizeof(dcraw_data_t) );
  vips_image_set_blob( demo_image, "raw_image_data",
		       (VipsCallbackFn) g_free, buf2, 
		       sizeof(dcraw_data_t) );
  
  pyramid.init( demo_image );
}


PF::RawImage::~RawImage()
{
  if( image ) PF_UNREF( image, "RawImage::~RawImage() image" );
  if( demo_image ) PF_UNREF( demo_image, "RawImage::~RawImage() demo_image" );
	std::cout<<"RawImage::~RawImage() called."<<std::endl;
	if( !(cache_file_name.empty()) )
		unlink( cache_file_name.c_str() );
	if( !(cache_file_name2.empty()) )
		unlink( cache_file_name2.c_str() );
}


std::map<Glib::ustring, PF::RawImage*> PF::raw_images;


