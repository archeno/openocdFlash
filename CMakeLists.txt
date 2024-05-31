cmake_minimum_required(VERSION 3.25)

project(openocdFlash VERSION 1.0.2 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/versionConfig.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/versionConfig.h)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

if(ANDROID)
  add_library(openocdFlash SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
else()
  add_executable(openocdFlash WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    res.qrc
    myapp.rc
    decoder.c
    decoder.h
  )
add_subdirectory(cJSON)
endif()

target_link_libraries(openocdFlash PRIVATE Qt5::Widgets)
