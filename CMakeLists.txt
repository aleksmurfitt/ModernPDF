cmake_minimum_required(VERSION 3.13)

project(PDF-Generator VERSION 1.0
                  DESCRIPTION "Component to generate Thirdfort PDFs"
                  LANGUAGES CXX)

include(collect_dependencies.cmake)

set(APK_PACKAGES
    pkgconfig)

set(BREW_PACKAGES
    pkg-config
    icu4c
    harfbuzz)

# Required dynamic libraries
set(PKG_CONF_MODS
    pkg-config
    icu-i18n
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

if("${IS_DOCKER}")
    message("Installing binaries...")
    install(TARGETS main RUNTIME DESTINATION bin)
endif()