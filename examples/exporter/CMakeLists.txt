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
# Authored by: Ken VanDine <ken.vandine@canonical.com>

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(
  exporter

  exporter.cpp
  exampleexporter.cpp
)

target_link_libraries(exporter Qt5::Core Qt5::Gui Qt5::DBus)

set_target_properties(
  exporter
  PROPERTIES
  AUTOMOC TRUE
)

target_link_libraries(
  exporter
 
  content-hub 
)
