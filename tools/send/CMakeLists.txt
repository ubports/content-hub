# Copyright © 2015 Canonical Ltd.
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
# Authored by: Ken VanDine <ken.vandine@canonical.com>

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
)

add_executable(
  content-hub-send

  exporter.cpp
  autoexporter.cpp
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content/debug.cpp
)

target_link_libraries(content-hub-send Qt5::Core Qt5::Gui Qt5::DBus)

set_target_properties(
  content-hub-send
  PROPERTIES
  AUTOMOC TRUE
)

target_link_libraries(
  content-hub-send
 
  content-hub 
  ${UBUNTU_LAUNCH_LDFLAGS}
)

install(
  TARGETS content-hub-send
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES content-hub-send.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
    FILES content-hub-send.url-dispatcher
    DESTINATION share/url-dispatcher/urls
)
