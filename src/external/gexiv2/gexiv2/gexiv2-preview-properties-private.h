/*
 * gexiv2-preview-properties-private.h
 *
 * Author(s)
 * Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_PREVIEW_PROPERTIES_PRIVATE_H__
#define __GEXIV2_PREVIEW_PROPERTIES_PRIVATE_H__

#include <gexiv2/gexiv2-preview-properties.h>
#include <exiv2/preview.hpp>

G_BEGIN_DECLS

struct _GExiv2PreviewPropertiesPrivate
{
  Exiv2::PreviewProperties *props;
};


GExiv2PreviewProperties* gexiv2_preview_properties_new (Exiv2::PreviewProperties &props);
void gexiv2_preview_properties_free(GExiv2PreviewProperties *self);

G_END_DECLS

#endif /* __GEXIV2_PREVIEW_PROPERTIES_PRIVATE_H__ */
