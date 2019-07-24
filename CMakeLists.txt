cmake_minimum_required(VERSION 3.5)
project(libsimplemail VERSION 1.4.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Include our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build in shared lib mode" ON)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Core Network)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

#
# Options
#
option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)

# NONE

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "simplemail default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#
# Configure files
#
set (PREFIXDIR "${CMAKE_INSTALL_PREFIX}")
set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBNAME}/")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share")
set (PKGDATADIR "${DATA_INSTALL_DIR}")
set (LIBDIR "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
set (PKGLIBDIR "${LIBDIR}/simplemail")
set (GETTEXT_PACKAGE "simplemail")
set (LOCALE_DIR "${DATADIR}/locale")
set (VERSION "${SIMPLEMAIL_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Output directory for libraries")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
if (ENABLE_MAINTAINER_CFLAGS)
    set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")
    add_definitions(${MAINTAINER_CFLAGS}
        -DQT_DEPRECATED_WARNINGS
    )
endif ()

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050c00
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleMail
    ${CMAKE_CURRENT_BINARY_DIR}
)

# cmake config files
configure_file(${CMAKE_MODULE_PATH}/simplemailqt5-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/simplemailqt5-config.cmake
    @ONLY
)
configure_file(${CMAKE_MODULE_PATH}/simplemailqt5-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/simplemailqt5-config-version.cmake
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simplemailqt5-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/simplemailqt5-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simplemailqt5/
)
install(EXPORT SimpleMailQt5Targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/simplemailqt5/"
    FILE SimpleMailQt5Targets.cmake
    NAMESPACE SimpleMail::
    COMPONENT Devel
)

add_subdirectory(src)
add_subdirectory(demos)
