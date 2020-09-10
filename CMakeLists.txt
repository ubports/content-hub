# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Thomas Voss <thomas.voss@canonical.com>

cmake_minimum_required(VERSION 2.8)

project(content-hub)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(ENABLE_TESTS "Build tests" ON)
option(ENABLE_DOC "Build documentation" ON)

include (GNUInstallDirs)
include (GSettings)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing -fPIC -pthread -Wno-unused-function")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

# Workaround for libexecdir on debian
if (EXISTS "/etc/debian_version")
  set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}")
endif()

set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core)
find_package(Qt5DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(UBUNTU_LAUNCH REQUIRED ubuntu-app-launch-2)
pkg_check_modules(GSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(NIH REQUIRED libnih)
pkg_check_modules(NIH_DBUS REQUIRED libnih-dbus)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(UBUNTU_DOWNLOAD_MANAGER REQUIRED ubuntu-download-manager-client)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(APPARMOR REQUIRED libapparmor)

pkg_check_modules(LIBERTINE libertine)
if(LIBERTINE_FOUND)
   message("Bulding with libertine support")
else()
   message(WARNING "Bulding without libertine support")
endif()

add_definitions(-DDEBUG_ENABLED)

set(GETTEXT_PACKAGE "content-hub")
add_definitions(-DI18N_DOMAIN="${GETTEXT_PACKAGE}")

set(CONTENT_HUB_VERSION_MAJOR 0)
set(CONTENT_HUB_VERSION_MINOR 2)
set(CONTENT_HUB_VERSION_PATCH 0)
set(CONTENT_HUB_VERSION "${CONTENT_HUB_VERSION_MAJOR}.${CONTENT_HUB_VERSION_MINOR}.${CONTENT_HUB_VERSION_PATCH}")

set(QML_API_VERSION_MAJOR 1)

include_directories(include)

file(GLOB_RECURSE I18N_QML_SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     import/Ubuntu/Content/**.qml)
list(SORT I18N_QML_SRC_FILES)

file(GLOB_RECURSE I18N_CPP_SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     src/com/ubuntu/content/detail/**.cpp)
list(SORT I18N_CPP_SRC_FILES)

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set (GETTEXT_PACKAGE "content-hub")

if (${ENABLE_DOC})
  add_subdirectory(doc)
endif()
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(import)
add_subdirectory(examples)
add_subdirectory(tools)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

# Build and install a pkg-config file
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${prefix}/bin)
set(libdir ${prefix}/${CMAKE_INSTALL_LIBDIR})
set(pkg-name "libcontent-hub")
set(glib-pkg-name "libcontent-hub-glib")
configure_file(libcontent-hub.pc.in libcontent-hub.pc @ONLY)
configure_file(libcontent-hub-glib.pc.in libcontent-hub-glib.pc @ONLY)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libcontent-hub.pc
  ${CMAKE_CURRENT_BINARY_DIR}/libcontent-hub-glib.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

if (${ENABLE_TESTS})
  include (EnableCoverageReport)

  #####################################################################
  # Enable code coverage calculation with gcov/gcovr/lcov
  # Usage:
  #  * Switch build type to coverage (use ccmake or cmake-gui)
  #  * Invoke make, make test, make coverage
  #  * Find html report in subdir coveragereport
  #  * Find xml report feasible for jenkins in coverage.xml
  #####################################################################
  IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

  find_package(Gtest REQUIRED)
  include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})
  enable_testing()

  add_subdirectory(tests)

# There's no nice way to format this. Thanks CMake.
  add_test(LGPL-required
    /bin/sh -c "! grep -rl 'GNU General' ${PROJECT_SOURCE_DIR}/src/client ${PROJECT_SOURCE_DIR}/include/client ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/include/shared"
  )
  add_test(GPL-required
    /bin/sh -c "! grep -rl 'GNU Lesser' ${PROJECT_SOURCE_DIR}/src/server ${PROJECT_SOURCE_DIR}/include/server ${PROJECT_SOURCE_DIR}/include/test ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/examples"
  )
endif()
