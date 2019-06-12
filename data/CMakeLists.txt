cmake_minimum_required(VERSION 2.6)

add_subdirectory(pixmaps)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/photoflow.desktop DESTINATION ./share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/photoflow.appdata.xml DESTINATION ./share/metainfo)
#install(FILES darktable.appdata.xml DESTINATION ./share/appdata)


if (OpenColorIO_FOUND)
    include(ExternalProject)

    ExternalProject_Add(filmic-blender
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
        DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/filmic-blender-download"
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/filmic-blender"
        URL "https://github.com/sobotka/filmic-blender/archive/1.1.1.tar.gz"
        TIMEOUT 10
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND     ""
    )
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/filmic-blender" DESTINATION ./share/photoflow/ocio-configs)
    
    ExternalProject_Add(aces-ocio
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
        DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/ocio-configs-download"
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ocio-configs"
        GIT_REPOSITORY "https://github.com/imageworks/OpenColorIO-Configs.git"
        GIT_TAG 0bb079c08be410030669cbf5f19ff869b88af953
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND     ""
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ocio-configs/aces_1.0.3/config.ocio" DESTINATION ./share/photoflow/ocio-configs/aces_1.0.3)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ocio-configs/aces_1.0.3/README.md" DESTINATION ./share/photoflow/ocio-configs/aces_1.0.3)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ocio-configs/aces_1.0.3/luts" DESTINATION ./share/photoflow/ocio-configs/aces_1.0.3)
endif (OpenColorIO_FOUND)
