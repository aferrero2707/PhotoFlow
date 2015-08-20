IF(MINGW)
  SET(GMIC_FLAGS "-std=gnu++11 -Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -fpermissive")
ELSEIF(APPLE)
   #SET(GMIC_FLAGS "-DPF_DISABLE_GMIC -std=c++11 -Wno-error=c++11-narrowing -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
   SET(GMIC_FLAGS "-std=c++11 -Wno-error=c++11-narrowing -Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -fpermissive")
ELSE(MINGW)
  SET(GMIC_FLAGS "-std=gnu++11 -Wno-error=narrowing -Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -fpermissive")
ENDIF(MINGW)

set(COMPILE_FLAGS "${GMIC_FLAGS} -I${CMAKE_SOURCE_DIR}/src/dt -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${INSTALL_PREFIX}\"' ")

if(Msgfmt_FOUND)
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -DLOCALEDIR=\"${LOCALE_DIR}\"")
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

SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g -DNDEBUG -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_VERBDEBUG "-Wall -O0 -g -DDO_WARNINGS ${COMPILE_FLAGS}") 
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${COMPILE_FLAGS}  -Wno-unused-parameter") 
SET(CMAKE_CXX_FLAGS_TEST "-O3 -g -DNDEBUG ${COMPILE_FLAGS}") 

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/LibRaw)

if(LENSFUN_FOUND)
  link_directories( ${LENSFUN_LIBRARY_DIRS}  )
  include_directories( ${LENSFUN_INCLUDE_DIRS}  )  
  set(CMAKE_CXX_FLAGS "-DPF_HAS_LENSFUN")
endif()

include_directories(${OPENEXR_INCLUDE_DIRS})
include_directories(${TIFF_INCLUDE_DIRS})
include_directories(${JPEG_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})

link_directories(${OPENEXR_LIBRARY_DIRS})
link_directories(${TIFF_LIBRARY_DIRS})
link_directories(${JPEG_LIBRARY_DIRS})
link_directories(${PNG_LIBRARY_DIRS})

include_directories(${XML2_INCLUDE_DIRS})
link_directories(${XML2_LIBRARY_DIRS})

include_directories(${LCMS22_INCLUDE_DIRS})
link_directories(${LCMS22_LIBRARY_DIRS})

include_directories(${EXIF_INCLUDE_DIRS})
link_directories(${EXIF_LIBRARY_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})
link_directories(${EXIV2_LIBRARY_DIRS})
include_directories(${GEXIV2_INCLUDE_DIRS})
link_directories(${GEXIV2_LIBRARY_DIRS})

include_directories(${ORC_INCLUDE_DIRS})

include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS}  )

include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS}  )
include_directories(${PANGOFT2_INCLUDE_DIRS})
link_directories(${PANGOFT2_LIBRARY_DIRS}  )

include_directories(${VIPS_INCLUDE_DIRS})
link_directories(${VIPS_LIBRARY_DIRS}  )


FILE(GLOB DTIncludes dt/common/*.h)
FILE(GLOB DTSources dt/common/*.c)

add_library (pfdt STATIC
  ${DTIncludes}
  ${DTSources}
)

SET_TARGET_PROPERTIES(pfdt PROPERTIES COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/src/dt -std=gnu99")


FILE(GLOB BaseIncludes base/*.hh)
FILE(GLOB BaseSources base/*.cc)

#FILE(GLOB RTIncludes rt/*.hh rt/rtengine/*.h rt/rtengine/*.hh rt/rtexif/*.h rt/rtexif/*.hh rt/rtgui/*.h)
#FILE(GLOB RTSources rt/*.cc rt/rtengine/*.c rt/rtengine/*.cc rt/rtexif/*.cc rt/rtgui/*.cc)
FILE(GLOB RTIncludes 
  rt/rtengine/rawimagesource.h #rt/rtengine/rt_math.h rt/rtengine/opthelper.h rt/rtengine/sleef.c rt/rtengine/sleefsseavx.c rt/rtengine/helpersse2.h
)
FILE(GLOB RTSources 
  rt/rtengine/rawimagesource.cc
  rt/rtengine/amaze_demosaic_RT.cc
  rt/rtengine/igv_demosaic_RT.cc
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
add_library(libraw_static STATIC IMPORTED)
set_target_properties(libraw_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/LibRaw/liblibraw_r.a)
add_subdirectory(external/LibRaw)
list(APPEND LIB_DEPS libraw_r)
list(APPEND STATIC_LIBS libraw_static)

#FILE(GLOB LibRawIncludes external/LibRaw/libraw/*.h)
#set(LibRawSources external/LibRaw/internal/dcraw_common.cpp external/LibRaw/src/libraw_cxx.cpp external/LibRaw/internal/dcraw_fileio.cpp external/LibRaw/internal/demosaic_packs.cpp external/LibRaw/src/libraw_c_api.cpp external/LibRaw/src/libraw_datastream.cpp)


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
#target_link_libraries(pfbase ${STATIC_LIBS})


FILE(GLOB GuiIncludes gui/*.hh gui/widgets/*.hh gui/operations/*.hh gui/operations/gmic/*.hh)
FILE(GLOB GuiSources gui/*.cc gui/widgets/*.cc gui/operations/*.cc gui/operations/gmic/*.cc)

add_library (pfgui STATIC
  ${GuiIncludes}
  ${GuiSources}
)


IF(MINGW)
  #list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi tre_regex psapi)
  list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi psapi)
ELSEIF(APPLE)
  list(APPEND ADDITIONAL_LIBS)
ELSE(MINGW)
  list(APPEND ADDITIONAL_LIBS fftw3_threads pthread gomp)
ENDIF(MINGW)

add_executable(photoflow # name of the executable on Windows will be example.exe 
  main.cc 
  )

#add_dependencies(photoflow ${LIB_DEPS})

target_link_libraries(photoflow ${LIBS} 
  pfgui
  pfbase 
  pfdt
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  ${EXIV2_LIBRARIES}
  ${GEXIV2_LIBRARIES}
  ${LENSFUN_LIBRARIES}
  ${SIGC2_LIBRARIES}
  ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
  ${PIXMAN_LIBRARIES} 
  ${GTKMM_LIBRARIES} 
  ${GLIBMM_LIBRARIES} 
  ${GLIB_LIBRARIES} 
  ${GMODULE_LIBRARIES} 
  ${GOBJECT_LIBRARIES} 
  ${GTHREAD_LIBRARIES} 
  ${ZLIB_LIBRARIES}
  #${LIBRAW_LIBRARIES}  
  ${STATIC_LIBS}
  ${ORC_LIBRARIES}
  fftw3
  ${ADDITIONAL_LIBS}
  #gmon
  )



add_executable(pfbatch pfbatch.cc)

#add_dependencies(photoflow ${LIB_DEPS})

target_link_libraries(pfbatch ${LIBS} 
  pfbase 
  pfdt
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
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
  ${STATIC_LIBS}
  ${ORC_LIBRARIES}
  fftw3
  ${ADDITIONAL_LIBS}
  #gmon
  )



INSTALL(TARGETS photoflow RUNTIME DESTINATION bin)
INSTALL(TARGETS pfbatch RUNTIME DESTINATION bin)

INSTALL(DIRECTORY ../themes DESTINATION share/photoflow)
INSTALL(DIRECTORY ../icons DESTINATION share/photoflow)
INSTALL(DIRECTORY ../help DESTINATION share/photoflow)
INSTALL(FILES ../src/vips/gmic/gmic/src/gmic_def.gmic DESTINATION share/photoflow)


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


