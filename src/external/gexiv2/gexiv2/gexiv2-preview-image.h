/*
 * gexiv2-preview-image.h
 *
 * Author(s)
 * 	Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_PREVIEW_IMAGE_H__
#define __GEXIV2_PREVIEW_IMAGE_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GEXIV2_TYPE_PREVIEW_IMAGE \
	(gexiv2_preview_image_get_type ())
	
#define GEXIV2_PREVIEW_IMAGE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEXIV2_TYPE_PREVIEW_IMAGE, GExiv2PreviewImage))
	
#define GEXIV2_IS_PREVIEW_IMAGE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEXIV2_TYPE_PREVIEW_IMAGE))
	
#define GEXIV2_PREVIEW_IMAGE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GEXIV2_TYPE_PREVIEW_IMAGE, GExiv2PreviewImageClass))
	
#define GEXIV2_IS_PREVIEW_IMAGE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEXIV2_TYPE_PREVIEW_IMAGE))
	
#define GEXIV2_PREVIEW_IMAGE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEXIV2_TYPE_PREVIEW_IMAGE, GExiv2PreviewImageClass))
	
#define GEXIV2_PREVIEW_IMAGE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GEXIV2_TYPE_PREVIEW_IMAGE, GExiv2PreviewImagePrivate))


typedef struct _GExiv2PreviewImage			GExiv2PreviewImage;
typedef struct _GExiv2PreviewImageClass		GExiv2PreviewImageClass;
typedef struct _GExiv2PreviewImagePrivate	GExiv2PreviewImagePrivate;

struct _GExiv2PreviewImage
{
	GObject parent_instance;

	/*< private >*/
	GExiv2PreviewImagePrivate *priv;
};

struct _GExiv2PreviewImageClass
{
	GObjectClass parent_class;
};

/* basic functions */

GType 			gexiv2_preview_image_get_type			(void);

/**
 * gexiv2_preview_image_free:
 *
 * Releases the preview image and all associated memory.
 */
void			gexiv2_preview_image_free				(GExiv2PreviewImage *self);


/* preview image properties */

/**
 * gexiv2_preview_image_get_data:
 * @size: (out) (skip): The size of the buffer holding the data
 *
 * Returns: (transfer none) (array length=size): The raw image data
 */
const guint8*	gexiv2_preview_image_get_data			(GExiv2PreviewImage *self, guint32 *size);

/**
 * gexiv2_preview_image_get_mime_type:
 *
 * Returns: (transfer-none): The preview image's MIME type.
 */
const gchar*	gexiv2_preview_image_get_mime_type		(GExiv2PreviewImage *self);

/**
 * gexiv2_preview_image_get_extension:
 *
 * Returns: (transfer-none): The preview image's recommended file extension.
 */
const gchar*	gexiv2_preview_image_get_extension		(GExiv2PreviewImage *self);

/**
 * gexiv2_preview_image_get_width:
 *
 * Returns: The preview image's display width in pixels.
 */
guint32			gexiv2_preview_image_get_width			(GExiv2PreviewImage *self);

/**
 * gexiv2_preview_image_get_height:
 *
 * Returns: The preview image's display height in pixels.
 */
guint32			gexiv2_preview_image_get_height			(GExiv2PreviewImage *self);

/**
 * gexiv2_preview_image_write_file:
 * @path: (in): The file path to write the preview image to.
 *
 * Returns: The number of bytes written to the file.
 */
glong			gexiv2_preview_image_write_file			(GExiv2PreviewImage *self, const gchar *path);


G_END_DECLS

#endif /* __GEXIV2_PREVIEW_IMAGE_H__ */

