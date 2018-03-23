/*
 * Copyright (C) 2015 Caliste Damien.
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

PanelBackground {
    id: toolbar

    property Item flickable
    property bool forceHidden
    property bool autoShowHide: true
    property int offset: _active && !forceHidden ? height : 0

    property bool _canFlick: flickable.contentHeight > flickable.height
    property bool _timerActivated
    property bool _active: !forceHidden && (!_canFlick || _timerActivated)
    property int _previousContentY

    function show() {
        autoHideTimer.stop()
        _timerActivated = true
        if (autoShowHide) autoHideTimer.restart()
    }
    function hide() {
        _timerActivated = false
        autoHideTimer.stop()
    }

    onAutoShowHideChanged: {
        if (autoShowHide) {
            if (_timerActivated) {
                autoHideTimer.start()
            }
        } else {
            autoHideTimer.stop()
            // Keep a transiting (and a not transited yet) toolbar visible.
            _timerActivated = _timerActivated || (offset > 0)
        }
    }

    onForceHiddenChanged: {
        // Ensure that toolbar will not come back after forceHidden is false again.
        if (forceHidden) {
            _timerActivated = false
            autoHideTimer.stop()
        }
    }

    Behavior on offset { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }

    Connections {
        target: flickable
        onContentYChanged: {
            if (!flickable.movingVertically) {
                return
            }

            if (autoShowHide) {
                _timerActivated = flickable.contentY < _previousContentY

                if (_timerActivated) {
                    autoHideTimer.restart()
                }
            }

            _previousContentY = flickable.contentY
        }
    }

    Timer {
        id: autoHideTimer
        interval: 4000
        onTriggered: _timerActivated = false
    }
}
