/*
 * gio-stream-io.cpp
 *
 * Author(s)
 *     Mike Gemuende <mike@gemuende.de>
 *
 * This is free software. See COPYING for details.
 *
 * The class StreamIo is derived from BasicIo to provide usage of a
 * ManagedStream to exiv2.
 */

#include "gexiv2-stream-io.h"
#include "gexiv2-managed-stream.h"

#include <exiv2/basicio.hpp>
#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>

#include <exception>

StreamIo::StreamIo (ManagedStreamCallbacks* cb)
    : cb (cb), is_open (FALSE) {
    /* at least reading and seeking must be possible to read metatada */
    if ( ! cb->CanRead (cb->handle))
        throw std::exception ();
    
    if ( ! cb->CanSeek (cb->handle))
        throw std::exception ();

    can_write = cb->CanWrite (cb->handle);
}

StreamIo::~StreamIo () {
    memio.reset (NULL);
}

int StreamIo::munmap () {
    int result = 0;
    
    /* remove current memio object */
    if (memio.get () != NULL) {
        result = memio->munmap ();
        memio.reset (NULL);
    }
    
    return result;
}

Exiv2::byte* StreamIo::mmap (bool isWriteable) {
    /* mmap requires to map the whole data to memory, so we just use the MemIo
       class ad fill it with our stream to emulate this */
    memio.reset (new Exiv2::MemIo ());
    
    memio->write (*this);
    
    return memio->mmap (isWriteable);
}

Exiv2::BasicIo::AutoPtr StreamIo::temporary () const {
    /* here again, we just juse the memory for temporary buffer */
    return Exiv2::BasicIo::AutoPtr (new Exiv2::MemIo ());
}

long StreamIo::write (const Exiv2::byte* data, long write_count) {
    if ( ! can_write)
        return 0;
    
    long written_bytes = 0;
    
    while (write_count > written_bytes) {
    
        int write = MIN (write_count - written_bytes, G_MAXINT32);
        
        /* because of a marshalling problem on managed side, we shift the
           pointer and do NOT use the offset parameter */
        cb->Write (cb->handle, (char*)data + written_bytes, 0, write);
        
        written_bytes += write;
    }

    return written_bytes;
}

long StreamIo::write (Exiv2::BasicIo& src) {
    if ( ! can_write)
        return 0;
    
    if (static_cast<BasicIo*>(this) == &src) return 0;

    if (!src.isopen()) return 0;

        Exiv2::byte buffer [4096];
        long read_count = 0;
        long written_bytes = 0;
        
        while ((read_count = src.read (buffer, sizeof(buffer)))) {
            write(buffer, read_count);
            written_bytes += read_count;
        }

        return written_bytes;
}

void StreamIo::transfer (Exiv2::BasicIo& src) {
    /* reopen (reset position) the stream */
    open ();
    
    src.open ();
    
    write (src);
}

int StreamIo::putb (Exiv2::byte data) {
    if ( write (&data, 1) == 1)
        return data;
    
    return EOF;
}

int StreamIo::seek (long offset, Position position) {
    // FIXME: handle Error
    switch (position) {
        case (beg):
            cb->Seek (cb->handle, offset, Begin);
            break;
        case (cur):
            cb->Seek (cb->handle, offset, Current);
            break;
        case (end):
            cb->Seek (cb->handle, offset, End);
            break;
    }
    
    return 0;
}

long StreamIo::tell () const {
    return cb->Position (cb->handle);
}

long StreamIo::size () const {
    return cb->Length (cb->handle);
}

int StreamIo::open () {
    seek (0, beg);

    is_open = TRUE;
    return 0;
}

bool StreamIo::isopen () const {
    return is_open;
}

int StreamIo::close () {
    is_open = FALSE;

    return 0;
}

Exiv2::DataBuf StreamIo::read (long read_count) {
    Exiv2::DataBuf buffer (read_count);
    
    long read_bytes = read (buffer.pData_, buffer.size_);

    buffer.size_ = read_bytes;
    
    return buffer;
}

long StreamIo::read (Exiv2::byte* buf, long read_count) {
    long read_bytes = 0;
    
    while (read_count > read_bytes) {
        /* because of a marshalling problem on managed side, we shift the
           pointer and do NOT use the offset parameter */
        int read = cb->Read (cb->handle, (char*)buf + read_bytes, 0, MIN (read_count - read_bytes, G_MAXINT32));
        
        if (read <= 0)
            break;
        
        read_bytes += read;
    }
    
    return read_bytes;
}

int StreamIo::getb () {
    guchar b;
    
    StreamIo::read (&b, 1);

    if (eof ())
        return EOF;

    return b;
}

int StreamIo::error () const {
    return 0;
}

bool StreamIo::eof () const {
    return (cb->Length (cb->handle) == cb->Position (cb->handle));
}

std::string StreamIo::path () const {
    return "managed stream";
}

#ifdef EXV_UNICODE_PATH
std::wstring StreamIo::wpath() const {
    std::string p = path();
    std::wstring w(p.length(), L' ');
    std::copy(p.begin(), p.end(), w.begin());
    return w;
}
#endif

