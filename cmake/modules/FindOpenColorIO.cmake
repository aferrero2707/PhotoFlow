#
# Variables defined by this module:
#   OpenColorIO_FOUND
#
# Targets exported by this module:
# OpenColorIO
#
# Usage: 
#   FIND_PACKAGE( OpenColorIO )
#   FIND_PACKAGE( OpenColorIO REQUIRED )
#

find_path(OpenColorIO_INCLUDE_DIR OpenColorIO/OpenColorIO.h PATH_SUFFIXES include)
find_library(OpenColorIO_LIBRARIES NAMES OpenColorIO OpenColorIO)
if(OpenColorIO_LIBRARIES)
	get_filename_component(OpenColorIO_LIBRARY_DIR ${OpenColorIO_LIBRARIES} DIRECTORY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenColorIO REQUIRED_VARS OpenColorIO_LIBRARIES OpenColorIO_INCLUDE_DIR OpenColorIO_LIBRARY_DIR)
mark_as_advanced(OpenColorIO_LIBRARIES OpenColorIO_INCLUDE_DIR OpenColorIO_LIBRARY_DIR)
