import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kirimochi 1.0

GridView {
    id: cardGridView

    // ---- Values depend on page ----
    property int cardPadding: 10
    property int titleAreaHeight: 48

    // ---- Constant ----
    property int minSideMargin: Theme.spacingXl
    property int cellSpacing: Theme.spacingLg
    property int minCardWidth: 220
    property int maxCardWidth: 260

    // Breakpoints: the column count only changes at these
    // width thresholds, so resizing the window doesn't cause
    // cards to reflow continuously. Within a band, cardWidth
    // stretches between minCardWidth/maxCardWidth to absorb
    // the extra space, and the whole grid is centered by
    // Layout.maximumWidth + AlignHCenter.
    function columnsForWidth(w) {
        if (w >= 1800)
            return 6;
        if (w >= 1500)
            return 5;
        return 4;
    }

    readonly property int availableWidth: parent.width - minSideMargin * 2
    readonly property int columns: columnsForWidth(availableWidth)
    readonly property int cardWidth: Math.max(minCardWidth, Math.min(maxCardWidth, Math.floor((availableWidth - cellSpacing * (columns - 1)) / columns)))
    readonly property real cardHeight: cardPadding + (cardWidth - cardPadding * 2) * 9 / 16 + 8 + titleAreaHeight + cardPadding
    readonly property int gridContentWidth: cardWidth * columns + cellSpacing * (columns - 1)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: gridContentWidth
    Layout.alignment: Qt.AlignHCenter
    Layout.margins: Theme.spacingXl

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    keyNavigationEnabled: true

    cellWidth: cardWidth + cellSpacing
    cellHeight: cardHeight + cellSpacing

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
    }
}
