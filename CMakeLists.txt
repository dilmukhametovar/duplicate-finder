cmake_minimum_required(VERSION 3.16)

project(duplicate-finder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/core/HashWorker.cpp
    src/model/DuplicateGroupsModel.cpp
    src/core/DuplicateScanner.cpp
    src/utils/fileutils.cpp
)

set(HEADERS
    src/ui/mainwindow.h
    src/core/FileEntry.h
    src/core/CancellationToken.h
    src/core/DuplicateScanner.h
    src/core/HashWorker.h
    src/model/DuplicateGroupsModel.h
    src/utils/fileutils.h
)

add_executable(duplicate-finder
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(duplicate-finder PRIVATE src)

target_link_libraries(duplicate-finder PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_options(duplicate-finder PRIVATE -Wall -Wextra -Wpedantic)

include(GNUInstallDirs)
install(TARGETS duplicate-finder
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(duplicate-finder)
endif()
