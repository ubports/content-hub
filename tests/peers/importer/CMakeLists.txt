# Copyright © 2014 Canonical Ltd.
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

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(
  content-hub-test-importer

  importer.cpp
  autoimporter.cpp
)

target_link_libraries(content-hub-test-importer Qt5::Core Qt5::Gui Qt5::DBus)

set_target_properties(
  content-hub-test-importer
  PROPERTIES
  AUTOMOC TRUE
)

target_link_libraries(
  content-hub-test-importer
 
  content-hub 
)

install(
  TARGETS content-hub-test-importer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES content-hub-test-importer.json
  DESTINATION ${CMAKE_INSTALL_DATADIR}/content-hub/peers
  RENAME content-hub-test-importer
)

install(
  FILES content-hub-test-importer.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
  FILES content-hub-test-importer.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps/
)
