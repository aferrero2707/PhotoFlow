/*
 * gexiv2-startup.cpp
 *
 * Author(s)
 * 	Clint Rogers <clinton@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

#include <exiv2/xmp.hpp>
#include "gexiv2-startup.h"
#include "gexiv2-version.h"

gboolean gexiv2_initialize(void) {
    return Exiv2::XmpParser::initialize();
}

gint gexiv2_get_version(void) {
    return GEXIV2_MAJOR_VERSION * 100 * 100 +
           GEXIV2_MINOR_VERSION * 100 +
           GEXIV2_MICRO_VERSION;
}
