/*
 * gexiv2-startup.h
 *
 * Author(s)
 *  Clint Rogers <clinton@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_STARTUP_H__
#define __GEXIV2_STARTUP_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * gexiv2_initialize:
 *
 * gexiv2 requires initialization before its methods are used.  In particular, this call must be
 * made in a thread-safe fashion.  Best practice is to call from the application's main thread and
 * not to use any gexiv2 code until it has returned.
 *
 * Returns: TRUE if initialized.  If FALSE, gexiv2 should not be used (unable to initialize
 * properly).
 */
gboolean gexiv2_initialize(void);

G_END_DECLS
#endif
