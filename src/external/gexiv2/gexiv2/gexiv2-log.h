/*
 * gexiv2-log.h
 *
 * Author(s)
 * 	Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_LOG_H__
#define __GEXIV2_LOG_H__

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	GEXIV2_LOG_LEVEL_DEBUG	= 0,
	GEXIV2_LOG_LEVEL_INFO	= 1,
	GEXIV2_LOG_LEVEL_WARN	= 2,
	GEXIV2_LOG_LEVEL_ERROR	= 3,
	GEXIV2_LOG_LEVEL_MUTE	= 4
} GExiv2LogLevel;

/**
 * GExiv2LogHandler:
 * @level: The #Gexiv2LogLevel for the particular message
 * @msg: (in): The log message
 *
 * The log handler can be set by #gexiv2_log_set_handler.  When set, the log handler will receive
 * all log messages emitted by Exiv2 and gexiv2.  It's up to the handler to decide where (and if)
 * the images are displayed or stored.
 */
typedef void (*GExiv2LogHandler)(GExiv2LogLevel level, const gchar *msg);

/**
 * gexiv2_log_get_level:
 *
 * Returns: The current #GExiv2LogLevel.  Messages below this level will not be logged.
 */
GExiv2LogLevel		gexiv2_log_get_level(void);

/**
 * gexiv2_log_set_level:
 * @level: The #GExiv2LogLevel gexiv2 should respect.
 *
 * Log messages below this level will not be logged.
 */
void				gexiv2_log_set_level(GExiv2LogLevel level);

/**
 * gexiv2_log_get_handler
 *
 * Returns: The current #GExiv2LogHandler, or the default if none set.  See
 * #gexiv2_log_get_default_handler.
 */
GExiv2LogHandler	gexiv2_log_get_handler(void);

/**
 * gexiv2_log_get_default_handler
 *
 * Returns: The default #GExiv2LogHandler, which uses Exiv2's built-in handler.  Exiv2 will send the
 * message to stderr.
 */
GExiv2LogHandler	gexiv2_log_get_default_handler(void);

/**
 * gexiv2_log_set_handler:
 * @handler: A #GExiv2LogHandler callback to begin receiving log messages from Exiv2 and gexiv2
 *
 * This method is not thread-safe.  It's best to set this before beginning to use gexiv2.
 */
void				gexiv2_log_set_handler(GExiv2LogHandler handler);

/**
 * gexiv2_log_use_glib_logging:
 *
 * When called, gexiv2 will install it's own #GExiv2LogHandler which redirects all Exiv2 and gexiv2
 * log messages to GLib's logging calls (g_debug, g_message, g_warning, and g_critical for the
 * respective #GExiv2LogLevel value).  #GEXIV2_LOG_LEVEL_MUTE logs are dropped.
 *
 * One advantage to using this is that GLib's logging control and handlers can be used rather than
 * gexiv2's ad hoc scheme.  It also means an application can use GLib logging and have all its
 * messages routed through the same calls.
 */
void				gexiv2_log_use_glib_logging(void);

G_END_DECLS

#endif /* __GEXIV2_LOG_H__ */
