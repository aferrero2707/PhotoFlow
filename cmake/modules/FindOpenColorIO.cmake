#
# Variables defined by this module:
#   OCIO_FOUND
#
# Targets exported by this module:
# OpenColorIO
#
# Usage: 
#   FIND_PACKAGE( OpenColorIO )
#   FIND_PACKAGE( OpenColorIO REQUIRED )
#

find_path(OCIO_INCLUDE_DIR OpenColorIO/OpenColorIO.h PATH_SUFFIXES include)
find_library(OCIO_LIBRARIES NAMES OCIO OpenColorIO)
if(OCIO_LIBRARIES)
	get_filename_component(OCIO_LIBRARY_DIR ${OCIO_LIBRARIES} DIRECTORY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OCIO REQUIRED_VARS OCIO_LIBRARIES OCIO_INCLUDE_DIR OCIO_LIBRARY_DIR)
mark_as_advanced(OCIO_LIBRARIES OCIO_INCLUDE_DIR OCIO_LIBRARY_DIR)
