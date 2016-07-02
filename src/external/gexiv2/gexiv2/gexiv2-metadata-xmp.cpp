/*
 * gexiv2-metadata-xmp.cpp
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
#include <exiv2/xmp.hpp>

G_BEGIN_DECLS

gboolean gexiv2_metadata_has_xmp (GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    return !(self->priv->image->xmpData().empty());
}

void gexiv2_metadata_clear_xmp(GExiv2Metadata *self) {
    g_return_if_fail(GEXIV2_IS_METADATA (self));
    g_return_if_fail(self->priv->image.get() != NULL);
    
    self->priv->image->xmpData().clear();
}

gchar *gexiv2_metadata_generate_xmp_packet(GExiv2Metadata *self, 
    GExiv2XmpFormatFlags xmp_format_flags, guint32 padding) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::XmpData &xmp_data = self->priv->image->xmpData();
    try {
        if (Exiv2::XmpParser::encode(self->priv->image->xmpPacket(), xmp_data, xmp_format_flags, padding) == 0)
          return g_strdup(self->priv->image->xmpPacket().c_str());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gchar *gexiv2_metadata_get_xmp_packet(GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    try {
        return g_strdup(self->priv->image->xmpPacket().c_str());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gboolean gexiv2_metadata_has_xmp_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::XmpData &xmp_data = self->priv->image->xmpData();
    
    for (Exiv2::XmpData::iterator it = xmp_data.begin(); it != xmp_data.end(); ++it) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0)
            return TRUE;
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_clear_xmp_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::XmpData &xmp_data = self->priv->image->xmpData();
    
    gboolean erased = FALSE;
    
    Exiv2::XmpData::iterator it = xmp_data.begin();
    while (it != xmp_data.end()) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0) {
            it = xmp_data.erase(it);
            erased = TRUE;
        } else {
            it++;
        }
    }
    
    return erased;
}

gchar** gexiv2_metadata_get_xmp_tags (GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    // get a copy of the original XmpData and sort it by key, preserving the original
    Exiv2::XmpData xmp_data = Exiv2::XmpData(self->priv->image->xmpData());
    xmp_data.sortByKey ();
    
    GSList *list = NULL;
    GSList *list_iter;
    gchar** data;
    gint count = 0;
    
    for (Exiv2::XmpData::iterator it = xmp_data.begin(); it != xmp_data.end(); ++it) {
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

gchar* gexiv2_metadata_get_xmp_tag_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();
    
    try {
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;
        
        if (it != xmp_data.end())
            return g_strdup (it->toString ().c_str ());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gchar* gexiv2_metadata_get_xmp_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();
    
    try {
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;
        
        if (it != xmp_data.end()) {
            std::ostringstream os;
            it->write (os);
            
            return g_strdup (os.str ().c_str ());
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gboolean gexiv2_metadata_set_xmp_tag_struct (GExiv2Metadata *self, const gchar* tag, GExiv2StructureType type) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);

    Exiv2::XmpTextValue tv("");
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();

    switch (type) {
      case GEXIV2_STRUCTURE_XA_NONE:
        tv.read("");  // Clear the value
        tv.setXmpArrayType(Exiv2::XmpValue::xaNone);
        break;
      case GEXIV2_STRUCTURE_XA_ALT:
        tv.read("");
        tv.setXmpArrayType(Exiv2::XmpValue::xaAlt);
        break;
      case GEXIV2_STRUCTURE_XA_BAG:
        tv.read("");
        tv.setXmpArrayType(Exiv2::XmpValue::xaBag);
        break;
      case GEXIV2_STRUCTURE_XA_SEQ:
        tv.read("");
        tv.setXmpArrayType(Exiv2::XmpValue::xaSeq);
        break;
      default:
        g_warning("Invalid structure type.");
        return FALSE;
        break;
    }

    try {
        xmp_data.add(Exiv2::XmpKey(tag), &tv);
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_set_xmp_tag_string (GExiv2Metadata *self, const gchar* tag, 
    const gchar* value) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(value != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        self->priv->image->xmpData()[tag] = value;
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

glong gexiv2_metadata_get_xmp_tag_long (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), 0);
    g_return_val_if_fail(tag != NULL, 0);
    g_return_val_if_fail(self->priv->image.get() != NULL, 0);
    
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();
    
    try {
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;
        
        if (it != xmp_data.end())
            return it->toLong ();
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return 0;
}

gboolean gexiv2_metadata_set_xmp_tag_long (GExiv2Metadata *self, const gchar* tag, glong value) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        self->priv->image->xmpData()[tag] = value;
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gchar** gexiv2_metadata_get_xmp_tag_multiple (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();
    
    try {
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;
        
        if (it != xmp_data.end()) {
            int size = it->count ();
            gchar **array = g_new (gchar*, size + 1);
            array[size] = NULL;
            
            for (int i = 0; i < it->count (); i++)
                array[i] = g_strdup (it->toString (i).c_str ());
            
            return array;
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    gchar **array = g_new (gchar*, 1);
    array[0] = NULL;
    
    return array;
}

gboolean gexiv2_metadata_set_xmp_tag_multiple (GExiv2Metadata *self, const gchar* tag, 
    const gchar** values) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(values != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();
    
    try {
        /* first clear existing tag */
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;
        
        if (it != xmp_data.end())
            xmp_data.erase (it);
        
        /* ... and then set the others */
        const gchar **val_it = values;
        while (*val_it != NULL) {
            xmp_data[tag] = static_cast<const std::string> (*val_it);
            ++val_it;
        }
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

const gchar* gexiv2_metadata_get_xmp_tag_label (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        return Exiv2::XmpProperties::propertyTitle(Exiv2::XmpKey(tag));
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_xmp_tag_description (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        return Exiv2::XmpProperties::propertyDesc(Exiv2::XmpKey(tag));
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_xmp_tag_type (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        return Exiv2::TypeInfo::typeName(Exiv2::XmpProperties::propertyType(Exiv2::XmpKey(tag)));
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

GBytes* gexiv2_metadata_get_xmp_tag_raw (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);

    Exiv2::XmpData& xmp_data = self->priv->image->xmpData();

    try {
        Exiv2::XmpData::iterator it = xmp_data.findKey(Exiv2::XmpKey(tag));
        while (it != xmp_data.end() && it->count() == 0)
            it++;

        if (it != xmp_data.end()) {
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

gboolean gexiv2_metadata_register_xmp_namespace (const gchar* name, const gchar* prefix) {
    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(prefix != NULL, FALSE);

    try {
        Exiv2::XmpProperties::ns(prefix);
    } catch (Exiv2::Error& error) {
        // No namespace, OK to register
        Exiv2::XmpProperties::registerNs(name, prefix);
        return TRUE;
    }

    return FALSE;
}

gboolean gexiv2_metadata_unregister_xmp_namespace (const gchar* name) {
    g_return_val_if_fail(name != NULL, FALSE);

    std::string prefix = Exiv2::XmpProperties::prefix(name);

    if (!prefix.empty()) {
        // Unregister
        Exiv2::XmpProperties::unregisterNs(name);

        try {
            Exiv2::XmpProperties::ns(prefix);
        } catch (Exiv2::Error& error) {
            // Namespace successfully removed
            return TRUE;
        }
    }

    return FALSE;
}

void gexiv2_metadata_unregister_all_xmp_namespaces (void) {
    Exiv2::XmpProperties::unregisterNs();
}

G_END_DECLS
