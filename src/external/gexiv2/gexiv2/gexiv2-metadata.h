/*
 * gexiv2-metadata.h
 *
 * Author(s)
 * 	Mike Gemuende <mike@gemuende.de>
 * 	Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_METADATA_H__
#define __GEXIV2_METADATA_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <gexiv2/gexiv2-managed-stream.h>
#include <gexiv2/gexiv2-preview-properties.h>
#include <gexiv2/gexiv2-preview-image.h>

G_BEGIN_DECLS

#define GEXIV2_TYPE_METADATA \
	(gexiv2_metadata_get_type ())
	
#define GEXIV2_METADATA(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEXIV2_TYPE_METADATA, GExiv2Metadata))
	
#define GEXIV2_IS_METADATA(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEXIV2_TYPE_METADATA))
	
#define GEXIV2_METADATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GEXIV2_TYPE_METADATA, GExiv2MetadataClass))
	
#define GEXIV2_IS_METADATA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEXIV2_TYPE_METADATA))
	
#define GEXIV2_METADATA_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEXIV2_TYPE_METADATA, GExiv2MetadataClass))
	
#define GEXIV2_METADATA_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GEXIV2_TYPE_METADATA, GExiv2MetadataPrivate))


/**
 * Gexiv2Orientation:
 *
 * The orientation of an image is defined as the location of it's x,y origin.  More than rotation,
 * orientation allows for every variation of rotation, flips, and mirroring to be described in
 * 3 bits of data.
 *
 * A handy primer to Orientation can be found at http://jpegclub.org/exif_orientation.html
 */
typedef enum {
	GEXIV2_ORIENTATION_MIN			= 0,
	GEXIV2_ORIENTATION_UNSPECIFIED	= 0,
	GEXIV2_ORIENTATION_NORMAL		= 1,
	GEXIV2_ORIENTATION_HFLIP		= 2,
	GEXIV2_ORIENTATION_ROT_180		= 3,
	GEXIV2_ORIENTATION_VFLIP		= 4,
	GEXIV2_ORIENTATION_ROT_90_HFLIP	= 5,
	GEXIV2_ORIENTATION_ROT_90		= 6,
	GEXIV2_ORIENTATION_ROT_90_VFLIP	= 7,
	GEXIV2_ORIENTATION_ROT_270		= 8,
	GEXIV2_ORIENTATION_MAX			= 8
} GExiv2Orientation;

typedef enum {
  GEXIV2_STRUCTURE_XA_NONE = 0,
  GEXIV2_STRUCTURE_XA_ALT  = 20,
  GEXIV2_STRUCTURE_XA_BAG  = 21,
  GEXIV2_STRUCTURE_XA_SEQ  = 22,
  GEXIV2_STRUCTURE_XA_LANG = 23
} GExiv2StructureType;

/**
 * GExiv2XmpFormatFlags:
 * Options to control the format of the serialized XMP packet
 * Taken from: exiv2/src/xmp.hpp
 *
 */
typedef enum {
  GEXIV2_OMIT_PACKET_WRAPPER   = 0x0010UL,  //!< Omit the XML packet wrapper.
  GEXIV2_READ_ONLY_PACKET      = 0x0020UL,  //!< Default is a writeable packet.
  GEXIV2_USE_COMPACT_FORMAT    = 0x0040UL,  //!< Use a compact form of RDF.
  GEXIV2_INCLUDE_THUMBNAIL_PAD = 0x0100UL,  //!< Include a padding allowance for a thumbnail image.
  GEXIV2_EXACT_PACKET_LENGTH   = 0x0200UL,  //!< The padding parameter is the overall packet length.
  GEXIV2_WRITE_ALIAS_COMMENTS  = 0x0400UL,  //!< Show aliases as XML comments.
  GEXIV2_OMIT_ALL_FORMATTING   = 0x0800UL   //!< Omit all formatting whitespace.
} GExiv2XmpFormatFlags;

typedef struct _GExiv2Metadata			GExiv2Metadata;
typedef struct _GExiv2MetadataClass		GExiv2MetadataClass;
typedef struct _GExiv2MetadataPrivate	GExiv2MetadataPrivate;

/**
 * GExiv2Metadata:
 *
 * An object holding all the Exiv2 metadata.  Previews, if present, are also available.
 *
 * As gexiv2 is only a wrapper around Exiv2, it's better to read its documentaiton to understand
 * the full scope of what it offers: http://www.exiv2.org/
 *
 * In particular, rather than providing a getter/setter method pair for every metadata value
 * available for images (of which there are thousands), Exiv2 uses a dotted addressing scheme.
 * For example, to access a photo's EXIF Orientation field, the caller passes to Exiv2
 * "Exif.Photo.Orientation".  These <em>tags</em> (in Exiv2 parlance) are key to using Exiv2 (and
 * therefore gexiv2) to its fullest.
 *
 * A full reference for all supported Exiv2 tags can be found at http://www.exiv2.org/metadata.html
 */
struct _GExiv2Metadata
{
	GObject parent_instance;

	/*< private >*/
	GExiv2MetadataPrivate *priv;
};

struct _GExiv2MetadataClass
{
	GObjectClass parent_class;
};

/* basic functions */

GType 			gexiv2_metadata_get_type			(void);

/**
 * gexiv2_metadata_new:
 *
 * Returns: (transfer full): A fully constructed #GExiv2Metadata ready to be used
 */
GExiv2Metadata* gexiv2_metadata_new					(void);

/**
 * gexiv2_metadata_free:
 *
 * Destroys the #GExiv2Metadata object and frees all associated memory.
 */
void			gexiv2_metadata_free				(GExiv2Metadata *self);

/**
 * gexiv2_metadata_open_path:
 * @path: Path to the file you want to open
 *
 * The file must be an image format supported by Exiv2.
 *
 * Returns: Boolean success indicator
 */
gboolean		gexiv2_metadata_open_path			(GExiv2Metadata *self, const gchar *path, GError **error);

/**
 * gexiv2_metadata_open_buf:
 * @data: (array length=n_data): A buffer containing the data to be read
 * @n_data: (skip): The length of the buffer
 *
 * The buffer must be an image format supported by Exiv2.
 *
 * Returns: Boolean success indicator
 */
gboolean		gexiv2_metadata_open_buf			(GExiv2Metadata *self, const guint8 *data, glong n_data, GError **error);

/**
 * gexiv2_metadata_open_stream:
 * @cb: A #ManagedStreamCallbacks struct offering stream access.
 *
 * The stream must be an image format supported by Exiv2.
 *
 * Returns: Boolean success indicator
 */
gboolean		gexiv2_metadata_open_stream			(GExiv2Metadata *self, ManagedStreamCallbacks* cb, GError **error);

/**
 * gexiv2_metadata_from_app1_segment:
 * @data: (array length=n_data): A buffer containing the data to be read
 * @n_data: (skip): The length of the buffer
 *
 * Load only an EXIF buffer, typically stored in a JPEG's APP1 segment.
 *
 * Returns: Boolean success indicator.
 */
gboolean		gexiv2_metadata_from_app1_segment	(GExiv2Metadata *self, const guint8 *data, glong n_data, GError **error);

/**
 * gexiv2_metadata_save_file:
 * @path: Path to the file you want to save to.
 *
 * Saves the metadata to the specified file by reading the file into memory,copying this object's
 * metadata into the image, then writing the image back out.
 *
 * Returns: Boolean success indicator.
 */
gboolean		gexiv2_metadata_save_file			(GExiv2Metadata *self, const gchar *path, GError **error);

/**
 * gexiv2_metadata_save_stream:
 * @cb: A #ManagedStreamCallbacks struct offering stream access.
 *
 * Saves the metadata to the stream by reading the stream into memory,copying this object's
 * metadata into the image, then writing the image as a stream back out.
 *
 * Returns: Boolean success indicator.
 */
gboolean		gexiv2_metadata_save_stream			(GExiv2Metadata *self, ManagedStreamCallbacks* cb, GError **error);

/**
 * gexiv2_metadata_has_tag:
 * @tag: Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE if the tag is present.
 */
gboolean		gexiv2_metadata_has_tag				(GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_clear_tag:
 * @tag: Exiv2 tag
 *
 * Removes the Exiv2 tag from the metadata object.
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE if the tag was present.
 */
gboolean		gexiv2_metadata_clear_tag			(GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_clear:
 *
 * Removes all tags for all domains (EXIF, IPTC, and XMP).
 */
void			gexiv2_metadata_clear				(GExiv2Metadata *self);

/**
 * gexiv2_metadata_is_exif_tag:
 * @tag: An Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE if the Exiv2 tag is for the EXIF domain.
 */
gboolean		gexiv2_metadata_is_exif_tag				(const gchar* tag);

/**
 * gexiv2_metadata_is_iptc_tag:
 * @tag: An Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE if the Exiv2 tag is for the IPTC domain.
 */
gboolean		gexiv2_metadata_is_iptc_tag				(const gchar* tag);

/**
 * gexiv2_metadata_is_xmp_tag:
 * @tag: An Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE if the Exiv2 tag is for the XMP domain.
 */
gboolean		gexiv2_metadata_is_xmp_tag				(const gchar* tag);

/**
 * gexiv2_metadata_get_tag_label:
 * @tag: An Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer none) (allow-none): The tag's label
 */
const gchar*	gexiv2_metadata_get_tag_label		(const gchar *tag);

/**
 * gexiv2_metadata_get_tag_description:
 * @tag: An Exiv2 tag
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer none) (allow-none): The tag's description
 */
const gchar*	gexiv2_metadata_get_tag_description	(const gchar *tag);

/**
 * gexiv2_metadata_get_tag_type:
 * @tag: An Exiv2 tag
 *
 * The names of the various Exiv2 tag types can be found at Exiv2::TypeId,
 * http://exiv2.org/doc/namespaceExiv2.html#5153319711f35fe81cbc13f4b852450c
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer none) (allow-none): The tag's type name.
 */
const gchar*	gexiv2_metadata_get_tag_type	(const gchar *tag);

/**
 * gexiv2_metadata_get_supports_exif:
 *
 * Returns: TRUE if the loaded image type supports writing EXIF metadata.
 */
gboolean		gexiv2_metadata_get_supports_exif	(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_supports_iptc:
 *
 * Returns: TRUE if the loaded image type supports writing IPTC metadata.
 */
gboolean		gexiv2_metadata_get_supports_iptc	(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_supports_xmp:
 *
 * Returns: TRUE if the loaded image type supports writing XMP metadata.
 */
gboolean		gexiv2_metadata_get_supports_xmp	(GExiv2Metadata *self);

/**
 * gexiv2_get_mime_type:
 *
 * Returns: (transfer none): The MIME type of the loaded image, NULL if not loaded or unknown.
 */
const gchar*	gexiv2_metadata_get_mime_type		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_pixel_width:
 *
 * Returns: The <em>actual</em> unoriented display width in pixels of the loaded image.  This may be
 * different than the width reported by various metadata tags, i.e. "Exif.Photo.PixelXDimension".
 */
gint			gexiv2_metadata_get_pixel_width		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_pixel_height:
 *
 * Returns: The <em>actual</em> unoriented display height in pixels of the loaded image.  This may
 * be different than the height reported by various metadata tags, i.e. "Exif.Photo.PixelYDimension".
 */
gint			gexiv2_metadata_get_pixel_height	(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_tag_string:
 * @tag: Exiv2 tag name
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer full) (allow-none): The tag's value as a string
 */
gchar*			gexiv2_metadata_get_tag_string		(GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_set_tag_string:
 * @tag: Exiv2 tag name
 * @value: The value to set or replace the existing value
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE on success
 */
gboolean		gexiv2_metadata_set_tag_string		(GExiv2Metadata *self, const gchar* tag, const gchar* value);

/**
 * gexiv2_metadata_set_tag_struct:
 * @tag: Exiv2 tag name
 * @type: The GExiv2StructureType specifying the type of structure
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE on success
 */
gboolean gexiv2_metadata_set_xmp_tag_struct (GExiv2Metadata *self, const gchar* tag, GExiv2StructureType type);

/**
 * gexiv2_metadata_get_tag_interpreted_string:
 * @tag: Exiv2 tag name
 *
 * An interpreted string is one fit for user display.  It may display units or use formatting
 * appropriate to the type of data the tag holds.
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer full) (allow-none): The tag's interpreted value as a string
 */
gchar*			gexiv2_metadata_get_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_get_tag_long:
 * @tag: Exiv2 tag name
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: The tag's value as a glong
 */
glong			gexiv2_metadata_get_tag_long		(GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_set_tag_long:
 * @tag: Exiv2 tag name
 * @value: The value to set or replace the existing value
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: TRUE on success
 */
gboolean		gexiv2_metadata_set_tag_long		(GExiv2Metadata *self, const gchar* tag, glong value);


/**
 * gexiv2_metadata_get_tag_multiple:
 * @tag: Exiv2 tag name
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer full) (allow-none) (array zero-terminated=1): The multiple string values of
 * the tag
 */
gchar**			gexiv2_metadata_get_tag_multiple	(GExiv2Metadata *self, const gchar* tag);

/**
 * gexiv2_metadata_set_tag_multiple:
 * @tag: Exiv2 tag name
 * @values: (array zero-terminated=1): An array of values to set or replace the existing value(s)
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer full): Boolean success value
 */
gboolean		gexiv2_metadata_set_tag_multiple	(GExiv2Metadata *self, const gchar* tag, const gchar** values);

/**
 * gexiv2_metadata_get_tag_raw:
 * @tag: Exiv2 tag name
 *
 * The Exiv2 Tag Reference can be found at http://exiv2.org/metadata.html
 *
 * Returns: (transfer full) (allow-none): The tag's raw value as a byte array
 */
GBytes*			gexiv2_metadata_get_tag_raw			(GExiv2Metadata *self, const gchar* tag);

/**
 * EXIF functions
 */

/**
 * gexiv2_metadata_has_exif:
 *
 * Returns: TRUE if EXIF metadata is present in the loaded image
 */
gboolean		gexiv2_metadata_has_exif			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_clear_exif:
 *
 * Clears all EXIF metadata from the loaded image.
 */
void			gexiv2_metadata_clear_exif			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_exif_tags:
 *
 * Returns: (transfer full) (array zero-terminated=1): A list of the available EXIF tags in the
 * loaded image
 */
gchar**			gexiv2_metadata_get_exif_tags		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_exif_tag_rational:
 * @tag: (in): The tag you want the rational value for
 * @nom: (out): The numerator
 * @den: (out): The denominator
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_exif_tag_rational (GExiv2Metadata *self, const gchar* tag, gint* nom, gint* den);

/**
 * gexiv2_metadata_set_exif_tag_rational:
 * @tag: (in): The Exiv2 tag
 * @nom: Rational numerator
 * @den: Rational denominator
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_set_exif_tag_rational (GExiv2Metadata *self, const gchar* tag, gint nom, gint den);

/**
 * gexiv2_metadata_get_exif_thumbnail:
 * @buffer: (out) (array length=size) (transfer full): Where to store the thumbnail data
 * @size: (skip): Size of the thumbnail's buffer
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_exif_thumbnail (GExiv2Metadata *self, guint8** buffer, gint* size);

/**
 * gexiv2_metadata_set_exif_thumbnail_from_file:
 * @path: (in): Path of image file
 *
 * Sets or replaces the EXIF thumbnail with the image in the file
 *
 * Returns: Boolean success value
 */
gboolean		gexiv2_metadata_set_exif_thumbnail_from_file (GExiv2Metadata *self, const gchar *path, GError **error);

/**
 * gexiv2_metadata_set_exif_thumbnail_from_buffer:
 * @buffer: (array length=size): A buffer containing thumbnail data
 * @size: (skip): Size of the thumbnail's buffer
 */
void			gexiv2_metadata_set_exif_thumbnail_from_buffer (GExiv2Metadata *self, const guint8 *buffer, gint size);

/**
 * gexiv2_metadata_erase_exif_thumbnail:
 *
 * Removes the EXIF thumbnail from the loaded image.
 */
void			gexiv2_metadata_erase_exif_thumbnail (GExiv2Metadata *self);


/**
 * XMP functions
 */

/**
 * gexiv2_metadata_has_xmp:
 *
 * Returns: TRUE if XMP metadata is present in the loaded image
 */
gboolean		gexiv2_metadata_has_xmp				(GExiv2Metadata *self);

/**
 * gexiv2_metadata_clear_xmp:
 *
 * Clears all XMP metadata from the loaded image.
 */
void			gexiv2_metadata_clear_xmp			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_generate_xmp_packet:
 *
 * Returns: (transfer full) (allow-none): Encode the XMP packet and return as a NUL-terminated string.
 */
gchar*		gexiv2_metadata_generate_xmp_packet	(GExiv2Metadata *self, GExiv2XmpFormatFlags xmp_format_flags, guint32 padding);

/**
 * gexiv2_metadata_get_xmp_packet:
 *
 * Returns: (transfer full) (allow-none): The currently-encoded XMP packet (see gexiv2_metadata_generate_xmp_packet).
 */
gchar*			gexiv2_metadata_get_xmp_packet		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_xmp_tags:
 *
 * Returns: (transfer full) (array zero-terminated=1): A list of the available XMP tags
 */
gchar**			gexiv2_metadata_get_xmp_tags		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_register_xmp_namespace:
 * @name: (in): XMP URI name (should end in /)
 * @prefix: (in): XMP namespace prefix
 *
 * Returns: (skip): Boolean success value
 */
gboolean        gexiv2_metadata_register_xmp_namespace (const gchar* name, const gchar *prefix);

/**
 * gexiv2_metadata_unregister_xmp_namespace:
 * @name: (in): XMP URI name (should end in /)
 *
 * Returns: (skip): Boolean success value
 */
gboolean        gexiv2_metadata_unregister_xmp_namespace (const gchar* name);

/**
 * gexiv2_metadata_unregister_all_xmp_namespaces:
 */
void            gexiv2_metadata_unregister_all_xmp_namespaces (void);

/* IPTC functions */

/**
 * gexiv2_metadata_has_iptc:
 *
 * Returns: TRUE if IPTC metadata is present in the loaded image
 */
gboolean		gexiv2_metadata_has_iptc			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_clear_iptc:
 *
 * Clears all IPTC metadata from the loaded image.
 */
void			gexiv2_metadata_clear_iptc			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_iptc_tags:
 *
 * Returns: (transfer full) (array zero-terminated=1): A list of the available IPTC tags
 */
gchar**			gexiv2_metadata_get_iptc_tags		(GExiv2Metadata *self);

/**
 * Composite getters/setters and helpful functions.
 */

/**
 * gexiv2_metadata_get_orientation:
 *
 * Returns: The EXIF Orientation field.
 */
GExiv2Orientation gexiv2_metadata_get_orientation 	(GExiv2Metadata *self);

/**
 * gexiv2_metadata_set_orientation:
 * @orientation: The new #Gexiv2Orientation for the image.
 *
 * The orientation must be valid and cannot be #GEXIV2_ORIENTATION_UNSPECIFIED.
 */
void			gexiv2_metadata_set_orientation		(GExiv2Metadata *self, GExiv2Orientation orientation);

/**
 * gexiv2_metadata_get_comment:
 *
 * A composite accessor that uses the first available metadata field from a list of well-known
 * locations to find the photo's comment (or description).  These fields are:
 *
 * Exif.Image.ImageDescription
 * Exif.Photo.UserComment
 * Exif.Image.XPComment
 * Iptc.Application2.Caption
 *
 * Note that Exif.Image.ImageDescription is <em>not</em> technically a description field and is
 * described in the EXIF specification as "the title of the image".  Also, it does not support
 * two-byte character codes for encoding.  However, it's still used here for legacy reasons.
 *
 * For fine-grained control, it's recommened to use Exiv2 tags directly rather than this method,
 * which is more useful for quick or casual use.
 *
 * Returns: (transfer full) (allow-none): The photo's comment field.
 */
gchar*			gexiv2_metadata_get_comment			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_set_comment:
 * @comment: Comment string to set
 *
 * This is a composite setter that will set a number of fields to the supplied value.  See
 * #gexiv2_metadata_get_comment for more informtion.
 */
void			gexiv2_metadata_set_comment			(GExiv2Metadata *self, const gchar* comment);

/**
 * gexiv2_metadata_clear_comment:
 *
 * This is a composite clear method that will clear a number of fields.  See
 * #gexiv2_metadata_get_comment for more informtion.
 */
void			gexiv2_metadata_clear_comment		(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_exposure_time:
 * @nom: (out): The numerator
 * @den: (out): The denominator
 *
 * Returns the exposure time in seconds (shutter speed, <em>not</em> date-time of exposure) as a
 * rational.  See https://en.wikipedia.org/wiki/Shutter_speed for more information.
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_exposure_time	(GExiv2Metadata *self, gint *nom, gint *den);

/**
 * gexiv2_metadata_get_fnumber:
 *
 * See https://en.wikipedia.org/wiki/F-number for more information.
 *
 * Returns: The exposure Fnumber as a gdouble, or -1.0 if tag is not present or invalid.
 */
gdouble			gexiv2_metadata_get_fnumber			(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_focal_length:
 *
 * See https://en.wikipedia.org/wiki/Flange_focal_distance for more information.
 *
 * Returns: The focal length as a gdouble, or -1.0 if tag is not present or invalid.
 */
gdouble			gexiv2_metadata_get_focal_length	(GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_iso_speed:
 *
 * See https://en.wikipedia.org/wiki/Iso_speed for more information.
 *
 * Returns: The ISO speed rating as a gint, or 0 if tag is not present or invalid.
 */
gint			gexiv2_metadata_get_iso_speed		(GExiv2Metadata *self);

/**
 * GPS functions
 */

/**
 * gexiv2_metadata_get_gps_longitude:
 * @longitude: (out): Variable to store the longitude value
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_gps_longitude			(GExiv2Metadata *self, gdouble *longitude);

/**
 * gexiv2_metadata_get_gps_latitude:
 * @latitude: (out): Variable to store the latitude value
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_gps_latitude			(GExiv2Metadata *self, gdouble *latitude);

/**
 * gexiv2_metadata_get_gps_altitude:
 * @altitude: (out): Variable to store the altitude value
 *
 * Returns: (skip): Boolean success value
 */
gboolean		gexiv2_metadata_get_gps_altitude			(GExiv2Metadata *self, gdouble *altitude);

/**
 * gexiv2_metadata_get_gps_info:
 * @longitude: (out): Storage for longitude value
 * @latitude: (out): Storage for latitude value
 * @altitude: (out): Storage for altitude value
 *
 * Returns: (skip): Boolean success value.
 */
gboolean		gexiv2_metadata_get_gps_info				(GExiv2Metadata *self, gdouble *longitude, gdouble *latitude, gdouble *altitude);

/**
 * gexiv2_metadata_set_gps_info:
 * @longitude: Longitude value to set or replace current value
 * @latitude: Latitude value to set or replace current value
 * @altitude: Altitude value to set or replace current value
 *
 * Returns: (skip): Boolean success value.
 */
gboolean		gexiv2_metadata_set_gps_info				(GExiv2Metadata *self, gdouble longitude, gdouble latitude, gdouble altitude);

/**
 * gexiv2_metadata_delete_gps_info:
 *
 * Removes all GPS metadata from the loaded image
 */
void			gexiv2_metadata_delete_gps_info			(GExiv2Metadata *self);

/**
 * Preview Manager
 */

/**
 * gexiv2_metadata_get_preview_properties:
 *
 * An image may have stored one or more previews, often of different qualities, sometimes of
 * different image formats than the containing image.  This call returns the properties of all
 * previews Exiv2 finds within the loaded image.  Use #gexiv2_metadata_get_preview_image to
 * load a particular preview into memory.
 *
 * Returns: (transfer none) (allow-none) (array zero-terminated=1): An array of
 * #GExiv2PreviewProperties instances, one for each preview present in the loaded image.
 */
GExiv2PreviewProperties** gexiv2_metadata_get_preview_properties (GExiv2Metadata *self);

/**
 * gexiv2_metadata_get_preview_image:
 * @props: A #GExiv2PreviewProperties instance
 *
 * Returns: (transfer full): A #GExiv2PreviewImage instance for the particular
 * #GExiv2PreviewProperties.
 */
GExiv2PreviewImage* gexiv2_metadata_get_preview_image		(GExiv2Metadata *self, GExiv2PreviewProperties *props);

G_END_DECLS

#endif /* __GEXIV2_METADATA_H__ */

