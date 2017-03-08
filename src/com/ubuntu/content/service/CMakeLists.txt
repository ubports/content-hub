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

add_definitions(-DQT_NO_KEYWORDS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt5_add_dbus_adaptor(
  CONTENT_SERVICE_SKELETON ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content/detail/com.ubuntu.content.Service.xml
  detail/service.h com::ubuntu::content::detail::Service)

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content
  ${GIO_INCLUDE_DIRS}
  ${GSETTINGS_INCLUDE_DIRS}
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
  ${LIBERTINE_INCLUDE_DIRS}
)

add_executable(
  content-hub-service

  main.cpp
  registry.cpp
  registry-updater.cpp
  ../debug.cpp
  ../utils.cpp
  ${CONTENT_SERVICE_SKELETON}
)

set_target_properties(
  content-hub-service
  PROPERTIES
  AUTOMOC TRUE
)

qt5_use_modules(content-hub-service Core DBus Gui)

target_link_libraries(
  content-hub-service
 
  content-hub
  ${GIO_LDFLAGS}
  ${GSETTINGS_LDFLAGS}
  ${UBUNTU_LAUNCH_LDFLAGS}
  ${LIBERTINE_LDFLAGS}
)

install(
  TARGETS content-hub-service
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES com.ubuntu.content.dbus.Service.service 
  DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

###########################
# GSettings
###########################

add_schema ("com.ubuntu.content.hub.gschema.xml")

install(
  FILES xorg.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/content-hub/icons/
)
