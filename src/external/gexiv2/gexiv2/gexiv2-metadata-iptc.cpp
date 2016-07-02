/*
 * gexiv2-metadata-iptc.cpp
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
#include <exiv2/iptc.hpp>

G_BEGIN_DECLS

gboolean gexiv2_metadata_has_iptc (GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    return ! (self->priv->image->iptcData().empty());
}

gboolean gexiv2_metadata_has_iptc_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::IptcData &iptc_data = self->priv->image->iptcData();
    
    for (Exiv2::IptcData::iterator it = iptc_data.begin(); it != iptc_data.end(); ++it) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0)
            return TRUE;
    }
    
    return FALSE;
}

gboolean gexiv2_metadata_clear_iptc_tag(GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA(self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::IptcData &iptc_data = self->priv->image->iptcData();
    
    gboolean erased = FALSE;
    
    Exiv2::IptcData::iterator it = iptc_data.begin();
    while (it != iptc_data.end()) {
        if (it->count() > 0 && g_ascii_strcasecmp(tag, it->key().c_str()) == 0) {
            it = iptc_data.erase(it);
            erased = TRUE;
        } else {
            it++;
        }
    }
    
    return erased;
}

void gexiv2_metadata_clear_iptc (GExiv2Metadata *self) {
    g_return_if_fail(GEXIV2_IS_METADATA (self));
    g_return_if_fail(self->priv->image.get() != NULL);

    self->priv->image->iptcData().clear ();
}

gchar** gexiv2_metadata_get_iptc_tags (GExiv2Metadata *self) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    // get a copy of the IptcData and sort it by key, preserving the original
    Exiv2::IptcData iptc_data = Exiv2::IptcData(self->priv->image->iptcData());
    iptc_data.sortByKey();
    
    GSList *list = NULL;
    GSList *list_iter;
    gchar** data;
    gint count = 0;
    
    for (Exiv2::IptcData::iterator it = iptc_data.begin(); it != iptc_data.end(); ++it) {
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

gchar* gexiv2_metadata_get_iptc_tag_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::IptcData& iptc_data = self->priv->image->iptcData();
    
    try {
        Exiv2::IptcData::iterator it = iptc_data.findKey(Exiv2::IptcKey(tag));
        while (it != iptc_data.end() && it->count() == 0)
            it++;
        
        if (it != iptc_data.end())
            return g_strdup (it->toString ().c_str ());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gchar* gexiv2_metadata_get_iptc_tag_interpreted_string (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::IptcData& iptc_data = self->priv->image->iptcData();
    
    try {
        Exiv2::IptcData::iterator it = iptc_data.findKey(Exiv2::IptcKey(tag));
        while (it != iptc_data.end() && it->count() == 0)
            it++;
        
        if (it != iptc_data.end()) {
            std::ostringstream os;
            it->write (os);
            
            return g_strdup (os.str ().c_str ());
        }
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

gboolean gexiv2_metadata_set_iptc_tag_string (GExiv2Metadata *self, const gchar* tag,
    const gchar* value) {
    g_return_val_if_fail (GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(value != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    try {
        self->priv->image->iptcData()[tag] = value;
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

gchar** gexiv2_metadata_get_iptc_tag_multiple (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);
    
    Exiv2::IptcData &iptc_data = self->priv->image->iptcData();
    
    GSList *list = NULL;
    GSList *list_iter;
    gchar** values;
    gint count = 0;
    
    try {
        Exiv2::IptcKey key (tag);
        for (Exiv2::IptcData::iterator it = iptc_data.begin(); it != iptc_data.end(); ++it) {
            if (it->count() > 0 && key.key () == it->key ()) {
                list = g_slist_prepend (list, g_strdup (it->toString ().c_str ()));
                count++;
            }
        }
        
        values = g_new (gchar*, count + 1);
        values[count --] = NULL;
        for (list_iter = list; list_iter != NULL; list_iter = list_iter->next)
            values[count--] = static_cast<gchar*>(list_iter->data);
        
        g_slist_free (list);
        
        return values;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    g_slist_free_full (list, g_free);
    
    return NULL;
}

gboolean gexiv2_metadata_set_iptc_tag_multiple (GExiv2Metadata *self, const gchar* tag,
    const gchar** values) {
    g_return_val_if_fail (GEXIV2_IS_METADATA (self), FALSE);
    g_return_val_if_fail(tag != NULL, FALSE);
    g_return_val_if_fail(values != NULL, FALSE);
    g_return_val_if_fail(self->priv->image.get() != NULL, FALSE);
    
    Exiv2::IptcData& iptc_data = self->priv->image->iptcData();
    
    try {
        /* first clear all existing ... */
        Exiv2::IptcKey iptc_key(tag);
        Exiv2::IptcData::iterator iptc_it = iptc_data.begin();
        while (iptc_it != iptc_data.end()) {
            if (iptc_it->count() > 0 && iptc_key.key () == iptc_it->key ())
                iptc_it = iptc_data.erase (iptc_it);
            else
                ++iptc_it;
        }
        
        /* ... and then set the others */
        Exiv2::Value::AutoPtr iptc_value = Exiv2::Value::create(Exiv2::string);
            
        const gchar **it = values;
        while (*it != NULL) {
            iptc_value->read (static_cast<const std::string>(*it));
            iptc_data.add (iptc_key, iptc_value.get());
            
            ++it;
        }
        
        return TRUE;
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return FALSE;
}

const gchar* gexiv2_metadata_get_iptc_tag_label (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::IptcKey key (tag);
        return Exiv2::IptcDataSets::dataSetTitle (key.tag (), key.record ());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_iptc_tag_description (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::IptcKey key (tag);
        return Exiv2::IptcDataSets::dataSetDesc (key.tag (), key.record ());
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

const gchar* gexiv2_metadata_get_iptc_tag_type (const gchar* tag) {
    g_return_val_if_fail(tag != NULL, NULL);
    
    try {
        Exiv2::IptcKey key (tag);
        return Exiv2::TypeInfo::typeName(Exiv2::IptcDataSets::dataSetType(key.tag(), key.record()));
    } catch (Exiv2::Error& e) {
        LOG_ERROR(e);
    }
    
    return NULL;
}

GBytes* gexiv2_metadata_get_iptc_tag_raw (GExiv2Metadata *self, const gchar* tag) {
    g_return_val_if_fail(GEXIV2_IS_METADATA (self), NULL);
    g_return_val_if_fail(tag != NULL, NULL);
    g_return_val_if_fail(self->priv->image.get() != NULL, NULL);

    Exiv2::IptcData& iptc_data = self->priv->image->iptcData();

    try {
        Exiv2::IptcData::iterator it = iptc_data.findKey(Exiv2::IptcKey(tag));
        while (it != iptc_data.end() && it->count() == 0)
            it++;

        if (it != iptc_data.end()) {
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
