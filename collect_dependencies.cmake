function(ConfigurePlatform DevelopmentList DockerList PackageList)
    if("${IS_DOCKER}")
        message("-- Running in Docker")
        add_definitions(-DDOCKER)
        set(PKG_INSTALL_CMD "apk" "add" PARENT_SCOPE)
        set(PKG_MANAGER "apk" PARENT_SCOPE)
        set(CAN_INSTALL TRUE PARENT_SCOPE)

        # Apk package names.
        # '-' indicates a package that cannot be installed from apk
        # for example pangocairo which is part of cairo. If we fail to
        # locate pangocairo, we *have* to abort.
        set(PackageList "${DockerList}" PARENT_SCOPE)
    else()
        message("-- Running on development")
        set(PKG_INSTALL_CMD "brew" "install" PARENT_SCOPE)
        set(PKG_MANAGER "brew" PARENT_SCOPE)

        if(NOT CAN_INSTALL)
            # Check if brew is installed
            execute_process(COMMAND brew --version RESULT_VARIABLE _EXIT_CODE OUTPUT_QUIET ERROR_QUIET)
            if(NOT _EXIT_CODE EQUAL 0)
                message(STATUS "Couldn't find brew in PATH. If libraries or packages are missing they will not be installed, and build will fail")
                set(CAN_INSTALL FALSE)
            else()
                message("-- Found brew. Missing libraries will be installed automatically")
                set(CAN_INSTALL TRUE CACHE INTERNAL "Whether we can install packages")
            endif()
        else()
            message("-- Found brew. Missing libraries will be installed automatically")
        endif()
        # Brew package names in case we need to install them.
        # '-' indicates a package that cannot be installed from brew
        # for example pangocairo which is part of cairo. If we fail to
        # locate pangocairo, we *have* to abort.
        set(PackageList "${DevelopmentList}" PARENT_SCOPE)
    endif()
endfunction()

function(FindDependecies Dependencies Packages CFlags Libs IncludeDirs)
    list(APPEND CMAKE_MESSAGE_INDENT "   ")
    foreach(PKG INSTALL IN ZIP_LISTS "${Dependencies}" "${Packages}")
        if("${PKG}" STREQUAL "pkg-config" OR (NOT ${PKG}_CFLAGS_OTHER AND NOT ${PKG}_LINK_LIBRARIES AND NOT ${PKG}_INCLUDE_DIRS))
            if(NOT "${PKG}" STREQUAL "pkg-config")
                pkg_check_modules(${PKG} QUIET ${PKG})
            else()
                find_package(PkgConfig QUIET)
                if(PkgConfig_FOUND)
                    set(${PKG}_FOUND "1")
                else()
                    set(${PKG}_FOUND "0")
                endif()
            endif()
            if(${PKG}_FOUND EQUAL 1)
                message("-- Found ${PKG}")
            else()
                if("${INSTALL}" STREQUAL "-" OR NOT CAN_INSTALL)
                    message(FATAL_ERROR "-- Couldn't install ${PKG}")
                endif()
                message("-- Couldn't find ${PKG}. Attempting to install...")
                execute_process(COMMAND ${PKG_INSTALL_CMD} "${INSTALL}" RESULT_VARIABLE _EXIT_CODE OUTPUT_QUIET ERROR_QUIET)
                if(NOT _EXIT_CODE EQUAL 0)
                    message(FATAL_ERROR "-- Couldn't install ${PKG}: ${PKG_MANAGER} exited with ${_EXIT_CODE}")
                else()
                    list(APPEND CMAKE_MESSAGE_INDENT "   ")
                    message("-- Successfully installed ${PKG}")
                    list(POP_BACK CMAKE_MESSAGE_INDENT)
                endif()

                if(NOT "${PKG}" STREQUAL "pkg-config")
                    pkg_check_modules(${PKG} QUIET ${PKG})
                else()
                    find_package(PkgConfig QUIET)
                    if(PkgConfig_FOUND)
                        set(${PKG}_FOUND "1")
                    else()
                        set(${PKG}_FOUND "0")
                    endif()
                endif()

                if(${PKG}_FOUND EQUAL 1)
                    message("-- Found ${PKG}")
                else()
                    message(FATAL_ERROR "Still couldn't find ${PKG}: '${${PKG}_FOUND}'")
                endif()
            endif()
            set(${PKG}_CFLAGS_OTHER "${${PKG}_CFLAGS_OTHER}" CACHE INTERNAL "${PKG} C-Flags")
            set(${PKG}_LINK_LIBRARIES "${${PKG}_LINK_LIBRARIES}" CACHE INTERNAL "${PKG} Library Paths and Flags")
            set(${PKG}_INCLUDE_DIRS "${${PKG}_INCLUDE_DIRS}" CACHE INTERNAL "${PKG} Include Directories")
        else()
            message("-- Found ${PKG}")
        endif()
        list(APPEND ${CFlags} ${${PKG}_CFLAGS_OTHER})
        list(APPEND ${Libs} ${${PKG}_LINK_LIBRARIES})
        list(APPEND ${IncludeDirs} ${${PKG}_INCLUDE_DIRS})
    endforeach()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
    set(${CFlags} ${${CFlags}} PARENT_SCOPE)
    set(${Libs} ${${Libs}} PARENT_SCOPE)
    set(${IncludeDirs} ${${IncludeDirs}} PARENT_SCOPE)

endfunction(FindDependecies Dependencies)

function(FetchSubmodules Submodules Directory)
    message("-- Checking for submodules")
    if(NOT $IS_DOCKER)
        # Collect submodules  

        # Fetch all existing submodules
        execute_process(COMMAND git submodule update --remote
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        RESULT_VARIABLE _EXIT_CODE
                        OUTPUT_QUIET ERROR_QUIET)

        if($_EXIT_CODE EQUAL 0)
            message(FATAL_ERROR "-- Failed to update submodules — check your git configuration")
        endif()

        list(APPEND CMAKE_MESSAGE_INDENT "   ")
        foreach(LIB ${${Submodules}})
            # Check if submodule has been initialised, i.e. fetched
            if(EXISTS ${${Directory}}/${LIB}/.gitignore)
                # Nothing to do here
                message("-- Found ${LIB}")
            else()
                # Initialise the submodule
                message("-- Fetching ${LIB}...")
                execute_process(COMMAND git submodule update --init -- libs/${LIB}
                                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
                                RESULT_VARIABLE _EXIT_CODE
                                OUTPUT_QUIET ERROR_QUIET)
                if(NOT $_EXIT_CODE EQUAL 0)
                    message("-- Successfully fetched ${LIB}")
                else()
                    message(FATAL_ERROR "-- Failed to fetch ${LIB} — check your git configuration")
                endif()
            endif()

            # Add the library as a dependency to build with this project
            message("-- Configuring ${LIB}")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${${Directory}}/${LIB})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endforeach()
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    else()
        execute_process(COMMAND git submodule update --init --recursive
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
                        RESULT_VARIABLE _EXIT_CODE
                        OUTPUT_QUIET ERROR_QUIET)
        if(NOT $_EXIT_CODE EQUAL 0)
            message("-- Successfully fetched submodules")
        else()
            message(FATAL_ERROR "-- Failed to fetch submodules — check your git configuration")
        endif()
        foreach(LIB ${Submodules})
            add_subdirectory(${${Directory}}/${LIB})
        endforeach()
    endif()    
endfunction(FetchSubmodules)
