/*
 * gexiv2-preview-image-private.h
 *
 * Author(s)
 * Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_PREVIEW_IMAGE_PRIVATE_H__
#define __GEXIV2_PREVIEW_IMAGE_PRIVATE_H__

#include <gexiv2/gexiv2-preview-image.h>
#include <exiv2/preview.hpp>

G_BEGIN_DECLS

struct _GExiv2PreviewImagePrivate
{
    Exiv2::PreviewImage *image;
    gchar *mime_type;
    gchar *extension;
};

GExiv2PreviewImage* gexiv2_preview_image_new (Exiv2::PreviewManager *manager, 
   const Exiv2::PreviewProperties &props);

G_END_DECLS

#endif /* __GEXIV2_PREVIEW_IMAGE_PRIVATE_H__ */
