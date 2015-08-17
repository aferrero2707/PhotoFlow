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

#include <gexiv2/gexiv2-metadata.h>

#include "../base/pf_mkstemp.hh"
#include "../base/rawmatrix.hh"

#include "../rt/rtengine/camconst.h"
#include "raw_image.hh"



PF::RawImage::RawImage( const std::string f ):
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
  if( result != 0 ) {
    std::cout<<"RawImage::RawImage(): raw_loader->open_file("<<file_name<<") failed"<<std::endl;
    delete raw_loader;
    return;
  }
  if( (raw_loader->imgdata.idata.cdesc[0] != 'R') ||
      (raw_loader->imgdata.idata.cdesc[1] != 'G') ||
      (raw_loader->imgdata.idata.cdesc[2] != 'B') ||
      (raw_loader->imgdata.idata.cdesc[3] != 'G') ) {
    std::cout<<"RawImage::RawImage(): not an RGBG image"<<std::endl;
    delete raw_loader;
    return;
  }

  raw_loader->imgdata.params.no_auto_bright = 1;
  result = raw_loader->unpack();
  if( result != 0 ) {
    std::cout<<"RawImage::RawImage(): unpack failed"<<std::endl;
    delete raw_loader;
    return;
  }

  raw_loader->raw2image();

#ifdef DO_WARNINGS
#warning "TODO: add a custom subtract_black() function that works in unbounded mode"
#endif
  raw_loader->subtract_black();

	int iwidth = raw_loader->imgdata.sizes.iwidth;
	int iheight = raw_loader->imgdata.sizes.iheight;
  std::cout<<"LibRAW dimensions: raw width="<<raw_loader->imgdata.sizes.raw_width<<",height="<<raw_loader->imgdata.sizes.raw_height
           <<"    width="<<raw_loader->imgdata.sizes.width<<",height="<<raw_loader->imgdata.sizes.height
           <<"    iwidth="<<raw_loader->imgdata.sizes.iwidth<<",iheight="<<raw_loader->imgdata.sizes.iheight
           <<"    flip="<<raw_loader->imgdata.sizes.flip<<std::endl;
	pdata = &(raw_loader->imgdata);
	std::cout<<"LibRAW camera WB multipliers: "<<pdata->color.cam_mul[0]<<" "<<pdata->color.cam_mul[1]
					 <<" "<<pdata->color.cam_mul[2]<<" "<<pdata->color.cam_mul[3]<<std::endl;
	if(pdata->color.cam_mul[3] < 0.00000001) 
		pdata->color.cam_mul[3] = pdata->color.cam_mul[1];
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
	std::cout<<"DCRAW camera WB multipliers: "<<get_cam_mul(0)<<" "<<get_cam_mul(1)
					 <<" "<<get_cam_mul(2)<<" "<<get_cam_mul(3)<<std::endl;
	if(dcraw_data.color.cam_mul[3] < 0.00000001) 
		dcraw_data.color.cam_mul[3] = dcraw_data.color.cam_mul[1];
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
  //size_t pxsize = sizeof(float)+sizeof(guint8);
  size_t pxsize = sizeof(float)*2;
  guint8* rowbuf = (guint8*)malloc( iwidth*pxsize );
#ifndef NDEBUG
  std::cout<<"Row buffer allocated: "<<(void*)rowbuf<<std::endl;
#endif
  /* Normalized raw data to 65535 and build raw histogram
   * */
  // Allocate raw histogram and fill it with zero
  int* raw_hist = (int*)malloc( 65535*3*sizeof(int) );
  memset( raw_hist, 0, 65535*3*sizeof(int) );

	guint8* ptr;
	float* fptr;
  for(row=0;row<iheight;row++) {
    unsigned int row_offset = row*iwidth;
		ptr = rowbuf;
    for(col=0; col<iwidth; col++) {
#ifdef PF_USE_LIBRAW
      unsigned char color = (unsigned char)raw_loader->COLOR(row,col);
      float val = raw_loader->imgdata.image[row_offset+col][color];
			val /= raw_loader->imgdata.color.maximum;
			val *= 65535;
			//if( row==10 && col==11 ) {
				//std::cout<<"raw pixel @ (0,0): val="<<*fptr<<"  c="<<(int)ptr[sizeof(float)]<<std::endl;
				//std::cout<<"raw pixel @ ("<<row<<","<<col<<"): val="<<val<<"  c="<<(int)color<<std::endl;
			//}
#endif
#ifdef PF_USE_DCRAW_RT
      unsigned char color = (unsigned char)FC(row,col);
      float val = (data[row][col]-c_black[color])*65535/(this->get_white(color)-c_black[color]);
#endif

      // Fill raw histogram
      int hist_id = -1;
      int ch = -1;
      if( ch < 3 ) ch = color;
      else if( ch == 3 ) ch = 1;
      if( ch >= 0 ) hist_id = static_cast<int>( val*3 + ch );
      if( hist_id >= 0 ) {
        //std::cout<<"hist_id="<<hist_id
        raw_hist[hist_id] += 1;
      }

			fptr = (float*)ptr;
      //*fptr = val;
      //ptr[sizeof(float)] = color;
      fptr[0] = val;
      fptr[1] = color;
			ptr += pxsize;
    }
		/**/
    if( write( temp_fd, rowbuf, pxsize*iwidth ) != (pxsize*iwidth) )
      break;
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

  //std::cout<<"iwidth="<<iwidth<<std::endl
	//   <<"iheight="<<iheight<<std::endl
	//   <<"row="<<row<<"  sizeof(PF::raw_pixel_t)="<<sizeof(PF::raw_pixel_t)<<std::endl;
  //==================================================================
  // Load the raw file into a vips image
  VipsImage* in;
  if( vips_rawload( fname, &in, iwidth, 
										iheight, pxsize, NULL ) )
    return;
  //unlink( fname );
  
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_MULTIBAND;
  //VipsBandFormat format = VIPS_FORMAT_UCHAR;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  int nbands = 2;
  vips_copy( in, &image, 
	     "format", format,
	     "bands", nbands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  g_object_unref( in );
  
  
  //==================================================================
  // Save the raw histogram into the image
  vips_image_set_blob( image, "raw-hist",
           (VipsCallbackFn) g_free, raw_hist,
           sizeof(int)*65535*3 );

  // Save the LibRaw parameters into the image
  void* rawdata_buf = malloc( sizeof(dcraw_data_t) );
  if( !rawdata_buf ) return;
  memcpy( rawdata_buf, pdata, sizeof(dcraw_data_t) );
  vips_image_set_blob( image, "raw_image_data",
           (VipsCallbackFn) g_free, rawdata_buf,
           sizeof(dcraw_data_t) );

  // We read the EXIF data and store it in the image as a custom blob
  GExiv2Metadata* gexiv2_buf = gexiv2_metadata_new();
  gboolean gexiv2_success = gexiv2_metadata_open_path(gexiv2_buf, file_name.c_str(), NULL);
  if( gexiv2_success )
    std::cout<<"RawImage::RawImage(): setting gexiv2-data blob"<<std::endl;
    vips_image_set_blob( image, "gexiv2-data",
        (VipsCallbackFn) gexiv2_metadata_free, gexiv2_buf,
        sizeof(GExiv2Metadata) );

  PF::exif_read( &exif_data, file_name.c_str() );

  void* exifdata_buf = malloc( sizeof(exif_data_t) );
  if( !exifdata_buf ) return;
  memcpy( exifdata_buf, &exif_data, sizeof(exif_data_t) );
  vips_image_set_blob( image, PF_META_EXIF_NAME,
           (VipsCallbackFn) PF::exif_free, exifdata_buf,
           sizeof(exif_data_t) );

  //print_exif();

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
  memcpy( buf2, pdata, sizeof(dcraw_data_t) );
  dcraw_data_t* image_data = (dcraw_data_t*)buf2;
  std::cout<<"RawImage: WB = "<<image_data->color.cam_mul[0]<<" "<<image_data->color.cam_mul[1]<<" "<<image_data->color.cam_mul[2]<<std::endl;
  vips_image_set_blob( demo_image, "raw_image_data",
		       (VipsCallbackFn) g_free, buf2, 
		       sizeof(dcraw_data_t) );
  /**/
  buf2 = malloc( sizeof(exif_data_t) );
  if( !buf2 ) return;
  memcpy( buf2, &exif_data, sizeof(exif_data_t) );
  vips_image_set_blob( demo_image, PF_META_EXIF_NAME,
           (VipsCallbackFn) PF::exif_free, buf2,
           sizeof(exif_data_t) );
/**/
  pyramid.init( demo_image );

#ifdef PF_USE_LIBRAW
  delete raw_loader;
  //return;
#endif
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


VipsImage* PF::RawImage::get_image(unsigned int& level)
{
  if( level == 0 ) {
    GType type = vips_image_get_typeof(image, PF_META_EXIF_NAME );
    if( type ) {
      //std::cout<<"RawImage::get_image(): exif_custom_data found in image("<<image<<")"<<std::endl;
      //print_exif();
    } else std::cout<<"RawImage::get_image(): exif_custom_data not found in image("<<image<<")"<<std::endl;
    /*
#ifdef DO_WARNINGS
#warning "RawImage::get_image(): refreshing of exif metadata needed. This is not normal!"
#endif
    void* buf = malloc( sizeof(exif_data_t) );
    if( !buf ) return NULL;
    memcpy( buf, &exif_data, sizeof(exif_data_t) );
    vips_image_set_blob( image, PF_META_EXIF_NAME,
             (VipsCallbackFn) PF::exif_free, buf,
             sizeof(exif_data_t) );
     */
    PF_REF( image, "RawImage()::get_image(): level 0 ref");
    return image;
  }

  PF::PyramidLevel* plevel = pyramid.get_level( level );
  if( plevel ) {
    return plevel->image;
  }
  return NULL;
}


void PF::RawImage::print_exif(  PF::exif_data_t* data )
{
  std::cout<<"RawImage: (data)"<<std::endl
        <<"      camera maker: "<<data->exif_maker<<std::endl
        <<"      model: "<<data->exif_model<<std::endl
        <<"      lens: "<<data->exif_lens<<std::endl;
}

void PF::RawImage::print_exif()
{
  std::cout<<"RawImage:"<<std::endl
      <<"      camera maker: "<<exif_data.exif_maker<<std::endl
      <<"      model: "<<exif_data.exif_model<<std::endl
      <<"      lens: "<<exif_data.exif_lens<<std::endl;
  size_t bufsz;
  PF::exif_data_t* buf;
  if( !vips_image_get_blob( image, PF_META_EXIF_NAME,
      (void**)&buf,&bufsz ) ) {
    if( bufsz == sizeof(PF::exif_data_t) ) {
      std::cout<<"RawImage: (embedded)"<<std::endl
          <<"      camera maker: "<<buf->exif_maker<<std::endl
          <<"      model: "<<buf->exif_model<<std::endl
          <<"      lens: "<<buf->exif_lens<<std::endl;
    } else {
      std::cout<<"RawImage: wrong exif_custom_data size in image("<<image<<") before set_blob"<<std::endl;
    }
  } else {
    std::cout<<"RawImage: exif_custom_data not found in image("<<image<<") before set_blob"<<std::endl;
  }
}


std::map<Glib::ustring, PF::RawImage*> PF::raw_images;


