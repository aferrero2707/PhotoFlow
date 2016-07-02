/*
 *  File Copied from Moonlight. See Moonlight for license
 */


#ifndef __GEXIV2_MANAGED_STREAM__
#define __GEXIV2_MANAGED_STREAM__

#include <glib-object.h>

typedef enum {
    Begin = 0,
    Current = 1,
    End = 2
} WrapperSeekOrigin;

typedef gboolean (*Stream_CanSeek)  (void *handle);
typedef gboolean (*Stream_CanRead)  (void *handle);
typedef gboolean (*Stream_CanWrite) (void *handle);
typedef gint64   (*Stream_Length)   (void *handle);
typedef gint64   (*Stream_Position) (void *handle);
typedef gint32   (*Stream_Read)     (void *handle, void *buffer, gint32 offset, gint32 count);
typedef void     (*Stream_Write)    (void *handle, void *buffer, gint32 offset, gint32 count);
typedef void     (*Stream_Seek)     (void *handle, gint64 offset, WrapperSeekOrigin origin);
typedef void     (*Stream_Flush)    (void *handle);

struct _ManagedStreamCallbacks {
    void *handle;
    Stream_CanSeek CanSeek;
    Stream_CanRead CanRead;
    Stream_CanWrite CanWrite;
    Stream_Length Length;
    Stream_Position Position;
    Stream_Read Read;
    Stream_Write Write;
    Stream_Seek Seek;
    Stream_Flush Flush;
};

typedef struct _ManagedStreamCallbacks ManagedStreamCallbacks;

#endif /* __GEXIV2_MANAGED_STREAM__ */

