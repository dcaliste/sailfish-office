/*
 * Copyright (C) 2016 Caliste Damien.
 * Contact: Damien Caliste <dcaliste@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Repeater {
    id: root

    property alias startAttachX: start.attachX
    property alias stopAttachX: stop.attachX
    property bool dragging: start.press || stop.press

    visible: (model !== undefined && model.count > 0)

    /* Copy text to clipboard on first selection and when stop dragging. */
    onVisibleChanged: if (visible) Clipboard.text = model.text
    onDraggingChanged: if (!dragging) Clipboard.text = model.text

    delegate: Rectangle {
        opacity: 0.5
        color: Theme.highlightColor
        x: rect.x
        y: rect.y
        width: rect.width
        height: rect.height
    }

    children: [
        PDFSelectionHandle {
            id: start
            handle: root.model.start
            onDragged: root.model.start = at
        }
        , PDFSelectionHandle {
            id: stop
            handle: root.model.stop
            onDragged: root.model.stop = at
        }
    ]
}
