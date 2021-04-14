cmake_minimum_required(VERSION 3.5)

project(qtplayer LANGUAGES CXX)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "PREFIX=/usr" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(PkgConfig)
pkg_check_modules(MPV REQUIRED mpv)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        qthelper.hpp
        mpvwidget.cpp
        mpvwidget.h
        playlistwidget.cpp
        playlistwidget.h
        controlswidget.cpp
        controlswidget.h
        seekbar.cpp
        seekbar.h
        volumebar.cpp
        volumebar.h
        settings.cpp
        settings.h
)
set(QTPLAYER_RES qtplayer.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtplayer
        ${PROJECT_SOURCES}
        ${QTPLAYER_RES}
    )
else()
    add_executable(qtplayer
        ${PROJECT_SOURCES}
        ${QTPLAYER_RES}
    )
endif()

target_link_libraries(qtplayer
    Qt${QT_VERSION_MAJOR}::Widgets
    ${MPV_LIBRARIES})


install(TARGETS qtplayer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
set(DESKTOP_FILE res/qtplayer.desktop)
set(ICON_FILE res/qtplayer.svg)
install(FILES ${DESKTOP_FILE} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
install(FILES ${ICON_FILE} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps")
