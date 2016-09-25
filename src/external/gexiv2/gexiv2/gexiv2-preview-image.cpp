/*
 * gexiv2-preview-image.cpp
 *
 * Author(s)
 * Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#include "gexiv2-preview-image.h"
#include "gexiv2-preview-image-private.h"
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS
G_DEFINE_TYPE (GExiv2PreviewImage, gexiv2_preview_image, G_TYPE_OBJECT);

static void gexiv2_preview_image_finalize (GObject *object);

static void gexiv2_preview_image_init (GExiv2PreviewImage *self) {
    self->priv = GEXIV2_PREVIEW_IMAGE_GET_PRIVATE (self);
    
    self->priv->image = NULL;
    self->priv->mime_type = NULL;
    self->priv->extension = NULL;
}

static void gexiv2_preview_image_class_init (GExiv2PreviewImageClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    
    g_type_class_add_private (klass, sizeof (GExiv2PreviewImagePrivate));
    
    gobject_class->finalize = gexiv2_preview_image_finalize;
}

static void gexiv2_preview_image_finalize (GObject *object) {
    GExiv2PreviewImage *self = GEXIV2_PREVIEW_IMAGE (object);
    
    delete self->priv->image;
    g_free(self->priv->mime_type);
    g_free(self->priv->extension);
    
    G_OBJECT_CLASS (gexiv2_preview_image_parent_class)->finalize (object);
}

GExiv2PreviewImage* gexiv2_preview_image_new (Exiv2::PreviewManager *manager, 
	const Exiv2::PreviewProperties &props) {
    GExiv2PreviewImage* self = GEXIV2_PREVIEW_IMAGE (g_object_new (GEXIV2_TYPE_PREVIEW_IMAGE, NULL));
    
    self->priv->image = new Exiv2::PreviewImage(manager->getPreviewImage(props));
    self->priv->mime_type = g_strdup(self->priv->image->mimeType().c_str());
    self->priv->extension = g_strdup(self->priv->image->extension().c_str());
    
    return self;
}

void gexiv2_preview_image_free(GExiv2PreviewImage *self) {
    g_return_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self));
    
    g_object_unref(self);
}

const guint8* gexiv2_preview_image_get_data (GExiv2PreviewImage *self, guint32 *size) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), NULL);
    g_return_val_if_fail(size != NULL, NULL);
    
    *size = self->priv->image->size();
    
    return self->priv->image->pData();
}

const gchar * gexiv2_preview_image_get_mime_type (GExiv2PreviewImage *self) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), NULL);
    
    return self->priv->mime_type;
}

const gchar *gexiv2_preview_image_get_extension (GExiv2PreviewImage *self) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), NULL);
    
    return self->priv->extension;
}

guint32 gexiv2_preview_image_get_width (GExiv2PreviewImage *self) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), 0);
    
    return self->priv->image->width();
}

guint32 gexiv2_preview_image_get_height (GExiv2PreviewImage *self) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), 0);
    
    return self->priv->image->height();
}

glong gexiv2_preview_image_write_file (GExiv2PreviewImage *self, const gchar *path) {
    g_return_val_if_fail(GEXIV2_IS_PREVIEW_IMAGE(self), -1);
    g_return_val_if_fail(path != NULL && strlen(path) > 0, -1);
    
    return self->priv->image->writeFile(path);
}

G_END_DECLS

