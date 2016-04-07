cmake_minimum_required(VERSION 2.6)

add_subdirectory(pixmaps)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/photoflow.desktop DESTINATION ./share/applications)
#install(FILES darktable.appdata.xml DESTINATION ./share/appdata)

