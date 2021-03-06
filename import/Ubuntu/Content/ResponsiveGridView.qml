    /*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3

/*
   Essentially a GridView where you can specify the maximum number of columns it can have.
 */
Item {
    property int minimumHorizontalSpacing: units.gu(0.5)
    // property int minimumNumberOfColumns: 2 // FIXME: not implemented
    property int maximumNumberOfColumns: 6
    readonly property int columns: gridView.columns
    property alias verticalSpacing: gridView.verticalSpacing
    readonly property alias margins: gridView.margin
    property int delegateWidth
    property int delegateHeight
    property alias model: gridView.model
    property alias delegate: gridView.delegate
    readonly property int cellWidth: gridView.cellWidth
    readonly property int cellHeight: gridView.cellHeight
    property alias interactive: gridView.interactive
    readonly property alias flicking: gridView.flicking
    readonly property alias moving: gridView.moving
    readonly property alias pressDelay: gridView.pressDelay
    property alias highlightIndex: gridView.highlightIndex
    readonly property alias currentItem: gridView.currentItem

    function contentHeightForRows(rows) {
        return rows * cellHeight + verticalSpacing
    }

    GridView {
        id: gridView
        objectName: "responsiveGridViewGrid"

        anchors {
            fill: parent
            leftMargin: margin/2 + horizontalSpacing/2
            rightMargin: margin/2 - horizontalSpacing/2
            topMargin: verticalSpacing
        }

        onModelChanged: {
            clip = model && parent.height != contentHeightForRows(Math.ceil(model.count / columns))
        }

        function pixelToGU(value) {
            return Math.floor(value / units.gu(1));
        }

        function spacingForColumns(columns) {
            // spacing between columns as an integer number of GU, the remainder goes in the margins
            var spacingGU = pixelToGU(allocatableHorizontalSpace / (columns+1));
            return units.gu(spacingGU);
        }

        function columnsForSpacing(spacing) {
            // minimum margin is half of the spacing
            return Math.max(1, Math.floor(parent.width / (delegateWidth + spacing)));
        }

        property real allocatableHorizontalSpace: parent.width - columns * delegateWidth
        property int columns: Math.min(columnsForSpacing(minimumHorizontalSpacing), maximumNumberOfColumns)
        property real horizontalSpacing: spacingForColumns(columns)
        property real verticalSpacing: horizontalSpacing
        property int margin: allocatableHorizontalSpace - columns * horizontalSpacing
        property int highlightIndex: -1

        cellWidth: delegateWidth + horizontalSpacing
        cellHeight: delegateHeight + verticalSpacing

        onHighlightIndexChanged: {
            if (highlightIndex != -1) {
                currentIndex = highlightIndex
            }
        }
    }
}
