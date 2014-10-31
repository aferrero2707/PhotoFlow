#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g ")
#SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}  "-Wall -O0 -g") 
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g -p -DPF_VERBOSE_UNREF -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${CMAKE_INSTALL_PREFIX}\"' ") 
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "-pg") 
#SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -funroll-loops") 
SET(CMAKE_CXX_FLAGS_TEST "-O0 -g -DNDEBUG -DLIBRAW_NODLL -DPF_VERBOSE_UNREF ") 
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DLIBRAW_NODLL -DINSTALL_PREFIX='\"${CMAKE_INSTALL_PREFIX}\"' ") 

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
  
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGTKMM_3")
  
  set(GTKMM_LIBRARIES ${GTKMM3_LIBRARIES})
  
endif()

include_directories(${SIGC2_INCLUDE_DIRS})


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
  ${DTIncludes}
  ${DTSources}
  ${RTIncludes}
  ${RTSources}
  ${OpIncludes}
  ${OpSources}
  ${LibRawIncludes}
  ${LibRawSources}
)

SET_TARGET_PROPERTIES(pfbase PROPERTIES COMPILE_FLAGS "-std=gnu99")

add_library (pfvips #SHARED
  ${VipsIncludes}
  ${VipsSources}
)

add_library (pfgmic
  ${GMicIncludes}
  ${GMicSources}
)

SET_TARGET_PROPERTIES(pfgmic PROPERTIES COMPILE_FLAGS "-Dgmic_build -W  -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_display=0 -fno-ipa-sra -mtune=generic")

#add_library (pfoperations
#  ${OpIncludes}
#  ${OpSources}
#)

#SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_CFLAGS "-fPIC -DPIC")
SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_FLAGS "-Dgmic_build -Dcimg_use_vt100 -Dgmic_is_parallel -Dcimg_use_fftw3 -Dcimg_display=0 -fno-ipa-sra -mtune=generic -fpermissive")
#SET_TARGET_PROPERTIES(pfvips PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(pfvips PROPERTIES SUFFIX ".plg")


add_executable(photoflow # name of the executable on Windows will be example.exe 
  main.cc 
  ${GuiIncludes}
  ${GuiSources}
  )

#add_dependencies(photoflow ${LIB_DEPS})

IF(MINGW)
list(APPEND ADDITIONAL_LIBS ws2_32 shlwapi)
ELSE(MINGW)
list(APPEND ADDITIONAL_LIBS fftw3_threads)
ENDIF(MINGW)

target_link_libraries(photoflow ${LIBS} 
  pfbase 
  pfvips 
  pfgmic 
  ${SIGC2_LIBRARIES}
  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}
  #${LIBRAW_LIBRARIES}  
  ${STATIC_LIBS}
  fftw3
  ${ADDITIONAL_LIBS}
  #gmon
  )

INSTALL(TARGETS photoflow RUNTIME DESTINATION bin)
INSTALL(DIRECTORY ../themes DESTINATION share/photoflow)

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


