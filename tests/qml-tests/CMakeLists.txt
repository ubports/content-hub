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

set(TEST tst_QmlTests)
set(XVFB_CMD xvfb-run -a -s "-screen 0 640x480x24")
add_executable(${TEST} tst_QmlTests.cpp)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickTest REQUIRED)
target_link_libraries(${TEST} Qt5::Core Qt5::Qml Qt5::Quick Qt5::Test Qt5::QuickTest)
add_test(NAME ${TEST} COMMAND ${XVFB_CMD} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
set_tests_properties(${TEST} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=minimal;QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/import;APP_ID=com.some.app.for.testing")

# copy qml files under test to build dir
set(out_qml_files)
if(NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  # copy qml test files to build dir
  file(GLOB qmlTestFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)
  foreach(qmlTestFile ${qmlTestFiles})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${qmlTestFile}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${qmlTestFile}
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${qmlTestFile} ${CMAKE_CURRENT_BINARY_DIR}/${qmlTestFile})
  endforeach(qmlTestFile)
  add_custom_target(copy_qml_test_files_to_build_dir DEPENDS ${qmlTestFiles})
  add_dependencies(${TEST} copy_qml_test_files_to_build_dir)
endif()
