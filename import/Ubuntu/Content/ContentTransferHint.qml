/*
 * Copyright 2013 Canonical Ltd.
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
import Ubuntu.Content 1.3

/*!
    \qmltype ContentTransferHint
    \inqmlmodule Ubuntu.Content
    \brief Component that indicates that a transfer is active

    This component shows that the transfer is currently running, and the source
    application is active. It blocks all input during that time.
    Place this component on top of your view.

    See documentation for ContentHub to see an example
*/
Item {
    id: root

    /*! 
        \qmlproperty ContentTransfer ContentTransferHint::activeTransfer
        \brief The ContentTransfer to monitor the status of.

        This should be set to the currently active ContentTransfer, which
        will then cause the ContentTransferHint to become visible while
        the transfer is in progress.
    */    
    property var activeTransfer

    opacity: internal.isTransferRunning ? 1.0 : 0.0

    Component {
        id: dialog
        Dialog {
            id: dialogue
            title: i18n.dtr("content-hub", "Transfer in progress")

            ActivityIndicator {
                id: indicator
                anchors.top: parent.top
                anchors.topMargin: units.gu(6)
                running: internal.isTransferRunning
            }

            Button {
                id: cancelTransfer
                anchors.top: indicator.bottom
                anchors.topMargin: units.gu(4)
                text: i18n.dtr("content-hub", "Cancel")
                onClicked: {
                    root.activeTransfer.state = ContentTransfer.Aborted
                }
            }
        }
    }

    QtObject {
        id: internal
        property bool isTransferRunning: root.activeTransfer ?
                                             root.activeTransfer.state === ContentTransfer.InProgress || root.activeTransfer.state === ContentTransfer.Initiated
                                           : false
        property var dialogue

        onIsTransferRunningChanged: {
            if (isTransferRunning) {
                dialogue = PopupUtils.open(dialog, root);
            } else {
                PopupUtils.close(dialogue);
            }
        }
    }
}
