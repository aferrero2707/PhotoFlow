/*
 * gio-stream-io.h
 *
 * Author(s)
 * 	Mike Gemuende <mike@gemuende.de>
 *
 * This is free software. See COPYING for details.
 */

/*
 * The class StreamIo is derived from BasicIo to provide usage of a
 * ManagedStream to exiv2.
 */

#ifndef __GEXIV2_STREAM_IO_H__
#define __GEXIV2_STREAM_IO_H__

#include <gexiv2/gexiv2-managed-stream.h>
#include <exiv2/basicio.hpp>
#include <gio/gio.h>


class StreamIo : public Exiv2::BasicIo {
public:

	StreamIo (ManagedStreamCallbacks* cb);

	virtual ~StreamIo ();
	virtual int open ();
	virtual int close ();
	virtual long write (const Exiv2::byte* data, long wcount);
	virtual long write (Exiv2::BasicIo& src);
	virtual int putb (Exiv2::byte data);
	virtual Exiv2::DataBuf read (long rcount);
	virtual long read (Exiv2::byte* buf, long rcount);
	virtual int getb ();
	virtual void transfer (Exiv2::BasicIo& src);
	virtual int seek (long offset, Position pos);
	virtual Exiv2::byte* mmap (bool isWriteable = false);
	virtual int munmap ();
	virtual long tell () const;
	virtual long size () const;
	virtual bool isopen () const;
	virtual int error () const;
	virtual bool eof () const;
	virtual std::string path () const;
#ifdef EXV_UNICODE_PATH
	virtual std::wstring wpath () const;
#endif
	virtual BasicIo::AutoPtr temporary () const;

private:

	/* stream callbacks */
	ManagedStreamCallbacks* cb;
	
	/* used for mmap and  munmap */
	Exiv2::BasicIo::AutoPtr memio;

	/* closing does not mean closing the stream, because this would
	   destroy it. So just keep track about current state and let stream
	   open. */
	gboolean is_open;
	
	/* stream can be used for writing */
	gboolean can_write;
};


#endif /* __GEXIV2_STREAM_IO_H__ */

