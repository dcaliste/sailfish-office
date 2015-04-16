/*
 * Copyright (C) 2013-2014 Jolla Ltd.
 * Contact: Robin Burchell <robin.burchell@jolla.com>
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

Page {
    id: page;
    signal pageSelected(int pageNumber);

    property int pageCount
    property alias tocModel: tocListView.model;

    allowedOrientations: Orientation.All;

    SilicaListView {
        id: tocListView
        width: parent.width
        height: parent.height - gotoPage.height
        clip: true

        //: Page with PDF index
        //% "Index"
        header: PageHeader { title: qsTrId( "sailfish-office-he-pdf_index" ); }

        delegate: BackgroundItem {
            id: bg;

            Label {
                anchors {
                    left: parent.left;
                    leftMargin: Theme.paddingLarge * (model.level+1);
                    right: pageNumberLbl.left;
                    rightMargin: Theme.paddingLarge;
                    verticalCenter: parent.verticalCenter;
                }
                elide: Text.ElideRight;
                text: (model.title === undefined) ? "" : model.title;
                color: bg.highlighted ? Theme.highlightColor : Theme.primaryColor;
                truncationMode: TruncationMode.Fade;
            }
            Label {
                id: pageNumberLbl
                anchors {
                    right: parent.right;
                    rightMargin: Theme.paddingLarge;
                    verticalCenter: parent.verticalCenter;
                }
                text: (model.pageNumber === undefined) ? "" : model.pageNumber;
                color: bg.highlighted ? Theme.highlightColor : Theme.primaryColor;
            }

            onClicked: {
                page.pageSelected(model.pageNumber - 1);
                pageStack.navigateBack(PageStackAction.Animated);
            }
        }
    }

    PanelBackground {
        id: gotoPage
        anchors.top: tocListView.bottom
        width: parent.width
        height: Theme.itemSizeMedium

        TextField {
            x: Theme.paddingLarge
            width: parent.width - Theme.paddingMedium - Theme.paddingLarge
            anchors.verticalCenter: parent.verticalCenter

            //% "Go to page"
            placeholderText: qsTrId("sailfish-office-lb-goto-page")
            //% "document has %n pages"
            label: qsTrId("sailfish-office-lb-%n-pages", page.pageCount)

            // We enter page numbers
            inputMethodHints: Qt.ImhDigitsOnly
            EnterKey.enabled: text.length > 0 && Math.round(text) > 0 && Math.round(text) <= page.pageCount
            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            EnterKey.onClicked: {
                page.pageSelected(Math.round(text) - 1);
                pageStack.navigateBack(PageStackAction.Animated);
            }
        }
    }
}
