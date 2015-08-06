cmake_minimum_required(VERSION 2.8)

project(photoflow CXX C)

option(USE_NLS "Build Native Language Support (using gettext)" ON)

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
pkg_check_modules(GEXIV2 REQUIRED gexiv2)
MESSAGE( STATUS "GEXIV2_INCLUDE_DIRS:         " ${GEXIV2_INCLUDE_DIRS} )
MESSAGE( STATUS "GEXIV2_LIBRARY_DIRS:         " ${GEXIV2_LIBRARY_DIRS} )

pkg_check_modules(LENSFUN lensfun)
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
pkg_check_modules(GTKMM2 gtkmm-2.4) 
if(NOT GTKMM2_FOUND)
  pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0) 
  #MESSAGE( STATUS "GTKMM2_INCLUDE_DIRS:         " ${GTKMM2_INCLUDE_DIRS} )
  #MESSAGE( STATUS "GTKMM2_LIBRARY_DIRS:         " ${GTKMM2_LIBRARY_DIRS} )
endif()
  MESSAGE( STATUS "GTKMM2_FOUND:         " ${GTKMM2_FOUND} )
  MESSAGE( STATUS "GTKMM3_FOUND:         " ${GTKMM3_FOUND} )

  
IF(NOT MINGW)
#need to put here do setting LOCALE_DIR variable
if(USE_NLS)
find_package(Gettext)
if(Gettext_FOUND)
	if(NOT LOCALE_DIR)
    SET(LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/share/locale")
	endif(NOT LOCALE_DIR)

	if(NOT LOCALE_DIR)
		message(QUIET "Set LOCALE_DIR to path to get Gettext working")
	else()
		find_package(Msgfmt)
		if(Msgfmt_FOUND)
			message(STATUS "Found msgfmt to convert language file. Translation enabled")
			add_subdirectory(po)
		else()
			message(STATUS "Cannot find msgfmt to convert language file. Translation won't be enabled")
		endif()
	endif(NOT LOCALE_DIR)
endif(Gettext_FOUND)
endif(USE_NLS)
ENDIF(NOT MINGW)
MESSAGE( STATUS "Msgfmt_FOUND:         " ${Msgfmt_FOUND} )


add_subdirectory(src) 
