/* -*- C++ -*-
 * File: unprocessed_raw.cpp
 * Copyright 2009-2013 LibRaw LLC (info@libraw.org)
 * Created: Fri Jan 02, 2009
 *
 * LibRaw sample
 * Generates unprocessed raw image: with masked pixels and without black subtraction
 *

LibRaw is free software; you can redistribute it and/or modify
it under the terms of the one of three licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

3. LibRaw Software License 27032010
   (See file LICENSE.LibRaw.pdf provided in LibRaw distribution archive for details).

 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <sys/utime.h>
#include <winsock2.h>
#endif

#include "libraw/libraw.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#if !(LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,14))
#error This code is for LibRaw 0.14+ only
#endif

int main(int ac, char *av[])
{
  int  i, ret;
  bool verbose = true;
  LibRaw RawProcessor;
#define S RawProcessor.imgdata.sizes
#define OUT RawProcessor.imgdata.params

  if( (ret = RawProcessor.open_file(av[1])) != LIBRAW_SUCCESS)
    {
      fprintf(stderr,"Cannot open %s: %s\n",av[1],libraw_strerror(ret));
      return 1; // no recycle b/c open file will recycle itself
    }
  if(verbose)
    {
      printf("Image size: %dx%d\nRaw size: %dx%d\n",S.width,S.height,S.raw_width,S.raw_height);
      printf("Margins: top=%d, left=%d\n",
	     S.top_margin,S.left_margin);
    }
    
  if( (ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS)
    {
      fprintf(stderr,"Cannot unpack %s: %s\n",av[1],libraw_strerror(ret));
      return 1;
    }

  if(verbose)
    printf("Unpacked....\n");
    
  libraw_decoder_info_t decoder_info;
  RawProcessor.get_decoder_info(&decoder_info);
  if(!(decoder_info.decoder_flags & LIBRAW_DECODER_FLATFIELD))
    {
      printf("Only Bayer-pattern RAW files supported, sorry....\n");
      return 1;
    }

            
  int row, col;
  for(row=0;row<RawProcessor.imgdata.sizes.iheight;row++) {
    for(col=0;col<RawProcessor.imgdata.sizes.iwidth;col++) {
      ushort val = RawProcessor.imgdata.rawdata.raw_image[row*RawProcessor.imgdata.sizes.iwidth+col];
      int color = RawProcessor.COLOR(row,col);
      if( col < 8 )
	printf("row=%d  col=%d  color=%d\n",row,col,color);
    }
  }
  return 0;
}
