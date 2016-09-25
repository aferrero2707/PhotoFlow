/*
 * gexiv2-version.h
 *
 * This is free software. See COPYING for details.
 */

#ifndef __GEXIV2_VERSION_H__
#define __GEXIV2_VERSION_H__

#include <glib-object.h>

G_BEGIN_DECLS

`#'define GEXIV2_MAJOR_VERSION _VERSION_MAJOR_
`#'define GEXIV2_MINOR_VERSION _VERSION_MINOR_
`#'define GEXIV2_MICRO_VERSION _VERSION_MICRO_

/**
 * GEXIV2_CHECK_VERSION:
 *
 * Returns: TRUE if the gexiv2 library version is greater than or equal to the supplied version
 * requirement.
 */
#define GEXIV2_CHECK_VERSION(major, minor, micro) \
    (GEXIV2_MAJOR_VERSION > (major) || \
     (GEXIV2_MAJOR_VERSION == (major) && GEXIV2_MINOR_VERSION > (minor)) || \
     (GEXIV2_MAJOR_VERSION == (major) && GEXIV2_MINOR_VERSION == (minor) && \
      GEXIV2_MICRO_VERSION >= (micro)))

/**
 * gexiv2_get_version:
 *
 * Returns: The gexiv2 library's version number as a formatted decimal XXYYZZ, where XX is the
 * major version, YY is the minor version, and ZZ is the micro version.  For example, version
 * 0.6.1 will be returned as 000601.
 */
gint gexiv2_get_version (void);

G_END_DECLS
#endif
