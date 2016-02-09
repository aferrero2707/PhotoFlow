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

#include <stdint.h>

typedef uint32_t uint32;

#include <gexiv2/gexiv2-metadata.h>

#include "../base/pf_mkstemp.hh"
#include "../base/rawmatrix.hh"

#include "../rt/rtengine/camconst.h"
#include "raw_image.hh"

// define this function, it is only declared in rawspeed:
int rawspeed_get_number_of_processor_cores()
{
#ifdef _OPENMP
  return omp_get_num_procs();
#else
  return 1;
#endif
}



PF::RawImage::RawImage( const std::string _fname ):
	nref(1), file_name( _fname ),
  image( NULL ), demo_image( NULL )
{
	dcraw_data_t* pdata;
  file_name_real = file_name;
  int ifd = open( file_name_real.c_str(), O_RDONLY );
  if( ifd < 0 ) {
    char* fullpath = strdup( file_name_real.c_str() );
    const gchar* fname = g_basename( fullpath );
    ifd = open( fname, O_RDONLY );
    if( ifd < 0 ) {
      std::cout<<"RawImage::RawImage(): \""<<file_name<<"\" not found"<<std::endl;
      return;
    } else {
      close(ifd);
    }
    file_name_real = fname;
  } else {
    close(ifd);
  }

  int iwidth = 0, iheight = 0, crop_x = 0, crop_y = 0;

#ifdef PF_USE_LIBRAW
  LibRaw* raw_loader = new LibRaw();
  int result = raw_loader->open_file( file_name_real.c_str() );
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

  iwidth = raw_loader->imgdata.sizes.iwidth;
	iheight = raw_loader->imgdata.sizes.iheight;
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
  
#ifdef PF_USE_RAWSPEED
	std::string camfile = PF::PhotoFlow::Instance().get_data_dir() + "/rawspeed/cameras.xml";
  meta = new RawSpeed::CameraMetaData( camfile.c_str() );

	//for(int i = 0; i < 4; i++)
	//	for(int j = 0; j < 3; j++)
	//		dcraw_data.color.cam_xyz[i][j] = get_cam_xyz(i,j);
	pdata = &dcraw_data;

#ifdef __WIN32__
  const size_t len = strlen(file_name_real.c_str()) + 1;
  wchar_t filen[len];
  mbstowcs(filen, file_name_real.c_str(), len);
#else
  char filen[PATH_MAX] = { 0 };
  snprintf(filen, sizeof(filen), "%s", file_name_real.c_str());
#endif
  RawSpeed::FileReader f(filen);

#ifdef __APPLE__
  std::auto_ptr<RawSpeed::RawDecoder> d;
  std::auto_ptr<RawSpeed::FileMap> m;
#else
  std::unique_ptr<RawSpeed::RawDecoder> d;
  std::unique_ptr<RawSpeed::FileMap> m;
#endif

  try
  {
#ifdef __APPLE__
    m = auto_ptr<RawSpeed::FileMap>(f.readFile());
#else
    m = unique_ptr<RawSpeed::FileMap>(f.readFile());
#endif

    RawSpeed::RawParser t(m.get());
#ifdef __APPLE__
    d = auto_ptr<RawSpeed::RawDecoder>(t.getDecoder(meta));
#else
    d = unique_ptr<RawSpeed::RawDecoder>(t.getDecoder(meta));
#endif

    if(!d.get()) return ;

    d->failOnUnknown = true;
    d->checkSupport(meta);
    d->decodeRaw();
    d->decodeMetaData(meta);
    RawSpeed::RawImage r = d->mRaw;

    for (uint32 i=0; i<r->errors.size(); i++)
      fprintf(stderr, "[rawspeed] %s\n", r->errors[i]);

    pdata->idata.filters = r->cfa.getDcrawFilter();

    pdata->color.black = r->blackLevel;
    pdata->color.maximum = r->whitePoint;

    if(r->blackLevelSeparate[0] == -1 || r->blackLevelSeparate[1] == -1 || r->blackLevelSeparate[2] == -1
       || r->blackLevelSeparate[3] == -1)
    {
      r->calculateBlackAreas();
    }

    for(uint8_t i = 0; i < 4; i++) pdata->color.cblack[i] = r->blackLevelSeparate[i];

    if(r->blackLevel == -1)
    {
      float black = 0.0f;
      for(uint8_t i = 0; i < 4; i++)
      {
        black += pdata->color.cblack[i];
      }
      black /= 4.0f;

      pdata->color.black = CLAMP(black, 0, UINT16_MAX);
    }

    /*
     * FIXME
     * if(r->whitePoint == 65536)
     *   ???
     */

    // Grab the WB
    for(int i = 0; i < 3; i++) pdata->color.cam_mul[i] = r->metadata.wbCoeffs[i];
    pdata->color.cam_mul[3] = pdata->color.cam_mul[1];
    std::cout<<"RawSpeed camera WB multipliers: "<<pdata->color.cam_mul[0]<<" "<<pdata->color.cam_mul[1]
             <<" "<<pdata->color.cam_mul[2]<<" "<<pdata->color.cam_mul[3]<<std::endl;
    std::cout<<"RawSpeed black="<<pdata->color.black<<"  white="<<pdata->color.maximum<<std::endl;

    /*
    img->filters = 0u;
    if(!r->isCFA)
    {
      dt_imageio_retval_t ret = dt_imageio_open_rawspeed_sraw(img, r, mbuf);
      return ret;
    }

    img->bpp = r->getBpp();
    img->filters = r->cfa.getDcrawFilter();
    if(img->filters)
    {
      img->flags &= ~DT_IMAGE_LDR;
      img->flags |= DT_IMAGE_RAW;
      if(r->getDataType() == TYPE_FLOAT32) img->flags |= DT_IMAGE_HDR;
      // special handling for x-trans sensors
      if(img->filters == 9u)
      {
        // get 6x6 CFA offset from top left of cropped image
        // NOTE: This is different from how things are done with Bayer
        // sensors. For these, the CFA in cameras.xml is pre-offset
        // depending on the distance modulo 2 between raw and usable
        // image data. For X-Trans, the CFA in cameras.xml is
        // (currently) aligned with the top left of the raw data, and
        // hence it is shifted here to align with the top left of the
        // cropped image.
        iPoint2D tl_margin = r->getCropOffset();
        for(int i = 0; i < 6; ++i)
          for(int j = 0; j < 6; ++j)
          {
            img->xtrans_uncropped[j][i] = r->cfa.getColorAt(i % 6, j % 6);
            img->xtrans[j][i] = r->cfa.getColorAt((i + tl_margin.x) % 6, (j + tl_margin.y) % 6);
          }
      }
    }
    */

    // dimensions of uncropped image
    RawSpeed::iPoint2D dimUncropped = r->getUncroppedDim();
    //iwidth = dimUncropped.x;
    //iheight = dimUncropped.y;

    pdata->sizes.raw_width = dimUncropped.x;
    pdata->sizes.raw_height = dimUncropped.y;

    // dimensions of cropped image
    RawSpeed::iPoint2D dimCropped = r->dim;
    iwidth = dimCropped.x;
    iheight = dimCropped.y;

    pdata->sizes.width = iwidth;
    pdata->sizes.height = iheight;

    pdata->sizes.flip = 0;

    // crop - Top,Left corner
    RawSpeed::iPoint2D cropTL = r->getCropOffset();
    crop_x = cropTL.x;
    crop_y = cropTL.y;

    pdata->sizes.left_margin = crop_x;
    pdata->sizes.top_margin = crop_y;

    // crop - Bottom,Right corner
    RawSpeed::iPoint2D cropBR = dimUncropped - dimCropped - cropTL;

    std::cout<<"original width: "<<dimUncropped.x<<"  crop offset: "<<cropTL.x<<"  cropped width: "<<dimCropped.x<<std::endl;

    /*
    img->fuji_rotation_pos = r->metadata.fujiRotationPos;
    img->pixel_aspect_ratio = (float)r->metadata.pixelAspectRatio;

    void *buf = dt_mipmap_cache_alloc(mbuf, img);
    if(!buf) return DT_IMAGEIO_CACHE_FULL;
    */
    /*
     * since we do not want to crop black borders at this stage,
     * and we do not want to rotate image, we can just use memcpy,
     * as it is faster than dt_imageio_flip_buffers, but only if
     * buffer sizes are equal,
     * (from Klaus: r->pitch may differ from DT pitch (line to line spacing))
     * else fallback to generic dt_imageio_flip_buffers()
     */
    /*
    const size_t bufSize_mipmap = (size_t)img->width * img->height * img->bpp;
    const size_t bufSize_rawspeed = (size_t)r->pitch * dimUncropped.y;
    if(bufSize_mipmap == bufSize_rawspeed)
    {
      memcpy(buf, r->getDataUncropped(0, 0), bufSize_mipmap);
    }
    else
    {
      dt_imageio_flip_buffers((char *)buf, (char *)r->getDataUncropped(0, 0), r->getBpp(), dimUncropped.x,
                              dimUncropped.y, dimUncropped.x, dimUncropped.y, r->pitch, ORIENTATION_NONE);
    }
    */
  }

  catch(const std::exception &exc)
  {
    printf("[rawspeed] %s\n", exc.what());

    /* if an exception is raised lets not retry or handle the
     specific ones, consider the file as corrupted */
    return ;
  }
  catch(...)
  {
    printf("Unhandled exception in imageio_rawspeed\n");
    return ;
  }
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
  int* raw_hist = (int*)malloc( 65536*3*sizeof(int) );
  memset( raw_hist, 0, 65536*3*sizeof(int) );

  rawData.Init( iwidth, iheight, 0, 0 );

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
#ifdef PF_USE_RAWSPEED
      int col2 = col + crop_x;
      int row2 = row + crop_y;
			RawSpeed::RawImage r = d->mRaw;
      unsigned char color = r->cfa.getColorAt(col2,row2);
      float val = 0;
      switch(r->getDataType()) {
      case RawSpeed::TYPE_USHORT16: val = *((uint16_t*)r->getDataUncropped(col2,row2)); break;
      case RawSpeed::TYPE_FLOAT32: val = *((float*)r->getDataUncropped(col2,row2)); break;
      }
      //if( row<8 && col<8 ) {
      //  std::cout<<"raw pixel @ ("<<row<<","<<col<<"): val="<<val<<"  c="<<(int)color<<"  max="<<pdata->color.maximum<<std::endl;
      //}
      val -= pdata->color.black;
      val /= (pdata->color.maximum - pdata->color.black);
      val *= 65535;
      //if( row<8 && col<8 ) {
      //  std::cout<<"raw pixel @ ("<<row<<","<<col<<"): val="<<val<<"  c="<<(int)color<<" (scaled)"<<std::endl;
      //}

#endif

      // Fill raw histogram
      int hist_id = -1;
      int ch = -1;
      if( ch < 3 ) ch = color;
      else if( ch == 3 ) ch = 1;
      if( ch >= 0 ) hist_id = static_cast<int>( val*3 + ch );
      if( hist_id >= 65536*3 ) hist_id = 65536*3-1;
      if( hist_id >= 0 ) {
        //std::cout<<"hist_id="<<hist_id
        raw_hist[hist_id] += 1;
      }

			fptr = (float*)ptr;
      //*fptr = val;
      //ptr[sizeof(float)] = color;
      fptr[0] = val;
      fptr[1] = color;
      //std::cout<<"val="<<val<<"  color="<<color<<std::endl;
			ptr += pxsize;
			rawData[row][col] = val;
    }
		/**/
    if( write( temp_fd, rowbuf, pxsize*iwidth ) != (pxsize*iwidth) )
      break;
#ifndef NDEBUG
    if( (row%100) == 0 ) std::cout<<"  row "<<row<<" saved."<<std::endl;
#ifdef PF_USE_RAWSPEED
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

#ifdef PF_USE_RAWSPEED
  /* free auto pointers on spot */
  d.reset();
  m.reset();
#endif

  std::cout<<"Starting CA correction..."<<std::endl;
  CA_correct_RT();
  std::cout<<"... CA correction finished"<<std::endl;
  memcpy( pdata->color.ca_fitparams, fitparams, sizeof(fitparams) );
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 2; j++) {
      printf("i=%d j=%d par:",i,j);
      for(int k = 0; k < 16; k++)
        printf(" %f",fitparams[i][j][k]);
      printf("\n");
    }
  }
  //getchar();


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

  // We read the EXIF data and store it in the image as a custom blob
  GExiv2Metadata* gexiv2_buf = gexiv2_metadata_new();
  gboolean gexiv2_success = gexiv2_metadata_open_path(gexiv2_buf, file_name_real.c_str(), NULL);
  if( gexiv2_success ) {
    GExiv2Orientation orientation = gexiv2_metadata_get_orientation( gexiv2_buf );
    switch( orientation ) {
    case GEXIV2_ORIENTATION_ROT_90:
      pdata->sizes.flip = 6; break;
    case GEXIV2_ORIENTATION_ROT_180:
      pdata->sizes.flip = 3; break;
    case GEXIV2_ORIENTATION_ROT_270:
      pdata->sizes.flip = 5; break;
    default:
      break;
    }
    //std::cout<<"RawImage::RawImage(): setting gexiv2-data blob"<<std::endl;
    gexiv2_metadata_set_orientation( gexiv2_buf, GEXIV2_ORIENTATION_NORMAL );
    vips_image_set_blob( image, "gexiv2-data",
        (VipsCallbackFn) gexiv2_metadata_free, gexiv2_buf,
        sizeof(GExiv2Metadata) );
  }


  PF::exif_read( &exif_data, file_name_real.c_str() );
  void* exifdata_buf = malloc( sizeof(exif_data_t) );
  if( !exifdata_buf ) return;
  memcpy( exifdata_buf, &exif_data, sizeof(exif_data_t) );
  vips_image_set_blob( image, PF_META_EXIF_NAME,
           (VipsCallbackFn) PF::exif_free, exifdata_buf,
           sizeof(exif_data_t) );


  // Save the LibRaw parameters into the image
  void* rawdata_buf = malloc( sizeof(dcraw_data_t) );
  if( !rawdata_buf ) return;
  memcpy( rawdata_buf, pdata, sizeof(dcraw_data_t) );
  vips_image_set_blob( image, "raw_image_data",
           (VipsCallbackFn) g_free, rawdata_buf,
           sizeof(dcraw_data_t) );

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



////////////////////////////////////////////////////////////////
//
//      Chromatic Aberration Auto-correction
//
//      copyright (c) 2008-2010  Emil Martinec <ejmartin@uchicago.edu>
//
//
// code dated: November 26, 2010
//
//  CA_correct_RT.cc is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

using namespace std;
using namespace rtengine;



int PF::RawImage::LinEqSolve(int nDim, double* pfMatr, double* pfVect, double* pfSolution)
{
//==============================================================================
// return 1 if system not solving, 0 if system solved
// nDim - system dimension
// pfMatr - matrix with coefficients
// pfVect - vector with free members
// pfSolution - vector with system solution
// pfMatr becames trianglular after function call
// pfVect changes after function call
//
// Developer: Henry Guennadi Levkin
//
//==============================================================================

    double fMaxElem;
    double fAcc;

    int i, j, k, m;

    for(k = 0; k < (nDim - 1); k++) { // base row of matrix
        // search of line with max element
        fMaxElem = fabsf( pfMatr[k * nDim + k] );
        m = k;

        for (i = k + 1; i < nDim; i++) {
            if(fMaxElem < fabsf(pfMatr[i * nDim + k]) ) {
                fMaxElem = pfMatr[i * nDim + k];
                m = i;
            }
        }

        // permutation of base line (index k) and max element line(index m)
        if(m != k) {
            for(i = k; i < nDim; i++) {
                fAcc               = pfMatr[k * nDim + i];
                pfMatr[k * nDim + i] = pfMatr[m * nDim + i];
                pfMatr[m * nDim + i] = fAcc;
            }

            fAcc = pfVect[k];
            pfVect[k] = pfVect[m];
            pfVect[m] = fAcc;
        }

        if( pfMatr[k * nDim + k] == 0.) {
            //linear system has no solution
            return 1; // needs improvement !!!
        }

        // triangulation of matrix with coefficients
        for(j = (k + 1); j < nDim; j++) { // current row of matrix
            fAcc = - pfMatr[j * nDim + k] / pfMatr[k * nDim + k];

            for(i = k; i < nDim; i++) {
                pfMatr[j * nDim + i] = pfMatr[j * nDim + i] + fAcc * pfMatr[k * nDim + i];
            }

            pfVect[j] = pfVect[j] + fAcc * pfVect[k]; // free member recalculation
        }
    }

    for(k = (nDim - 1); k >= 0; k--) {
        pfSolution[k] = pfVect[k];

        for(i = (k + 1); i < nDim; i++) {
            pfSolution[k] -= (pfMatr[k * nDim + i] * pfSolution[i]);
        }

        pfSolution[k] = pfSolution[k] / pfMatr[k * nDim + k];
    }

    return 0;
}
//end of linear equation solver
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


void PF::RawImage::CA_correct_RT()
{
// multithreaded by Ingo Weyrich
#define TS 128      // Tile size
#define TSH 64      // Half Tile size
#define PIX_SORT(a,b) { if ((a)>(b)) {temp=(a);(a)=(b);(b)=temp;} }

    // Test for RGB cfa
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            if(FC(i, j) == 3) {
                printf("CA correction supports only RGB Color filter arrays\n");
                return;
            }

    volatile double progress = 0.0;
    //float mult[4] = {4.003360, 1.970959, 2.677305, 1.970959};
    float mult[4] = {dcraw_data.color.cam_mul[0], dcraw_data.color.cam_mul[1],
        dcraw_data.color.cam_mul[2], dcraw_data.color.cam_mul[3]};
    float max_mult = 0; for(int i=0;i<4;i++) if(max_mult<mult[i]) max_mult=mult[i];
    for(int i=0;i<4;i++) mult[i] /= max_mult;

    //if(plistener) {
    //    plistener->setProgress (progress);
    //}

    bool autoCA = true;
    // local variables
    int width = dcraw_data.sizes.width, height = dcraw_data.sizes.height;
    //temporary array to store simple interpolation of G
    float (*Gtmp);
    Gtmp = (float (*)) calloc ((height) * (width), sizeof * Gtmp);

    // temporary array to avoid race conflicts, only every second pixel needs to be saved here
    float (*RawDataTmp);
    RawDataTmp = (float*) malloc( height * width * sizeof(float) / 2);

    float   blockave[2][3] = {{0, 0, 0}, {0, 0, 0}}, blocksqave[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockdenom[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockvar[2][3];

    // Because we can't break parallel processing, we need a switch do handle the errors
    bool processpasstwo = true;

    //block CA shift values and weight assigned to block
    char        *buffer1;               // vblsz*hblsz*(3*2+1)
    float       (*blockwt);             // vblsz*hblsz
    float       (*blockshifts)[3][2];   // vblsz*hblsz*3*2


    const int border = 8;
    const int border2 = 16;

    int vz1, hz1;

    if((height + border2) % (TS - border2) == 0) {
        vz1 = 1;
    } else {
        vz1 = 0;
    }

    if((width + border2) % (TS - border2) == 0) {
        hz1 = 1;
    } else {
        hz1 = 0;
    }

    int vblsz, hblsz;
    vblsz = ceil((float)(height + border2) / (TS - border2) + 2 + vz1);
    hblsz = ceil((float)(width + border2) / (TS - border2) + 2 + hz1);

    buffer1 = (char *) malloc(vblsz * hblsz * (3 * 2 + 1) * sizeof(float));
    //merror(buffer1,"CA_correct()");
    memset(buffer1, 0, vblsz * hblsz * (3 * 2 + 1)*sizeof(float));
    // block CA shifts
    blockwt     = (float (*))           (buffer1);
    blockshifts = (float (*)[3][2])     (buffer1 + (vblsz * hblsz * sizeof(float)));

    //double fitparams[3][2][16];
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 2; j++) {
        for(int k = 0; k < 16; k++)
          fitparams[i][j][k] = 0;
      }
    }



    //order of 2d polynomial fit (polyord), and numpar=polyord^2
    int polyord = 4, numpar = 16;

    //#pragma omp parallel shared(Gtmp,width,height,blockave,blocksqave,blockdenom,blockvar,blockwt,blockshifts,fitparams,polyord,numpar)
    {
        int progresscounter = 0;

        int rrmin, rrmax, ccmin, ccmax;
        int top, left, row, col;
        int rr, cc, c, indx, indx1, i, j, k, m, n, dir;
        //number of pixels in a tile contributing to the CA shift diagnostic
        int areawt[2][3];
        //direction of the CA shift in a tile
        int GRBdir[2][3];
        //offset data of the plaquette where the optical R/B data are sampled
        int offset[2][3];
        int shifthfloor[3], shiftvfloor[3], shifthceil[3], shiftvceil[3];
        //number of tiles in the image
        int vblock, hblock;
        //int verbose=1;
        //flag indicating success or failure of polynomial fit
        int res;
        //shifts to location of vertical and diagonal neighbors
        const int v1 = TS, v2 = 2 * TS, v3 = 3 * TS, v4 = 4 * TS; //, p1=-TS+1, p2=-2*TS+2, p3=-3*TS+3, m1=TS+1, m2=2*TS+2, m3=3*TS+3;

        float eps = 1e-5f, eps2 = 1e-10f; //tolerance to avoid dividing by zero

        //adaptive weights for green interpolation
        float   wtu, wtd, wtl, wtr;
        //local quadratic fit to shift data within a tile
        float   coeff[2][3][3];
        //measured CA shift parameters for a tile
        float   CAshift[2][3];
        //polynomial fit coefficients
        //residual CA shift amount within a plaquette
        float   shifthfrac[3], shiftvfrac[3];
        //temporary storage for median filter
        float   temp, p[9];
        //temporary parameters for tile CA evaluation
        float   gdiff, deltgrb;
        //interpolated G at edge of plaquette
        float   Ginthfloor, Ginthceil, Gint, RBint, gradwt;
        //interpolated color difference at edge of plaquette
        float   grbdiffinthfloor, grbdiffinthceil, grbdiffint, grbdiffold;
        //per thread data for evaluation of block CA shift variance
        float   blockavethr[2][3] = {{0, 0, 0}, {0, 0, 0}}, blocksqavethr[2][3] = {{0, 0, 0}, {0, 0, 0}}, blockdenomthr[2][3] = {{0, 0, 0}, {0, 0, 0}}; //, blockvarthr[2][3];

        //low and high pass 1D filters of G in vertical/horizontal directions
        float   glpfh, glpfv;

        //max allowed CA shift
        const float bslim = 3.99;
        //gaussians for low pass filtering of G and R/B
        //static const float gaussg[5] = {0.171582, 0.15839, 0.124594, 0.083518, 0.0477063};//sig=2.5
        //static const float gaussrb[3] = {0.332406, 0.241376, 0.0924212};//sig=1.25

        //block CA shift values and weight assigned to block

        char        *buffer;            // TS*TS*16
        //rgb data in a tile
        float* rgb[3];
        //color differences
        float         (*grbdiff);       // TS*TS*4
        //green interpolated to optical sample points for R/B
        float         (*gshift);        // TS*TS*4
        //high pass filter for R/B in vertical direction
        float         (*rbhpfh);        // TS*TS*4
        //high pass filter for R/B in horizontal direction
        float         (*rbhpfv);        // TS*TS*4
        //low pass filter for R/B in horizontal direction
        float         (*rblpfh);        // TS*TS*4
        //low pass filter for R/B in vertical direction
        float         (*rblpfv);        // TS*TS*4
        //low pass filter for color differences in horizontal direction
        float         (*grblpfh);       // TS*TS*4
        //low pass filter for color differences in vertical direction
        float         (*grblpfv);       // TS*TS*4


        // assign working space; this would not be necessary
        // if the algorithm is part of the larger pre-interpolation processing
        buffer = (char *) malloc(3 * sizeof(float) * TS * TS + 8 * sizeof(float) * TS * TSH + 10 * 64 + 64);
        //merror(buffer,"CA_correct()");
        memset(buffer, 0, 3 * sizeof(float)*TS * TS + 8 * sizeof(float)*TS * TSH + 10 * 64 + 64);

        char    *data;
        data    = buffer;

//  buffers aligned to size of cacheline
//  data = (char*)( ( uintptr_t(buffer) + uintptr_t(63)) / 64 * 64);


        // shift the beginning of all arrays but the first by 64 bytes to avoid cache miss conflicts on CPUs which have <=4-way associative L1-Cache
        rgb[0]      = (float (*))       data;
        rgb[1]      = (float (*))       (data + 1 * sizeof(float) * TS * TS + 1 * 64);
        rgb[2]      = (float (*))       (data + 2 * sizeof(float) * TS * TS + 2 * 64);
        grbdiff     = (float (*))       (data + 3 * sizeof(float) * TS * TS + 3 * 64);
        gshift      = (float (*))       (data + 3 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 4 * 64);
        rbhpfh      = (float (*))       (data + 4 * sizeof(float) * TS * TS + 5 * 64);
        rbhpfv      = (float (*))       (data + 4 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 6 * 64);
        rblpfh      = (float (*))       (data + 5 * sizeof(float) * TS * TS + 7 * 64);
        rblpfv      = (float (*))       (data + 5 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 8 * 64);
        grblpfh     = (float (*))       (data + 6 * sizeof(float) * TS * TS + 9 * 64);
        grblpfv     = (float (*))       (data + 6 * sizeof(float) * TS * TS + sizeof(float) * TS * TSH + 10 * 64);


        if (autoCA) {
          //printf("width=%d  height=%d filters=%d\n",width,height,dcraw_data.idata.filters);
          std::cout<<"width="<<width<<"  height="<<height<<"  filters="<<dcraw_data.idata.filters<<std::endl;
            // Main algorithm: Tile loop
            //#pragma omp for collapse(2) schedule(dynamic) nowait
            for (top = -border ; top < height; top += TS - border2)
                for (left = -border; left < width; left += TS - border2) {
                    vblock = ((top + border) / (TS - border2)) + 1;
                    hblock = ((left + border) / (TS - border2)) + 1;
                    int bottom = min(top + TS, height + border);
                    int right  = min(left + TS, width + border);
                    int rr1 = bottom - top;
                    int cc1 = right - left;

                    //t1_init = clock();
                    if (top < 0) {
                        rrmin = border;
                    } else {
                        rrmin = 0;
                    }

                    if (left < 0) {
                        ccmin = border;
                    } else {
                        ccmin = 0;
                    }

                    if (bottom > height) {
                        rrmax = height - top;
                    } else {
                        rrmax = rr1;
                    }

                    if (right > width) {
                        ccmax = width - left;
                    } else {
                        ccmax = cc1;
                    }

                    // rgb from input CFA data
                    // rgb values should be floating point number between 0 and 1
                    // after white balance multipliers are applied

                    for (rr = rrmin; rr < rrmax; rr++)
                        for (row = rr + top, cc = ccmin; cc < ccmax; cc++) {
                            col = cc + left;
                            c = FC(rr, cc);
                            indx = row * width + col;
                            indx1 = rr * TS + cc;
                            //rgb[c][indx1] = (rawData[row][col]) / 65535.0f;
                            rgb[c][indx1] = (rawData[row][col]) * mult[c] / 65535.0f;
                            //rgb[indx1][c] = image[indx][c]/65535.0f;//for dcraw implementation
			    if(row<4 && col<4) printf("rawData[%d][%d](%d) = %f * %f = %f\n",row,col,c,(rawData[row][col]), mult[c], (rawData[row][col]) * mult[c]);
                        }

                    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    //fill borders
                    if (rrmin > 0) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = ccmin; cc < ccmax; cc++) {
                                c = FC(rr, cc);
                                rgb[c][rr * TS + cc] = rgb[c][(border2 - rr) * TS + cc];
                            }
                    }

                    if (rrmax < rr1) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = ccmin; cc < ccmax; cc++) {
                                c = FC(rr, cc);
                                rgb[c][(rrmax + rr)*TS + cc] = (rawData[(height - rr - 2)][left + cc]) / 65535.0f;
                                //rgb[(rrmax+rr)*TS+cc][c] = (image[(height-rr-2)*width+left+cc][c])/65535.0f;//for dcraw implementation
                            }
                    }

                    if (ccmin > 0) {
                        for (rr = rrmin; rr < rrmax; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][rr * TS + cc] = rgb[c][rr * TS + border2 - cc];
                            }
                    }

                    if (ccmax < cc1) {
                        for (rr = rrmin; rr < rrmax; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][rr * TS + ccmax + cc] = (rawData[(top + rr)][(width - cc - 2)]) / 65535.0f;
                                //rgb[rr*TS+ccmax+cc][c] = (image[(top+rr)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation
                            }
                    }

                    //also, fill the image corners
                    if (rrmin > 0 && ccmin > 0) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][(rr)*TS + cc] = (rawData[border2 - rr][border2 - cc]) / 65535.0f;
                                //rgb[(rr)*TS+cc][c] = (rgb[(border2-rr)*TS+(border2-cc)][c]);//for dcraw implementation
                            }
                    }

                    if (rrmax < rr1 && ccmax < cc1) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][(rrmax + rr)*TS + ccmax + cc] = (rawData[(height - rr - 2)][(width - cc - 2)]) / 65535.0f;
                                //rgb[(rrmax+rr)*TS+ccmax+cc][c] = (image[(height-rr-2)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation
                            }
                    }

                    if (rrmin > 0 && ccmax < cc1) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][(rr)*TS + ccmax + cc] = (rawData[(border2 - rr)][(width - cc - 2)]) / 65535.0f;
                                //rgb[(rr)*TS+ccmax+cc][c] = (image[(border2-rr)*width+(width-cc-2)][c])/65535.0f;//for dcraw implementation
                            }
                    }

                    if (rrmax < rr1 && ccmin > 0) {
                        for (rr = 0; rr < border; rr++)
                            for (cc = 0; cc < border; cc++) {
                                c = FC(rr, cc);
                                rgb[c][(rrmax + rr)*TS + cc] = (rawData[(height - rr - 2)][(border2 - cc)]) / 65535.0f;
                                //rgb[(rrmax+rr)*TS+cc][c] = (image[(height-rr-2)*width+(border2-cc)][c])/65535.0f;//for dcraw implementation
                            }
                    }

                    //end of border fill
                    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


                    for (j = 0; j < 2; j++)
                        for (k = 0; k < 3; k++)
                            for (c = 0; c < 3; c += 2) {
                                coeff[j][k][c] = 0;
                            }

                    //end of initialization


                    for (rr = 3; rr < rr1 - 3; rr++)
                        for (row = rr + top, cc = 3, indx = rr * TS + cc; cc < cc1 - 3; cc++, indx++) {
                            col = cc + left;
                            c = FC(rr, cc);

                            if (c != 1) {
                                //compute directional weights using image gradients
                                wtu = 1.0 / SQR(eps + fabsf(rgb[1][indx + v1] - rgb[1][indx - v1]) + fabsf(rgb[c][indx] - rgb[c][indx - v2]) + fabsf(rgb[1][indx - v1] - rgb[1][indx - v3]));
                                wtd = 1.0 / SQR(eps + fabsf(rgb[1][indx - v1] - rgb[1][indx + v1]) + fabsf(rgb[c][indx] - rgb[c][indx + v2]) + fabsf(rgb[1][indx + v1] - rgb[1][indx + v3]));
                                wtl = 1.0 / SQR(eps + fabsf(rgb[1][indx + 1] - rgb[1][indx - 1]) + fabsf(rgb[c][indx] - rgb[c][indx - 2]) + fabsf(rgb[1][indx - 1] - rgb[1][indx - 3]));
                                wtr = 1.0 / SQR(eps + fabsf(rgb[1][indx - 1] - rgb[1][indx + 1]) + fabsf(rgb[c][indx] - rgb[c][indx + 2]) + fabsf(rgb[1][indx + 1] - rgb[1][indx + 3]));

                                //store in rgb array the interpolated G value at R/B grid points using directional weighted average
                                rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
                            }

                            if (row > -1 && row < height && col > -1 && col < width) {
                                Gtmp[row * width + col] = rgb[1][indx];
                            }
                        }

                    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                    for (rr = 4; rr < rr1 - 4; rr++)
                        for (cc = 4 + (FC(rr, 2) & 1), indx = rr * TS + cc, c = FC(rr, cc); cc < cc1 - 4; cc += 2, indx += 2) {


                            rbhpfv[indx >> 1] = fabsf(fabsf((rgb[1][indx] - rgb[c][indx]) - (rgb[1][indx + v4] - rgb[c][indx + v4])) +
                                                      fabsf((rgb[1][indx - v4] - rgb[c][indx - v4]) - (rgb[1][indx] - rgb[c][indx])) -
                                                      fabsf((rgb[1][indx - v4] - rgb[c][indx - v4]) - (rgb[1][indx + v4] - rgb[c][indx + v4])));
                            rbhpfh[indx >> 1] = fabsf(fabsf((rgb[1][indx] - rgb[c][indx]) - (rgb[1][indx + 4] - rgb[c][indx + 4])) +
                                                      fabsf((rgb[1][indx - 4] - rgb[c][indx - 4]) - (rgb[1][indx] - rgb[c][indx])) -
                                                      fabsf((rgb[1][indx - 4] - rgb[c][indx - 4]) - (rgb[1][indx + 4] - rgb[c][indx + 4])));

                            /*ghpfv = fabsf(fabsf(rgb[indx][1]-rgb[indx+v4][1])+fabsf(rgb[indx][1]-rgb[indx-v4][1]) -
                             fabsf(rgb[indx+v4][1]-rgb[indx-v4][1]));
                             ghpfh = fabsf(fabsf(rgb[indx][1]-rgb[indx+4][1])+fabsf(rgb[indx][1]-rgb[indx-4][1]) -
                             fabsf(rgb[indx+4][1]-rgb[indx-4][1]));
                             rbhpfv[indx] = fabsf(ghpfv - fabsf(fabsf(rgb[indx][c]-rgb[indx+v4][c])+fabsf(rgb[indx][c]-rgb[indx-v4][c]) -
                             fabsf(rgb[indx+v4][c]-rgb[indx-v4][c])));
                             rbhpfh[indx] = fabsf(ghpfh - fabsf(fabsf(rgb[indx][c]-rgb[indx+4][c])+fabsf(rgb[indx][c]-rgb[indx-4][c]) -
                             fabsf(rgb[indx+4][c]-rgb[indx-4][c])));*/

                            glpfv = 0.25 * (2.0 * rgb[1][indx] + rgb[1][indx + v2] + rgb[1][indx - v2]);
                            glpfh = 0.25 * (2.0 * rgb[1][indx] + rgb[1][indx + 2] + rgb[1][indx - 2]);
                            rblpfv[indx >> 1] = eps + fabsf(glpfv - 0.25 * (2.0 * rgb[c][indx] + rgb[c][indx + v2] + rgb[c][indx - v2]));
                            rblpfh[indx >> 1] = eps + fabsf(glpfh - 0.25 * (2.0 * rgb[c][indx] + rgb[c][indx + 2] + rgb[c][indx - 2]));
                            grblpfv[indx >> 1] = glpfv + 0.25 * (2.0 * rgb[c][indx] + rgb[c][indx + v2] + rgb[c][indx - v2]);
                            grblpfh[indx >> 1] = glpfh + 0.25 * (2.0 * rgb[c][indx] + rgb[c][indx + 2] + rgb[c][indx - 2]);
                        }

                    areawt[0][0] = areawt[1][0] = 1;
                    areawt[0][2] = areawt[1][2] = 1;

                    // along line segments, find the point along each segment that minimizes the color variance
                    // averaged over the tile; evaluate for up/down and left/right away from R/B grid point
                    for (rr = 8; rr < rr1 - 8; rr++)
                        for (cc = 8 + (FC(rr, 2) & 1), indx = rr * TS + cc, c = FC(rr, cc); cc < cc1 - 8; cc += 2, indx += 2) {

//                  areawt[0][c]=areawt[1][c]=0;

                            //in linear interpolation, color differences are a quadratic function of interpolation position;
                            //solve for the interpolation position that minimizes color difference variance over the tile

                            //vertical
                            gdiff = 0.3125 * (rgb[1][indx + TS] - rgb[1][indx - TS]) + 0.09375 * (rgb[1][indx + TS + 1] - rgb[1][indx - TS + 1] + rgb[1][indx + TS - 1] - rgb[1][indx - TS - 1]);
                            deltgrb = (rgb[c][indx] - rgb[1][indx]);

                            gradwt = fabsf(0.25 * rbhpfv[indx >> 1] + 0.125 * (rbhpfv[(indx >> 1) + 1] + rbhpfv[(indx >> 1) - 1]) ) * (grblpfv[(indx >> 1) - v1] + grblpfv[(indx >> 1) + v1]) / (eps + 0.1 * grblpfv[(indx >> 1) - v1] + rblpfv[(indx >> 1) - v1] + 0.1 * grblpfv[(indx >> 1) + v1] + rblpfv[(indx >> 1) + v1]);

                            coeff[0][0][c] += gradwt * deltgrb * deltgrb;
                            coeff[0][1][c] += gradwt * gdiff * deltgrb;
                            coeff[0][2][c] += gradwt * gdiff * gdiff;
//                  areawt[0][c]+=1;

                            //horizontal
                            gdiff = 0.3125 * (rgb[1][indx + 1] - rgb[1][indx - 1]) + 0.09375 * (rgb[1][indx + 1 + TS] - rgb[1][indx - 1 + TS] + rgb[1][indx + 1 - TS] - rgb[1][indx - 1 - TS]);
                            deltgrb = (rgb[c][indx] - rgb[1][indx]);

                            gradwt = fabsf(0.25 * rbhpfh[indx >> 1] + 0.125 * (rbhpfh[(indx >> 1) + v1] + rbhpfh[(indx >> 1) - v1]) ) * (grblpfh[(indx >> 1) - 1] + grblpfh[(indx >> 1) + 1]) / (eps + 0.1 * grblpfh[(indx >> 1) - 1] + rblpfh[(indx >> 1) - 1] + 0.1 * grblpfh[(indx >> 1) + 1] + rblpfh[(indx >> 1) + 1]);

                            coeff[1][0][c] += gradwt * deltgrb * deltgrb;
                            coeff[1][1][c] += gradwt * gdiff * deltgrb;
                            coeff[1][2][c] += gradwt * gdiff * gdiff;
//                  areawt[1][c]+=1;

                            //  In Mathematica,
                            //  f[x_]=Expand[Total[Flatten[
                            //  ((1-x) RotateLeft[Gint,shift1]+x RotateLeft[Gint,shift2]-cfapad)^2[[dv;;-1;;2,dh;;-1;;2]]]]];
                            //  extremum = -.5Coefficient[f[x],x]/Coefficient[f[x],x^2]
                        }

                    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    /*
                    for (rr=4; rr < rr1-4; rr++)
                        for (cc=4+(FC(rr,2)&1), indx=rr*TS+cc, c = FC(rr,cc); cc < cc1-4; cc+=2, indx+=2) {


                            rbhpfv[indx] = SQR(fabs((rgb[indx][1]-rgb[indx][c])-(rgb[indx+v4][1]-rgb[indx+v4][c])) +
                                                fabs((rgb[indx-v4][1]-rgb[indx-v4][c])-(rgb[indx][1]-rgb[indx][c])) -
                                                fabs((rgb[indx-v4][1]-rgb[indx-v4][c])-(rgb[indx+v4][1]-rgb[indx+v4][c])));
                            rbhpfh[indx] = SQR(fabs((rgb[indx][1]-rgb[indx][c])-(rgb[indx+4][1]-rgb[indx+4][c])) +
                                                fabs((rgb[indx-4][1]-rgb[indx-4][c])-(rgb[indx][1]-rgb[indx][c])) -
                                                fabs((rgb[indx-4][1]-rgb[indx-4][c])-(rgb[indx+4][1]-rgb[indx+4][c])));


                            glpfv = 0.25*(2*rgb[indx][1]+rgb[indx+v2][1]+rgb[indx-v2][1]);
                            glpfh = 0.25*(2*rgb[indx][1]+rgb[indx+2][1]+rgb[indx-2][1]);
                            rblpfv[indx] = eps+fabs(glpfv - 0.25*(2*rgb[indx][c]+rgb[indx+v2][c]+rgb[indx-v2][c]));
                            rblpfh[indx] = eps+fabs(glpfh - 0.25*(2*rgb[indx][c]+rgb[indx+2][c]+rgb[indx-2][c]));
                            grblpfv[indx] = glpfv + 0.25*(2*rgb[indx][c]+rgb[indx+v2][c]+rgb[indx-v2][c]);
                            grblpfh[indx] = glpfh + 0.25*(2*rgb[indx][c]+rgb[indx+2][c]+rgb[indx-2][c]);
                        }

                    for (c=0;c<3;c++) {areawt[0][c]=areawt[1][c]=0;}

                    // along line segments, find the point along each segment that minimizes the color variance
                    // averaged over the tile; evaluate for up/down and left/right away from R/B grid point
                    for (rr=rrmin+8; rr < rrmax-8; rr++)
                        for (cc=ccmin+8+(FC(rr,2)&1), indx=rr*TS+cc, c = FC(rr,cc); cc < ccmax-8; cc+=2, indx+=2) {

                            if (rgb[indx][c]>0.8*clip_pt || Gtmp[indx]>0.8*clip_pt) continue;

                            //in linear interpolation, color differences are a quadratic function of interpolation position;
                            //solve for the interpolation position that minimizes color difference variance over the tile

                            //vertical
                            gdiff=0.3125*(rgb[indx+TS][1]-rgb[indx-TS][1])+0.09375*(rgb[indx+TS+1][1]-rgb[indx-TS+1][1]+rgb[indx+TS-1][1]-rgb[indx-TS-1][1]);
                            deltgrb=(rgb[indx][c]-rgb[indx][1])-0.5*((rgb[indx-v4][c]-rgb[indx-v4][1])+(rgb[indx+v4][c]-rgb[indx+v4][1]));

                            gradwt=fabs(0.25*rbhpfv[indx]+0.125*(rbhpfv[indx+2]+rbhpfv[indx-2]) );// *(grblpfv[indx-v2]+grblpfv[indx+v2])/(eps+0.1*grblpfv[indx-v2]+rblpfv[indx-v2]+0.1*grblpfv[indx+v2]+rblpfv[indx+v2]);
                            if (gradwt>eps) {
                            coeff[0][0][c] += gradwt*deltgrb*deltgrb;
                            coeff[0][1][c] += gradwt*gdiff*deltgrb;
                            coeff[0][2][c] += gradwt*gdiff*gdiff;
                            areawt[0][c]++;
                            }

                            //horizontal
                            gdiff=0.3125*(rgb[indx+1][1]-rgb[indx-1][1])+0.09375*(rgb[indx+1+TS][1]-rgb[indx-1+TS][1]+rgb[indx+1-TS][1]-rgb[indx-1-TS][1]);
                            deltgrb=(rgb[indx][c]-rgb[indx][1])-0.5*((rgb[indx-4][c]-rgb[indx-4][1])+(rgb[indx+4][c]-rgb[indx+4][1]));

                            gradwt=fabs(0.25*rbhpfh[indx]+0.125*(rbhpfh[indx+v2]+rbhpfh[indx-v2]) );// *(grblpfh[indx-2]+grblpfh[indx+2])/(eps+0.1*grblpfh[indx-2]+rblpfh[indx-2]+0.1*grblpfh[indx+2]+rblpfh[indx+2]);
                            if (gradwt>eps) {
                            coeff[1][0][c] += gradwt*deltgrb*deltgrb;
                            coeff[1][1][c] += gradwt*gdiff*deltgrb;
                            coeff[1][2][c] += gradwt*gdiff*gdiff;
                            areawt[1][c]++;
                            }

                            //  In Mathematica,
                            //  f[x_]=Expand[Total[Flatten[
                            //  ((1-x) RotateLeft[Gint,shift1]+x RotateLeft[Gint,shift2]-cfapad)^2[[dv;;-1;;2,dh;;-1;;2]]]]];
                            //  extremum = -.5Coefficient[f[x],x]/Coefficient[f[x],x^2]
                        }*/
                    for (c = 0; c < 3; c += 2) {
                        for (j = 0; j < 2; j++) { // vert/hor
                            //printf("hblock %d vblock %d j %d c %d areawt %d \n",hblock,vblock,j,c,areawt[j][c]);
                            //printf("hblock %d vblock %d j %d c %d areawt %d ",hblock,vblock,j,c,areawt[j][c]);

                            if (areawt[j][c] > 0 && coeff[j][2][c] > eps2) {
                                CAshift[j][c] = coeff[j][1][c] / coeff[j][2][c];
                                blockwt[vblock * hblsz + hblock] = areawt[j][c] * coeff[j][2][c] / (eps + coeff[j][0][c]) ;
                            } else {
                                CAshift[j][c] = 17.0;
                                blockwt[vblock * hblsz + hblock] = 0;
                            }

                            //if (c==0 && j==0) printf("vblock= %d hblock= %d denom= %f areawt= %d \n",vblock,hblock,coeff[j][2][c],areawt[j][c]);

                            //printf("%f  \n",CAshift[j][c]);

                            //data structure = CAshift[vert/hor][color]
                            //j=0=vert, 1=hor

                            //offset gives NW corner of square containing the min; j=0=vert, 1=hor

                            if (fabsf(CAshift[j][c]) < 2.0f) {
                                blockavethr[j][c] += CAshift[j][c];
                                blocksqavethr[j][c] += SQR(CAshift[j][c]);
                                blockdenomthr[j][c] += 1;
                            }
                        }//vert/hor
                    }//color

                    // CAshift[j][c] are the locations
                    // that minimize color difference variances;
                    // This is the approximate _optical_ location of the R/B pixels

                    for (c = 0; c < 3; c += 2) {
                        //evaluate the shifts to the location that minimizes CA within the tile
                        blockshifts[(vblock)*hblsz + hblock][c][0] = (CAshift[0][c]); //vert CA shift for R/B
                        blockshifts[(vblock)*hblsz + hblock][c][1] = (CAshift[1][c]); //hor CA shift for R/B
                        //data structure: blockshifts[blocknum][R/B][v/h]
                        //if (c==0) printf("vblock= %d hblock= %d blockshiftsmedian= %f \n",vblock,hblock,blockshifts[(vblock)*hblsz+hblock][c][0]);
                    }

                }

            //end of diagnostic pass
            //#pragma omp critical
            {
                for (j = 0; j < 2; j++)
                    for (c = 0; c < 3; c += 2) {
                        blockdenom[j][c] += blockdenomthr[j][c];
                        blocksqave[j][c] += blocksqavethr[j][c];
                        blockave[j][c]   += blockavethr[j][c];
                    }
            }
            //#pragma omp barrier

            //#pragma omp single
            {
                for (j = 0; j < 2; j++)
                    for (c = 0; c < 3; c += 2) {
                        if (blockdenom[j][c]) {
                            blockvar[j][c] = blocksqave[j][c] / blockdenom[j][c] - SQR(blockave[j][c] / blockdenom[j][c]);
                        } else {
                            processpasstwo = false;
                            printf ("blockdenom vanishes \n");
                            break;
                        }
                    }

                //printf ("tile variances %f %f %f %f \n",blockvar[0][0],blockvar[1][0],blockvar[0][2],blockvar[1][2] );


                // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

                //now prepare for CA correction pass
                //first, fill border blocks of blockshift array
                if(processpasstwo) {
                    for (vblock = 1; vblock < vblsz - 1; vblock++) { //left and right sides
                        for (c = 0; c < 3; c += 2) {
                            for (i = 0; i < 2; i++) {
                                blockshifts[vblock * hblsz][c][i] = blockshifts[(vblock) * hblsz + 2][c][i];
                                blockshifts[vblock * hblsz + hblsz - 1][c][i] = blockshifts[(vblock) * hblsz + hblsz - 3][c][i];
                            }
                        }
                    }

                    for (hblock = 0; hblock < hblsz; hblock++) { //top and bottom sides
                        for (c = 0; c < 3; c += 2) {
                            for (i = 0; i < 2; i++) {
                                blockshifts[hblock][c][i] = blockshifts[2 * hblsz + hblock][c][i];
                                blockshifts[(vblsz - 1)*hblsz + hblock][c][i] = blockshifts[(vblsz - 3) * hblsz + hblock][c][i];
                            }
                        }
                    }

                    //end of filling border pixels of blockshift array

                    //initialize fit arrays
                    double  polymat[3][2][256], shiftmat[3][2][16];

                    for (i = 0; i < 256; i++) {
                        polymat[0][0][i] = polymat[0][1][i] = polymat[2][0][i] = polymat[2][1][i] = 0;
                    }

                    for (i = 0; i < 16; i++) {
                        shiftmat[0][0][i] = shiftmat[0][1][i] = shiftmat[2][0][i] = shiftmat[2][1][i] = 0;
                    }

                    int numblox[3] = {0, 0, 0};

                    float bstemp[2];

                    for (vblock = 1; vblock < vblsz - 1; vblock++)
                        for (hblock = 1; hblock < hblsz - 1; hblock++) {
                            // block 3x3 median of blockshifts for robustness
                            for (c = 0; c < 3; c += 2) {
                                for (dir = 0; dir < 2; dir++) {
                                    p[0] = blockshifts[(vblock - 1) * hblsz + hblock - 1][c][dir];
                                    p[1] = blockshifts[(vblock - 1) * hblsz + hblock][c][dir];
                                    p[2] = blockshifts[(vblock - 1) * hblsz + hblock + 1][c][dir];
                                    p[3] = blockshifts[(vblock) * hblsz + hblock - 1][c][dir];
                                    p[4] = blockshifts[(vblock) * hblsz + hblock][c][dir];
                                    p[5] = blockshifts[(vblock) * hblsz + hblock + 1][c][dir];
                                    p[6] = blockshifts[(vblock + 1) * hblsz + hblock - 1][c][dir];
                                    p[7] = blockshifts[(vblock + 1) * hblsz + hblock][c][dir];
                                    p[8] = blockshifts[(vblock + 1) * hblsz + hblock + 1][c][dir];
                                    PIX_SORT(p[1], p[2]);
                                    PIX_SORT(p[4], p[5]);
                                    PIX_SORT(p[7], p[8]);
                                    PIX_SORT(p[0], p[1]);
                                    PIX_SORT(p[3], p[4]);
                                    PIX_SORT(p[6], p[7]);
                                    PIX_SORT(p[1], p[2]);
                                    PIX_SORT(p[4], p[5]);
                                    PIX_SORT(p[7], p[8]);
                                    PIX_SORT(p[0], p[3]);
                                    PIX_SORT(p[5], p[8]);
                                    PIX_SORT(p[4], p[7]);
                                    PIX_SORT(p[3], p[6]);
                                    PIX_SORT(p[1], p[4]);
                                    PIX_SORT(p[2], p[5]);
                                    PIX_SORT(p[4], p[7]);
                                    PIX_SORT(p[4], p[2]);
                                    PIX_SORT(p[6], p[4]);
                                    PIX_SORT(p[4], p[2]);
                                    bstemp[dir] = p[4];
                                    //if (c==0 && dir==0) printf("vblock= %d hblock= %d blockshiftsmedian= %f \n",vblock,hblock,p[4]);
                                }

                                //if (verbose) fprintf (stderr,_("tile vshift hshift (%d %d %4f %4f)...\n"),vblock, hblock, blockshifts[(vblock)*hblsz+hblock][c][0], blockshifts[(vblock)*hblsz+hblock][c][1]);

                                //now prepare coefficient matrix; use only data points within two std devs of zero
                                if (SQR(bstemp[0]) > 4.0 * blockvar[0][c] || SQR(bstemp[1]) > 4.0 * blockvar[1][c]) {
                                    continue;
                                }

                                numblox[c]++;

                                for (dir = 0; dir < 2; dir++) {
                                    for (i = 0; i < polyord; i++) {
                                        for (j = 0; j < polyord; j++) {
                                            for (m = 0; m < polyord; m++)
                                                for (n = 0; n < polyord; n++) {
                                                    polymat[c][dir][numpar * (polyord * i + j) + (polyord * m + n)] += (float)pow((double)vblock, i + m) * pow((double)hblock, j + n) * blockwt[vblock * hblsz + hblock];
                                                }

                                            shiftmat[c][dir][(polyord * i + j)] += (float)pow((double)vblock, i) * pow((double)hblock, j) * bstemp[dir] * blockwt[vblock * hblsz + hblock];
                                        }

                                        //if (c==0 && dir==0) {printf("i= %d j= %d shiftmat= %f \n",i,j,shiftmat[c][dir][(polyord*i+j)]);}
                                    }//monomials
                                }//dir

                            }//c
                        }//blocks

                    numblox[1] = min(numblox[0], numblox[2]);

                    //if too few data points, restrict the order of the fit to linear
                    if (numblox[1] < 32) {
                        polyord = 2;
                        numpar = 4;

                        if (numblox[1] < 10) {

                            printf ("numblox = %d \n", numblox[1]);
                            processpasstwo = false;
                        }
                    }

                    if(processpasstwo)

                        //fit parameters to blockshifts
                        for (c = 0; c < 3; c += 2)
                            for (dir = 0; dir < 2; dir++) {
                                res = LinEqSolve(numpar, polymat[c][dir], shiftmat[c][dir], fitparams[c][dir]);

                                if (res) {
                                    printf("CA correction pass failed -- can't solve linear equations for color %d direction %d...\n", c, dir);
                                    processpasstwo = false;
                                }
                            }
                    printf("CA correction parameters fitted.\n");

                }

                //fitparams[polyord*i+j] gives the coefficients of (vblock^i hblock^j) in a polynomial fit for i,j<=4
            }
            //end of initialization for CA correction pass
            //only executed if cared and cablue are zero
        }


        // clean up
        free(buffer);


    }

    free(Gtmp);
    free(buffer1);
    free(RawDataTmp);

    //if(plistener) {
    //    plistener->setProgress(1.0);
    //}

#undef TS
#undef TSH
#undef PIX_SORT
}



std::map<Glib::ustring, PF::RawImage*> PF::raw_images;


