#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g ")
#SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}  "-Wall -O0 -g") 
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g -p -DDO_WARNINGS -DPF_VERBOSE_UNREF -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${INSTALL_PREFIX}\"' ") 
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "-pg") 
#SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -funroll-loops") 
SET(CMAKE_CXX_FLAGS_TEST "-O3 -g -DNDEBUG -DLIBRAW_NODLL -DPF_VERBOSE_UNREF -DINSTALL_PREFIX='\"${INSTALL_PREFIX}\"' ") 
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${INSTALL_PREFIX}\"' ") 

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBMM_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
link_directories(${GLIBMM_LIBRARY_DIRS})
  
if(GTKMM2_FOUND)
  
  link_directories(
    ${GTKMM2_LIBRARY_DIRS}  )
  
  include_directories(    ${GTKMM2_INCLUDE_DIRS}  )
  
  #set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGTKMM_2")
  set(CMAKE_CXX_FLAGS "-DGTKMM_2")

  set(GTKMM_LIBRARIES ${GTKMM2_LIBRARIES})
  
endif()

if(GTKMM3_FOUND)
  
  link_directories(
    ${GTKMM3_LIBRARY_DIRS}  )
  
  include_directories(    ${GTKMM3_INCLUDE_DIRS}  )
  
  #set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGTKMM_3")
  set(CMAKE_CXX_FLAGS "-DGTKMM_3 ${CMAKE_CXX_FLAGS}")
  
  set(GTKMM_LIBRARIES ${GTKMM3_LIBRARIES})
  
endif()

include_directories(    ${GLIB_INCLUDE_DIRS}  )
include_directories(${SIGC2_INCLUDE_DIRS})

include_directories(${OPENEXR_INCLUDE_DIRS})
include_directories(${TIFF_INCLUDE_DIRS})
include_directories(${JPEG_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})

link_directories(${OPENEXR_LIBRARY_DIRS})
link_directories(${TIFF_LIBRARY_DIRS})
link_directories(${JPEG_LIBRARY_DIRS})
link_directories(${PNG_LIBRARY_DIRS})

include_directories(${XML2_INCLUDE_DIRS})

include_directories(${EXIF_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})

include_directories(${LENSFUN_INCLUDE_DIRS})
link_directories(${LENSFUN_LIBRARY_DIRS})

include_directories(${ORC_INCLUDE_DIRS})

include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS}  )

include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS}  )
include_directories(${PANGOFT2_INCLUDE_DIRS})
link_directories(${PANGOFT2_LIBRARY_DIRS}  )

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS}  )
include_directories(${GMODULE_INCLUDE_DIRS})
link_directories(${GMODULE_LIBRARY_DIRS}  )
include_directories(${GOBJECT_INCLUDE_DIRS})
link_directories(${GOBJECT_LIBRARY_DIRS}  )
include_directories(${GTHREAD_INCLUDE_DIRS})
link_directories(${GTHREAD_LIBRARY_DIRS}  )


#include_directories(${LIBXML_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${LIBXML_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/LibRaw)

#link_directories(${LIBRAW_LIBRARY_DIRS}  )
link_directories(${VIPS_LIBRARY_DIRS}  )
link_directories(${VIPSCC_LIBRARY_DIRS}  )
include_directories(${VIPS_INCLUDE_DIRS})
include_directories(${VIPSCC_INCLUDE_DIRS})


#
# the libraw part is a bit of a hack:
# the static linking didn't work since it was pulling -lstdc++ and -lm into linker flags.
# so we do a custom dependency and pretend an imported liblibraw_r.a so no other -l are
# appended.
#
add_library(libraw_static STATIC IMPORTED)
set_target_properties(libraw_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external/LibRaw/liblibraw_r.a)
#add_subdirectory(external/LibRaw)
#list(APPEND LIB_DEPS libraw_r)
#list(APPEND STATIC_LIBS libraw_static)

set(LibRawSources external/LibRaw/internal/dcraw_common.cpp external/LibRaw/src/libraw_cxx.cpp external/LibRaw/internal/dcraw_fileio.cpp external/LibRaw/internal/demosaic_packs.cpp external/LibRaw/src/libraw_c_api.cpp external/LibRaw/src/libraw_datastream.cpp)


FILE(GLOB LibRawIncludes external/LibRaw/libraw/*.h)
#FILE(GLOB LibRawSources external/LibRaw/src/*.cpp external/LibRaw/internal/*.cpp)

FILE(GLOB GuiIncludes gui/*.hh gui/widgets/*.hh gui/operations/*.hh gui/operations/gmic/*.hh)
FILE(GLOB GuiSources gui/*.cc gui/widgets/*.cc gui/operations/*.cc gui/operations/gmic/*.cc)

FILE(GLOB BaseIncludes base/*.hh)
FILE(GLOB BaseSources base/*.cc)

FILE(GLOB DTIncludes dt/common/*.h)
FILE(GLOB DTSources dt/common/*.c dt/external/*.c)

FILE(GLOB RTIncludes rt/*.hh rt/rtengine/*.h rt/rtengine/*.hh rt/rtexif/*.h rt/rtexif/*.hh rt/rtgui/*.h)
FILE(GLOB RTSources rt/*.cc rt/rtengine/*.c rt/rtengine/*.cc rt/rtexif/*.cc rt/rtgui/*.cc)
#FILE(GLOB RTIncludes rt/rtengine/*.h rt/rtengine/*.hh rt/rtexif/*.h rt/rtexis/*.hh rt/rtgui/*.h)
#FILE(GLOB RTSources rt/rtengine/*.cc rt/rtexif/*.cc rt/rtgui/*.cc)

#FILE(GLOB VipsIncludes vips/plugin/*.h vips/plugin/*.hh)
#FILE(GLOB VipsSources vips/plugin/*.c vips/plugin/*.cc)
FILE(GLOB VipsIncludes vips/*.h vips/*.hh vips/gmic/*.h)
FILE(GLOB VipsSources vips/*.c vips/*.cc vips/gmic/*.c vips/gmic/*.cpp)

FILE(GLOB GMicIncludes vips/gmic/gmic/src/*.h)
FILE(GLOB GMicSources vips/gmic/gmic/src/gmic.cpp)

FILE(GLOB OpIncludes operations/*.hh operations/gmic/*.hh)
FILE(GLOB OpSources operations/*.cc operations/gmic/*.cc)

add_library (pfbase
  ${BaseIncludes}
  ${BaseSources}
  ${RTIncludes}
  ${RTSources}
  ${OpIncludes}
  ${OpSources}
  ${LibRawIncludes}
  ${LibRawSources}
)

add_library (pfdt
  ${DTIncludes}
  ${DTSources}
)

SET_TARGET_PROPERTIES(pfdt PROPERTIES COMPILE_FLAGS "-std=gnu99")

add_library (pfvips #SHARED
  ${VipsIncludes}
  ${VipsSources}
)

add_library (pfgmic
  ${GMicIncludes}
  ${GMicSources}
)

#SET_TARGET_PROPERTIES(pfgmic PROPERTIES COMPILE_FLAGS "-Dgmic_build -W  -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fopenmp -Dcimg_use_openmp -fno-ipa-sra -mtune=generic")

#add_library (pfoperations
#  ${OpIncludes}
#  ${OpSources}
#)

#SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_CFLAGS "-fPIC -DPIC")
IF(MINGW)
  SET_TARGET_PROPERTIES(pfgmic PROPERTIES COMPILE_FLAGS "-Dgmic_build -W  -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -mtune=generic")
  SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_FLAGS "-Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fno-ipa-sra -mtune=generic -fpermissive")
ELSEIF(APPLE)
  SET_TARGET_PROPERTIES(pfgmic PROPERTIES COMPILE_FLAGS "-Dgmic_build -W  -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -mtune=generic")
  SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_FLAGS "-Dgmic_build -Dcimg_use_vt100 -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -Dcimg_use_fftw3_singlethread -mtune=generic -fpermissive")
ELSE(MINGW)
  SET_TARGET_PROPERTIES(pfgmic PROPERTIES COMPILE_FLAGS "-Dgmic_build -W  -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fopenmp -Dcimg_use_openmp -fno-ipa-sra -mtune=generic")
  SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_FLAGS "-Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_use_tiff -Dcimg_use_zlib -Dcimg_display=0 -fopenmp -Dcimg_use_openmp -fno-ipa-sra -mtune=generic -fpermissive")
ENDIF(MINGW)
#SET_TARGET_PROPERTIES(pfvips PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(pfvips PROPERTIES SUFFIX ".plg")


add_executable(photoflow # name of the executable on Windows will be example.exe 
  main.cc 
  ${GuiIncludes}
  ${GuiSources}
  )

#add_dependencies(photoflow ${LIB_DEPS})

IF(MINGW)
  list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi tre_regex psapi)
ELSEIF(APPLE)
  list(APPEND ADDITIONAL_LIBS)
ELSE(MINGW)
  list(APPEND ADDITIONAL_LIBS fftw3_threads pthread gomp)
ENDIF(MINGW)

target_link_libraries(photoflow ${LIBS} 
  pfbase 
  pfdt
  pfvips 
  pfgmic 
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  ${EXIV2_LIBRARIES}
  ${LENSFUN_LIBRARIES}
  ${SIGC2_LIBRARIES}
  ${PANGO_LIBRARIES} ${PANGOFT2_LIBRARIES} 
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
  pfvips 
  pfgmic 
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  ${TIFF_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${OPENEXR_LIBRARIES}
  ${XML2_LIBRARIES}
  ${EXIF_LIBRARIES}
  ${EXIV2_LIBRARIES}
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
INSTALL(DIRECTORY ../help DESTINATION share/photoflow)
INSTALL(FILES ../src/vips/gmic/gmic/src/gmic_def.gmic DESTINATION share/photoflow)


add_executable(vector tests/vector.cc)
INSTALL(TARGETS vector RUNTIME DESTINATION bin)

#add_executable(invert # name of the executable on Windows will be example.exe 
#  benchmarks/invert.cc 
#  )
#SET_TARGET_PROPERTIES(invert PROPERTIES COMPILE_FLAGS "-fpermissive")

#target_link_libraries(invert ${LIBS} pfbase pfvips ${SIGC2_LIBRARIES}
#  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
#  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES} 
#  )

#add_executable(refcount # name of the executable on Windows will be example.exe 
#  tests/refcount.cc 
#  )
#SET_TARGET_PROPERTIES(refcount PROPERTIES COMPILE_FLAGS "-fpermissive")

#target_link_libraries(refcount ${LIBS}   ${SIGC2_LIBRARIES}
#  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
#  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES} 
#  )


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


