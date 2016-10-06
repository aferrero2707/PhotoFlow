/*
 * gexiv2-metadata-exif.cpp
 *
 * Author(s)
 *  Mike Gemuende <mike@gemuende.de>
 *  Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#include "gexiv2-metadata.h"
#include "gexiv2-metadata-private.h"
#include <string>
#include <glib-object.h>
#include <exiv2/exif.hpp>

G_BEGIN_DECLS

gboolean gexiv2_metadata_has_exif (GExiv2Metadata *self) {
    g_return_val_if_fail (GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    return ! (self->priv->image->exifData().empty());
}


gboolean gexiv2_metadata_has_exif_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::ExifData &exif_data = self->priv->image->exifData();
    for (Exiv2::ExifData::iterator it = exif_data.begin(); it != exif_data.end(); ++it) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0)
            return TRUE;
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_clear_exif_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::ExifData &exif_data = self->priv->image->exifData();
    
    gboolean erased = FALSE;
    
    Exiv2::ExifData::iterator it = exif_data.begin();
    while (it != exif_data.end()) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0) {
            it = exif_data.erase(it);
            erased = TRUE;
        } else {
            it++;
        }
    }
    
    return erased;
}

void gexiv2_metadata_clear_exif (GExiv2Metadata *self) {
    g_return_if_fail (GEXIV2_IS_METADATA (self));
    g_return_if_fail(self->priv->image.get() != NULL);
    
    self->priv->image->exifData().clear ();
}

gchar** gexiv2_metadata_get_exif_tags (GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    // get a copy of the ExifData and sort it by tags, preserving sort of original
    Exiv2::ExifData exif_data = Exiv2::ExifData(self->priv->image->exifData());
    exif_data.sortByKey();
    
    GSList *list = NULL;
    GSList *list_iter;
    gchar** data;
    gint count = 0;
    
    for (Exiv2::ExifData::iterator it = exif_data.begin(); it != exif_data.end(); ++it) {
        if (it->count() > 0) {
            list = g_slist_prepend (list, g_strdup (it->key ().c_str ()));
            count++;
        }
    }
    
    data = g_new (gchar*, count + 1);
    data[count --] = NULL;
    for (list_iter = list; list_iter != NULL; list_iter = list_iter->next)
        data[count--] = static_cast<gchar*>(list_iter->data);

    g_slist_free (list);

    return data;
}

gchar* gexiv2_metadata_get_exif_tag_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::ExifData &exif_data = self->priv->image->exifData();
    
    try {
        Exiv2::ExifData::iterator it = exif_data.findKey(Exiv2::ExifKey(tag));
        while (it != exif_data.end() && it->count() == 0)
            it++;
        
        if (it != exif_data.end())
            return g_strdup (it->toString ().c_str ());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gchar* gexiv2_metadata_get_exif_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::ExifData &exif_data = self->priv->image->exifData();
    
    try {
        Exiv2::ExifData::iterator it = exif_data.findKey(Exiv2::ExifKey(tag));
        while (it != exif_data.end() && it->count() == 0)
            it++;
        
        if (it != exif_data.end()) {
            std::ostringstream os;
            it->write (os, &exif_data);
            
            return g_strdup (os.str ().c_str ());
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gboolean gexiv2_metadata_set_exif_tag_string (GExiv2Metadata *self, const gchar* tag, const gchar* value) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(value != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        self->priv->image->exifData()[tag] = value;
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

glong gexiv2_metadata_get_exif_tag_long (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), 0);
    g_return_val_if_fail(tag != NULL, 0);
    g_return_val_if_fail(self->priv->image.get() != NULL, 0);
    
    Exiv2::ExifData& exif_data = self->priv->image->exifData();
    
    try {
        Exiv2::ExifData::iterator it = exif_data.findKey(Exiv2::ExifKey(tag));
        while (it != exif_data.end() && it->count() == 0)
            it++;
        
        if (it != exif_data.end())
            return it->toLong ();
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return 0;
}

gboolean gexiv2_metadata_set_exif_tag_long (GExiv2Metadata *self, const gchar* tag, glong value) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        self->priv->image->exifData()[tag] = static_cast<int32_t>(value);
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_get_exif_tag_rational (GExiv2Metadata *self, const gchar* tag, gint* nom,
    gint* den) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(nom != NULL, FALSE);
    g_return_val_if_fail(den != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::ExifData& exif_data = self->priv->image->exifData();
    
    try {
        Exiv2::ExifData::iterator it = exif_data.findKey(Exiv2::ExifKey(tag));
        while (it != exif_data.end() && it->count() == 0)
            it++;
        
        if (it != exif_data.end()) {
            Exiv2::Rational r = it->toRational();
            *nom = r.first;
            *den = r.second;
            
            return TRUE;
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_set_exif_tag_rational (GExiv2Metadata *self, const gchar* tag, gint nom, 
    gint den) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        Exiv2::Rational r;
        r.first = nom;
        r.second = den;
        self->priv->image->exifData()[tag] = r;
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gdouble gexiv2_metadata_get_exif_tag_rational_as_double (GExiv2Metadata *self, const gchar* tag, gdouble def) {
    gint nom, den;
    if (!gexiv2_metadata_get_exif_tag_rational(self, tag, &nom, &den))
        return def;
    
    return (den != 0.0) ? (double) nom / (double) den : def;
}

const gchar* gexiv2_metadata_get_exif_tag_label (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::ExifKey key(tag);
        return g_intern_string(key.tagLabel().c_str());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_exif_tag_description (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::ExifKey key(tag);
        return g_intern_string(key.tagDesc().c_str());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_exif_tag_type (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::ExifKey key(tag);
        return Exiv2::TypeInfo::typeName(key.defaultTypeId());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

GBytes* gexiv2_metadata_get_exif_tag_raw (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);

    Exiv2::ExifData &exif_data = self->priv->image->exifData();

    try {
        Exiv2::ExifData::iterator it = exif_data.findKey(Exiv2::ExifKey(tag));
        while (it != exif_data.end() && it->count() == 0)
            it++;

        if (it != exif_data.end()) {
            long size = it->size();
            if( size > 0 ) {
                gpointer data = g_malloc(size);
                it->copy((Exiv2::byte*)data, Exiv2::invalidByteOrder);
                return g_bytes_new_take(data, size);
            }
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }

    return NULL;
}

G_END_DECLS
