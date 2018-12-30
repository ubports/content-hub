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

project(content-hub-plugin)

find_package(Qt5Quick REQUIRED)
set(CMAKE_AUTOMOC TRUE)

set(QT_IMPORTS_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml")
set(CONTENT_HUB_IMPORTS_DIR "${QT_IMPORTS_DIR}/Ubuntu/Content")

set(PLUGIN ubuntu-content-hub-plugin)

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${NIH_INCLUDE_DIRS}
    ${NIH_DBUS_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${UBUNTU_LAUNCH_INCLUDE_DIRS}
)

set(PLUGIN_HDRS
    contenthandler.h
    contenthub.h
    contenthubplugin.h
    contenticonprovider.h
    contentitem.h
    contentpeer.h
    contentpeermodel.h
    contentscope.h
    contentstore.h
    contenttransfer.h
    contenttype.h
    qmlimportexporthandler.h
    )

set(PLUGIN_SRC
    contenthandler.cpp
    contenthub.cpp
    contenthubplugin.cpp
    contenticonprovider.cpp
    contentitem.cpp
    contentpeer.cpp
    contentpeermodel.cpp
    contentscope.cpp
    contentstore.cpp
    contenttransfer.cpp
    contenttype.cpp
    qmlimportexporthandler.cpp
    ../../../src/com/ubuntu/content/debug.cpp
    )

add_library(${PLUGIN} MODULE ${PLUGIN_SRC} ${PLUGIN_HDRS})

target_link_libraries(${PLUGIN} Qt5::Core Qt5::Qml Qt5::Quick Qt5::DBus)
target_link_libraries(
    ${PLUGIN}
    content-hub
    ${NIH_LIBRARIES}
    ${NIH_DBUS_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${UBUNTU_LAUNCH_LDFLAGS}
)

install(TARGETS ${PLUGIN} DESTINATION ${CONTENT_HUB_IMPORTS_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qmldir DESTINATION ${CONTENT_HUB_IMPORTS_DIR})

file(GLOB QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml qmldir)
install(FILES ${QML_FILES} DESTINATION ${CONTENT_HUB_IMPORTS_DIR})

if(NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  # copy qml files and assets over to build dir to be able to import them uninstalled
  foreach(_file ${QML_FILES})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${CMAKE_CURRENT_BINARY_DIR}/${_file})
  endforeach(_file)
  add_custom_target(copy_files_to_build_dir DEPENDS ${QML_FILES})
  add_dependencies(${PLUGIN} copy_files_to_build_dir)
endif()

if(NOT CMAKE_CROSSCOMPILING)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plugins.qmltypes
    COMMAND qmlplugindump-qt5 -noinstantiate -notrelocatable Ubuntu.Content 1.1 ../../ > ${CMAKE_CURRENT_BINARY_DIR}/plugins.qmltypes
    DEPENDS ${PLUGIN}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(Content_generated_files ALL SOURCES ${CMAKE_CURRENT_BINARY_DIR}/plugins.qmltypes)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins.qmltypes DESTINATION ${CONTENT_HUB_IMPORTS_DIR})
endif()
