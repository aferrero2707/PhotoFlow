IF(MINGW)
  SET(GMIC_FLAGS "-mwindows -std=gnu++11 -Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -fpermissive")
ELSEIF(APPLE)
   #SET(GMIC_FLAGS "-DPF_DISABLE_GMIC -std=c++11 -Wno-error=c++11-narrowing -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
   SET(GMIC_FLAGS "-mmacosx-version-min=10.8 -stdlib=libc++ -std=c++11 -Wno-error=c++11-narrowing -Wc++11-extensions -Dgmic_build -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
   #SET(GMIC_FLAGS "-Wno-error=c++11-narrowing -Dgmic_build -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
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

# Check for openmp
find_package(OpenMP)
if(OpenMP_C_FLAGS AND USE_OPENMP)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} ${OpenMP_C_FLAGS}")
  set(COMPILE_FLAGS "${COMPILE_FLAGS} ${OpenMP_CXX_FLAGS}")
  if(WIN32)
    list(APPEND ADDITIONAL_LIBS gomp)
  endif(WIN32)
endif(OpenMP_C_FLAGS AND USE_OPENMP)


#SET(CMAKE_CXX_FLAGS_DEBUG " -Wall -O0 -g -DNDEBUG -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_DEBUG " -O0 -g -DNDEBUG ${COMPILE_FLAGS} -Wno-unused-parameter") 
SET(CMAKE_CXX_FLAGS_VERBDEBUG " -Wall -O0 -g -DDO_WARNINGS ${COMPILE_FLAGS}") 
#SET(CMAKE_CXX_FLAGS_RELEASE " -O3 -ffast-math -ftree-vectorize -DNDEBUG ${COMPILE_FLAGS}  -Wno-unused-parameter") 
#SET(CMAKE_CXX_FLAGS_TEST " -O3 -g -ffast-math -ftree-vectorize -Wno-unused-parameter -DNDEBUG ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_RELEASE        "   -O3 -ftree-vectorize -DNDEBUG ${COMPILE_FLAGS}  -Wno-unused-parameter -Wno-deprecated-declarations")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -ftree-vectorize -DNDEBUG ${COMPILE_FLAGS}  -Wno-unused-parameter -Wno-deprecated-declarations")
SET(CMAKE_CXX_FLAGS_TEST "            -O3 -g -ftree-vectorize -Wno-unused-parameter -DNDEBUG ${COMPILE_FLAGS}") 

#link_directories("/this/is/a/test/path")

include_directories(${EXPAT_INCLUDE_DIRS})
include_directories(${ICONV_INCLUDE_DIRS})

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
#include_directories(${EXIV2_INCLUDE_DIRS})
#link_directories(${EXIV2_LIBRARY_DIRS})

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
  FILE(GLOB BaseSources ${CMAKE_CURRENT_BINARY_DIR}/version.cc base/*.cc base/icc/*.cc)
else (AUTOMATED_VERSION_CHECK)
  FILE(GLOB BaseSources version.cc base/*.cc base/icc/*.cc)
endif (AUTOMATED_VERSION_CHECK)

FILE(GLOB RTIncludes 
  rt/rtengine/rawimagesource.h #rt/rtengine/rt_math.h rt/rtengine/opthelper.h rt/rtengine/sleef.c rt/rtengine/sleefsseavx.c rt/rtengine/helpersse2.h
  rt/rtengine/dcp.h rt/rtexif/rtexif.h
  rt/rtengine/dcraw.h 
  rt/rtengine/myfile.h 
  rt/rtengine/rtengine.h 
  rt/rtengine/opthelper.h 
  rt/rtengine/rawimage.h 
  rt/rtengine/noncopyable.h
  rt/rtengine/imageio.h
  rt/rtengine/camconst.h
  rt/rtengine/cJSON.h
)
FILE(GLOB RTSources 
  rt/rtengine/rawimagesource.cc
  rt/rtengine/CA_correct_RT.cc
  rt/rtengine/amaze_demosaic_RT.cc
  rt/rtengine/igv_demosaic_RT.cc
  rt/rtengine/lmmse_demosaic_RT.cc
  rt/rtengine/xtrans_demosaic_RT.cc
  rt/rtengine/dcp.cc rt/rtengine/curves.cc rt/rtengine/diagonalcurves.cc rt/rtengine/flatcurves.cc rt/rtengine/color.cc rt/rtexif/*.cc
  rt/rtengine/dcraw.cc
  rt/rtengine/myfile.cc
  rt/rtengine/rawimage.cc
  rt/rtengine/camconst.cc
  rt/rtengine/cJSON.c
  rt/rtengine/settings.cc
)

FILE(GLOB VipsIncludes vips/*.h vips/*.hh vips/gmic/*.h)
FILE(GLOB VipsSources vips/*.c vips/*.cc vips/gmic/*.c vips/gmic/*.cpp)

FILE(GLOB GMicIncludes vips/gmic/gmic/src/*.h)
FILE(GLOB GMicSources vips/gmic/gmic/src/gmic.cpp)

FILE(GLOB OpIncludes operations/*.hh operations/gmic/*.hh legacy/operations/*.hh)
FILE(GLOB OpSources operations/*.cc operations/gmic/*.cc legacy/operations/*.cc)

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


if(BUNDLED_EXIV2)
#  include(ExternalProject)
#  ExternalProject_Add(exiv2
#    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/exiv2
#    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/exiv2/configure --enable-static=yes --enable-shared=no --prefix=${CMAKE_CURRENT_BINARY_DIR}/exiv2-install
#    #CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/exiv2/configure --enable-static=yes --enable-shared=yes --prefix=${CMAKE_CURRENT_BINARY_DIR}/exiv2-install
#    BUILD_COMMAND ${MAKE})
#  set(EXIV2_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/exiv2-install/include)
#  set(EXIV2_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/exiv2-install/lib)
#  set(EXIV2_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/exiv2-install/lib/libexiv2.a)
  LIST(APPEND CMAKE_MODULE_PATH " ${CMAKE_SOURCE_DIR}/src/external/exiv2/config ")
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include/exiv2)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/external/exiv2/include/exiv2)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/external/exiv2/include)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/external/exiv2/src)
  set( EXIV2_ENABLE_SHARED OFF CACHE BOOL "Build exiv2 as a shared library (dll)" FORCE )
  set( EXIV2_ENABLE_BUILD_SAMPLES OFF CACHE BOOL "Build the unit tests" FORCE )
  set( EXIV2_ENABLE_CURL OFF CACHE BOOL "USE Libcurl for HttpIo" FORCE )
  set( EXIV2_ENABLE_SSH OFF CACHE BOOL "USE Libssh for SshIo" FORCE )
  add_library(exiv2_static STATIC IMPORTED)
  set_target_properties(exiv2_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/exiv2/src/libexiv2.a)
  add_subdirectory(external/exiv2)
  list(APPEND LIB_DEPS exiv2lib)
  #list(APPEND STATIC_LIBS exiv2_static)
  list(APPEND EXIV2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/external/exiv2/src/libexiv2.a")
  list(APPEND EXIV2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/external/exiv2/xmpsdk/libxmp.a")
endif(BUNDLED_EXIV2)
include_directories(${EXIV2_INCLUDE_DIRS})
#link_directories(${EXIV2_LIBRARY_DIRS})
MESSAGE( STATUS "EXIV2_LIBRARIES: " ${EXIV2_LIBRARIES} )



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


#if(BUNDLED_GEXIV2)
#  include(ExternalProject)
#  ExternalProject_Add(gexiv2
#    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2
#    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2/configure --enable-static=yes --enable-shared=no --prefix=${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install
#    #CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/gexiv2/configure --enable-static=yes --enable-shared=yes --prefix=${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install
#    BUILD_COMMAND ${MAKE})
#  set(GEXIV2_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/include)
#  set( GEXIV2_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/lib)
#  set(GEXIV2_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/gexiv2-install/lib/libgexiv2.a)
#endif(BUNDLED_GEXIV2)
#include_directories(${GEXIV2_INCLUDE_DIRS})
#link_directories(${GEXIV2_LIBRARY_DIRS})
#MESSAGE( STATUS "GEXIV2_LIBRARIES: " ${GEXIV2_LIBRARIES} )


# Film CLUTs
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gmic_film_cluts_download.cmake "file(DOWNLOAD http://gmic.eu/gmic_film_cluts.gmz ${CMAKE_BINARY_DIR}/gmic_film_cluts.gmz)")
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/gmic_film_cluts.gmz
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/gmic_film_cluts_download.cmake
  #COMMAND ${WGET_EXECUTABLE} http://gmic.eu/gmic_stdlib.h -O ${CMAKE_SOURCE_DIR}/src/gmic_stdlib.h
  COMMAND touch ${CMAKE_BINARY_DIR}/gmic_film_cluts.gmz
)
add_custom_target(gmic_film_cluts_download ALL DEPENDS ${CMAKE_BINARY_DIR}/gmic_film_cluts.gmz)



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
#  ${GMicIncludes}
#  ${GMicSources}
)


add_library (pfgmic STATIC
  ${GMicIncludes}
  ${GMicSources}
)


#add_dependencies(pfbase ${LIB_DEPS} gmic_film_cluts_download)
add_dependencies(pfbase ${LIB_DEPS})


#FILE(GLOB GuiIncludes gui/*.hh gui/widgets/*.hh gui/operations/*.hh gui/operations/gmic/*.hh legacy/gui/operations/*.hh)
#FILE(GLOB GuiSources gui/*.cc gui/widgets/*.cc gui/operations/*.cc gui/operations/gmic/*.cc legacy/gui/operations/*.cc)
FILE(GLOB GuiIncludes gui/*.hh gui/widgets/*.hh gui/operations/*.hh gui/operations/gmic/*.hh legacy/gui/operations/*.hh plugin/*.hh)
FILE(GLOB GuiSources gui/*.cc gui/widgets/*.cc gui/operations/*.cc gui/operations/gmic/*.cc legacy/gui/operations/*.cc plugin/pluginwindow.cc)

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
  list(APPEND ADDITIONAL_LIBS fftw3_threads pthread dl)
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
  pfgmic 
  pfdt
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${EXIV2_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${ICONV_LIBRARIES}
  ${STATIC_LIBS}
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  #${GEXIV2_LIBRARIES}
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



INSTALL(TARGETS photoflow RUNTIME DESTINATION bin)

#--------------------------------------------------
#
# phf_stack executable
#
set(BUILD_PHF_STACK 0)

IF(BUILD_PHF_STACK)
add_executable(phf_stack phf_stack.cc)
IF(APPLE)
  set_target_properties(phf_stack PROPERTIES LINK_FLAGS " -framework ApplicationServices ")
ENDIF(APPLE)

target_link_libraries(phf_stack ${LIBS} 
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${EXIV2_LIBRARIES}
  #${GEXIV2_LIBRARIES}
  ${GLIB_LIBRARIES} 
  ${GMODULE_LIBRARIES} 
  ${GOBJECT_LIBRARIES} 
  ${ZLIB_LIBRARIES}
  ${ADDITIONAL_LIBS}
  #gmon
  )

INSTALL(TARGETS phf_stack RUNTIME DESTINATION bin)
ENDIF(BUILD_PHF_STACK)


INSTALL(DIRECTORY ../presets DESTINATION share/photoflow)
INSTALL(DIRECTORY ../themes DESTINATION share/photoflow)
INSTALL(DIRECTORY ../icc DESTINATION share/photoflow)
INSTALL(DIRECTORY ../icons DESTINATION share/photoflow)
INSTALL(DIRECTORY ../help DESTINATION share/photoflow)
INSTALL(FILES ../src/vips/gmic/gmic/src/gmic_def.gmic DESTINATION share/photoflow)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/gmic_film_cluts.gmz DESTINATION share/photoflow/gmic)
INSTALL(FILES ../data/gmic_film_cluts.gmz DESTINATION share/photoflow)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/pluginrc DESTINATION share/photoflow/gimp-2.8)
#INSTALL(FILES ../data/gmic_film_cluts.gmz DESTINATION share/photoflow/gimp-2.8)




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


