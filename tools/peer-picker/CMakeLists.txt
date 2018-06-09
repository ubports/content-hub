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
)

qt5_add_resources(RESOURCES qml.qrc)
add_executable(
  content-hub-peer-picker

  main.cpp
  peer-picker.cpp
  ${RESOURCES}
)

find_package(Qt5Quick REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Widgets REQUIRED)
target_link_libraries(content-hub-peer-picker Qt5::Core Qt5::Gui Qt5::Quick Qt5::Qml Qt5::Widgets)

set_target_properties(
  content-hub-peer-picker
  PROPERTIES
  AUTOMOC TRUE
)

install(
  TARGETS content-hub-peer-picker
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES content-hub-peer-picker.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
