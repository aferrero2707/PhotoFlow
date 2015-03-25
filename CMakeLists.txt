cmake_minimum_required(VERSION 2.8)

project(photoflow)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(PkgConfig)

MESSAGE(STATUS "Check for LCMS2 availability...")
#FIND_PACKAGE(LCMS2)
pkg_check_modules(LCMS2 REQUIRED lcms2) 

find_package (TIFF REQUIRED)

find_package (PNG REQUIRED)

find_package (JPEG REQUIRED)

find_package (ZLIB REQUIRED)

pkg_check_modules(FFTW3 REQUIRED fftw3>=3.0)

pkg_check_modules(XML2 REQUIRED libxml-2.0)

pkg_check_modules(EXIF REQUIRED libexif)
#pkg_check_modules(EXIV2 REQUIRED exiv2)
find_package (Exiv2)

pkg_check_modules(LENSFUN REQUIRED lensfun)
MESSAGE( STATUS "LENSFUN_INCLUDE_DIRS:         " ${LENSFUN_INCLUDE_DIRS} )
MESSAGE( STATUS "LENSFUN_LIBRARY_DIRS:         " ${LENSFUN_LIBRARY_DIRS} )
MESSAGE( STATUS "LENSFUN_LIBRARIES:         " ${LENSFUN_LIBRARIES})


IF(NOT MINGW)
pkg_check_modules(OPENEXR OpenEXR)
pkg_check_modules(ORC REQUIRED orc-0.4)
ENDIF(NOT MINGW)

#FIND_PACKAGE(LibRaw 0.13 REQUIRED)
#pkg_check_modules(LIBRAW REQUIRED libraw)

#find_package (LibXML++ REQUIRED)
#pkg_check_modules(LIBXML REQUIRED libxml++-2.6) 

pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

pkg_check_modules(VIPS REQUIRED vips>=7.42.0)
#pkg_check_modules(VIPSCC REQUIRED vipsCC)
MESSAGE( STATUS "VIPS_INCLUDE_DIRS:         " ${VIPS_INCLUDE_DIRS} )
MESSAGE( STATUS "VIPS_LIBRARY_DIRS:         " ${VIPS_LIBRARY_DIRS} )

pkg_check_modules(PANGO pango) 
pkg_check_modules(PANGOFT2 pangoft2) 
pkg_check_modules(GLIB glib-2.0) 
pkg_check_modules(GMODULE gmodule-2.0) 
pkg_check_modules(GTHREAD gthread-2.0) 
pkg_check_modules(GOBJECT gobject-2.0) 
pkg_check_modules(GLIBMM glibmm-2.4) 
#pkg_check_modules(GTKMM3 gtkmm-3.0<=3.4.0) 
pkg_check_modules(GTKMM3 gtkmm-3.0) 
if(NOT GTKMM3_FOUND)
  pkg_check_modules(GTKMM2 REQUIRED gtkmm-2.4) 
  MESSAGE( STATUS "GTKMM2_INCLUDE_DIRS:         " ${GTKMM2_INCLUDE_DIRS} )
  MESSAGE( STATUS "GTKMM2_LIBRARY_DIRS:         " ${GTKMM2_LIBRARY_DIRS} )
endif()

  
add_subdirectory(src) 
