cmake_minimum_required(VERSION 2.8.9)

project(photoflow CXX C)

# options controlling the build process
IF(MINGW)
  option(BUNDLED_LENSFUN "Use bundled LensFun library" ON)
  option(USE_NLS "Build Native Language Support (using gettext)" ON)
ELSEIF(APPLE)
  option(BUNDLED_LENSFUN "Use bundled LensFun library" ON)
  option(USE_NLS "Build Native Language Support (using gettext)" ON)
ELSE(MINGW)
  option(BUNDLED_GEXIV2 "Use bundled Gexiv2 library" OFF)
  option(BUNDLED_LENSFUN "Use bundled LensFun library" ON)
  option(USE_NLS "Build Native Language Support (using gettext)" ON)
ENDIF(MINGW)
option(BABL_FLIPS_DISABLED "BABL flips disabled" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 0)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 0)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 0)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 0)


find_package(PkgConfig)

MESSAGE(STATUS "Check for LCMS2 availability...")
#FIND_PACKAGE(LCMS2)
pkg_check_modules(LCMS2 REQUIRED lcms2) 

find_package (TIFF REQUIRED)

find_package (PNG REQUIRED)

find_package (JPEG REQUIRED)

find_package (ZLIB REQUIRED)

#pkg_check_modules(FFTW3 REQUIRED fftw3>=3.0)
pkg_check_modules(FFTW3 REQUIRED fftw3)

pkg_check_modules(XML2 REQUIRED libxml-2.0)

pkg_check_modules(EXIF REQUIRED libexif)
#pkg_check_modules(EXIV2 REQUIRED exiv2)
find_package (Exiv2)
#if(NOT BUNDLED_GEXIV2)
#  pkg_check_modules(GEXIV2 REQUIRED gexiv2)
#  MESSAGE( STATUS "GEXIV2_INCLUDE_DIRS:         " ${GEXIV2_INCLUDE_DIRS} )
#  MESSAGE( STATUS "GEXIV2_LIBRARY_DIRS:         " ${GEXIV2_LIBRARY_DIRS} )
#endif(NOT BUNDLED_GEXIV2)

if(NOT BUNDLED_LENSFUN)
  pkg_check_modules(LENSFUN REQUIRED lensfun)
  MESSAGE( STATUS "LENSFUN_INCLUDE_DIRS:         " ${LENSFUN_INCLUDE_DIRS} )
  MESSAGE( STATUS "LENSFUN_LIBRARY_DIRS:         " ${LENSFUN_LIBRARY_DIRS} )
  MESSAGE( STATUS "LENSFUN_LIBRARIES:         " ${LENSFUN_LIBRARIES})
endif(NOT BUNDLED_LENSFUN)


#IF(NOT MINGW)
pkg_check_modules(OPENEXR OpenEXR)
pkg_check_modules(ORC orc-0.4)
#ENDIF(NOT MINGW)

#FIND_PACKAGE(LibRaw 0.13 REQUIRED)
#pkg_check_modules(LIBRAW REQUIRED libraw)

#find_package (LibXML++ REQUIRED)
#pkg_check_modules(LIBXML REQUIRED libxml++-2.6) 

pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

pkg_check_modules(VIPS REQUIRED vips>=8.3.3)
#pkg_check_modules(VIPSCC REQUIRED vipsCC)
MESSAGE( STATUS "VIPS_INCLUDE_DIRS:         " ${VIPS_INCLUDE_DIRS} )
MESSAGE( STATUS "VIPS_LIBRARY_DIRS:         " ${VIPS_LIBRARY_DIRS} )

pkg_check_modules(PIXMAN pixman-1 REQUIRED) 
pkg_check_modules(PANGO pango) 
pkg_check_modules(PANGOFT2 pangoft2) 
pkg_check_modules(GLIB glib-2.0) 
pkg_check_modules(GMODULE gmodule-2.0) 
pkg_check_modules(GTHREAD gthread-2.0) 
pkg_check_modules(GOBJECT gobject-2.0) 
pkg_check_modules(GLIBMM glibmm-2.4) 
#pkg_check_modules(GTKMM3 gtkmm-3.0<=3.4.0) 

if(NOT USE_GTKMM3)
pkg_check_modules(GIMP gimp-2.0>=2.9.0) 
if (GIMP_FOUND)
    pkg_check_modules(GIMP_2_9 gimp-2.0>=2.9.0)
    if (GIMP_2_9_FOUND)
		pkg_check_modules(GIMPUI gimpui-2.0>=2.9.0) 
	    if (GIMPUI_FOUND)
        	set (HAVE_GIMP_2_9 "1")
	    endif (GIMPUI_FOUND)
    else (GIMP_2_9_FOUND)
        set (HAVE_GIMP_2_9 "0")
    endif (GIMP_2_9_FOUND)
    MESSAGE( STATUS "GIMP_2_9_CFLAGS:         " ${GIMP_2_9_CFLAGS} )
    MESSAGE( STATUS "GIMP_2_9_INCLUDE_DIRS:         " ${GIMP_2_9_INCLUDE_DIRS} )
    MESSAGE( STATUS "GIMP_2_9_LIBRARY_DIRS:         " ${GIMP_2_9_LIBRARY_DIRS} )
    MESSAGE( STATUS "GIMP_2_9_LDFLAGS:         " ${GIMP_2_9_LDFLAGS} )
endif (GIMP_FOUND)
endif(NOT USE_GTKMM3)

if (APPLE)
    find_package (MacIntegration REQUIRED)
endif (APPLE)


if(USE_GTKMM3)
  pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0) 
else(USE_GTKMM3)
  pkg_check_modules(GTKMM2 REQUIRED gtkmm-2.4) 
endif(USE_GTKMM3)

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


set(THEME hicolor)
set(THEME_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/icons/${THEME})

add_subdirectory(src) 
add_subdirectory(data)
