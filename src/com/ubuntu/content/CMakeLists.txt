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

add_subdirectory(service)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${NIH_INCLUDE_DIRS}
  ${NIH_DBUS_INCLUDE_DIRS}
  ${UBUNTU_LAUNCH_INCLUDE_DIRS}
#  ${LIBERTINE_INCLUDE_DIRS}
  ${UBUNTU_DOWNLOAD_MANAGER_INCLUDE_DIRS}
  ${NOTIFY_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
  ${APPARMOR_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
)

qt5_add_dbus_interface(
  CONTENT_SERVICE_STUB ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Service.xml 
  ContentServiceInterface)
qt5_add_dbus_adaptor(
  CONTENT_SERVICE_SKELETON ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Service.xml 
  detail/service.h com::ubuntu::content::detail::Service)

qt5_add_dbus_interface(
  CONTENT_TRANSFER_STUB ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Transfer.xml
  ContentTransferInterface)
qt5_add_dbus_adaptor(
  CONTENT_TRANSFER_SKELETON ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Transfer.xml 
  detail/transfer.h com::ubuntu::content::detail::Transfer)

qt5_add_dbus_interface(
  CONTENT_PASTE_STUB ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Paste.xml
  ContentPasteInterface)
qt5_add_dbus_adaptor(
  CONTENT_PASTE_SKELETON ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Paste.xml 
  detail/paste.h com::ubuntu::content::detail::Paste)

qt5_add_dbus_interface(
  CONTENT_HANDLER_STUB ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Handler.xml 
  ContentHandlerInterface)
qt5_add_dbus_adaptor(
  CONTENT_HANDLER_SKELETON ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Handler.xml 
  detail/handler.h com::ubuntu::content::detail::Handler)

qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/hub.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/import_export_handler.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/item.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/paste.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/peer.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/store.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/transfer.h)
qt5_wrap_cpp(CONTENT_HUB_MOCS ${CMAKE_SOURCE_DIR}/include/com/ubuntu/content/type.h)

add_library(
  content-hub SHARED

  hub.cpp
  import_export_handler.cpp
  item.cpp
  paste.cpp
  peer.cpp
  store.cpp
  transfer.cpp
  type.cpp
  utils.cpp
  debug.cpp

  detail/app_manager.cpp
  detail/paste.cpp
  detail/service.cpp
  detail/transfer.cpp
  detail/handler.cpp
  detail/i18n.cpp
  detail/mir-helper.cpp

  ${CONTENT_HUB_MOCS}
  ${CONTENT_SERVICE_STUB}
  ${CONTENT_SERVICE_SKELETON}
  ${CONTENT_PASTE_STUB}
  ${CONTENT_PASTE_SKELETON}
  ${CONTENT_TRANSFER_STUB}
  ${CONTENT_TRANSFER_SKELETON}
  ${CONTENT_HANDLER_STUB}
  ${CONTENT_HANDLER_SKELETON}
)

set_target_properties(
  content-hub

  PROPERTIES
  VERSION ${CONTENT_HUB_VERSION_MAJOR}.${CONTENT_HUB_VERSION_MINOR}.${CONTENT_HUB_VERSION_PATCH}
  SOVERSION ${CONTENT_HUB_VERSION_MAJOR}
  AUTOMOC TRUE
)

target_link_libraries(content-hub Qt5::Core Qt5::DBus Qt5::Gui)

target_link_libraries(content-hub
    ${UBUNTU_LAUNCH_LDFLAGS}
#    ${LIBERTINE_LDFLAGS}
    ${NIH_LIBRARIES}
    ${NIH_DBUS_LIBRARIES}
    ${UBUNTU_DOWNLOAD_MANAGER_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${MIRCLIENT_LIBRARIES}
    ${APPARMOR_LDFLAGS}
)

find_program(
  GDBUS_CODEGEN
  gdbus-codegen
)

set(
  DBUS_XML
  ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Handler.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Paste.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Service.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/detail/com.ubuntu.content.Transfer.xml
)

set(
  DBUS_ANNOTATIONS
  --annotate com.ubuntu.content.dbus.Paste.Charge\(\)[mimeData] org.gtk.GDBus.C.ForceGVariant true
  --annotate com.ubuntu.content.dbus.Paste.MimeData\(\)[mimeData] org.gtk.GDBus.C.ForceGVariant true
  --annotate com.ubuntu.content.dbus.Service.CreatePaste\(\)[mimeData] org.gtk.GDBus.C.ForceGVariant true
  --annotate com.ubuntu.content.dbus.Service.GetLatestPasteData\(\)[mimeData] org.gtk.GDBus.C.ForceGVariant true
  --annotate com.ubuntu.content.dbus.Service.GetPasteData\(\)[mimeData] org.gtk.GDBus.C.ForceGVariant true
)

add_custom_command(
  OUTPUT glib/content-hub-glib.h glib/content-hub-glib.c
  COMMAND ${CMAKE_COMMAND} -E make_directory glib
  COMMAND ${GDBUS_CODEGEN}
          --interface-prefix com.ubuntu.content.dbus
          --generate-c-code glib/content-hub-glib
          --c-namespace ContentHub
          ${DBUS_ANNOTATIONS}
          ${DBUS_XML}
  DEPENDS ${DBUS_XML}
  VERBATIM
)

add_library(
  content-hub-glib SHARED
  glib/content-hub-glib.h
  glib/content-hub-glib.c
)

target_link_libraries(
  content-hub-glib
  ${GIO_LIBRARIES}
)

set_target_properties(
  content-hub-glib
  PROPERTIES
  VERSION ${CONTENT_HUB_VERSION_MAJOR}.${CONTENT_HUB_VERSION_MINOR}.${CONTENT_HUB_VERSION_PATCH}
  SOVERSION ${CONTENT_HUB_VERSION_MAJOR}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/glib/content-hub-glib.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/com/ubuntu/content/glib
)

install(
  TARGETS content-hub content-hub-glib
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
