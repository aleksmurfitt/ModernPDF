cmake_minimum_required(VERSION 3.12...3.24)

if(${CMAKE_VERSION} VERSION_LESS 3.19)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
project(PDF-Generator VERSION 1.0
                  DESCRIPTION "Library to generate PDFs"
                  LANGUAGES CXX)

include(cmake/collect_dependencies.cmake)

set(APK_PACKAGES
    pkgconfig
    harfbuzz)

set(BREW_PACKAGES
    pkg-config
    harfbuzz
    -)

# Required dynamic libraries
set(PKG_CONF_MODS
    pkg-config
    harfbuzz
    harfbuzz-subset)

# Submodules
set(SUB_MODS)

# Submodule directory
set(SUB_MOD_DIR "${PROJECT_SOURCE_DIR}/libs")

# Library names, etc. will depend on platform
ConfigurePlatform(BREW_PACKAGES APK_PACKAGES INSTALL_MODS)

FindDependecies(PKG_CONF_MODS INSTALL_MODS CFLAGS LIBS INCLUDE_DIRS)

FetchSubmodules(SUB_MODS SUB_MOD_DIR)

find_package(qpdf REQUIRED)

add_subdirectory(src)
add_subdirectory(examples)

if("${IS_DOCKER}")
    message("Installing binaries...")
    install(TARGETS main RUNTIME DESTINATION bin)
endif()
