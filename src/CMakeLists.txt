IF(MINGW)
  SET(GMIC_FLAGS "-mwindows -std=gnu++11 -Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -fpermissive")
ELSEIF(APPLE)
   #SET(GMIC_FLAGS "-DPF_DISABLE_GMIC -std=c++11 -Wno-error=c++11-narrowing -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
   SET(GMIC_FLAGS "-mmacosx-version-min=10.8 -stdlib=libc++ -std=c++11 -Wno-error=c++11-narrowing -Wc++11-extensions -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
   #SET(GMIC_FLAGS "-Wno-error=c++11-narrowing -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
ELSE(MINGW)
  SET(GMIC_FLAGS "-std=gnu++11 -Wno-error=narrowing -Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -fpermissive")
ENDIF(MINGW)

set(COMPILE_FLAGS " ${GMIC_FLAGS} -I${CMAKE_SOURCE_DIR}/src/dt -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${INSTALL_PREFIX}\"' ")

if(Msgfmt_FOUND)
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -DLOCALEDIR=\"${LOCALE_DIR}\"")
endif()

if(BUNDLED_LENSFUN)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -DBUNDLED_LENSFUN=1")
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/external/lensfun/include)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/external/lensfun/include/lensfun)
endif()

if(BABL_FLIPS_DISABLED)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -DBABL_FLIPS_DISABLED=1")
endif()

if(GTKMM2_FOUND)
  link_directories(${GTKMM2_LIBRARY_DIRS}  )
  include_directories(${GTKMM2_INCLUDE_DIRS}  )
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -DGTKMM_2")
  set(GTKMM_LIBRARIES ${GTKMM2_LIBRARIES})
endif()

if(GTKMM3_FOUND)
  link_directories(${GTKMM3_LIBRARY_DIRS}  )
  include_directories(${GTKMM3_INCLUDE_DIRS}  )
  set(COMPILE_FLAGS "${COMPILE_FLAGS} -DGTKMM_3")  
  set(GTKMM_LIBRARIES ${GTKMM3_LIBRARIES})
endif()

#SET(CMAKE_CXX_FLAGS_DEBUG " -Wall -O0 -g -DNDEBUG -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_DEBUG " -O0 -g -DNDEBUG -Wno-unused-parameter -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_VERBDEBUG " -Wall -O0 -g -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_RELEASE " -O3 -ffast-math -ftree-vectorize -DNDEBUG ${COMPILE_FLAGS}  -Wno-unused-parameter") 
SET(CMAKE_CXX_FLAGS_TEST " -O3 -g -ffast-math -ftree-vectorize -Wno-unused-parameter -DNDEBUG ${COMPILE_FLAGS}") 

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
include_directories(${GMODULE_INCLUDE_DIRS})
link_directories(${GMODULE_LIBRARY_DIRS}  )
include_directories(${GOBJECT_INCLUDE_DIRS})
link_directories(${GOBJECT_LIBRARY_DIRS}  )
include_directories(${GTHREAD_INCLUDE_DIRS})
link_directories(${GTHREAD_LIBRARY_DIRS}  )

include_directories(${GLIBMM_INCLUDE_DIRS})
link_directories(${GLIBMM_LIBRARY_DIRS})

include_directories(${PIXMAN_INCLUDE_DIRS})
link_directories(${PIXMAN_LIBRARY_DIRS})

include_directories(${SIGC2_INCLUDE_DIRS})
link_directories(${SIGC2_LIBRARY_DIRS})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/LibRaw)


include_directories(${OPENEXR_INCLUDE_DIRS})
link_directories(${OPENEXR_LIBRARY_DIRS})
include_directories(${TIFF_INCLUDE_DIRS})
link_directories(${TIFF_LIBRARY_DIRS})
include_directories(${JPEG_INCLUDE_DIRS})
link_directories(${JPEG_LIBRARY_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
link_directories(${PNG_LIBRARY_DIRS})


include_directories(${XML2_INCLUDE_DIRS})
link_directories(${XML2_LIBRARY_DIRS})

include_directories(${LCMS22_INCLUDE_DIRS})
link_directories(${LCMS22_LIBRARY_DIRS})

include_directories(${EXIF_INCLUDE_DIRS})
link_directories(${EXIF_LIBRARY_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})
link_directories(${EXIV2_LIBRARY_DIRS})

include_directories(${ORC_INCLUDE_DIRS})
link_directories(${ORC_LIBRARY_DIRS})

include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS}  )

include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS}  )
include_directories(${PANGOFT2_INCLUDE_DIRS})
link_directories(${PANGOFT2_LIBRARY_DIRS}  )

include_directories(${VIPS_INCLUDE_DIRS})
link_directories(${VIPS_LIBRARY_DIRS}  )

if (APPLE)
	include_directories(${MacIntegration_INCLUDE_DIRS})
	link_directories(${MacIntegration_LIBRARY_DIRS}  )
    set (MACINTEG_LIBS ${MacIntegration_LIBRARIES})
endif (APPLE)

include_directories(${CMAKE_SOURCE_DIR}/src)


if (AUTOMATED_VERSION_CHECK)
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%H
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest commit date of the working branch
execute_process(
  COMMAND git log -1 --format=%ci
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

MESSAGE( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
MESSAGE( STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR} )
# eventually update the version.cc file
execute_process(
  COMMAND ${CMAKE_SOURCE_DIR}/tools/update_version_info.sh 0 ${CMAKE_SOURCE_DIR}/VERSION ${GIT_BRANCH} ${GIT_COMMIT_DATE} ${GIT_COMMIT_HASH} ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION_UPDATE_SUCCESS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

MESSAGE( STATUS "tools/update_version_info.sh: " ${VERSION_UPDATE_SUCCESS} )
endif (AUTOMATED_VERSION_CHECK)



FILE(GLOB DTIncludes dt/common/*.h dt/develop/*.h)
FILE(GLOB DTSources dt/common/*.c dt/develop/masks/*.c)

add_library (pfdt STATIC
  ${DTIncludes}
  ${DTSources}
)

SET_TARGET_PROPERTIES(pfdt PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/src/dt -std=gnu99")


FILE(GLOB BaseIncludes version.hh base/*.hh)
if (AUTOMATED_VERSION_CHECK)
  FILE(GLOB BaseSources ${CMAKE_CURRENT_BINARY_DIR}/version.cc base/*.cc)
else (AUTOMATED_VERSION_CHECK)
  FILE(GLOB BaseSources version.cc base/*.cc)
endif (AUTOMATED_VERSION_CHECK)

FILE(GLOB RTIncludes 
  rt/rtengine/rawimagesource.h #rt/rtengine/rt_math.h rt/rtengine/opthelper.h rt/rtengine/sleef.c rt/rtengine/sleefsseavx.c rt/rtengine/helpersse2.h
)
FILE(GLOB RTSources 
  rt/rtengine/rawimagesource.cc
  rt/rtengine/CA_correct_RT.cc
  rt/rtengine/amaze_demosaic_RT.cc
  rt/rtengine/igv_demosaic_RT.cc
  rt/rtengine/lmmse_demosaic_RT.cc
  rt/rtengine/xtrans_demosaic_RT.cc
)

FILE(GLOB VipsIncludes vips/*.h vips/*.hh vips/gmic/*.h)
FILE(GLOB VipsSources vips/*.c vips/*.cc vips/gmic/*.c vips/gmic/*.cpp)

FILE(GLOB GMicIncludes vips/gmic/gmic/src/*.h)
FILE(GLOB GMicSources vips/gmic/gmic/src/gmic.cpp)

FILE(GLOB OpIncludes operations/*.hh operations/gmic/*.hh)
FILE(GLOB OpSources operations/*.cc operations/gmic/*.cc)

#
# the libraw part is a bit of a hack:
# the static linking didn't work since it was pulling -lstdc++ and -lm into linker flags.
# so we do a custom dependency and pretend an imported liblibraw_r.a so no other -l are
# appended.
#
#add_library(libraw_static STATIC IMPORTED)
#set_target_properties(libraw_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/LibRaw/liblibraw_r.a)
#add_subdirectory(external/LibRaw)
#list(APPEND LIB_DEPS libraw_r)
#list(APPEND STATIC_LIBS libraw_static)

#
# the rawspeed part is a bit of a hack:
# the static linking didn't work since it was pulling -lstdc++ and -lm into linker flags.
# so we do a custom dependency and pretend an imported librawsped.a so no other -l are
# appended.
#
set(CMAKE_INSTALL_DATAROOTDIR ${CMAKE_INSTALL_PREFIX}/share)
#add_library(rawspeed_static STATIC IMPORTED)
#set_target_properties(rawspeed_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/rawspeed/librawspeed.a)
# compile rawspeed
add_subdirectory(external/rawspeed)
list(APPEND LIB_DEPS rawspeed)
list(APPEND STATIC_LIBS rawspeed_static)



if(BUNDLED_LENSFUN)
  LIST(APPEND CMAKE_MODULE_PATH " ${CMAKE_SOURCE_DIR}/src/external/lensfun/cmake/modules ")
  set(BUILD_STATIC "ON")
  add_library(lensfun_static STATIC IMPORTED)
  set_target_properties(lensfun_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/lensfun/libs/lensfun/liblensfun.a)
INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES(regex.h HAVE_REGEX_H)
CHECK_INCLUDE_FILES(endian.h HAVE_ENDIAN_H)
  message( "HAVE_REGEX_H: " ${HAVE_REGEX_H} )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/external/lensfun/include)
IF(NOT HAVE_REGEX_H)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/include/regex)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libs/regex)
  #message( "INCLUDE_DIRECTORIES(" ${CMAKE_CURRENT_BINARY_DIR} "/include/regex" )
ENDIF(NOT HAVE_REGEX_H)
  add_subdirectory(external/lensfun)
  list(APPEND LIB_DEPS lensfun)
  list(APPEND STATIC_LIBS lensfun_static)
else(BUNDLED_LENSFUN)
  link_directories( ${LENSFUN_LIBRARY_DIRS}  )
  include_directories( ${LENSFUN_INCLUDE_DIRS}  )  
endif(BUNDLED_LENSFUN)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPF_HAS_LENSFUN ")


if(BUNDLED_GEXIV2)
  include(ExternalProject)
  ExternalProject_Add(gexiv2
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2/configure --enable-static=yes --enable-shared=no --prefix=${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install
    #CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2/configure --enable-static=yes --enable-shared=yes --prefix=${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install
    BUILD_COMMAND ${MAKE})
  set(GEXIV2_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/include)
  set( GEXIV2_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/lib)
  set(GEXIV2_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/lib/libgexiv2.a)
endif(BUNDLED_GEXIV2)
include_directories(${GEXIV2_INCLUDE_DIRS})
link_directories(${GEXIV2_LIBRARY_DIRS})

MESSAGE( STATUS "GEXIV2_LIBRARIES: " ${GEXIV2_LIBRARIES} )

add_library (pfbase STATIC
  ${BaseIncludes}
  ${BaseSources}
  ${VipsIncludes}
  ${VipsSources}
  ${RTIncludes}
  ${RTSources}
  ${OpIncludes}
  ${OpSources}
  ${LibRawIncludes}
  ${LibRawSources}
  ${GMicIncludes}
  ${GMicSources}
)


add_dependencies(pfbase ${LIB_DEPS})


FILE(GLOB GuiIncludes gui/*.hh gui/widgets/*.hh gui/operations/*.hh gui/operations/gmic/*.hh)
FILE(GLOB GuiSources gui/*.cc gui/widgets/*.cc gui/operations/*.cc gui/operations/gmic/*.cc)

add_library (pfgui STATIC
  ${GuiIncludes}
  ${GuiSources}
)


IF(MINGW)
  list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi tre_regex psapi)
  #list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi psapi)
ELSEIF(APPLE)
  list(APPEND ADDITIONAL_LIBS)
ELSE(MINGW)
  list(APPEND ADDITIONAL_LIBS fftw3_threads pthread gomp)
ENDIF(MINGW)

list(APPEND ADDITIONAL_LIBS pugixml)

IF(APPLE)
  list(APPEND LIBS "-framework CoreServices")
ENDIF(APPLE)

#--------------------------------------------------
#
# photoflow executable
#
add_executable(photoflow main.cc)
IF(APPLE)
  set_target_properties(photoflow PROPERTIES LINK_FLAGS " -framework ApplicationServices ")
ENDIF(APPLE)

target_link_libraries(photoflow ${LIBS} 
  pfgui
  pfbase 
  pfdt
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${STATIC_LIBS}
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  ${EXIV2_LIBRARIES}
  ${GEXIV2_LIBRARIES}
  ${LENSFUN_LIBRARIES}
  ${SIGC2_LIBRARIES}
  ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
  ${PIXMAN_LIBRARIES} 
  ${MACINTEG_LIBS}
  ${GTKMM_LIBRARIES} 
  ${GLIBMM_LIBRARIES} 
  ${GLIB_LIBRARIES} 
  ${GMODULE_LIBRARIES} 
  ${GOBJECT_LIBRARIES} 
  ${GTHREAD_LIBRARIES} 
  ${ZLIB_LIBRARIES}
  #${LIBRAW_LIBRARIES}  
  ${ORC_LIBRARIES}
  fftw3
  ${ADDITIONAL_LIBS}
  #gmon
  )



#--------------------------------------------------
#
# pfbatch executable
#
add_executable(pfbatch pfbatch.cc)

target_link_libraries(pfbatch ${LIBS} 
  pfbase 
  pfdt
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${STATIC_LIBS}
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  ${EXIV2_LIBRARIES}
  ${GEXIV2_LIBRARIES}
  ${LENSFUN_LIBRARIES}
  ${SIGC2_LIBRARIES}
  ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
  ${GLIBMM_LIBRARIES} 
  ${GLIB_LIBRARIES} 
  ${GMODULE_LIBRARIES} 
  ${GOBJECT_LIBRARIES} 
  ${GTHREAD_LIBRARIES} 
  ${ZLIB_LIBRARIES}
  #${LIBRAW_LIBRARIES}  
  ${ORC_LIBRARIES}
  fftw3
  ${ADDITIONAL_LIBS}
  #gmon
  )


INSTALL(TARGETS photoflow RUNTIME DESTINATION bin)
INSTALL(TARGETS pfbatch RUNTIME DESTINATION bin)

INSTALL(DIRECTORY ../presets DESTINATION share/photoflow)
INSTALL(DIRECTORY ../themes DESTINATION share/photoflow)
INSTALL(DIRECTORY ../icc DESTINATION share/photoflow)
INSTALL(DIRECTORY ../icons DESTINATION share/photoflow)
INSTALL(DIRECTORY ../help DESTINATION share/photoflow)
INSTALL(FILES ../src/vips/gmic/gmic/src/gmic_def.gmic DESTINATION share/photoflow)


#--------------------------------------------------
#
# GIMP plug-in
#
if (GIMP_FOUND)
  FILE(GLOB PluginIncludes plugin/*.hh)
  FILE(GLOB PluginSources plugin/pluginwindow.cc plugin/pfgimp.cc)

  include_directories(${GIMP_2_9_INCLUDE_DIRS})
  link_directories(${GIMP_2_9_LIBRARY_DIRS})
  include_directories(${GIMPUI_INCLUDE_DIRS})
  link_directories(${GIMPUI_LIBRARY_DIRS})

  add_executable(pfgimp ${PluginSources})

  set_target_properties(pfgimp PROPERTIES COMPILE_FLAGS " -DHAVE_GIMP_2_9=${HAVE_GIMP_2_9} ")

  target_link_libraries(pfgimp ${LIBS} 
    pfgui
    pfbase 
    pfdt
    ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
    ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
    ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
    ${STATIC_LIBS}
    ${OPENEXR_LIBRARIES}
    ${XML2_LIBRARIES}
    ${EXIF_LIBRARIES}
    ${EXIV2_LIBRARIES}
    ${GEXIV2_LIBRARIES}
    ${LENSFUN_LIBRARIES}
    ${SIGC2_LIBRARIES}
    ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
    ${PIXMAN_LIBRARIES} 
    ${MACINTEG_LIBS}
    #${GIMP_LIBS}
    ${GIMP_2_9_LIBRARIES}
    ${GIMPUI_LIBRARIES}
    ${GTKMM_LIBRARIES} 
    ${GLIBMM_LIBRARIES} 
    ${GLIB_LIBRARIES} 
    ${GMODULE_LIBRARIES} 
    ${GOBJECT_LIBRARIES} 
    ${GTHREAD_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    #${LIBRAW_LIBRARIES}  
    ${ORC_LIBRARIES}
    fftw3
    ${ADDITIONAL_LIBS}
    #gmon
  )
  
  
  FILE(GLOB LoadPluginIncludes plugin/*.hh)
  FILE(GLOB LoadPluginSources plugin/pluginwindow.cc plugin/pfgimp-load.cc)

  add_executable(pfgimp-load ${LoadPluginSources})

  set_target_properties(pfgimp-load PROPERTIES COMPILE_FLAGS " -DHAVE_GIMP_2_9=${HAVE_GIMP_2_9} ")

  target_link_libraries(pfgimp-load ${LIBS} 
    pfgui
    pfbase 
    pfdt
    ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
    ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
    ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
    ${STATIC_LIBS}
    ${OPENEXR_LIBRARIES}
    ${XML2_LIBRARIES}
    ${EXIF_LIBRARIES}
    ${EXIV2_LIBRARIES}
    ${GEXIV2_LIBRARIES}
    ${LENSFUN_LIBRARIES}
    ${SIGC2_LIBRARIES}
    ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
    ${PIXMAN_LIBRARIES} 
    ${MACINTEG_LIBS}
    #${GIMP_LIBS}
    ${GIMP_2_9_LIBRARIES}
    ${GIMPUI_LIBRARIES}
    ${GTKMM_LIBRARIES} 
    ${GLIBMM_LIBRARIES} 
    ${GLIB_LIBRARIES} 
    ${GMODULE_LIBRARIES} 
    ${GOBJECT_LIBRARIES} 
    ${GTHREAD_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    #${LIBRAW_LIBRARIES}  
    ${ORC_LIBRARIES}
    fftw3
    ${ADDITIONAL_LIBS}
    #gmon
  )
  
  INSTALL(TARGETS pfgimp pfgimp-load RUNTIME DESTINATION ${GIMP_2_9_PREFIX}/lib/gimp/2.0/plug-ins)
endif (GIMP_FOUND)


  get_property(idirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  foreach(dir ${idirs})
    message(STATUS "include dir='${dir}'")
  endforeach()
  get_property(ldirs DIRECTORY PROPERTY LINK_DIRECTORIES)
  foreach(dir ${ldirs})
    message(STATUS "library dir='${dir}'")
  endforeach()

#if(GTKMM3_FOUND)

#add_executable(scrollwin tests/scrollwin.cc)

#target_link_libraries(scrollwin
  #${SIGC2_LIBRARIES}
  #${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
  #${PIXMAN_LIBRARIES} 
#  ${GTKMM_LIBRARIES} 
  #${GLIBMM_LIBRARIES} 
  #${GLIB_LIBRARIES} 
  #${GMODULE_LIBRARIES} 
  #${GOBJECT_LIBRARIES} 
  #${GTHREAD_LIBRARIES} 
#  )

#endif(GTKMM3_FOUND)

#add_executable(cast tests/cast.c)

#target_link_libraries(cast ${VIPS_LIBRARIES})


#add_executable(rawsave tests/rawsave.c)

#target_link_libraries(rawsave ${VIPS_LIBRARIES})


#add_executable(introspect # name of the executable on Windows will be example.exe 
#  vips/introspect.c 
#  )

#target_link_libraries(introspect ${VIPS_LIBRARIES}
#  )


#add_executable(draw # name of the executable on Windows will be example.exe 
#  tests/draw.cc
#  )

#target_link_libraries(draw ${LIBS} pfbase pfvips ${SIGC2_LIBRARIES}
#  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
#  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES} 
  #-lws2_32
#  )


#add_executable(draw2 tests/draw2.cc)

#target_link_libraries(draw2 ${LIBS} pfbase pfvips ${SIGC2_LIBRARIES}
#  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
#  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES} ${LIBRAW_LIBRARIES}  dl)


#add_executable(unprocessed_raw tests/unprocessed_raw.cc)

#SET_TARGET_PROPERTIES(unprocessed_raw PROPERTIES COMPILE_FLAGS "-fopenmp")
#SET_TARGET_PROPERTIES(unprocessed_raw PROPERTIES LINK_FLAGS "-fopenmp")

#target_link_libraries(unprocessed_raw ${LIBS} ${SIGC2_LIBRARIES}
#  ${TIFF_LIBRARIES} ${LIBRAW_LIBRARIES} ${LCMS2_LIBRARIES} dl)


