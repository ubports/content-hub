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

project(test CXX C)
qt5_wrap_cpp(MOCS test_harness.h)

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content
  ${GSETTINGS_INCLUDE_DIRS}
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
)

set(TESTS
  app_hub_communication_default_source
  app_hub_communication_known_sources
  app_hub_communication_stores
  app_hub_communication_transfer
  app_hub_communication_paste
  app_hub_communication_handler
  test_utils
  test_types
  mimedata_test
  glib_test
)

set(TEST_LIBS
  ${GMOCK_LIBRARY}
  ${GTEST_BOTH_LIBRARIES}
  content-hub
)

foreach(test ${TESTS})
  # set the sources per test
  set(${test}_SOURCES
    ${test}.cpp
    ${MOCS}
    transfer_files
  )
endforeach(test)

foreach(test ${TESTS})
  # set targets, linked libs and test
  add_executable(${test}
    ${${test}_SOURCES}
  )
  find_package(Qt5Test REQUIRED)
  target_link_libraries(${test} Qt5::Core Qt5::Gui Qt5::DBus Qt5::Test)

  target_link_libraries(${test}
    ${TEST_LIBS}
  )

  add_test(NAME ${test} COMMAND dbus-test-runner --task ${CMAKE_CURRENT_BINARY_DIR}/${test})

  SET_TESTS_PROPERTIES(${test}
    PROPERTIES ENVIRONMENT "CONTENT_HUB_TESTING=1")
endforeach(test)

add_executable(
  test_registry_updater
  test_registry_updater.cpp
  ${MOCS}
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content/service/registry-updater.cpp
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content/service/registry.cpp
  good.json
  bad.json
  source_all.json
)

target_link_libraries(test_registry_updater Qt5::Core Qt5::Gui Qt5::DBus Qt5::Test)
target_link_libraries(test_registry_updater content-hub ${GTEST_ALL_LIBRARIES} ${GSETTINGS_LDFLAGS})
add_test(NAME test_registry_updater COMMAND dbus-test-runner --task ${CMAKE_CURRENT_BINARY_DIR}/test_registry_updater)

SET_TESTS_PROPERTIES(test_registry_updater
  PROPERTIES ENVIRONMENT "CONTENT_HUB_TESTING=1")

set_target_properties(
  test_registry_updater
  PROPERTIES
  AUTOMOC TRUE
)

file(COPY good.json bad.json source_all.json DESTINATION .)

target_link_libraries(glib_test content-hub-glib)

add_custom_command(
  OUTPUT transfer_files
  COMMAND cmake -E touch file1 file2 file3
)
