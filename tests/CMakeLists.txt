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

find_package(Gtest REQUIRED)
enable_testing()

add_definitions(-DQT_NO_KEYWORDS)

include_directories (
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content
  ${GIO_INCLUDE_DIRS}
#  ${LIBERTINE_INCLUDE_DIRS}
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}/src/com/ubuntu/content
  ${GMOCK_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIR}
  ${MIRCLIENT_INCLUDE_DIR}
)

add_subdirectory(acceptance-tests)
add_subdirectory(qml-tests)
add_subdirectory(peers)
