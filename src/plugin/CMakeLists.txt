FILE(GLOB PluginIncludes *.hh)
FILE(GLOB PluginSources *.cc)

exec_program(gimptool-2.0 ARGS --cflags OUTPUT_VARIABLE GIMP_C_FLAGS)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GIMP_C_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GIMP_C_FLAGS}")
exec_program(gimptool-2.0 ARGS --libs OUTPUT_VARIABLE GIMP_LIBS)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GIMP_LIBS}")

add_executable(pfgimp # name of the executable on Windows will be example.exe 
  ${PluginSources}
  )

set_target_properties(pfgimp PROPERTIES COMPILE_FLAGS "${GIMP_C_FLAGS} -DHAVE_GIMP_2_9=${HAVE_GIMP_2_9}")
#set_target_properties(pfgimp PROPERTIES LINK_FLAGS "${GIMP_LIBS}")
#set_target_properties(pfgimp PROPERTIES LINK_FLAGS " -L/Users/aferrero/gtk/inst/lib -lgimpui-2.0 -lgimpwidgets-2.0 -lgimpmodule-2.0 -lgimp-2.0 -lgimpmath-2.0 -lgimpconfig-2.0 -lgimpcolor-2.0 -lgimpbase-2.0 -lgtk-quartz-2.0 -lgdk-quartz-2.0 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl ")

target_link_libraries(pfgimp ${LIBS} 
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
  ${MACINTEG_LIBS}
  ${GIMP_LIBS}
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
  
INSTALL(TARGETS pfgimp RUNTIME DESTINATION share/photoflow/plug-in)
  
#SET_TARGET_PROPERTIES(unprocessed_raw PROPERTIES LINK_FLAGS "-fopenmp")

#target_link_libraries(unprocessed_raw ${LIBS} ${SIGC2_LIBRARIES}
#  ${TIFF_LIBRARIES} ${LIBRAW_LIBRARIES} ${LCMS2_LIBRARIES} dl)


