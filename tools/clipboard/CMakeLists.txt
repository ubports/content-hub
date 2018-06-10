# Copyright © 2016 Canonical Ltd.
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
# Authored by: Arthur Mello <arthur.mello@canonical.com>

set(CLIPBOARD_APP content-hub-clipboard)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/com/ubuntu/content
  ${CMAKE_BINARY_DIR}/src/com/ubuntu/content
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
)

set(clipboard_app_HDRS
    paste-data-provider.h
    paste-image-provider.h
    paste-data-model.h
    paste-data-filter-model.h
    clipboardapplication.h)

set(clipboard_app_SRCS
    paste-data-provider.cpp
    paste-image-provider.cpp
    paste-data-model.cpp
    paste-data-filter-model.cpp
    clipboardapplication.cpp
    main.cpp)

qt5_add_resources(RESOURCES qml.qrc)

add_executable(${CLIPBOARD_APP}
    ${clipboard_app_SRCS}
    ${RESOURCES}
)

find_package(Qt5Quick REQUIRED)
find_package(Qt5Widgets REQUIRED)
target_link_libraries(${CLIPBOARD_APP} Qt5::Core Qt5::Gui Qt5::Quick Qt5::Qml Qt5::Widgets Qt5::DBus)
find_package(Qt5Core 5.4 REQUIRED)
include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    )

set_target_properties(${CLIPBOARD_APP}
  PROPERTIES
  AUTOMOC TRUE
)

target_link_libraries(${CLIPBOARD_APP}
  content-hub
  ${UBUNTU_LAUNCH_LDFLAGS}
)

install(
  TARGETS ${CLIPBOARD_APP}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES content-hub-clipboard.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
