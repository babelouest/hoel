#
# Hoel library
#
# CMake file used to build all programs
#
# Copyright 2018 Silvio Clecio <silvioprog@gmail.com>
# Copyright 2018-2023 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

cmake_minimum_required(VERSION 3.14)

project(hoel C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# library info

set(HOEL_LIBS )

set(PROJECT_DESCRIPTION "C Database abstraction library with json based language")
set(PROJECT_HOMEPAGE_URL "https://github.com/babelouest/hoel/")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/hoel/issues")
set(LIBRARY_VERSION_MAJOR "1")
set(LIBRARY_VERSION_MINOR "4")
set(LIBRARY_VERSION_PATCH "28")
set(ORCANIA_VERSION_REQUIRED "2.3.3")
set(YDER_VERSION_REQUIRED "1.4.19")
set(JANSSON_VERSION_REQUIRED "2.4")

set(PROJECT_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(PROJECT_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

if (${LIBRARY_VERSION_MAJOR} VERSION_LESS 10)
    set (LIBRARY_VERSION_MAJOR_PAD "0${LIBRARY_VERSION_MAJOR}")
else ()
    set (LIBRARY_VERSION_MAJOR_PAD "${LIBRARY_VERSION_MAJOR}")
endif ()
if (${LIBRARY_VERSION_MINOR} VERSION_LESS 10)
    set (LIBRARY_VERSION_MINOR_PAD "0${LIBRARY_VERSION_MINOR}")
else ()
    set (LIBRARY_VERSION_MINOR_PAD "${LIBRARY_VERSION_MINOR}")
endif ()
if (${LIBRARY_VERSION_PATCH} VERSION_LESS 10)
    set (LIBRARY_VERSION_PATCH_PAD "0${LIBRARY_VERSION_PATCH}")
else ()
    set (LIBRARY_VERSION_PATCH_PAD "${LIBRARY_VERSION_PATCH}")
endif ()
set(PROJECT_VERSION_NUMBER "${LIBRARY_VERSION_MAJOR_PAD}${LIBRARY_VERSION_MINOR_PAD}${LIBRARY_VERSION_PATCH_PAD}")

set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(LIBRARY_SOVERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")

# cmake modules

set(H_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
list(APPEND CMAKE_MODULE_PATH "${H_CMAKE_MODULE_PATH}")

include(GNUInstallDirs)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)

# check if _GNU_SOURCE is available

if (NOT _GNU_SOURCE)
  check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

  if (NOT _GNU_SOURCE)
    unset(_GNU_SOURCE CACHE)
    check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
  endif ()
endif ()

if (_GNU_SOURCE)
  add_definitions(-D_GNU_SOURCE)
endif ()

# directories and source

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${INC_DIR})

set(LIB_SRC
    ${INC_DIR}/hoel.h
    ${INC_DIR}/h-private.h
    ${SRC_DIR}/hoel-simple-json.c
    ${SRC_DIR}/hoel-mariadb.c
    ${SRC_DIR}/hoel-pgsql.c
    ${SRC_DIR}/hoel-sqlite.c
    ${SRC_DIR}/hoel.c)

# dependencies

# pthread libraries
find_package (Threads)
list(APPEND HOEL_LIBS ${CMAKE_THREAD_LIBS_INIT})

include(FindJansson)
set(JANSSON_MIN_VERSION 2.4)
find_package(Jansson ${JANSSON_MIN_VERSION} REQUIRED)
list(APPEND HOEL_LIBS Jansson::Jansson)

option(WITH_SQLITE3 "Use SQLite3 library" ON)

if (WITH_SQLITE3)
  include(FindSQLite3)
  find_package(SQLite3 REQUIRED)
  if (SQLite3_FOUND)
    list(APPEND HOEL_LIBS SQLite::SQLite3)
    include_directories(${SQLite3_INCLUDE_DIRS})
    set(_HOEL_SQLITE ON)
  else ()
    message("SQLite3 not found")
    set(_HOEL_SQLITE OFF)
  endif ()
else ()
  set(_HOEL_SQLITE OFF)
endif ()

option(WITH_MARIADB "Use MariaDB library" ON)

if (WITH_MARIADB)
  include(FindMYSQL)
  find_package(MYSQL REQUIRED)
  if (MYSQL_FOUND)
    list(APPEND HOEL_LIBS MysqlDBClient::MysqlDBClient)
    include_directories(${MYSQL_INCLUDE_DIRS})
    set(_HOEL_MARIADB ON)
  else ()
    message("MariaDB not found")
    set(_HOEL_MARIADB OFF)
  endif ()
else ()
  set(_HOEL_MARIADB OFF)
endif ()

option(WITH_PGSQL "Use PostgreSQL library" ON)

if (WITH_PGSQL)
  include(FindLibPQ)
  find_package(LibPQ REQUIRED)
  if(LIBPQ_FOUND)
    list(APPEND HOEL_LIBS PostgreSQL::PostgreSQL)
    include_directories(${LIBPQ_INCLUDE_DIRS})
    set(_HOEL_PGSQL ON)
  else ()
    message("LibPQ not found")
    set(_HOEL_PGSQL OFF)
  endif()
else ()
  set(_HOEL_PGSQL OFF)
endif ()

# build hoel-cfg.h file
configure_file(${INC_DIR}/hoel-cfg.h.in ${PROJECT_BINARY_DIR}/hoel-cfg.h)
set (CMAKE_EXTRA_INCLUDE_FILES ${PROJECT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR})

if (NOT WITH_PGSQL AND NOT WITH_MARIADB AND NOT WITH_SQLITE3)
  MESSAGE(FATAL_ERROR "You must enable at least one database backend: Sqlite3, MariaDB or PostgreSQL")
endif ()

set (PKGCONF_REQ "")
set (PKGCONF_REQ_PRIVATE "liborcania, libyder")
if (WITH_SQLITE3)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, sqlite3")
endif ()
if (WITH_MARIADB)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, mariadb")
endif ()
if (WITH_PGSQL)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libpq")
endif ()

option(BUILD_STATIC "Build static library." OFF)

# static library

if (BUILD_STATIC)
    add_library(hoel_static STATIC ${LIB_SRC})
    add_library(Hoel::Hoel-static ALIAS hoel_static)
    target_include_directories(hoel_static
        PUBLIC "$<BUILD_INTERFACE:${INC_DIR}>"
        PUBLIC "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
    target_link_libraries(hoel_static PUBLIC ${HOEL_LIBS})
    target_compile_definitions(hoel_static PUBLIC O_STATIC_LIBRARY)
    set_target_properties(hoel_static PROPERTIES
            PUBLIC_HEADER "${INC_DIR}/hoel.h;${PROJECT_BINARY_DIR}/hoel-cfg.h"
            OUTPUT_NAME hoel
            EXPORT_NAME Hoel-static)
    if (MSVC)
        set_target_properties(hoel_static PROPERTIES
                OUTPUT_NAME hoel-static)
    endif ()
    if (NOT MSVC)
        target_compile_options(hoel_static PRIVATE -Wextra -Wconversion)
    endif ()
    set(hoel_lib hoel_static)
endif ()

# shared library

add_library(hoel SHARED ${LIB_SRC})
add_library(Hoel::Hoel ALIAS hoel)
target_include_directories(hoel
    PUBLIC "$<BUILD_INTERFACE:${INC_DIR}>"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
    PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(hoel PUBLIC ${HOEL_LIBS})
set_target_properties(hoel PROPERTIES
        PUBLIC_HEADER "${INC_DIR}/hoel.h;${PROJECT_BINARY_DIR}/hoel-cfg.h"
        VERSION "${LIBRARY_VERSION}"
        SOVERSION "${LIBRARY_SOVERSION}"
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
        EXPORT_NAME Hoel)
if (WIN32)
    set_target_properties(hoel PROPERTIES SUFFIX "-${LIBRARY_VERSION_MAJOR}.dll")
endif ()
if (NOT MSVC)
    target_compile_options(hoel PRIVATE -Wextra -Wconversion)
endif()
set(hoel_lib hoel)

# documentation

option(BUILD_HOEL_DOCUMENTATION "Build the documentation." OFF)
if (BUILD_HOEL_DOCUMENTATION)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.cfg)
        set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
        
        configure_file(${doxyfile_in} ${doxyfile} @ONLY)
        
        add_custom_target(doc
                          COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_in}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          COMMENT "Generating documentation with Doxygen"
                          VERBATIM)

    else ()
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif ()
endif ()

# dependencies

find_package(Orcania ${ORCANIA_VERSION_REQUIRED} REQUIRED)
if ("${ORCANIA_VERSION_STRING}" VERSION_GREATER_EQUAL "${ORCANIA_VERSION_REQUIRED}")
    message(STATUS "Orcania found: ${ORCANIA_VERSION_STRING}")
else ()
    message( FATAL_ERROR "Orcania version required: ${ORCANIA_VERSION_REQUIRED} - version installed: ${ORCANIA_VERSION_STRING}")
endif ()

if (BUILD_STATIC)
    if(TARGET Orcania::Orcania-static)
        target_link_libraries(hoel_static PUBLIC $<TARGET_NAME:Orcania::Orcania-static>)
    else()
        target_link_libraries(hoel_static PUBLIC $<TARGET_NAME:Orcania::Orcania>)
    endif()
else()
    target_link_libraries(hoel PUBLIC $<TARGET_NAME:Orcania::Orcania>)
endif ()

find_package(Yder ${YDER_VERSION_REQUIRED} REQUIRED)
if ("${YDER_VERSION_STRING}" VERSION_GREATER_EQUAL "${YDER_VERSION_REQUIRED}")
    message(STATUS "Yder found: ${YDER_VERSION_STRING}")
else ()
    message( FATAL_ERROR "Yder version required: ${YDER_VERSION_REQUIRED} - version installed: ${YDER_VERSION_STRING}")
endif ()

if (BUILD_STATIC)
    if(TARGET Yder::Yder-static)
        target_link_libraries(hoel_static PUBLIC $<TARGET_NAME:Yder::Yder-static>)
    else()
        target_link_libraries(hoel_static PUBLIC $<TARGET_NAME:Yder::Yder>)
    endif()
else()
    target_link_libraries(hoel PUBLIC $<TARGET_NAME:Yder::Yder>)
endif ()

# tests

option(BUILD_HOEL_TESTING "Build the testing tree." OFF) # because we don not use include(CTest)

if (BUILD_HOEL_TESTING)
    find_package(Check REQUIRED)
    if (CHECK_FOUND)
        if (NOT WIN32 AND NOT APPLE)
            include(FindSubunit)
            find_package(Subunit REQUIRED)
        endif ()

        enable_testing()

        set(CMAKE_CTEST_COMMAND ctest -V)

        set(TST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
        set(TEST_LIBS hoel Check::Check ${HOEL_LIBS})
        if (NOT WIN32)
            find_package(Threads REQUIRED)
            list(APPEND TEST_LIBS ${CMAKE_THREAD_LIBS_INIT} m)
        endif ()
        if (NOT APPLE AND NOT WIN32)
            list(APPEND TEST_LIBS rt)
        endif ()
        if (NOT WIN32 AND NOT APPLE)
            list(APPEND TEST_LIBS Subunit::Subunit)
        endif ()
        
        set(TESTS core multi)

        configure_file(
                "${H_CMAKE_MODULE_PATH}/CTestCustom.cmake.in"
                "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
                @ONLY)
    
        foreach (t ${TESTS})
            add_executable(${t} EXCLUDE_FROM_ALL ${TST_DIR}/${t}.c)
            target_include_directories(${t} PUBLIC ${TST_DIR})
            target_link_libraries(${t} PRIVATE ${TEST_LIBS})
            add_test(NAME ${t}
                     WORKING_DIRECTORY ${TST_DIR}
                     COMMAND ${t})
        endforeach ()

    endif ()
endif ()

# install target

option(INSTALL_HEADER "Install the header files" ON) # Install hoel.h or not

if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(PKGCONFIG_TARGET_INCLUDES "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  set(PKGCONFIG_TARGET_INCLUDES "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()

if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(PKGCONFIG_TARGET_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
  set(PKGCONFIG_TARGET_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()

configure_file(libhoel.pc.in libhoel.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libhoel.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(TARGETS hoel)
if (BUILD_STATIC)
    list(APPEND TARGETS hoel_static)
endif ()

if (INSTALL_HEADER)
	install(TARGETS ${TARGETS} EXPORT HoelExports
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY examples/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/ COMPONENT runtime)
    install(FILES README.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
else ()
    install(TARGETS ${TARGETS} EXPORT HoelExports
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif ()

if (INSTALL_HEADER)
    set(HOEL_INSTALL_CMAKEDIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake/Hoel")
    if (WIN32 AND NOT MINGW)
        set(HOEL_INSTALL_CMAKEDIR_DEFAULT "cmake")
    endif ()
    set(HOEL_INSTALL_CMAKEDIR ${HOEL_INSTALL_CMAKEDIR_DEFAULT} CACHE STRING "Location where to install the cmake config files")

    install(EXPORT HoelExports DESTINATION "${HOEL_INSTALL_CMAKEDIR}"
        NAMESPACE "Hoel::"
        FILE "HoelTargets.cmake")

    configure_package_config_file(cmake-modules/HoelConfig.cmake.in HoelConfig.cmake
        INSTALL_DESTINATION "${HOEL_INSTALL_CMAKEDIR}")
    write_basic_package_version_file(HoelConfigVersion.cmake
        COMPATIBILITY AnyNewerVersion)

    install(FILES
                cmake-modules/FindMYSQL.cmake
                cmake-modules/FindLibPQ.cmake
                cmake-modules/FindJansson.cmake
                "${PROJECT_BINARY_DIR}/HoelConfig.cmake"
                "${PROJECT_BINARY_DIR}/HoelConfigVersion.cmake"
            DESTINATION "${HOEL_INSTALL_CMAKEDIR}")
endif ()

# uninstall target

if (NOT TARGET uninstall)
    configure_file(
            "${H_CMAKE_MODULE_PATH}/CMakeUninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()

# packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

if (INSTALL_HEADER)
    set(PACKAGE_FILE_NAME
            "lib${CMAKE_PROJECT_NAME}-dev_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
else ()
    set(PACKAGE_FILE_NAME
            "lib${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
endif ()
set(PACKAGE_IGNORED_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/;/.git/;.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_PACKAGE_NAME "libhoel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Database abstraction library in C")
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

option(BUILD_RPM "Build a RPM for your system" OFF)
if (BUILD_RPM)
  set(CPACK_GENERATOR "TGZ;DEB;RPM")
  set(CPACK_RPM_PACKAGE_LICENSE "LGPL")
  set(CPACK_RPM_PACKAGE_URL "http://babelouest.github.io/hoel/")
endif ()

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "mail@babelouest.org")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/babelouest/hoel")

if (INSTALL_HEADER)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), liborcania (>= ${ORCANIA_VERSION_REQUIRED}), libyder (>= ${YDER_VERSION_REQUIRED})")
  if (WITH_SQLITE3)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsqlite3-dev (>= 3.5.9)")
  endif ()

  if (WITH_MARIADB)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, default-libmysqlclient-dev|libmariadb-dev")
  endif ()

  if (WITH_PGSQL)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libpq-dev (>= 9.0~)")
  endif ()

  if (NOT ${RELEASE_CODENAME} STREQUAL "stretch" AND NOT ${RELEASE_CODENAME} STREQUAL "bionic")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjansson-dev (>= ${JANSSON_VERSION_REQUIRED})")
  endif ()
else ()
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), liborcania (>= ${ORCANIA_VERSION_REQUIRED}), libyder (>= ${YDER_VERSION_REQUIRED})")
  if (WITH_SQLITE3)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsqlite3-0 (>= 3.5.9)")
  endif ()

  if (WITH_MARIADB)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, default-mysql-client|libmariadb3|libmariadb2")
  endif ()

  if (WITH_PGSQL)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libpq5 (>= 9.0~)")
  endif ()

  if (NOT ${RELEASE_CODENAME} STREQUAL "stretch" AND NOT ${RELEASE_CODENAME} STREQUAL "bionic")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjansson4 (>= ${JANSSON_VERSION_REQUIRED})")
  endif ()
endif ()

set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist_h
        COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

message(STATUS "SQLITE3 library support:    ${WITH_SQLITE3}")
message(STATUS "MariaDB library support:    ${WITH_MARIADB}")
message(STATUS "PostgreSQL library support: ${WITH_PGSQL}")
message(STATUS "Build static library:       ${BUILD_STATIC}")
message(STATUS "Build testing tree:         ${BUILD_HOEL_TESTING}")
message(STATUS "Install the header files:   ${INSTALL_HEADER}")
message(STATUS "Build RPM package:          ${BUILD_RPM}")
message(STATUS "Build documentation:        ${BUILD_HOEL_DOCUMENTATION}")
