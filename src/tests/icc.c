/* Compile with
 
   gcc rawsave.c `pkg-config vips --cflags --libs`
 
*/
 
#include <vips/vips.h>
#include <lcms2.h>
 
// Create the ICC virtual profile for adobe rgb space
cmsHPROFILE
dt_colorspaces_create_adobergb_profile(void)
{
  cmsHPROFILE  hAdobeRGB;

  cmsCIEXYZTRIPLE Colorants =
  {
    {0.609741, 0.311111, 0.019470},
    {0.205276, 0.625671, 0.060867},
    {0.149185, 0.063217, 0.744568}
  };

  cmsCIEXYZ black = { 0, 0, 0 };
  cmsCIEXYZ D65 = { 0.95045, 1, 1.08905 };
  cmsToneCurve* transferFunction;

  // AdobeRGB's "2.2" gamma is technically defined as 2 + 51/256
  transferFunction = cmsBuildGamma(NULL, 2.19921875);

  hAdobeRGB = cmsCreateProfilePlaceholder(0);

  cmsSetProfileVersion(hAdobeRGB, 2.1);

  cmsMLU *mlu0 = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(mlu0, "en", "US", "Public Domain");
  cmsMLU *mlu1 = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(mlu1, "en", "US", "Adobe RGB (compatible)");
  cmsMLU *mlu2 = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(mlu2, "en", "US", "Darktable");
  cmsMLU *mlu3 = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(mlu3, "en", "US", "Adobe RGB");
  // this will only be displayed when the embedded profile is read by for example GIMP
  cmsWriteTag(hAdobeRGB, cmsSigCopyrightTag,          mlu0);
  cmsWriteTag(hAdobeRGB, cmsSigProfileDescriptionTag, mlu1);
  cmsWriteTag(hAdobeRGB, cmsSigDeviceMfgDescTag,      mlu2);
  cmsWriteTag(hAdobeRGB, cmsSigDeviceModelDescTag,    mlu3);
  cmsMLUfree(mlu0);
  cmsMLUfree(mlu1);
  cmsMLUfree(mlu2);
  cmsMLUfree(mlu3);

  cmsSetDeviceClass(hAdobeRGB, cmsSigDisplayClass);
  cmsSetColorSpace(hAdobeRGB, cmsSigRgbData);
  cmsSetPCS(hAdobeRGB, cmsSigXYZData);

  cmsWriteTag(hAdobeRGB, cmsSigMediaWhitePointTag, &D65);
  cmsWriteTag(hAdobeRGB, cmsSigMediaBlackPointTag, &black);

  cmsWriteTag(hAdobeRGB, cmsSigRedColorantTag, (void*) &Colorants.Red);
  cmsWriteTag(hAdobeRGB, cmsSigGreenColorantTag, (void*) &Colorants.Green);
  cmsWriteTag(hAdobeRGB, cmsSigBlueColorantTag, (void*) &Colorants.Blue);

  cmsWriteTag(hAdobeRGB, cmsSigRedTRCTag, (void*) transferFunction);
  cmsLinkTag(hAdobeRGB, cmsSigGreenTRCTag, cmsSigRedTRCTag );
  cmsLinkTag(hAdobeRGB, cmsSigBlueTRCTag, cmsSigRedTRCTag );

  return hAdobeRGB;
}


int
main( int argc, char **argv )
{
  VipsImage *image;
  VipsImage *image2;
  VipsImage *out;
 
  if( vips_init( argv[0] ) )
    vips_error_exit( "unable to init" );

  im_concurrency_set( 1 );

  if( !(image = vips_image_new_from_file( argv[1], NULL )) )
    vips_error_exit( NULL );
 
  GType type = vips_image_get_typeof(image, VIPS_META_ICC_NAME );
  if( type ) printf( "Embedded profile found after vips_image_new_from_file()\n" );
  else printf( "Embedded profile not found after vips_image_new_from_file()\n" );

  cmsHPROFILE out_profile = dt_colorspaces_create_adobergb_profile();
  if( out_profile ) {
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( out_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( out_profile, buf, &out_length);
    vips_image_set_blob( image, VIPS_META_ICC_NAME,
       (VipsCallbackFn) g_free, buf, out_length );
    //char tstr[1024];
    //cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    //printf("RawOutputPar::build(): embedded profile: %s\n",tstr);
  }

  type = vips_image_get_typeof(image, VIPS_META_ICC_NAME );
  if( type ) printf( "Embedded profile found after vips_image_set_blob()\n" );
  else printf( "Embedded profile not found after vips_image_set_blob()\n" );


  vips_image_write_to_file( image, argv[2], NULL );
  type = vips_image_get_typeof(image, VIPS_META_ICC_NAME );
  if( type ) printf( "Embedded profile found after vips_image_write_to_file()\n" );
  else printf( "Embedded profile not found after vips_image_write_to_file()\n" );

  g_object_unref( image );

  return( 0 );
}
