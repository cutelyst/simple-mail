cmake_minimum_required(VERSION 3.9)
project(libsimplemail VERSION 3.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

if (NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network REQUIRED)
endif ()
find_package(Qt${QT_VERSION_MAJOR} 5.15.0 COMPONENTS Core Network REQUIRED)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Include our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build in shared lib mode" ON)
option(BUILD_WITH_SMIME "Build with SMIME Support" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Forbid in-tree building
if(CMAKE_CURRENT_SOURCE_DIR MATCHES CMAKE_BINARY_DIR)
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif()

#
# Options
#
option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)

# NONE

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "simplemail default install prefix" FORCE)
endif()

option(BUILD_DEMOS "Build the demos" ON)

#
# Custom C flags
#
if (ENABLE_MAINTAINER_CFLAGS AND NOT MSVC)
    set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")
    add_definitions(${MAINTAINER_CFLAGS}
        -DQT_DEPRECATED_WARNINGS
    )
endif ()

# cmake config files
configure_file(${CMAKE_MODULE_PATH}/simplemailqtX-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Config.cmake
    @ONLY
)
configure_file(${CMAKE_MODULE_PATH}/simplemailqtX-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ConfigVersion.cmake
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}/
)

install(EXPORT SimpleMailTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}/
    FILE SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Targets.cmake
    NAMESPACE SimpleMail::
    COMPONENT Devel
)

if(BUILD_WITH_SMIME)
    find_package(OpenSSL REQUIRED)
endif()

add_subdirectory(src)
if (BUILD_DEMOS)
    add_subdirectory(demos)
endif ()

include(CPackConfig)
