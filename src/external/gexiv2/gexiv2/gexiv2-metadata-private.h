/*
 * gexiv2-metadata-private.h
 *
 * Author(s)
 *  Mike Gemuende <mike@gemuende.de>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_METADATA_PRIVATE_H__
#define __GEXIV2_METADATA_PRIVATE_H__

#include <gexiv2/gexiv2-metadata.h>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/xmp.hpp>
#include <exiv2/preview.hpp>

G_BEGIN_DECLS

struct _GExiv2MetadataPrivate
{
    Exiv2::Image::AutoPtr image;
    gchar* comment;
    gchar* mime_type;
    gint pixel_width;
    gint pixel_height;
    gboolean supports_exif;
    gboolean supports_xmp;
    gboolean supports_iptc;
    Exiv2::PreviewManager *preview_manager;
    GExiv2PreviewProperties **preview_properties;
};

#define LOG_ERROR(e) \
    g_warning("%s", e.what());

/* private EXIF functions */

gboolean		gexiv2_metadata_has_exif_tag		(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_clear_exif_tag		(GExiv2Metadata *self, const gchar* tag);
gchar*			gexiv2_metadata_get_exif_tag_string	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_exif_tag_string	(GExiv2Metadata *self, const gchar* tag, const gchar* value);
gchar*			gexiv2_metadata_get_exif_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag);
glong			gexiv2_metadata_get_exif_tag_long	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_exif_tag_long	(GExiv2Metadata *self, const gchar* tag, glong value);
gdouble			gexiv2_metadata_get_exif_tag_rational_as_double (GExiv2Metadata *self, const gchar* tag, gdouble def);
const gchar*	gexiv2_metadata_get_exif_tag_label	(const gchar* tag);
const gchar*	gexiv2_metadata_get_exif_tag_description (const gchar* tag);
const gchar*	gexiv2_metadata_get_exif_tag_type (const gchar* tag);

GBytes*			gexiv2_metadata_get_exif_tag_raw	(GExiv2Metadata *self, const gchar* tag);

/* private XMP functions */

gboolean		gexiv2_metadata_clear_xmp_tag		(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_has_xmp_tag			(GExiv2Metadata *self, const gchar* tag);
gchar*			gexiv2_metadata_get_xmp_tag_string	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_xmp_tag_string	(GExiv2Metadata *self, const gchar* tag, const gchar* value);
gchar*			gexiv2_metadata_get_xmp_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag);
glong			gexiv2_metadata_get_xmp_tag_long	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_xmp_tag_long	(GExiv2Metadata *self, const gchar* tag, glong value);
gchar**			gexiv2_metadata_get_xmp_tag_multiple (GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_xmp_tag_multiple (GExiv2Metadata *self, const gchar* tag, const gchar** values);

const gchar*	gexiv2_metadata_get_xmp_tag_label		(const gchar* tag);
const gchar*	gexiv2_metadata_get_xmp_tag_description	(const gchar* tag);
const gchar*	gexiv2_metadata_get_xmp_tag_type	(const gchar* tag);

GBytes*			gexiv2_metadata_get_xmp_tag_raw		(GExiv2Metadata *self, const gchar* tag);

/* private IPTC functions */

gboolean		gexiv2_metadata_clear_iptc_tag		(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_has_iptc_tag		(GExiv2Metadata *self, const gchar* tag);
gchar*			gexiv2_metadata_get_iptc_tag_string	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_iptc_tag_string	(GExiv2Metadata *self, const gchar* tag, const gchar* value);
gchar*			gexiv2_metadata_get_iptc_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag);
gchar**			gexiv2_metadata_get_iptc_tag_multiple	(GExiv2Metadata *self, const gchar* tag);
gboolean		gexiv2_metadata_set_iptc_tag_multiple	(GExiv2Metadata *self, const gchar* tag, const gchar** values);

const gchar*	gexiv2_metadata_get_iptc_tag_label	(const gchar* tag);
const gchar*	gexiv2_metadata_get_iptc_tag_description	(const gchar* tag);
const gchar*	gexiv2_metadata_get_iptc_tag_type	(const gchar* tag);

GBytes*			gexiv2_metadata_get_iptc_tag_raw	(GExiv2Metadata *self, const gchar* tag);

G_END_DECLS

#endif /* __GEXIV2_METADATA_PRIVATE_H__ */
