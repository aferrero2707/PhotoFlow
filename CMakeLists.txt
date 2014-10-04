cmake_minimum_required(VERSION 2.8)

project(photoflow)

find_package(PkgConfig)

MESSAGE(STATUS "Check for LCMS2 availability...")
#FIND_PACKAGE(LCMS2)
pkg_check_modules(LCMS2 REQUIRED lcms2) 

find_package (TIFF REQUIRED)

#FIND_PACKAGE(LibRaw 0.13 REQUIRED)
#pkg_check_modules(LIBRAW REQUIRED libraw)

#find_package (LibXML++ REQUIRED)
#pkg_check_modules(LIBXML REQUIRED libxml++-2.6) 

pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

pkg_check_modules(VIPS REQUIRED vips>=7.40.8)
#pkg_check_modules(VIPSCC REQUIRED vipsCC)
MESSAGE( STATUS "VIPS_INCLUDE_DIRS:         " ${VIPS_INCLUDE_DIRS} )
MESSAGE( STATUS "VIPS_LIBRARY_DIRS:         " ${VIPS_LIBRARY_DIRS} )

pkg_check_modules(GTKMM3 gtkmm-3.0) 
if(NOT GTKMM3_FOUND)
  pkg_check_modules(GTKMM2 REQUIRED gtkmm-2.4) 
endif()

  
add_subdirectory(src) 
