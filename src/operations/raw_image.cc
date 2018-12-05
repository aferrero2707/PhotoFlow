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

#include <assert.h>
#include <stdint.h>
#include <memory>
#include <array>
#include <stdio.h>
#include <math.h>
#include <float.h>

typedef uint32_t uint32;

#include "../base/exif_data.hh"

//#include <gexiv2/gexiv2-metadata.h>

#include "../base/pf_mkstemp.hh"
#include "../base/rawmatrix.hh"

#include "../rt/rtengine/rawimage.h"
#include "../rt/rtengine/camconst.h"
#include "../rt/rtengine/median.h"
#include "raw_image.hh"

#include "amaze_demosaic.hh"
#include "fast_demosaic_xtrans.hh"

#include "../external/darktable/src/external/adobe_coeff.c"




static bool dt_exif_read_exif_tag(Exiv2::ExifData &exifData, Exiv2::ExifData::const_iterator *pos, std::string key)
{
  try
  {
    return (*pos = exifData.findKey(Exiv2::ExifKey(key))) != exifData.end() && (*pos)->size();
  }
  catch(Exiv2::AnyError &e)
  {
    std::string s(e.what());
    std::cerr << "[exiv2] " << s << std::endl;
    return false;
  }
}
#define FIND_EXIF_TAG(key) dt_exif_read_exif_tag(exifData, &pos, key)


// Color space conversion to/from XYZ; color spaces adapted to D50 using Bradford transform
constexpr double xyz_sRGB[3][3] = {
    {0.4360747,  0.3850649, 0.1430804},
    {0.2225045,  0.7168786,  0.0606169},
    {0.0139322,  0.0971045,  0.7141733}
};

// define this function, it is only declared in rawspeed:
int rawspeed_get_number_of_processor_cores()
{
#ifdef _OPENMP
  return omp_get_num_procs();
#else
  return 1;
#endif
}


static void rawspeed_lookup_makermodel(const rawspeed::Camera *cam, const char *maker, const char *model,
    char *mk, int mk_len, char *md, int md_len,
    char *al, int al_len)
{
  int got_it_done = FALSE;
  try {
    if (cam)
    {
      g_strlcpy(mk, cam->canonical_make.c_str(), mk_len);
      g_strlcpy(md, cam->canonical_model.c_str(), md_len);
      g_strlcpy(al, cam->canonical_alias.c_str(), al_len);
      got_it_done = TRUE;
    }
  }
  catch(const std::exception &exc)
  {
    printf("[rawspeed] %s\n", exc.what());
  }

#ifndef NDEBUG
  std::cout<<"rawspeed_lookup_makermodel: cam="<<cam<<"  got_it_done="<<got_it_done<<std::endl;
#endif
  if (!got_it_done)
  {
    // We couldn't find the camera or caught some exception, just punt and pass
    // through the same values
    g_strlcpy(mk, maker, mk_len);
    g_strlcpy(md, model, md_len);
    g_strlcpy(al, model, al_len);
  }
}



dcraw_data_t* PF::get_raw_data( VipsImage* image )
{
  if( !image ) return NULL;
  dcraw_data_t* raw_data;
  size_t blobsz;
  if( !vips_image_get_blob( image, "raw_image_data",(void**)&raw_data, &blobsz ) &&
      blobsz == sizeof(dcraw_data_t) ) {
    return raw_data;
  }
  return NULL;
}



bool PF::check_xtrans( unsigned filters )
{
  return( filters == 9u);
}



PF::RawImage::RawImage( const std::string _fname ):
                nref(1), file_name( _fname ),
                image( NULL ), demo_image( NULL ),
                raw_hist( NULL ), pdata( NULL )
{
#ifndef NDEBUG
  std::cout<<"RawImage::RawImage(): opening file \""<<file_name<<"\""<<std::endl;
#endif
  //dcraw_data_t* pdata;
  file_name_real = file_name;
  int ifd = open( file_name_real.c_str(), O_RDONLY );
  if( ifd < 0 ) {
    char* fullpath = strdup( file_name_real.c_str() );
    gchar* fname = g_path_get_basename( fullpath );
    gchar* fname2 = g_build_filename( PF::PhotoFlow::Instance().get_current_image_dir().c_str(),
        fname, NULL );
    std::cout<<"RawImage::RawImage(): file \""<<file_name_real<<"\" not found"<<std::endl;
    std::cout<<"                      trying with \""<<fname2<<"\""<<std::endl;
    ifd = open( fname2, O_RDONLY );
    if( ifd < 0 ) {
      std::cout<<"RawImage::RawImage(): \""<<fname2<<"\" not found"<<std::endl;
      return;
    } else {
      close(ifd);
    }
    file_name_real = fname2;
    g_free( fname );
    g_free( fname2 );
  } else {
    close(ifd);
  }
  //#ifndef NDEBUG
  std::cout<<"RawImage::RawImage(): opening file \""<<file_name_real<<"\""<<std::endl;
  //#endif

  iwidth = 0, iheight = 0, crop_x = 0, crop_y = 0;

  PF::exif_read( &exif_data, file_name_real.c_str() );

#ifndef NDEBUG
  std::cout<<"maker: \""<<exif_data.exif_maker<<"\" model: \""<<exif_data.exif_model<<"\""<<std::endl;
#endif

  if( !load_rawspeed() ) {
    std::cout<<"RawImage::RawImage(): failed to load file \""<<file_name<<"\" using RawSpeed"<<std::endl;
    if( !load_rawtherapee() ) {
      std::cout<<"RawImage::RawImage(): failed to load file \""<<file_name<<"\" using RawTherapee"<<std::endl;
      return;
    }
  }

  PF::exiv2_data_t* exiv2_buf = new PF::exiv2_data_t;
  // read embedded color matrix as used in DNGs
  {
    try
    {
      Exiv2::ExifData::const_iterator pos;
      Exiv2::BasicIo::AutoPtr file (new Exiv2::FileIo (file_name_real));
      //std::unique_ptr<Exiv2::Image> image(Exiv2::ImageFactory::open(file));
      exiv2_buf->image = Exiv2::ImageFactory::open(file);
      assert(exiv2_buf->image.get() != 0);
      exiv2_buf->image->readMetadata();
      bool res = true;

      // EXIF metadata
      Exiv2::ExifData &exifData = exiv2_buf->image->exifData();
      if(!exifData.empty()) {
        int is_1_65 = -1, is_2_65 = -1; // -1: not found, 0: some random type, 1: D65
        if(FIND_EXIF_TAG("Exif.Image.CalibrationIlluminant1"))
        {
          is_1_65 = (pos->toLong() == 21) ? 1 : 0;
        }
        if(FIND_EXIF_TAG("Exif.Image.CalibrationIlluminant2"))
        {
          is_2_65 = (pos->toLong() == 21) ? 1 : 0;
        }


        float d65_color_matrix[9];
        d65_color_matrix[0] = INFINITY;
        bool has_embedded = false;
        // use the d65 (type == 21) matrix if we found it, otherwise use whatever we got
        Exiv2::ExifData::const_iterator cm1_pos = exifData.findKey(Exiv2::ExifKey("Exif.Image.ColorMatrix1"));
        Exiv2::ExifData::const_iterator cm2_pos = exifData.findKey(Exiv2::ExifKey("Exif.Image.ColorMatrix2"));
        if(is_1_65 == 1 && cm1_pos != exifData.end() && cm1_pos->count() == 9 && cm1_pos->size()) {
          for(int i = 0; i < 9; i++) d65_color_matrix[i] = cm1_pos->toFloat(i);
          has_embedded = true;
        } else if(is_2_65 == 1 && cm2_pos != exifData.end() && cm2_pos->count() == 9 && cm2_pos->size()) {
          for(int i = 0; i < 9; i++) d65_color_matrix[i] = cm2_pos->toFloat(i);
          has_embedded = true;
        } else if(cm1_pos != exifData.end() && cm1_pos->count() == 9 && cm1_pos->size()) {
          for(int i = 0; i < 9; i++) d65_color_matrix[i] = cm1_pos->toFloat(i);
          has_embedded = true;
        } else if(cm2_pos != exifData.end() && cm2_pos->count() == 9 && cm2_pos->size()) {
          for(int i = 0; i < 9; i++) d65_color_matrix[i] = cm2_pos->toFloat(i);
          has_embedded = true;
        }
//#ifndef NDEBUG
        printf("d65_color_matrix[0]: %.4f\n", d65_color_matrix[0]);
        printf("has_embedded: %d\n", (int)has_embedded);
        printf("isfinite(d65_color_matrix[0]): %d\n", std::isfinite(d65_color_matrix[0]));
//#endif
        if( has_embedded ) {
          for(int i = 0; i < 3; i++) pdata->color.cam_xyz[0][i] = d65_color_matrix[i];
          for(int i = 0; i < 3; i++) pdata->color.cam_xyz[1][i] = d65_color_matrix[i+3];
          for(int i = 0; i < 3; i++) pdata->color.cam_xyz[3][i] = d65_color_matrix[i+3];
          for(int i = 0; i < 3; i++) pdata->color.cam_xyz[2][i] = d65_color_matrix[i+6];
//#ifndef NDEBUG
          printf("pdata->color.cam_xyz (embedded):\n");
          for(int k = 0; k < 3; k++)
          {
            //printf("    %.4f %.4f %.4f\n",xyz_to_cam[k][0],xyz_to_cam[k][1],xyz_to_cam[k][2]);
            printf("    %.4f %.4f %.4f\n",pdata->color.cam_xyz[k][0],pdata->color.cam_xyz[k][1],pdata->color.cam_xyz[k][2]);
          }
//#endif
        }

        Exiv2::ExifData::const_iterator orient_pos = exifData.findKey(Exiv2::ExifKey("Exif.Image.Orientation"));
        if( orient_pos != exifData.end() && orient_pos->count() == 1 && orient_pos->size() ) {
          PF::ExifOrientation orientation = (PF::ExifOrientation)orient_pos->toLong(0);
          switch( orientation ) {
          case PF_EXIF_ORIENTATION_ROT_90:
            pdata->sizes.flip = 6; break;
          case PF_EXIF_ORIENTATION_ROT_180:
            pdata->sizes.flip = 3; break;
          case PF_EXIF_ORIENTATION_ROT_270:
            pdata->sizes.flip = 5; break;
          default:
            break;
          }

          Exiv2::XmpData& xmp_data = exiv2_buf->image->xmpData();
          exifData["Exif.Image.Orientation"] = static_cast<uint16_t> (PF_EXIF_ORIENTATION_NORMAL);
          xmp_data["Xmp.tiff.Orientation"] = static_cast<uint16_t> (PF_EXIF_ORIENTATION_NORMAL);
        }

        // Erase thumbnail data
        Exiv2::ExifThumb exifThumb(exifData);
        std::string thumbExt = exifThumb.extension();
        if(!thumbExt.empty()) {
          exifThumb.erase();
        }

      }


    }
    catch(Exiv2::AnyError &e)
    {
      std::string s(e.what());
      std::cerr << "[exiv2] " << file_name_real << ": " << s << std::endl;
      //return 1;
    }
  }



  //==================================================================
  // Save the raw histogram into the image
  vips_image_set_blob( image, "raw-hist",
      (VipsCallbackFn) g_free, raw_hist,
      sizeof(int)*65535*3 );


  vips_image_set_blob( image, "exiv2-data",
      (VipsCallbackFn) exiv2_free, exiv2_buf, sizeof(PF::exiv2_data_t) );


  /*
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
   */

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
  fast_demosaic = NULL;
  if( is_xtrans() ) fast_demosaic = PF::new_fast_demosaic_xtrans();
  else fast_demosaic = PF::new_fast_demosaic();
  fast_demosaic->get_par()->set_image_hints( image );
  fast_demosaic->get_par()->set_format( VIPS_FORMAT_FLOAT );
  fast_demosaic->get_par()->set_demand_hint( VIPS_DEMAND_STYLE_FATSTRIP );
  std::vector<VipsImage*> in2;
  in2.push_back( image );
  unsigned int level = 0;
  VipsImage* out_demo = fast_demosaic->get_par()->build( in2, 0, NULL, NULL, level );
  //g_object_unref( image );

  VipsImage* out_demo2;

  vips_copy( out_demo, &out_demo2,
      "format", VIPS_FORMAT_FLOAT,
      "bands", (int)3,
      "coding", VIPS_CODING_NONE,
      "interpretation", VIPS_INTERPRETATION_RGB,
      NULL );
  g_object_unref( out_demo );

  int tw = 64, th = 64;
  // reserve two complete rows of tiles
  int nt = out_demo2->Xsize*2/tw;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  int threaded = 1, persistent = 0;

  if( vips_tilecache(out_demo2, &demo_image,
      "tile_width", tw, "tile_height", th, "max_tiles", nt,
      "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
    std::cout<<"GaussBlurPar::build(): vips_tilecache() failed."<<std::endl;
    demo_image = out_demo2;
  } else {
    PF_UNREF( out_demo2, "OpParBase::build_many(): out_demo2 unref" );
  }

  pyramid.init( demo_image );
  std::cout<<"RawImage::RawImage() finished\n";
  return;

  /*
  sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int fd = pf_mkstemp( fname );
  if( fd < 0 )
    return;
  std::cout<<"RawImage: cache file: "<<fname<<std::endl;
  cache_file_name2 = fname;

  std::cout<<"RawImage: calling vips_rawsave_fd()"<<std::endl;
  vips_rawsave_fd( out_demo, fd, NULL );
  std::cout<<"RawImage: vips_rawsave_fd() finished"<<std::endl;
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

  buf2 = malloc( sizeof(exif_data_t) );
  if( !buf2 ) return;
  memcpy( buf2, &exif_data, sizeof(exif_data_t) );
  vips_image_set_blob( demo_image, PF_META_EXIF_NAME,
           (VipsCallbackFn) PF::exif_free, buf2,
           sizeof(exif_data_t) );

  pyramid.init( demo_image );
   */
}


static bool convert_locale(const std::string& us, std::string& s)
{
#ifndef _WIN32
  s = us;
#else
  std::string ls = us;//Glib::locale_from_utf8(us);
  std::cout<<"ustring_to_string: ls: \""<<ls<<"\"\n";
  // turn the locale ANSI encoded string into UTF-8 so that FileReader can
  // turn it into UTF-16 later
  int size = MultiByteToWideChar(CP_ACP, 0, &(ls[0]), -1, NULL, 0);
  std::wstring ws;
  ws.resize(size);
  MultiByteToWideChar(CP_ACP, 0, &(ls[0]), -1, &(ws[0]), size);
  std::wcout<<"ustring_to_string: ws: \""<<ws<<"\"\n";
  size = WideCharToMultiByte(CP_UTF8, 0, &(ws[0]), -1, NULL, 0,
                             NULL, NULL);
  s.resize(size);
  WideCharToMultiByte(CP_UTF8, 0, &(ws[0]), -1, &(s[0]), size,
                      NULL, NULL);
  std::cout<<"ustring_to_string: s: \""<<s<<"\"\n";
#endif
  /*
  std::ostringstream ostr;
  try {
    ostr << us;
    s = ostr.str();
  } catch(Glib::ConvertError& e) {
    std::cout<<"ustring_to_string: falling back to direct conversion"<<std::endl;
    s = us;
  }
  std::cout<<"ustring_to_string: s=\""<<s<<"\""<<std::endl;
  */
  return true;
}


bool PF::RawImage::load_rawspeed()
{
#ifdef __WIN32__
  std::string camfile = PF::PhotoFlow::Instance().get_data_dir() + "\\rawspeed\\cameras.xml";
#else
  std::string camfile = PF::PhotoFlow::Instance().get_data_dir() + "/rawspeed/cameras.xml";
#endif
  //#ifndef NDEBUG
  std::cout<<"RawImage::load_rawspeed(): RAWSpeed camera file: "<<camfile<<std::endl;
  //#endif
  meta = new rawspeed::CameraMetaData( camfile.c_str() );
  //#ifndef NDEBUG
  std::cout<<"RawImage::load_rawspeed(): meta="<<(void*)meta<<std::endl;
  //#endif

  if( !meta ) {
    std::cout<<"RawImage::load_rawspeed(): unable to load camera metadata from \""<<camfile<<"\""<<std::endl;
    return false;
  }

  //for(int i = 0; i < 4; i++)
  //	for(int j = 0; j < 3; j++)
  //		dcraw_data.color.cam_xyz[i][j] = get_cam_xyz(i,j);
  pdata = &dcraw_data;

  std::cout<<"RawImage::load_rawspeed: file_name_real: \""<<file_name_real<<"\"\n";
/*
  char filen[PATH_MAX] = { 0 };
  snprintf(filen, sizeof(filen), "%s", file_name_real.c_str());
  std::cout<<"RawImage::load_rawspeed(): input file: "<<filen<<std::endl;
  rawspeed::FileReader f(filen);
*/
  std::string filen;
  convert_locale(file_name_real, filen);
  std::cout<<"RawImage::load_rawspeed: filen: \""<<filen<<"\"\n";
  rawspeed::FileReader f(&(filen[0]));

  //#ifdef __APPLE__
  //  std::auto_ptr<rawspeed::RawDecoder> d;
  //  std::auto_ptr<rawspeed::Buffer> m;
  //#else
  std::unique_ptr<rawspeed::RawDecoder> d;
  std::unique_ptr<const rawspeed::Buffer> m;
  //#endif

  try
  {
    //#ifdef __APPLE__
    //    m = std::auto_ptr<rawspeed::Buffer>(f.readFile());
    //#else
    m = std::unique_ptr<const rawspeed::Buffer>(f.readFile());
    //#endif
    //#ifndef NDEBUG
    std::cout<<"RawImage::load_rawspeed(): FileMap object: "<<(void*)m.get()<<std::endl;
    //#endif
    if(!m.get()) {
      std::cout<<"RawImage::load_rawspeed(): unable to create FileMap object"<<std::endl;
      return false;
    }

    rawspeed::RawParser t(m.get());
    //#ifdef __APPLE__
    //    d = std::auto_ptr<rawspeed::RawDecoder>(t.getDecoder(meta));
    //#else
    d = std::unique_ptr<rawspeed::RawDecoder>(t.getDecoder(meta));
    //#endif

    if(!d.get()) {
      std::cout<<"RawImage::load_rawspeed(): unable to create RawDecoder object"<<std::endl;
      return false;
    }

    d->failOnUnknown = true;
    d->checkSupport(meta);
    d->decodeRaw();
    d->decodeMetaData(meta);
    rawspeed::RawImage& r = d->mRaw;

    for (uint32 i=0; i<r->getErrors().size(); i++)
      fprintf(stderr, "[rawspeed] %s\n", r->getErrors()[i].c_str());

    // Get CFA pattern
    pdata->idata.filters = r->cfa.getDcrawFilter();
    if(pdata->idata.filters == 9u)
    {
      // get 6x6 CFA offset from top left of cropped image
      // NOTE: This is different from how things are done with Bayer
      // sensors. For these, the CFA in cameras.xml is pre-offset
      // depending on the distance modulo 2 between raw and usable
      // image data. For X-Trans, the CFA in cameras.xml is
      // (currently) aligned with the top left of the raw data, and
      // hence it is shifted here to align with the top left of the
      // cropped image.
      rawspeed::iPoint2D tl_margin = r->getCropOffset();
      for(int i = 0; i < 6; ++i)
        for(int j = 0; j < 6; ++j)
        {
          pdata->idata.xtrans_uncropped[j][i] = r->cfa.getColorAt(i % 6, j % 6);
          pdata->idata.xtrans[j][i] = r->cfa.getColorAt((i + tl_margin.x) % 6, (j + tl_margin.y) % 6);
        }
    }

    if(r->blackLevelSeparate[0] == -1 || r->blackLevelSeparate[1] == -1 || r->blackLevelSeparate[2] == -1
        || r->blackLevelSeparate[3] == -1)
    {
      r->calculateBlackAreas();
    }

    std::cout<<"RawSpeed: r->blackLevel="<<r->blackLevel<<std::endl
        <<"r->blackLevelSeparate=";
    for(uint8_t i = 0; i < 4; i++) {
      std::cout<<r->blackLevelSeparate[i]<<" ";
    }
    std::cout<<std::endl;

    //for(uint8_t i = 0; i < 4; i++) pdata->color.cblack[i] = pdata->color.black;

    /*
     * FIXME
     * if(r->whitePoint == 65536)
     *   ???
     */

    // Grab the WB
    for(int i = 0; i < 3; i++) pdata->color.cam_mul[i] = r->metadata.wbCoeffs[i];
    pdata->color.cam_mul[3] = pdata->color.cam_mul[1];

    // dimensions of uncropped image
    rawspeed::iPoint2D dimUncropped = r->getUncroppedDim();
    //iwidth = dimUncropped.x;
    //iheight = dimUncropped.y;

    pdata->sizes.raw_width = dimUncropped.x;
    pdata->sizes.raw_height = dimUncropped.y;

    // dimensions of cropped image
    rawspeed::iPoint2D dimCropped = r->dim;
    iwidth = dimCropped.x;
    iheight = dimCropped.y;

    pdata->sizes.width = iwidth;
    pdata->sizes.height = iheight;

    pdata->sizes.flip = 0;

    // crop - Top,Left corner
    rawspeed::iPoint2D cropTL = r->getCropOffset();
    crop_x = cropTL.x;
    crop_y = cropTL.y;

    pdata->sizes.left_margin = crop_x;
    pdata->sizes.top_margin = crop_y;

    // crop - Bottom,Right corner
    rawspeed::iPoint2D cropBR = dimUncropped - dimCropped - cropTL;

    //#ifndef NDEBUG
    std::cout<<"original width: "<<dimUncropped.x<<"  crop offset: "<<cropTL.y<<","<<cropTL.x<<"  cropped width: "<<dimCropped.x<<std::endl;
    //#endif

    pdata->color.black = r->blackLevel;
    pdata->color.maximum = r->whitePoint;

    if( !is_xtrans() ) {
      for(int row = 0; row < 2; row++) {
        for(int col = 0; col < 2; col++) {
          unsigned int color = FC(row,col);
          if( color == 1 && FC(row,col+1) == 2 ) color = 3;
          if(color > 3) color = 0;
          unsigned int color2 = BL(row,col);
          if(color2 > 3) color2 = 0;
          pdata->color.cblack[color] = r->blackLevelSeparate[color2];
        }
      }

      if(r->blackLevel <= 0) {
        float black = 0.0f;
        for(uint8_t i = 0; i < 4; i++) {
          black += pdata->color.cblack[i];
        }
        black /= 4.0f;

        pdata->color.black = CLAMP(black, 0, UINT16_MAX);
        //      } else {
        //        if( pdata->color.cblack[0]<=0 && pdata->color.cblack[1]<=0 &&
        //            pdata->color.cblack[2]<=0 && pdata->color.cblack[3]<=0 ) {
        //          pdata->color.cblack[0] = pdata->color.black;
        //          pdata->color.cblack[1] = pdata->color.black;
        //          pdata->color.cblack[2] = pdata->color.black;
        //          pdata->color.cblack[3] = pdata->color.black;
        //        }
      }
    } else {
      for(uint8_t i = 0; i < 4; i++) {
        pdata->color.cblack[i] = pdata->color.black;
      }
    }
//#ifndef NDEBUG
    std::cout<<"RawSpeed camera WB multipliers: "<<pdata->color.cam_mul[0]<<" "<<pdata->color.cam_mul[1]<<" "<<pdata->color.cam_mul[2]<<" "<<pdata->color.cam_mul[3]<<std::endl;
    std::cout<<"RawSpeed black="<<pdata->color.black<<"  white="<<pdata->color.maximum<<std::endl;
//#endif
  }

  catch(const std::exception &exc)
  {
    printf("[rawspeed] %s\n", exc.what());

    /* if an exception is raised lets not retry or handle the
     specific ones, consider the file as corrupted */
    return false;
  }
  catch(...)
  {
    printf("Unhandled exception in imageio_rawspeed\n");
    return false;
  }


  //==================================================================
  // Save decoded data to cache file on disk.
  // The pixel values are normalized to the [0..65535] range 
  // using the default black and white levels.
  /*
  char fname[500];
  sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int temp_fd = pf_mkstemp( fname );
  if( temp_fd < 0 ) return;
  std::cout<<"RawImage: cache file: "<<fname<<std::endl;
	cache_file_name = fname;
   */

#ifndef NDEBUG
  std::cout<<"Saving raw data to buffer..."<<std::endl;
#endif
  int row, col, col2;
  //size_t pxsize = sizeof(PF::RawPixel);
  //size_t pxsize = sizeof(float)+sizeof(guint8);
  size_t pxsize = sizeof(float)*2;
  guint8* rawbuf = (guint8*)malloc( pxsize*iwidth*iheight );
  if( !rawbuf ) {
    std::cout<<"RawImage::load_rawspeed(): cannot allocate raw buffer, size: "<<pxsize*iwidth*iheight/(1024*1024)<<"MB"<<std::endl;
    return false;
  }
#ifndef NDEBUG
  std::cout<<"Raw buffer allocated: "<<(void*)rawbuf<<"    size: "<<pxsize*iwidth*iheight/(1024*1024)<<"MB"<<std::endl;
#endif
  /* Normalized raw data to 65535 and build raw histogram
   * */
  // Allocate raw histogram and fill it with zero
  raw_hist = (int*)malloc( 65536*3*sizeof(int) );
  if( !raw_hist ) {
    std::cout<<"RawImage::load_rawspeed(): cannot allocate raw histogram buffer"<<std::endl;
    return false;
  }
#ifndef NDEBUG
  std::cout<<"Raw histogram buffer allocated: "<<(void*)raw_hist<<std::endl;
#endif
  memset( raw_hist, 0, 65536*3*sizeof(int) );

#ifndef NDEBUG
  std::cout<<"Initialising rawData structure..."<<std::endl;
#endif
  rawData.Init( iwidth, iheight, 0, 0 );
#ifndef NDEBUG
  std::cout<<"... rawData structure initialised"<<std::endl;
#endif

  guint8* ptr = rawbuf;
  float* fptr;
#ifndef NDEBUG
  std::cout<<"RawImage: crop_x="<<crop_x<<" crop_y="<<crop_y<<std::endl;
#endif
  std::cout<<"RawImage: filters="<<dcraw_data.idata.filters<<std::endl;

  float ca_cam_mul[4];
  for(int i = 0; i < 4; i++) ca_cam_mul[i] = pdata->color.cam_mul[i];
  float ca_cam_mul_max = 0;
  for(int i = 0; i < 4; i++) if(ca_cam_mul_max<ca_cam_mul[i]) ca_cam_mul_max=ca_cam_mul[i];

  rawspeed::RawImage& r = d->mRaw;
  for(row=0;row<iheight;row++) {
    unsigned int row_offset = row*iwidth;
    //#ifndef NDEBUG
    //std::cout<<"  row: "<<row<<std::endl;
    //if( (row%100) == 0 ) std::cout<<"  saving row "<<row<<""<<std::endl;
    //#endif
    for(col=0; col<iwidth; col++) {
      int col2 = col + crop_x;
      int row2 = row + crop_y;
      //unsigned char color = (is_xtrans()) ? r->cfa.getColorAt(col2,row2) : r->cfa.getColorAt(col,row);
      unsigned char color = (is_xtrans()) ? r->cfa.getColorAt(col2,row2) : FC(row,col);
      unsigned char color4 = color;
      if( !is_xtrans() ) {
        if( color4 == 1 && FC(row,col+1) == 2 ) color4 = 3;
        color = color4;
      }
      float val = 0;
      float nval = 0;
      switch(r->getDataType()) {
      case rawspeed::TYPE_USHORT16: val = *((uint16_t*)r->getDataUncropped(col2,row2)); break;
      case rawspeed::TYPE_FLOAT32: val = *((float*)r->getDataUncropped(col2,row2)); break;
      }
      if(true && row<4 && col<4) {
        std::cout<<"  raw("<<row<<","<<col<<"): "<<val<<"  color="<<(int)color<<"  color4="<<(int)color4<<"  getColorAt()="<<(int)r->cfa.getColorAt(col,row)<<"  FC="<<(int)FC(row,col)<<"  BL="<<(int)BL(row,col)<<std::endl;
      }
      float black = pdata->color.cblack[color4];
      nval = val - black;
      nval /= (pdata->color.maximum - black);
      nval *= 65535;


      // Fill raw histogram
      int hist_id = -1;
      int ch = -1;
      if( color < 3 ) ch = color;
      else if( color == 3 ) ch = 1;
      if( ch >= 0 ) hist_id = static_cast<int>( val*3 + ch );
      if( hist_id >= 65536*3 ) hist_id = 65536*3-1;
      if( hist_id >= 0 ) {
        //std::cout<<"hist_id="<<hist_id
        raw_hist[hist_id] += 1;
      }

      fptr = (float*)ptr;
      fptr[0] = val;
      fptr[1] = color;
      ptr += pxsize;

      if(row<4 && col<4) {
        printf("row=%d col=%d c4=%d  rawData[row][col]=%f  scale_mul[c4]=%f  val=%f\n",
            row, col, color4, val, pdata->color.cam_mul[color4], nval);
      }
      nval *= ca_cam_mul[color4]/ca_cam_mul_max;
      rawData[row][col] = nval;
      //rawData[row].color(col) = color;
    }
  }

  g_strlcpy(exif_data.camera_maker, r->metadata.canonical_make.c_str(), sizeof(exif_data.camera_maker));
  g_strlcpy(exif_data.camera_model, r->metadata.canonical_model.c_str(), sizeof(exif_data.camera_model));
  g_strlcpy(exif_data.camera_alias, r->metadata.canonical_alias.c_str(), sizeof(exif_data.camera_alias));

  // Now we just create a makermodel by concatenation
  g_strlcpy(exif_data.camera_makermodel, exif_data.camera_maker, sizeof(exif_data.camera_makermodel));
  int maker_len = strlen(exif_data.camera_maker);
  exif_data.camera_makermodel[maker_len] = ' ';
  g_strlcpy(exif_data.camera_makermodel+maker_len+1, exif_data.camera_model, sizeof(exif_data.camera_makermodel)-maker_len-1);

  //#ifndef NDEBUG
  std::cout<<"RawImage: Camera maker/model data:"<<std::endl
      <<"  exif_data.exif_maker: "<<exif_data.exif_maker<<std::endl
      <<"  exif_data.exif_model: "<<exif_data.exif_model<<std::endl
      <<"  exif_data.camera_maker: "<<exif_data.camera_maker<<std::endl
      <<"  exif_data.camera_model: "<<exif_data.camera_model<<std::endl
      <<"  exif_data.camera_alias: "<<exif_data.camera_alias<<std::endl
      <<"  exif_data.camera_makermodel: "<<exif_data.camera_makermodel<<std::endl;
  //#endif


  //float cam_xyz[12];
  float cam_xyz[4][3];
  pdata->color.cam_xyz[0][0] = NAN;
//#ifndef NDEBUG
  std::cout<<"Getting default camera matrix for makermodel=\""<<exif_data.camera_makermodel<<"\""<<std::endl;
//#endif
  dt_dcraw_adobe_coeff(exif_data.camera_makermodel, (float(*)[12])pdata->color.cam_xyz);

//#ifndef NDEBUG
  printf("pdata->color.cam_xyz:\n");
  for(int k = 0; k < 3; k++)
  {
    //printf("    %.4f %.4f %.4f\n",xyz_to_cam[k][0],xyz_to_cam[k][1],xyz_to_cam[k][2]);
    printf("    %.4f %.4f %.4f\n",pdata->color.cam_xyz[k][0],pdata->color.cam_xyz[k][1],pdata->color.cam_xyz[k][2]);
  }
//#endif

  /* free auto pointers on spot */
  d.reset();
  m.reset();

  //std::cout<<"iwidth="<<iwidth<<std::endl
  //   <<"iheight="<<iheight<<std::endl
  //   <<"row="<<row<<"  sizeof(PF::raw_pixel_t)="<<sizeof(PF::raw_pixel_t)<<std::endl;
  //==================================================================
  // Load the RAW image data into a vips image
//#ifndef NDEBUG
  std::cout<<"RawImage: rawData.GetBuffer()="<<(void*)rawData.GetBuffer()<<std::endl;
  std::cout<<"Starting CA correction..."<<std::endl;
//#endif
  //CA_correct_RT_old();
  CA_correct_RT();
//#ifndef NDEBUG
  std::cout<<"... CA correction finished"<<std::endl;
//#endif
  memcpy( pdata->color.ca_fitparams, fitparams, sizeof(fitparams) );
//#ifndef NDEBUG
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 2; j++) {
      printf("i=%d j=%d par:",i,j);
      for(int k = 0; k < 16; k++)
        printf(" %f",fitparams[i][j][k]);
      printf("\n");
    }
  }
//#endif
  rawData.Reset();
//#ifndef NDEBUG
  std::cout<<"RawImage: rawData.Reset() called"<<std::endl;
//#endif


#ifndef NDEBUG
  std::cout<<"  buffer size: "<<sizeof(float)*iwidth*iheight<<" bytes"<<std::endl;
#endif
  VipsImage* ti = vips_image_new_from_memory_copy(
      rawbuf, sizeof(float)*2*iwidth*iheight,
      iwidth, iheight, 2, VIPS_FORMAT_FLOAT );
#ifndef NDEBUG
  std::cout<<"Deleting Raw buffer: "<<(void*)rawbuf<<std::endl;
#endif
  free( rawbuf );
#ifndef NDEBUG
  std::cout<<"Raw buffer deleted"<<std::endl;
#endif
  if( !ti ) {
    std::cout<<"RawImage: ERROR creating Vips image from memory"<<std::endl;
    return false;
  }

  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_MULTIBAND;
  //VipsBandFormat format = VIPS_FORMAT_UCHAR;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  int nbands = 2;
  vips_copy( ti, &image,
      "format", format,
      "bands", nbands,
      "coding", coding,
      "interpretation", interpretation,
      NULL );
  g_object_unref( ti );

#ifndef NDEBUG
  std::cout<<"RawImage: after vips_copy()"<<std::endl;
#endif

  if (!exif_data.camera_maker[0] || !exif_data.camera_model[0] || !exif_data.camera_alias[0]) {
    const rawspeed::Camera *cam = meta->getCamera(exif_data.exif_maker, exif_data.exif_model, "");
    if (!cam) {
      std::cout<<"RawImage: getting rawspeed camera in DNG mode"<<std::endl;
      cam = meta->getCamera(exif_data.exif_maker, exif_data.exif_model, "dng");
    }
//#ifndef NDEBUG
    std::cout<<"RawImage: calling rawspeed_lookup_makermodel()"<<std::endl;
//#endif
    // We need to use the exif values, so let's get rawspeed to munge them
    rawspeed_lookup_makermodel(cam, exif_data.exif_maker, exif_data.exif_model,
        exif_data.camera_maker, sizeof(exif_data.camera_maker),
        exif_data.camera_model, sizeof(exif_data.camera_model),
        exif_data.camera_alias, sizeof(exif_data.camera_alias));
  }

//#ifndef NDEBUG
  std::cout<<"RawImage::load_rawspeed() finished"<<std::endl;
//#endif
  return true;
}


bool PF::RawImage::load_rawtherapee()
{
  //if( !rtengine::CameraConstantsStore::getInstance() )
  //  rtengine::CameraConstantsStore::initCameraConstants(PF::PhotoFlow::Instance().get_base_dir(),"");

  rtengine::RawImage rt_image(file_name_real);
  rtengine::RawImage* ri = &rt_image;
  int errCode = rt_image.loadRaw();
#ifndef NDEBUG
  std::cout<<"RawImage::load_rawtherapee(): errCode="<<errCode<<std::endl;
#endif
  if (errCode) {
    return false;
  }

  rt_image.compress_image();
#ifndef NDEBUG
  std::cout<<"RawImage::load_rawtherapee(): compress_image() finished."<<std::endl;
#endif

  pdata = &dcraw_data;

  // Get CFA pattern
  pdata->idata.filters = ri->get_filters();

  // First we get the "as shot" ("Camera") white balance and store it
  float scale_mul[4]; // multiplier for each color
  float c_black[4]; // copy of cblack Dcraw for black level
  float c_white[4];
  float pre_mul[4];
  // FIXME: get_colorsCoeff not so much used nowadays, when we have calculate_scale_mul() function here
  ri->get_colorsCoeff( pre_mul, scale_mul, c_black, false);//modify  for black level
  pdata->color.black = c_black[0];
  for(uint8_t i = 0; i < 4; i++) pdata->color.cblack[i] = c_black[i];
  pdata->color.maximum = ri->get_white(0);

  // Grab the WB
  for(int i = 0; i < 4; i++) pdata->color.cam_mul[i] = ri->get_cam_mul(i);
  if(pdata->color.cam_mul[3] < 0.00000001)
    pdata->color.cam_mul[3] = pdata->color.cam_mul[1];
#ifndef NDEBUG
  std::cout<<"RawTherapee camera WB multipliers: "<<pdata->color.cam_mul[0]<<" "<<pdata->color.cam_mul[1]                                                                                                       <<" "<<pdata->color.cam_mul[2]<<" "<<pdata->color.cam_mul[3]<<std::endl;
  std::cout<<"RawTherapee black="<<pdata->color.black<<"  white="<<pdata->color.maximum<<std::endl;
#endif

  //for(int i = 0; i < 4; i++)
  //  for(int j = 0; j < 3; j++)
  //    dcraw_data.color.cam_xyz[i][j] = ri->get_cam_xyz(i,j);

  // dimensions of uncropped image
  //pdata->sizes.raw_width = dimUncropped.x;
  //pdata->sizes.raw_height = dimUncropped.y;

  // dimensions of cropped image
  pdata->sizes.width = ri->get_width();
  pdata->sizes.height = ri->get_height();
  iwidth = ri->get_width();
  iheight = ri->get_height();

  pdata->sizes.flip = 0;

  // crop - Top,Left corner
  pdata->sizes.left_margin = ri->get_leftmargin();
  pdata->sizes.top_margin = ri->get_topmargin();

  // create profile
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      imatrices.rgb_cam[i][j] = ri->get_rgb_cam(i, j);
      std::cout<<"RawImage::load_rawtherapee(): imatrices.rgb_cam["<<i<<"]["<<j<<"]="<<imatrices.rgb_cam[i][j]<<std::endl;
    }

  // compute inverse of the color transformation matrix
  // first arg is matrix, second arg is inverse
  inverse33 (imatrices.rgb_cam, imatrices.cam_rgb);

  memset (imatrices.xyz_cam, 0, sizeof(imatrices.xyz_cam));

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++) {
        imatrices.xyz_cam[i][j] += xyz_sRGB[i][k] * imatrices.rgb_cam[k][j];
      }

  //camProfile = iccStore->createFromMatrix (imatrices.xyz_cam, false, "Camera");
  inverse33 (imatrices.xyz_cam, imatrices.cam_xyz);

  pdata->color.cam_xyz[0][0] = NAN;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
      pdata->color.cam_xyz[i][j] = imatrices.cam_xyz[i][j];
      std::cout<<"RawImage::load_rawtherapee(): pdata->color.cam_xyz["<<i<<"]["<<j<<"]="<<pdata->color.cam_xyz[i][j]<<std::endl;
    }


#ifndef NDEBUG
  std::cout<<"Saving raw data to buffer..."<<std::endl;
#endif
  int row, col, col2;
  //size_t pxsize = sizeof(PF::RawPixel);
  //size_t pxsize = sizeof(float)+sizeof(guint8);
  size_t pxsize = sizeof(float)*2;
  guint8* rawbuf = (guint8*)malloc( pxsize*iwidth*iheight );
#ifndef NDEBUG
  std::cout<<"Raw buffer allocated: "<<(void*)rawbuf<<std::endl;
#endif
  /* Normalized raw data to 65535 and build raw histogram
   * */
  // Allocate raw histogram and fill it with zero
  raw_hist = (int*)malloc( 65536*3*sizeof(int) );
  memset( raw_hist, 0, 65536*3*sizeof(int) );

  rawData.Init( iwidth, iheight, 0, 0 );

  float ca_cam_mul[4];
  for(int i = 0; i < 4; i++) ca_cam_mul[i] = pdata->color.cam_mul[i];
  float ca_cam_mul_max = 0;
  for(int i = 0; i < 4; i++) if(ca_cam_mul_max<ca_cam_mul[i]) ca_cam_mul_max=ca_cam_mul[i];

  guint8* ptr = rawbuf;
  float* fptr;
#ifndef NDEBUG
  std::cout<<"RawImage: crop_x="<<crop_x<<" crop_y="<<crop_y<<std::endl;
#endif
  for(row=0;row<iheight;row++) {
    unsigned int row_offset = row*iwidth;
    //#ifndef NDEBUG
    //std::cout<<"  row: "<<row<<std::endl;
    //if( (row%100) == 0 ) std::cout<<"  saving row "<<row<<""<<std::endl;
    //#endif
    for(col=0; col<iwidth; col++) {
      int col2 = col + crop_x;
      int row2 = row + crop_y;
      unsigned char color = ri->FC(row,col);
      //unsigned char color = (is_xtrans()) ? r->cfa.getColorAt(col2,row2) : FC(col,row);
      if( color > 3 ) color = 0;
      float val = ri->data[row][col];
      float nval = 0;
      if(false && row<8 && col<8) {
        std::cout<<"  raw("<<row<<","<<col<<"): "<<val<<","<<(int)color<<std::endl;
      }
      float black = pdata->color.cblack[color];
      nval = val - black;
      nval /= (pdata->color.maximum - black);
      nval *= 65535;


      // Fill raw histogram
      int hist_id = -1;
      int ch = -1;
      if( color < 3 ) ch = color;
      else if( color == 3 ) ch = 1;
      if( ch >= 0 ) hist_id = static_cast<int>( val*3 + ch );
      if( hist_id >= 65536*3 ) hist_id = 65536*3-1;
      if( hist_id >= 0 ) {
        //std::cout<<"hist_id="<<hist_id
        raw_hist[hist_id] += 1;
      }

      fptr = (float*)ptr;
      fptr[0] = val;
      fptr[1] = color;
      ptr += pxsize;

      nval *= ca_cam_mul[color]/ca_cam_mul_max;
      rawData[row][col] = nval;
      //rawData[row].color(col) = color;
    }
  }
  /*
    g_strlcpy(exif_data.camera_maker, r->metadata.canonical_make.c_str(), sizeof(exif_data.camera_maker));
    g_strlcpy(exif_data.camera_model, r->metadata.canonical_model.c_str(), sizeof(exif_data.camera_model));
    g_strlcpy(exif_data.camera_alias, r->metadata.canonical_alias.c_str(), sizeof(exif_data.camera_alias));

    // Now we just create a makermodel by concatenation
    g_strlcpy(exif_data.camera_makermodel, exif_data.camera_maker, sizeof(exif_data.camera_makermodel));
    int maker_len = strlen(exif_data.camera_maker);
    exif_data.camera_makermodel[maker_len] = ' ';
    g_strlcpy(exif_data.camera_makermodel+maker_len+1, exif_data.camera_model, sizeof(exif_data.camera_makermodel)-maker_len-1);
   */
#ifndef NDEBUG
  std::cout<<"RawImage: Camera maker/model data:"<<std::endl
      <<"  exif_data.exif_maker: "<<exif_data.exif_maker<<std::endl
      <<"  exif_data.exif_model: "<<exif_data.exif_model<<std::endl
      <<"  exif_data.camera_maker: "<<exif_data.camera_maker<<std::endl
      <<"  exif_data.camera_model: "<<exif_data.camera_model<<std::endl
      <<"  exif_data.camera_alias: "<<exif_data.camera_alias<<std::endl
      <<"  exif_data.camera_makermodel: "<<exif_data.camera_makermodel<<std::endl;
#endif


  //std::cout<<"iwidth="<<iwidth<<std::endl
  //   <<"iheight="<<iheight<<std::endl
  //   <<"row="<<row<<"  sizeof(PF::raw_pixel_t)="<<sizeof(PF::raw_pixel_t)<<std::endl;
  //==================================================================
  // Load the RAW image data into a vips image
#ifndef NDEBUG
  std::cout<<"RawImage: rawData.GetBuffer()="<<(void*)rawData.GetBuffer()<<std::endl;
  std::cout<<"Starting CA correction..."<<std::endl;
#endif
  CA_correct_RT();
#ifndef NDEBUG
  std::cout<<"... CA correction finished"<<std::endl;
#endif
  memcpy( pdata->color.ca_fitparams, fitparams, sizeof(fitparams) );
#ifndef NDEBUG
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 2; j++) {
      printf("i=%d j=%d par:",i,j);
      for(int k = 0; k < 16; k++)
        printf(" %f",fitparams[i][j][k]);
      printf("\n");
    }
  }
#endif
  rawData.Reset();


#ifndef NDEBUG
  std::cout<<"  buffer size: "<<sizeof(float)*iwidth*iheight<<" bytes"<<std::endl;
#endif
  VipsImage* ti = vips_image_new_from_memory_copy(
      rawbuf, sizeof(float)*2*iwidth*iheight,
      iwidth, iheight, 2, VIPS_FORMAT_FLOAT );
#ifndef NDEBUG
  std::cout<<"Deleting Raw buffer: "<<(void*)rowbuf<<std::endl;
#endif
  free( rawbuf );
#ifndef NDEBUG
  std::cout<<"Raw buffer deleted"<<std::endl;
#endif
  if( !ti ) {
    std::cout<<"RawImage: ERROR creating Vips image from memory"<<std::endl;
    return false;
  }

  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_MULTIBAND;
  //VipsBandFormat format = VIPS_FORMAT_UCHAR;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  int nbands = 2;
  vips_copy( ti, &image,
      "format", format,
      "bands", nbands,
      "coding", coding,
      "interpretation", interpretation,
      NULL );
  g_object_unref( ti );

  return true;
}


PF::RawImage::~RawImage()
{
  if( image ) PF_UNREF( image, "RawImage::~RawImage() image" );
  if( demo_image ) PF_UNREF( demo_image, "RawImage::~RawImage() demo_image" );
  if( fast_demosaic ) delete fast_demosaic;
#ifndef NDEBUG
  std::cout<<"RawImage::~RawImage() called."<<std::endl;
#endif
  if( !(cache_file_name.empty()) )
    unlink( cache_file_name.c_str() );
  if( !(cache_file_name2.empty()) )
    unlink( cache_file_name2.c_str() );
}



void PF::RawImage::inverse33 (const double (*rgb_cam)[3], double (*cam_rgb)[3])
{
  double nom = (rgb_cam[0][2] * rgb_cam[1][1] * rgb_cam[2][0] - rgb_cam[0][1] * rgb_cam[1][2] * rgb_cam[2][0] -
      rgb_cam[0][2] * rgb_cam[1][0] * rgb_cam[2][1] + rgb_cam[0][0] * rgb_cam[1][2] * rgb_cam[2][1] +
      rgb_cam[0][1] * rgb_cam[1][0] * rgb_cam[2][2] - rgb_cam[0][0] * rgb_cam[1][1] * rgb_cam[2][2] );
  cam_rgb[0][0] = (rgb_cam[1][2] * rgb_cam[2][1] - rgb_cam[1][1] * rgb_cam[2][2]) / nom;
  cam_rgb[0][1] = -(rgb_cam[0][2] * rgb_cam[2][1] - rgb_cam[0][1] * rgb_cam[2][2]) / nom;
  cam_rgb[0][2] = (rgb_cam[0][2] * rgb_cam[1][1] - rgb_cam[0][1] * rgb_cam[1][2]) / nom;
  cam_rgb[1][0] = -(rgb_cam[1][2] * rgb_cam[2][0] - rgb_cam[1][0] * rgb_cam[2][2]) / nom;
  cam_rgb[1][1] = (rgb_cam[0][2] * rgb_cam[2][0] - rgb_cam[0][0] * rgb_cam[2][2]) / nom;
  cam_rgb[1][2] = -(rgb_cam[0][2] * rgb_cam[1][0] - rgb_cam[0][0] * rgb_cam[1][2]) / nom;
  cam_rgb[2][0] = (rgb_cam[1][1] * rgb_cam[2][0] - rgb_cam[1][0] * rgb_cam[2][1]) / nom;
  cam_rgb[2][1] = -(rgb_cam[0][1] * rgb_cam[2][0] - rgb_cam[0][0] * rgb_cam[2][1]) / nom;
  cam_rgb[2][2] = (rgb_cam[0][1] * rgb_cam[1][0] - rgb_cam[0][0] * rgb_cam[1][1]) / nom;
}


VipsImage* PF::RawImage::get_image(unsigned int& level)
{
  if( level == 0 ) {
#ifndef NDEBUG
    std::cout<<"RawImage::get_image(): checking exif_custom_data for image("<<image<<")"<<std::endl;
#endif
    if( image ) {
      GType type = vips_image_get_typeof(image, PF_META_EXIF_NAME );
      if( type ) {
        //std::cout<<"RawImage::get_image(): exif_custom_data found in image("<<image<<")"<<std::endl;
        //print_exif();
      } else std::cout<<"RawImage::get_image(): exif_custom_data not found in image("<<image<<")"<<std::endl;
    }
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
    fMaxElem = fabs( pfMatr[k * nDim + k] );
    m = k;

    for (i = k + 1; i < nDim; i++) {
      if(fMaxElem < fabs(pfMatr[i * nDim + k]) ) {
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
      printf("pfMatr[%d * %d + %d] == 0.", k, nDim, k);
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


void PF::RawImage::CA_correct_RT_old()
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
#ifndef NDEBUG
      //printf("width=%d  height=%d filters=%d\n",width,height,dcraw_data.idata.filters);
      std::cout<<"width="<<width<<"  height="<<height<<"  filters="<<dcraw_data.idata.filters<<std::endl;
#endif
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
              if(false && row<4 && col<4) printf("rawData[%d][%d](%d) = %f * %f = %f\n",row,col,c,(rawData[row][col]), mult[c], (rawData[row][col]) * mult[c]);
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

              glpfv = 0.25f * (2.0f * rgb[1][indx] + rgb[1][indx + v2] + rgb[1][indx - v2]);
              glpfh = 0.25f * (2.0f * rgb[1][indx] + rgb[1][indx + 2] + rgb[1][indx - 2]);
              rblpfv[indx >> 1] = eps + fabsf(glpfv - 0.25f * (2.0f * rgb[c][indx] + rgb[c][indx + v2] + rgb[c][indx - v2]));
              rblpfh[indx >> 1] = eps + fabsf(glpfh - 0.25f * (2.0f * rgb[c][indx] + rgb[c][indx + 2] + rgb[c][indx - 2]));
              grblpfv[indx >> 1] = glpfv + 0.25f * (2.0f * rgb[c][indx] + rgb[c][indx + v2] + rgb[c][indx - v2]);
              grblpfh[indx >> 1] = glpfh + 0.25f * (2.0f * rgb[c][indx] + rgb[c][indx + 2] + rgb[c][indx - 2]);
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
              gdiff = 0.3125f * (rgb[1][indx + TS] - rgb[1][indx - TS]) + 0.09375f * (rgb[1][indx + TS + 1] - rgb[1][indx - TS + 1] + rgb[1][indx + TS - 1] - rgb[1][indx - TS - 1]);
              deltgrb = (rgb[c][indx] - rgb[1][indx]);

              gradwt = fabsf(0.25f * rbhpfv[indx >> 1] + 0.125f * (rbhpfv[(indx >> 1) + 1] + rbhpfv[(indx >> 1) - 1]) ) * (grblpfv[(indx >> 1) - v1] + grblpfv[(indx >> 1) + v1]) / (eps + 0.1f * grblpfv[(indx >> 1) - v1] + rblpfv[(indx >> 1) - v1] + 0.1f * grblpfv[(indx >> 1) + v1] + rblpfv[(indx >> 1) + v1]);

              coeff[0][0][c] += gradwt * deltgrb * deltgrb;
              coeff[0][1][c] += gradwt * gdiff * deltgrb;
              coeff[0][2][c] += gradwt * gdiff * gdiff;
              //                  areawt[0][c]+=1;

              //horizontal
              gdiff = 0.3125f * (rgb[1][indx + 1] - rgb[1][indx - 1]) + 0.09375f * (rgb[1][indx + 1 + TS] - rgb[1][indx - 1 + TS] + rgb[1][indx + 1 - TS] - rgb[1][indx - 1 - TS]);
              deltgrb = (rgb[c][indx] - rgb[1][indx]);

              gradwt = fabsf(0.25f * rbhpfh[indx >> 1] + 0.125f * (rbhpfh[(indx >> 1) + v1] + rbhpfh[(indx >> 1) - v1]) ) * (grblpfh[(indx >> 1) - 1] + grblpfh[(indx >> 1) + 1]) / (eps + 0.1f * grblpfh[(indx >> 1) - 1] + rblpfh[(indx >> 1) - 1] + 0.1f * grblpfh[(indx >> 1) + 1] + rblpfh[(indx >> 1) + 1]);

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
                  if (false && c==0 && dir==0) printf("vblock= %d hblock= %d blockshiftsmedian= %f \n",vblock,hblock,p[4]);
                }

                if (false)
                  fprintf (stdout,_("tile vshift hshift (%d %d %4f %4f)...\n"),vblock, hblock, blockshifts[(vblock)*hblsz+hblock][c][0], blockshifts[(vblock)*hblsz+hblock][c][1]);

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
                    if (false && c==0 && dir==0) {printf("i= %d j= %d shiftmat= %f \n",i,polyord-1,shiftmat[c][dir][(polyord*i+polyord-1)]);}
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
          //#ifndef NDEBUG
          printf("CA correction parameters fitted.\n");
          //#endif
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


#undef __SSE2__
void PF::RawImage::CA_correct_RT()
{
  bool verbose = false;
  const double caautostrength = 8.0;
  // multithreaded and vectorized by Ingo Weyrich
  constexpr int ts = 192;
  constexpr int tsh = ts / 2;
  //shifts to location of vertical and diagonal neighbors
  constexpr int v1 = ts, v2 = 2 * ts, v3 = 3 * ts, v4 = 4 * ts; //, p1=-ts+1, p2=-2*ts+2, p3=-3*ts+3, m1=ts+1, m2=2*ts+2, m3=3*ts+3;

  // Test for RGB cfa
  for(int i = 0; i < 2; i++)
    for(int j = 0; j < 2; j++)
      if(FC(i, j) == 3) {
        printf("CA correction supports only RGB Colour filter arrays\n");
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
  int W = dcraw_data.sizes.width, H = dcraw_data.sizes.height;
  const int width = W + (W & 1), height = H;
  //temporary array to store simple interpolation of G
  float *Gtmp = (float (*)) malloc ((height * width) / 2 * sizeof * Gtmp);

  // temporary array to avoid race conflicts, only every second pixel needs to be saved here
  float *RawDataTmp = (float*) malloc( (height * width) * sizeof(float) / 2);

  float blockave[2][2] = {{0, 0}, {0, 0}}, blocksqave[2][2] = {{0, 0}, {0, 0}}, blockdenom[2][2] = {{0, 0}, {0, 0}}, blockvar[2][2];

  // Because we can't break parallel processing, we need a switch do handle the errors
  bool processpasstwo = true;

  constexpr int border = 8;
  constexpr int border2 = 16;

  const int vz1 = (height + border2) % (ts - border2) == 0 ? 1 : 0;
  const int hz1 = (width + border2) % (ts - border2) == 0 ? 1 : 0;
  const int vblsz = ceil((float)(height + border2) / (ts - border2) + 2 + vz1);
  const int hblsz = ceil((float)(width + border2) / (ts - border2) + 2 + hz1);

  //block CA shift values and weight assigned to block
  float* const blockwt = static_cast<float*>(calloc(vblsz * hblsz * (2 * 2 + 1), sizeof(float)));
  float (*blockshifts)[2][2] = (float (*)[2][2])(blockwt + vblsz * hblsz);

  //double fitparams[3][2][16];
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 2; j++)
      for(int k = 0; k < 16; k++)
        fitparams[i][j][k] = 0;

  //order of 2d polynomial fit (polyord), and numpar=polyord^2
  int polyord = 4, numpar = 16;

  constexpr float eps = 1e-5f, eps2 = 1e-10f; //tolerance to avoid dividing by zero

#pragma omp parallel
  {
    int progresscounter = 0;

    //direction of the CA shift in a tile
    int GRBdir[2][3];

    int shifthfloor[3], shiftvfloor[3], shifthceil[3], shiftvceil[3];

    //local quadratic fit to shift data within a tile
    float   coeff[2][3][2];
    //measured CA shift parameters for a tile
    float   CAshift[2][2];
    //polynomial fit coefficients
    //residual CA shift amount within a plaquette
    float   shifthfrac[3], shiftvfrac[3];
    //per thread data for evaluation of block CA shift variance
    float   blockavethr[2][2] = {{0, 0}, {0, 0}}, blocksqavethr[2][2] = {{0, 0}, {0, 0}}, blockdenomthr[2][2] = {{0, 0}, {0, 0}};

    // assign working space
    constexpr int buffersize = sizeof(float) * ts * ts + 8 * sizeof(float) * ts * tsh + 8 * 64 + 63;
    char *buffer = (char *) malloc(buffersize);
    char *data = (char*)( ( uintptr_t(buffer) + uintptr_t(63)) / 64 * 64);

    // shift the beginning of all arrays but the first by 64 bytes to avoid cache miss conflicts on CPUs which have <=4-way associative L1-Cache

    //rgb data in a tile
    float* rgb[3];
    rgb[0]         = (float (*)) data;
    rgb[1]         = (float (*)) (data + sizeof(float) * ts * tsh + 1 * 64);
    rgb[2]         = (float (*)) (data + sizeof(float) * (ts * ts + ts * tsh) + 2 * 64);

    //high pass filter for R/B in vertical direction
    float *rbhpfh  = (float (*)) (data + 2 * sizeof(float) * ts * ts + 3 * 64);
    //high pass filter for R/B in horizontal direction
    float *rbhpfv  = (float (*)) (data + 2 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 4 * 64);
    //low pass filter for R/B in horizontal direction
    float *rblpfh  = (float (*)) (data + 3 * sizeof(float) * ts * ts + 5 * 64);
    //low pass filter for R/B in vertical direction
    float *rblpfv  = (float (*)) (data + 3 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 6 * 64);
    //low pass filter for colour differences in horizontal direction
    float *grblpfh = (float (*)) (data + 4 * sizeof(float) * ts * ts + 7 * 64);
    //low pass filter for colour differences in vertical direction
    float *grblpfv = (float (*)) (data + 4 * sizeof(float) * ts * ts + sizeof(float) * ts * tsh + 8 * 64);
    float *grbdiff = rbhpfh; // there is no overlap in buffer usage => share
    //green interpolated to optical sample points for R/B
    float *gshift  = rbhpfv; // there is no overlap in buffer usage => share


    if (autoCA) {
#ifndef NDEBUG
      //printf("width=%d  height=%d filters=%d\n",width,height,dcraw_data.idata.filters);
      std::cout<<"width="<<width<<"  height="<<height<<"  filters="<<dcraw_data.idata.filters<<std::endl;
#endif
      // Main algorithm: Tile loop calculating correction parameters per tile
#pragma omp for collapse(2) schedule(dynamic) nowait
      for (int top = -border ; top < height; top += ts - border2)
        for (int left = -border; left < width; left += ts - border2) {
          memset(buffer, 0, buffersize);
          const int vblock = ((top + border) / (ts - border2)) + 1;
          const int hblock = ((left + border) / (ts - border2)) + 1;
          const int bottom = min(top + ts, height + border);
          const int right  = min(left + ts, width + border);
          const int rr1 = bottom - top;
          const int cc1 = right - left;
          const int rrmin = top < 0 ? border : 0;
          const int rrmax = bottom > height ? height - top : rr1;
          const int ccmin = left < 0 ? border : 0;
          const int ccmax = right > width ? width - left : cc1;

          // rgb from input CFA data
          // rgb values should be floating point numbers between 0 and 1
          // after white balance multipliers are applied

#ifdef __SSE2__
          vfloat c65535v = F2V(65535.f);
#endif

          for (int rr = rrmin; rr < rrmax; rr++) {
            int row = rr + top;
            int cc = ccmin;
            int col = cc + left;
#ifdef __SSE2__
            int c0 = FC(rr, cc);
            if(c0 == 1) {
              rgb[c0][rr * ts + cc] = rawData[row][col] / 65535.f;
              cc++;
              col++;
              c0 = FC(rr, cc);
            }
            int indx1 = rr * ts + cc;
            for (; cc < ccmax - 7; cc+=8, col+=8, indx1 += 8) {
              vfloat val1 = LVFU(rawData[row][col]) / c65535v;
              vfloat val2 = LVFU(rawData[row][col + 4]) / c65535v;
              vfloat nonGreenv = _mm_shuffle_ps(val1,val2,_MM_SHUFFLE( 2,0,2,0 ));
              STVFU(rgb[c0][indx1 >> 1], nonGreenv);
              STVFU(rgb[1][indx1], val1);
              STVFU(rgb[1][indx1 + 4], val2);
            }
#endif
for (; cc < ccmax; cc++, col++) {
  int c = FC(rr, cc);
  int indx1 = rr * ts + cc;
  rgb[c][indx1 >> ((c & 1) ^ 1)] = rawData[row][col] / 65535.f;
  if( verbose && row < 4 && col < 4) {
    printf("row=%d col=%d  rgb[%d][%d >> ((c & 1) ^ 1)]=%f / 65535.f\n",
        row, col, c, indx1, rawData[row][col]);
  }
}
          }

          // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
          //fill borders
          if (rrmin > 0) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = ccmin; cc < ccmax; cc++) {
                int c = FC(rr, cc);
                rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = rgb[c][((border2 - rr) * ts + cc) >> ((c & 1) ^ 1)];
              }
          }

          if (rrmax < rr1) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = ccmin; cc < ccmax; cc++) {
                int c = FC(rr, cc);
                rgb[c][((rrmax + rr)*ts + cc) >> ((c & 1) ^ 1)] = rawData[(height - rr - 2)][left + cc] / 65535.f;
              }
          }

          if (ccmin > 0) {
            for (int rr = rrmin; rr < rrmax; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = rgb[c][(rr * ts + border2 - cc) >> ((c & 1) ^ 1)];
              }
          }

          if (ccmax < cc1) {
            for (int rr = rrmin; rr < rrmax; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][(rr * ts + ccmax + cc) >> ((c & 1) ^ 1)] = rawData[(top + rr)][(width - cc - 2)] / 65535.f;
              }
          }

          //also, fill the image corners
          if (rrmin > 0 && ccmin > 0) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][(rr * ts + cc) >> ((c & 1) ^ 1)] = rawData[border2 - rr][border2 - cc] / 65535.f;
              }
          }

          if (rrmax < rr1 && ccmax < cc1) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][((rrmax + rr)*ts + ccmax + cc) >> ((c & 1) ^ 1)] = rawData[(height - rr - 2)][(width - cc - 2)] / 65535.f;
              }
          }

          if (rrmin > 0 && ccmax < cc1) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][(rr * ts + ccmax + cc) >> ((c & 1) ^ 1)] = rawData[(border2 - rr)][(width - cc - 2)] / 65535.f;
              }
          }

          if (rrmax < rr1 && ccmin > 0) {
            for (int rr = 0; rr < border; rr++)
              for (int cc = 0; cc < border; cc++) {
                int c = FC(rr, cc);
                rgb[c][((rrmax + rr)*ts + cc) >> ((c & 1) ^ 1)] = rawData[(height - rr - 2)][(border2 - cc)] / 65535.f;
              }
          }

          //end of border fill
          // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
          //end of initialization


#ifdef __SSE2__
          vfloat onev = F2V(1.f);
          vfloat epsv = F2V(eps);
#endif
          for (int rr = 3; rr < rr1 - 3; rr++) {
            int row = rr + top;
            int cc = 3 + (FC(rr,3) & 1);
            int indx = rr * ts + cc;
            int c = FC(rr,cc);
#ifdef __SSE2__
            for (; cc < cc1 - 9; cc+=8, indx+=8) {
              //compute directional weights using image gradients
              vfloat rgb1mv1v = LC2VFU(rgb[1][indx - v1]);
              vfloat rgb1pv1v = LC2VFU(rgb[1][indx + v1]);
              vfloat rgbcv = LVFU(rgb[c][indx >> 1]);
              vfloat temp1v = epsv + vabsf(rgb1mv1v - rgb1pv1v);
              vfloat wtuv = onev / SQRV(temp1v + vabsf(rgbcv - LVFU(rgb[c][(indx - v2) >> 1])) + vabsf(rgb1mv1v - LC2VFU(rgb[1][indx - v3])));
              vfloat wtdv = onev / SQRV(temp1v + vabsf(rgbcv - LVFU(rgb[c][(indx + v2) >> 1])) + vabsf(rgb1pv1v - LC2VFU(rgb[1][indx + v3])));
              vfloat rgb1m1v = LC2VFU(rgb[1][indx - 1]);
              vfloat rgb1p1v = LC2VFU(rgb[1][indx + 1]);
              vfloat temp2v = epsv + vabsf(rgb1m1v - rgb1p1v);
              vfloat wtlv = onev / SQRV(temp2v + vabsf(rgbcv - LVFU(rgb[c][(indx - 2) >> 1])) + vabsf(rgb1m1v - LC2VFU(rgb[1][indx - 3])));
              vfloat wtrv = onev / SQRV(temp2v + vabsf(rgbcv - LVFU(rgb[c][(indx + 2) >> 1])) + vabsf(rgb1p1v - LC2VFU(rgb[1][indx + 3])));

              //store in rgb array the interpolated G value at R/B grid points using directional weighted average
              STC2VFU(rgb[1][indx], (wtuv * rgb1mv1v + wtdv * rgb1pv1v + wtlv * rgb1m1v + wtrv * rgb1p1v) / (wtuv + wtdv + wtlv + wtrv));
            }

#endif
            for (; cc < cc1 - 3; cc+=2, indx+=2) {
              //compute directional weights using image gradients
              float wtu = 1.f / SQR(eps + fabsf(rgb[1][indx + v1] - rgb[1][indx - v1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx - v2) >> 1]) + fabsf(rgb[1][indx - v1] - rgb[1][indx - v3]));
              float wtd = 1.f / SQR(eps + fabsf(rgb[1][indx - v1] - rgb[1][indx + v1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx + v2) >> 1]) + fabsf(rgb[1][indx + v1] - rgb[1][indx + v3]));
              float wtl = 1.f / SQR(eps + fabsf(rgb[1][indx + 1] - rgb[1][indx - 1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx - 2) >> 1]) + fabsf(rgb[1][indx - 1] - rgb[1][indx - 3]));
              float wtr = 1.f / SQR(eps + fabsf(rgb[1][indx - 1] - rgb[1][indx + 1]) + fabsf(rgb[c][indx >> 1] - rgb[c][(indx + 2) >> 1]) + fabsf(rgb[1][indx + 1] - rgb[1][indx + 3]));

              //store in rgb array the interpolated G value at R/B grid points using directional weighted average
              rgb[1][indx] = (wtu * rgb[1][indx - v1] + wtd * rgb[1][indx + v1] + wtl * rgb[1][indx - 1] + wtr * rgb[1][indx + 1]) / (wtu + wtd + wtl + wtr);
              if( verbose && top<0 && left<0 && rr<8 && cc<8 ) {
                printf("rr=%d cc=%d  wtu=%f wtd=%f wtl=%f wtr=%f\n",
                    rr, cc, wtu, wtd, wtl, wtr);
              }
            }

            if (row > -1 && row < height) {
              int offset = (FC(row,max(left + 3, 0)) & 1);
              int col = max(left + 3, 0) + offset;
              int indx = rr * ts + 3 - (left < 0 ? (left+3) : 0) + offset;
#ifdef __SSE2__
              for(; col < min(cc1 + left - 3, width) - 7; col+=8, indx+=8) {
                STVFU(Gtmp[(row * width + col) >> 1], LC2VFU(rgb[1][indx]));
              }
#endif
              for(; col < min(cc1 + left - 3, width); col+=2, indx+=2) {
                Gtmp[(row * width + col) >> 1] = rgb[1][indx];
              }
            }

          }
          //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#ifdef __SSE2__
          vfloat zd25v = F2V(0.25f);
#endif
          for (int rr = 4; rr < rr1 - 4; rr++) {
            int cc = 4 + (FC(rr, 2) & 1);
            int indx = rr * ts + cc;
            int c = FC(rr, cc);
#ifdef __SSE2__
            for (; cc < cc1 - 10; cc += 8, indx += 8) {
              vfloat rgb1v = LC2VFU(rgb[1][indx]);
              vfloat rgbcv = LVFU(rgb[c][indx >> 1]);
              vfloat rgb1mv4 = LC2VFU(rgb[1][indx - v4]);
              vfloat rgb1pv4 = LC2VFU(rgb[1][indx + v4]);
              vfloat temp1v = vabsf(vabsf((rgb1v - rgbcv) - (rgb1pv4 - LVFU(rgb[c][(indx + v4) >> 1]))) +
                  vabsf(rgb1mv4 - LVFU(rgb[c][(indx - v4) >> 1]) - rgb1v + rgbcv) -
                  vabsf(rgb1mv4 - LVFU(rgb[c][(indx - v4) >> 1]) - rgb1pv4 + LVFU(rgb[c][(indx + v4) >> 1])));
              STVFU(rbhpfv[indx >> 1], temp1v);
              vfloat rgb1m4 = LC2VFU(rgb[1][indx - 4]);
              vfloat rgb1p4 = LC2VFU(rgb[1][indx + 4]);
              vfloat temp2v = vabsf(vabsf((rgb1v - rgbcv) - (rgb1p4 - LVFU(rgb[c][(indx + 4) >> 1]))) +
                  vabsf(rgb1m4 - LVFU(rgb[c][(indx - 4) >> 1]) - rgb1v + rgbcv) -
                  vabsf(rgb1m4 - LVFU(rgb[c][(indx - 4) >> 1]) - rgb1p4 + LVFU(rgb[c][(indx + 4) >> 1])));
              STVFU(rbhpfh[indx >> 1], temp2v);

              //low and high pass 1D filters of G in vertical/horizontal directions
              rgb1v = vmul2f(rgb1v);
              vfloat glpfvv = (rgb1v + LC2VFU(rgb[1][indx + v2]) + LC2VFU(rgb[1][indx - v2]));
              vfloat glpfhv = (rgb1v + LC2VFU(rgb[1][indx + 2]) + LC2VFU(rgb[1][indx - 2]));
              rgbcv = vmul2f(rgbcv);
              STVFU(rblpfv[indx >> 1], zd25v * vabsf(glpfvv - (rgbcv + LVFU(rgb[c][(indx + v2) >> 1]) + LVFU(rgb[c][(indx - v2) >> 1]))));
              STVFU(rblpfh[indx >> 1], zd25v * vabsf(glpfhv - (rgbcv + LVFU(rgb[c][(indx + 2) >> 1]) + LVFU(rgb[c][(indx - 2) >> 1]))));
              STVFU(grblpfv[indx >> 1], zd25v * (glpfvv + (rgbcv + LVFU(rgb[c][(indx + v2) >> 1]) + LVFU(rgb[c][(indx - v2) >> 1]))));
              STVFU(grblpfh[indx >> 1], zd25v * (glpfhv + (rgbcv + LVFU(rgb[c][(indx + 2) >> 1]) + LVFU(rgb[c][(indx - 2) >> 1]))));
            }

#endif
            for (; cc < cc1 - 4; cc += 2, indx += 2) {
              rbhpfv[indx >> 1] = fabsf(fabsf((rgb[1][indx] - rgb[c][indx >> 1]) - (rgb[1][indx + v4] - rgb[c][(indx + v4) >> 1])) +
                  fabsf((rgb[1][indx - v4] - rgb[c][(indx - v4) >> 1]) - (rgb[1][indx] - rgb[c][indx >> 1])) -
                  fabsf((rgb[1][indx - v4] - rgb[c][(indx - v4) >> 1]) - (rgb[1][indx + v4] - rgb[c][(indx + v4) >> 1])));
              rbhpfh[indx >> 1] = fabsf(fabsf((rgb[1][indx] - rgb[c][indx >> 1]) - (rgb[1][indx + 4] - rgb[c][(indx + 4) >> 1])) +
                  fabsf((rgb[1][indx - 4] - rgb[c][(indx - 4) >> 1]) - (rgb[1][indx] - rgb[c][indx >> 1])) -
                  fabsf((rgb[1][indx - 4] - rgb[c][(indx - 4) >> 1]) - (rgb[1][indx + 4] - rgb[c][(indx + 4) >> 1])));

              //low and high pass 1D filters of G in vertical/horizontal directions
              float glpfv = (2.f * rgb[1][indx] + rgb[1][indx + v2] + rgb[1][indx - v2]);
              float glpfh = (2.f * rgb[1][indx] + rgb[1][indx + 2] + rgb[1][indx - 2]);
              rblpfv[indx >> 1] = 0.25f * fabsf(glpfv - (2.f * rgb[c][indx >> 1] + rgb[c][(indx + v2) >> 1] + rgb[c][(indx - v2) >> 1]));
              rblpfh[indx >> 1] = 0.25f * fabsf(glpfh - (2.f * rgb[c][indx >> 1] + rgb[c][(indx + 2) >> 1] + rgb[c][(indx - 2) >> 1]));
              grblpfv[indx >> 1] = 0.25f * (glpfv + (2.f * rgb[c][indx >> 1] + rgb[c][(indx + v2) >> 1] + rgb[c][(indx - v2) >> 1]));
              grblpfh[indx >> 1] = 0.25f * (glpfh + (2.f * rgb[c][indx >> 1] + rgb[c][(indx + 2) >> 1] + rgb[c][(indx - 2) >> 1]));
            }
          }

          for (int dir = 0; dir < 2; dir++) {
            for (int k = 0; k < 3; k++) {
              for (int c = 0; c < 2; c++) {
                coeff[dir][k][c] = 0;
              }
            }
          }

#ifdef __SSE2__
          vfloat zd3v = F2V(0.3f);
          vfloat zd1v = F2V(0.1f);
          vfloat zd5v = F2V(0.5f);
#endif

          // along line segments, find the point along each segment that minimizes the colour variance
          // averaged over the tile; evaluate for up/down and left/right away from R/B grid point
          for (int rr = 8; rr < rr1 - 8; rr++) {
            int cc = 8 + (FC(rr, 2) & 1);
            int indx = rr * ts + cc;
            int c = FC(rr, cc);
#ifdef __SSE2__
            vfloat coeff00v = ZEROV;
            vfloat coeff01v = ZEROV;
            vfloat coeff02v = ZEROV;
            vfloat coeff10v = ZEROV;
            vfloat coeff11v = ZEROV;
            vfloat coeff12v = ZEROV;
            for (; cc < cc1 - 14; cc += 8, indx += 8) {

              //in linear interpolation, colour differences are a quadratic function of interpolation position;
              //solve for the interpolation position that minimizes colour difference variance over the tile

              //vertical
              vfloat temp1 = zd3v * (LC2VFU(rgb[1][indx + ts + 1]) - LC2VFU(rgb[1][indx - ts - 1]));
              vfloat temp2 = zd3v * (LC2VFU(rgb[1][indx - ts + 1]) - LC2VFU(rgb[1][indx + ts - 1]));
              vfloat gdiffvv = (LC2VFU(rgb[1][indx + ts]) - LC2VFU(rgb[1][indx - ts])) + (temp1 - temp2);
              vfloat deltgrbv = LVFU(rgb[c][indx >> 1]) - LC2VFU(rgb[1][indx]);

              vfloat gradwtvv = (LVFU(rbhpfv[indx >> 1]) + zd5v * (LVFU(rbhpfv[(indx >> 1) + 1]) + LVFU(rbhpfv[(indx >> 1) - 1]))) * (LVFU(grblpfv[(indx >> 1) - v1]) + LVFU(grblpfv[(indx >> 1) + v1])) / (epsv + zd1v * (LVFU(grblpfv[(indx >> 1) - v1]) + LVFU(grblpfv[(indx >> 1) + v1])) + LVFU(rblpfv[(indx >> 1) - v1]) + LVFU(rblpfv[(indx >> 1) + v1]));

              coeff00v += gradwtvv * deltgrbv * deltgrbv;
              coeff01v += gradwtvv * gdiffvv * deltgrbv;
              coeff02v += gradwtvv * gdiffvv * gdiffvv;

              //horizontal
              vfloat gdiffhv = (LC2VFU(rgb[1][indx + 1]) - LC2VFU(rgb[1][indx - 1])) + (temp1 + temp2);

              vfloat gradwthv = (LVFU(rbhpfh[indx >> 1]) + zd5v * (LVFU(rbhpfh[(indx >> 1) + v1]) + LVFU(rbhpfh[(indx >> 1) - v1]))) * (LVFU(grblpfh[(indx >> 1) - 1]) + LVFU(grblpfh[(indx >> 1) + 1])) / (epsv + zd1v * (LVFU(grblpfh[(indx >> 1) - 1]) + LVFU(grblpfh[(indx >> 1) + 1])) + LVFU(rblpfh[(indx >> 1) - 1]) + LVFU(rblpfh[(indx >> 1) + 1]));

              coeff10v += gradwthv * deltgrbv * deltgrbv;
              coeff11v += gradwthv * gdiffhv * deltgrbv;
              coeff12v += gradwthv * gdiffhv * gdiffhv;
            }

            coeff[0][0][c>>1] += vhadd(coeff00v);
            coeff[0][1][c>>1] += vhadd(coeff01v);
            coeff[0][2][c>>1] += vhadd(coeff02v);
            coeff[1][0][c>>1] += vhadd(coeff10v);
            coeff[1][1][c>>1] += vhadd(coeff11v);
            coeff[1][2][c>>1] += vhadd(coeff12v);

#endif
            for (; cc < cc1 - 8; cc += 2, indx += 2) {

              //in linear interpolation, colour differences are a quadratic function of interpolation position;
              //solve for the interpolation position that minimizes colour difference variance over the tile

              //vertical
              float gdiff = (rgb[1][indx + ts] - rgb[1][indx - ts]) + 0.3f * (rgb[1][indx + ts + 1] - rgb[1][indx - ts + 1] + rgb[1][indx + ts - 1] - rgb[1][indx - ts - 1]);
              float deltgrb = (rgb[c][indx >> 1] - rgb[1][indx]);

              float gradwt = (rbhpfv[indx >> 1] + 0.5f * (rbhpfv[(indx >> 1) + 1] + rbhpfv[(indx >> 1) - 1]) ) * (grblpfv[(indx >> 1) - v1] + grblpfv[(indx >> 1) + v1]) / (eps + 0.1f * (grblpfv[(indx >> 1) - v1] + grblpfv[(indx >> 1) + v1]) + rblpfv[(indx >> 1) - v1] + rblpfv[(indx >> 1) + v1]);

              coeff[0][0][c>>1] += gradwt * deltgrb * deltgrb;
              coeff[0][1][c>>1] += gradwt * gdiff * deltgrb;
              coeff[0][2][c>>1] += gradwt * gdiff * gdiff;

              //horizontal
              gdiff = (rgb[1][indx + 1] - rgb[1][indx - 1]) + 0.3f * (rgb[1][indx + 1 + ts] - rgb[1][indx - 1 + ts] + rgb[1][indx + 1 - ts] - rgb[1][indx - 1 - ts]);

              gradwt = (rbhpfh[indx >> 1] + 0.5f * (rbhpfh[(indx >> 1) + v1] + rbhpfh[(indx >> 1) - v1]) ) * (grblpfh[(indx >> 1) - 1] + grblpfh[(indx >> 1) + 1]) / (eps + 0.1f * (grblpfh[(indx >> 1) - 1] + grblpfh[(indx >> 1) + 1]) + rblpfh[(indx >> 1) - 1] + rblpfh[(indx >> 1) + 1]);

              coeff[1][0][c>>1] += gradwt * deltgrb * deltgrb;
              coeff[1][1][c>>1] += gradwt * gdiff * deltgrb;
              coeff[1][2][c>>1] += gradwt * gdiff * gdiff;

              //  In Mathematica,
              //  f[x_]=Expand[Total[Flatten[
              //  ((1-x) RotateLeft[Gint,shift1]+x RotateLeft[Gint,shift2]-cfapad)^2[[dv;;-1;;2,dh;;-1;;2]]]]];
              //  extremum = -.5Coefficient[f[x],x]/Coefficient[f[x],x^2]
            }
          }

          for (int dir = 0; dir < 2; dir++) {
            for (int k = 0; k < 3; k++) {
              for (int c = 0; c < 2; c++) {
                coeff[dir][k][c] *= 0.25f;
                if(k == 1) {
                  coeff[dir][k][c] *= 0.3125f;
                } else if(k == 2) {
                  coeff[dir][k][c] *= SQR(0.3125f);
                }
              }
            }
          }

          for (int c = 0; c < 2; c++) {
            for (int dir = 0; dir < 2; dir++) { // vert/hor

              // CAshift[dir][c] are the locations
              // that minimize colour difference variances;
              // This is the approximate _optical_ location of the R/B pixels
              if (coeff[dir][2][c] > eps2) {
                CAshift[dir][c] = coeff[dir][1][c] / coeff[dir][2][c];
                blockwt[vblock * hblsz + hblock] = coeff[dir][2][c] / (eps + coeff[dir][0][c]) ;
              } else {
                CAshift[dir][c] = 17.0;
                blockwt[vblock * hblsz + hblock] = 0;
              }

              //data structure = CAshift[vert/hor][colour]
              //dir : 0=vert, 1=hor

              //offset gives NW corner of square containing the min; dir : 0=vert, 1=hor
              if (fabsf(CAshift[dir][c]) < 2.0f) {
                blockavethr[dir][c] += CAshift[dir][c];
                blocksqavethr[dir][c] += SQR(CAshift[dir][c]);
                blockdenomthr[dir][c] += 1;
              }
              //evaluate the shifts to the location that minimizes CA within the tile
              blockshifts[vblock * hblsz + hblock][c][dir] = CAshift[dir][c]; //vert/hor CA shift for R/B

            }//vert/hor
          }//colour

          /*if(plistener) {
                        progresscounter++;

                        if(progresscounter % 8 == 0)
                            #pragma omp critical (cadetectpass1)
                        {
                            progress += (double)(8.0 * (ts - border2) * (ts - border2)) / (2 * height * width);

                            if (progress > 1.0) {
                                progress = 1.0;
                            }

                            plistener->setProgress(progress);
                        }
                    }*/

        }

      //end of diagnostic pass
#pragma omp critical (cadetectpass2)
      {
        for (int dir = 0; dir < 2; dir++)
          for (int c = 0; c < 2; c++) {
            blockdenom[dir][c] += blockdenomthr[dir][c];
            blocksqave[dir][c] += blocksqavethr[dir][c];
            blockave[dir][c]   += blockavethr[dir][c];
          }
      }
#pragma omp barrier

#pragma omp single
      {
        for (int dir = 0; dir < 2; dir++)
          for (int c = 0; c < 2; c++) {
            if (blockdenom[dir][c]) {
              blockvar[dir][c] = blocksqave[dir][c] / blockdenom[dir][c] - SQR(blockave[dir][c] / blockdenom[dir][c]);
            } else {
              processpasstwo = false;
              printf ("blockdenom vanishes \n");
              break;
            }
          }

        if( verbose )
          printf ("tile variances %f %f %f %f \n",blockvar[0][0],blockvar[1][0],blockvar[0][1],blockvar[1][1] );
        // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        //now prepare for CA correction pass
        //first, fill border blocks of blockshift array
        if(processpasstwo) {
          for (int vblock = 1; vblock < vblsz - 1; vblock++) { //left and right sides
            for (int c = 0; c < 2; c++) {
              for (int i = 0; i < 2; i++) {
                blockshifts[vblock * hblsz][c][i] = blockshifts[(vblock) * hblsz + 2][c][i];
                blockshifts[vblock * hblsz + hblsz - 1][c][i] = blockshifts[(vblock) * hblsz + hblsz - 3][c][i];
              }
            }
          }

          for (int hblock = 0; hblock < hblsz; hblock++) { //top and bottom sides
            for (int c = 0; c < 2; c++) {
              for (int i = 0; i < 2; i++) {
                blockshifts[hblock][c][i] = blockshifts[2 * hblsz + hblock][c][i];
                blockshifts[(vblsz - 1)*hblsz + hblock][c][i] = blockshifts[(vblsz - 3) * hblsz + hblock][c][i];
              }
            }
          }

          //end of filling border pixels of blockshift array

          //initialize fit arrays
          double polymat[2][2][256], shiftmat[2][2][16];

          for (int i = 0; i < 256; i++) {
            polymat[0][0][i] = polymat[0][1][i] = polymat[1][0][i] = polymat[1][1][i] = 0;
          }

          for (int i = 0; i < 16; i++) {
            shiftmat[0][0][i] = shiftmat[0][1][i] = shiftmat[1][0][i] = shiftmat[1][1][i] = 0;
          }

          int numblox[2] = {0, 0};

          for (int vblock = 1; vblock < vblsz - 1; vblock++)
            for (int hblock = 1; hblock < hblsz - 1; hblock++) {
              // block 3x3 median of blockshifts for robustness
              for (int c = 0; c < 2; c ++) {
                float bstemp[2];
                for (int dir = 0; dir < 2; dir++) {
                  //temporary storage for median filter
                  const std::array<float, 9> p = {
                      blockshifts[(vblock - 1) * hblsz + hblock - 1][c][dir],
                      blockshifts[(vblock - 1) * hblsz + hblock][c][dir],
                      blockshifts[(vblock - 1) * hblsz + hblock + 1][c][dir],
                      blockshifts[(vblock) * hblsz + hblock - 1][c][dir],
                      blockshifts[(vblock) * hblsz + hblock][c][dir],
                      blockshifts[(vblock) * hblsz + hblock + 1][c][dir],
                      blockshifts[(vblock + 1) * hblsz + hblock - 1][c][dir],
                      blockshifts[(vblock + 1) * hblsz + hblock][c][dir],
                      blockshifts[(vblock + 1) * hblsz + hblock + 1][c][dir]
                  };
                  bstemp[dir] = median(p);
                  if (verbose && c==0 && dir==0) printf("vblock= %d hblock= %d blockshiftsmedian= %f \n",vblock,hblock,p[4]);
                }

                if (verbose)
                  fprintf (stdout,_("tile vshift hshift (%d %d %4f %4f)...\n"),vblock, hblock, blockshifts[(vblock)*hblsz+hblock][c][0], blockshifts[(vblock)*hblsz+hblock][c][1]);

                //now prepare coefficient matrix; use only data points within caautostrength/2 std devs of zero
                if (SQR(bstemp[0]) > caautostrength * blockvar[0][c] || SQR(bstemp[1]) > caautostrength * blockvar[1][c]) {
                  continue;
                }

                numblox[c]++;

                for (int dir = 0; dir < 2; dir++) {
                  double powVblockInit = 1.0;
                  for (int i = 0; i < polyord; i++) {
                    double powHblockInit = 1.0;
                    for (int j = 0; j < polyord; j++) {
                      double powVblock = powVblockInit;
                      for (int m = 0; m < polyord; m++) {
                        double powHblock = powHblockInit;
                        for (int n = 0; n < polyord; n++) {
                          polymat[c][dir][numpar * (polyord * i + j) + (polyord * m + n)] += powVblock * powHblock * blockwt[vblock * hblsz + hblock];
                          powHblock *= hblock;
                        }
                        powVblock *= vblock;
                      }
                      shiftmat[c][dir][(polyord * i + j)] += powVblockInit * powHblockInit * bstemp[dir] * blockwt[vblock * hblsz + hblock];
                      powHblockInit *= hblock;
                    }
                    powVblockInit *= vblock;
                    if (verbose && c==0 && dir==0) {printf("i= %d j= %d shiftmat= %f \n",i,polyord-1,shiftmat[c][dir][(polyord*i+polyord-1)]);}
                  }//monomials
                }//dir
              }//c
            }//blocks

          numblox[1] = min(numblox[0], numblox[1]);

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
            for (int c = 0; c < 2; c++)
              for (int dir = 0; dir < 2; dir++) {
                if(verbose)
                  printf("numpar=%d c=%d dir=%d  polymat[c][dir][0]=%f shiftmat[c][dir][0]=%f\n",
                      numpar, c, dir, polymat[c][dir][0], shiftmat[c][dir][0]);
                if (LinEqSolve(numpar, polymat[c][dir], shiftmat[c][dir], fitparams[c][dir])) {
                  printf("CA correction pass failed -- can't solve linear equations for colour %d direction %d...\n", c, dir);
                  processpasstwo = false;
                }
              }
          //#ifndef NDEBUG
          if(verbose) printf("CA correction parameters fitted.\n");
          //#endif

        }

        //fitparams[polyord*i+j] gives the coefficients of (vblock^i hblock^j) in a polynomial fit for i,j<=4
      }
      //end of initialization for CA correction pass
      //only executed if autoCA is true
    }


    // clean up
    free(buffer);


  }

  free(Gtmp);
  //free(buffer1);
  free(RawDataTmp);

  //if(plistener) {
  //    plistener->setProgress(1.0);
  //}
}



std::map<std::string, PF::RawImage*> PF::raw_images;


