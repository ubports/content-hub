/*
 * Copyright 2013-2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import Ubuntu.Components.ListItems 1.3 as ListItem
import Ubuntu.Content 1.3

/*!
  \internal
  Documentation is in ContentPeerPicker.qdoc
*/
Item {
    id: root
    anchors.fill: parent

    property var handler
    property var contentType
    property alias showTitle: header.visible
    property var peer
    property var customPeerModelLoader
    property string headerText
    property var completed: false

    signal peerSelected
    signal cancelPressed

    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.palette.normal.background
    }

    ContentPageHeader {
        id: header
        title: headerText ? headerText : (handler === ContentHandler.Source) ? i18n.dtr("content-hub", "Choose from") : (handler === ContentHandler.Destination ? i18n.dtr("content-hub", "Open with") : i18n.dtr("content-hub", "Share to"))
        onCancel: {
            if(root.activeTransfer) {
                root.activeTransfer.state = ContentTransfer.Aborted;
            }
            cancelPressed()
        }
    }

    Loader {
        id: peerModelLoader
        active: false
        asynchronous: true
        sourceComponent: ContentPeerModel { }
        onLoaded: {
            item.handler = root.handler;
            item.contentType = root.contentType;
        }
    }

    Component.onCompleted: {
        if (root.visible) {
            if (customPeerModelLoader) {
                customPeerModelLoader.active = true;
            } else {
                peerModelLoader.active = true;
            }
        }
        completed = true;
    }

    onVisibleChanged: {
        if (completed) {
            if (customPeerModelLoader) {
                customPeerModelLoader.active = true;
            } else {
                peerModelLoader.active = true;
            }
        }
    }

    onHandlerChanged: {
        if (!customPeerModelLoader && peerModelLoader.item) {
            appPeers.model = undefined; // Clear grid view
            peerModelLoader.item.handler = root.handler;
            appPeers.model = peerModelLoader.item.peers;
        }
    }

    onContentTypeChanged: {
        if (!customPeerModelLoader && peerModelLoader.item) {
            appPeers.model = undefined; // Clear grid view
            peerModelLoader.item.contentType = root.contentType;
            appPeers.model = peerModelLoader.item.peers;
        }
    }

    Component {
        id: peerDelegate
        Item {
            width: units.gu(11)
            height: units.gu(16)
            AbstractButton {
                width: parent.width
                height: icon.height + label.height

                UbuntuShape {
                    id: icon
                    anchors {
                        top: parent.top
                        horizontalCenter: parent.horizontalCenter
                    }
                    radius: "medium"
                    width: units.gu(8)
                    height: units.gu(7.5)
                    sourceHorizontalAlignment: Image.AlignHCenter
                    sourceVerticalAlignment: Image.AlignVCenter
                    source: Image {
                        id: image
                        objectName: "image"
                        sourceSize { width: icon.width; height: icon.height }
                        asynchronous: true
                        cache: false
                        source: "image://content-hub/" + modelData.appId
                    }
                }

                Label {
                    id: label
                    objectName: "label"
                    anchors {
                        baseline: icon.bottom
                        baselineOffset: units.gu(2)
                        left: parent.left
                        right: parent.right
                        leftMargin: units.gu(1)
                        rightMargin: units.gu(1)
                    }

                    opacity: 0.9
                    fontSize: "small"
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.name || modelData.appId
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                }

                onClicked: {
                        peer = modelData
                        peerSelected()
                }
            }

        }
    }

    ListItem.Header {
        id: appTitle
        anchors.top: header.visible ? header.bottom : parent.top
        text: i18n.dtr("content-hub", "Apps")
    }

    Rectangle {
        id: apps
        color: ColorUtils.luminance(background.color) >= 0.85 ? "#FFFFFF" : Qt.darker(background.color)
        clip: true
        anchors {
            left: parent.left
            right: parent.right
            top: appTitle.bottom
            bottom: devTitle.visible ? devTitle.top : parent.bottom
            bottomMargin: units.gu(1)
        }

        Flickable {
            anchors.fill: parent

            ResponsiveGridView {
                id: appPeers
                anchors.fill: parent
                minimumHorizontalSpacing: units.gu(0.5)
                maximumNumberOfColumns: 6
                delegateWidth: units.gu(11)
                delegateHeight: units.gu(11)
                verticalSpacing: units.gu(2)
                model: customPeerModelLoader ? customPeerModelLoader.item.peers : peerModelLoader.item.peers //This line give error "TypeError: Cannot read property 'peers' of null" > Move to onCompleted?
                delegate: peerDelegate

                Label {
                    anchors.top: parent.top
                    anchors.topMargin: units.gu(2.5)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - units.gu(5)
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: root.handler === ContentHandler.Source ? i18n.dtr("content-hub", "Sorry, there aren't currently any apps installed that can provide this type of content.") : i18n.dtr("content-hub", "Sorry, there aren't currently any apps installed that can handle this type of content.")
                    visible: appPeers.model ? appPeers.model.length == 0 : false
                }
            }
        }
    }

    ListItem.Header {
        id: devTitle
        // TODO: make this visible when we have a way to populate devices
        visible: false
        anchors {
            left: parent.left
            right: parent.right
            top: apps.bottom
        }
        text: i18n.dtr("content-hub", "Devices")
    }

    Rectangle {
        id: devices
        // TODO: make this visible when we have a way to populate devices
        visible: false
        color: ColorUtils.luminance(background.color) >= 0.85 ? "#FFFFFF" : Qt.darker(background.color)
        width: parent.width
        radius: 0
        anchors {
            left: parent.left
            right: parent.right
            top: devTitle.bottom
            bottom: parent.bottom
            bottomMargin: units.gu(1)
        }

        Flickable {
            anchors.fill: parent

            ResponsiveGridView {
                id: devPeers
                anchors.fill: parent
                minimumHorizontalSpacing: units.gu(0.5)
                maximumNumberOfColumns: 6
                delegateWidth: units.gu(11)
                delegateHeight: units.gu(9.5)
                verticalSpacing: units.gu(2)
                delegate: peerDelegate
            }
        }
    }

}
