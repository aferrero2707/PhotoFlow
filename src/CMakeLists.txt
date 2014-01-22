#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g ")
#SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}  "-Wall -O0 -g") 
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g -p") 

if(GTKMM2_FOUND)
  
  link_directories(
    ${GTKMM2_LIBRARY_DIRS}  )
  
  include_directories(    ${GTKMM2_INCLUDE_DIRS}  )
  
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-DGTKMM_2")

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


include_directories(${LIBXML_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBXML_LIBRARIES})


link_directories(${VIPS_LIBRARY_DIRS}  )
link_directories(${VIPSCC_LIBRARY_DIRS}  )
include_directories(${VIPS_INCLUDE_DIRS})
include_directories(${VIPSCC_INCLUDE_DIRS})


FILE(GLOB GuiIncludes gui/*.hh)
FILE(GLOB GuiSources gui/*.cc)

FILE(GLOB BaseIncludes base/*.hh)
FILE(GLOB BaseSources base/*.cc)

#FILE(GLOB VipsIncludes vips/plugin/*.h vips/plugin/*.hh)
#FILE(GLOB VipsSources vips/plugin/*.c vips/plugin/*.cc)
FILE(GLOB VipsIncludes vips/*.h vips/*.hh)
FILE(GLOB VipsSources vips/*.c vips/*.cc)

FILE(GLOB OpIncludes operations/*.hh)
FILE(GLOB OpSources operations/*.cc)

add_library (pfbase
  ${BaseIncludes}
  ${BaseSources}
)

add_library (pfvips #SHARED
  ${VipsIncludes}
  ${VipsSources}
)

add_library (pfoperations
  ${OpIncludes}
  ${OpSources}
)

#SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_CFLAGS "-fPIC -DPIC")
SET_TARGET_PROPERTIES(pfvips PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC -DPIC")
#SET_TARGET_PROPERTIES(pfvips PROPERTIES PREFIX "")
#SET_TARGET_PROPERTIES(pfvips PROPERTIES SUFFIX ".plg")


add_executable(photoflow # name of the executable on Windows will be example.exe 
  main.cc 
  ${GuiIncludes}
  ${GuiSources}
  )


target_link_libraries(photoflow ${LIBS} pfbase pfoperations pfvips ${SIGC2_LIBRARIES}
  ${GTKMM_LIBRARIES} ${TIFF_LIBRARIES} ${LCMS2_LIBRARIES} 
  ${VIPS_LIBRARIES} ${VIPSCC_LIBRARIES}  dl
  )


