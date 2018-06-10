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
  ${MIRCLIENT_INCLUDE_DIRS}
#  ${LIBERTINE_INCLUDE_DIRS}
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

target_link_libraries(content-hub-service Qt5::Core Qt5::DBus Qt5::Gui)

target_link_libraries(
  content-hub-service

  content-hub
  ${GIO_LDFLAGS}
  ${GSETTINGS_LDFLAGS}
  ${UBUNTU_LAUNCH_LDFLAGS}
#  ${LIBERTINE_LDFLAGS}
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
# Systemd Unit File
###########################

# where to install
# Uncomment when we drop Vivid
# pkg_get_variable(SYSTEMD_USER_DIR systemd systemduserunitdir)
set (SYSTEMD_USER_DIR "/usr/lib/systemd/user")
message (STATUS "${SYSTEMD_USER_DIR} is the systemd user unit file install dir")

install(
  FILES content-hub.service
  DESTINATION ${SYSTEMD_USER_DIR}/
)

###########################
# GSettings
###########################

add_schema ("com.ubuntu.content.hub.gschema.xml")

install(
  FILES xorg.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/content-hub/icons/
)
