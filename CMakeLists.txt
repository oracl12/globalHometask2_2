cmake_minimum_required(VERSION 3.5)

project(222 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (LINUX)
    set(CMAKE_PREFIX_PATH "/home/oracl/Qt/6.6.0/gcc_64/lib/cmake")
endif (LINUX)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if (WIN32)
find_library(PDH_LIBRARY Pdh)
endif (WIN32)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(222
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        loadabiity.cpp
        loadability.h
        other.cpp
        other.h
    )

else()
    if(ANDROID)
        add_library(222 SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(222
            ${PROJECT_SOURCES}
        )
    endif()
endif()

if (WIN32)
    target_link_libraries(222 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${PDH_LIBRARY} iphlpapi)
else()
   target_link_libraries(222 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
endif()

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.222)
endif()
set_target_properties(222 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS 222
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(222)
endif()
