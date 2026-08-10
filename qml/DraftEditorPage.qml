pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kirimochi 1.0

Page {
    id: pageRoot

    property DraftEditorViewModel draftEditorViewModel
    property AssetLibraryViewModel assetLibraryViewModel
    property IdeaLibraryViewModel ideaLibraryViewModel

    property bool controlsExpanded: false

    property int previewStartMs: -1
    property int previewEndMs: -1

    function parseTimeString(timeString) {
        var parts = timeString.split(":");
        var hours = parseInt(parts[0], 10);
        var minutes = parseInt(parts[1], 10);
        var seconds = parseInt(parts[2], 10);

        return (hours * 3600 + minutes * 60 + seconds) * 1000;
    }

    header: RowLayout {
        width: parent.width
        spacing: 0

        ToolButton {
            text: qsTr("< Home")
            onClicked: AppNavigator.goToHomePage()
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Output")
            onClicked: pageRoot.draftEditorViewModel.exportDraft()
        }
    }

    Dialog {
        id: rangeDialog

        property var targetId: -1

        modal: true
        title: qsTr("Edit range")
        anchors.centerIn: Overlay.overlay
        width: 480
        height: 560
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: pageRoot.draftEditorViewModel.confirmRange(rangeDialog.targetId)
        onClosed: {
            pageRoot.draftEditorViewModel.srt_cue_list.reset();
            rangeDialog.targetId = -1;
        }

        ListView {
            id: cueListView

            anchors.fill: parent
            clip: true
            spacing: Theme.spacingXs

            model: pageRoot.draftEditorViewModel.srt_cue_list

            delegate: Rectangle {
                id: cueDelegate

                required property int index
                required property string start
                required property string end
                required property string text
                required property bool selected

                width: cueListView.width
                height: cueCol.implicitHeight + Theme.spacingSm * 2

                radius: Theme.radiusSm
                color: selected ? Theme.memoSurface : "transparent"
                border.width: 1
                border.color: selected ? Theme.borderAccent : Theme.borderDefault

                TapHandler {
                    onTapped: pageRoot.draftEditorViewModel.srt_cue_list.toggleSelect(cueDelegate.index)
                }

                ColumnLayout {
                    id: cueCol
                    anchors.fill: parent
                    anchors.margins: Theme.spacingSm
                    spacing: Theme.spacingXs

                    RowLayout {
                        spacing: Theme.spacingSm

                        Label {
                            text: cueDelegate.start
                            color: Theme.textMuted
                            font.pointSize: Theme.fontSizeSm
                        }
                        Label {
                            text: "→"
                            color: Theme.textMuted
                            font.pointSize: Theme.fontSizeSm
                        }
                        Label {
                            text: cueDelegate.end
                            color: Theme.textMuted
                            font.pointSize: Theme.fontSizeSm
                        }
                    }

                    Label {
                        text: cueDelegate.text
                        color: Theme.textSecondary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: cueListView.count === 0
                text: qsTr("No subtitles in range")
                color: Theme.textMuted
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: 640
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent

                TabBar {
                    id: tabBar
                    Layout.fillWidth: true

                    TabButton {
                        text: "Memo"
                    }
                    TabButton {
                        text: "Asset"
                    }
                    TabButton {
                        text: "Idea"
                    }
                }

                StackLayout {
                    id: stackLayout
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: tabBar.currentIndex

                    Page {
                        StackView {
                            id: pageStack
                            anchors.fill: parent
                            initialItem: gridComponent
                        }

                        Component {
                            id: gridComponent

                            Item {
                                CardGridView {
                                    // TODO: Duplicating GridView in HomePage.qml
                                    id: projectGrid

                                    anchors.fill: parent

                                    model: pageRoot.draftEditorViewModel.project_list

                                    delegate: Rectangle {
                                        id: projectDelegate

                                        required property string projectId
                                        required property string kind       // "Memo" or "Draft"
                                        required property string title
                                        required property string thumbnailPath
                                        required property string status     // "wip" or "done"

                                        readonly property int cardPadding: projectGrid.cardPadding
                                        readonly property int thumbnailWidth: width - cardPadding * 2
                                        readonly property real thumbnailHeight: thumbnailWidth * 9 / 16

                                        readonly property color normalColor: kind === "Draft" ? Theme.draftSurface : Theme.memoSurface
                                        readonly property color hoverColor: kind === "Draft" ? Theme.draftSurfaceHover : Theme.memoSurfaceHover
                                        readonly property color pressedColor: kind === "Draft" ? Theme.draftSurfacePressed : Theme.memoSurfacePressed

                                        width: projectGrid.cardWidth
                                        height: projectGrid.cardHeight

                                        radius: Theme.radiusMd
                                        activeFocusOnTab: true

                                        color: tapHandler.pressed ? pressedColor : hoverHandler.hovered ? hoverColor : normalColor

                                        border.width: activeFocus ? 2 : 1
                                        border.color: activeFocus ? Theme.borderAccent : hoverHandler.hovered ? Theme.borderStrong : Theme.borderDefault

                                        scale: tapHandler.pressed ? 0.985 : 1.0
                                        transformOrigin: Item.Center

                                        Accessible.role: Accessible.Button
                                        Accessible.name: title
                                        Accessible.description: kind + ", " + status

                                        HoverHandler {
                                            id: hoverHandler
                                            cursorShape: Qt.PointingHandCursor
                                        }

                                        TapHandler {
                                            id: tapHandler
                                            acceptedButtons: Qt.LeftButton

                                            onTapped: {
                                                projectDelegate.forceActiveFocus();
                                                pageRoot.draftEditorViewModel.addProject(projectDelegate.projectId);
                                                pageStack.push(listComponent);
                                            }
                                        }

                                        Behavior on color {
                                            ColorAnimation {
                                                duration: 100
                                            }
                                        }

                                        Behavior on border.color {
                                            ColorAnimation {
                                                duration: 100
                                            }
                                        }

                                        Behavior on scale {
                                            NumberAnimation {
                                                duration: 70
                                                easing.type: Easing.OutQuad
                                            }
                                        }

                                        Rectangle {
                                            id: thumbnailContainer

                                            anchors.top: parent.top
                                            anchors.left: parent.left
                                            anchors.right: parent.right

                                            anchors.topMargin: projectDelegate.cardPadding
                                            anchors.leftMargin: projectDelegate.cardPadding
                                            anchors.rightMargin: projectDelegate.cardPadding

                                            height: projectDelegate.thumbnailHeight

                                            radius: Theme.radiusSm
                                            color: Theme.bgPage
                                            clip: true

                                            Image {
                                                id: thumbnailImage

                                                anchors.fill: parent

                                                source: projectDelegate.thumbnailPath
                                                fillMode: Image.PreserveAspectCrop
                                                asynchronous: true
                                                smooth: true

                                                sourceSize.width: thumbnailContainer.width * 2
                                                sourceSize.height: thumbnailContainer.height * 2
                                            }

                                            Rectangle {
                                                id: statusBadge

                                                anchors.right: parent.right
                                                anchors.bottom: parent.bottom
                                                anchors.rightMargin: 8
                                                anchors.bottomMargin: 8

                                                width: statusText.implicitWidth + 16
                                                height: 24
                                                radius: height / 2

                                                color: projectDelegate.status === "done" ? Theme.statusDoneBg : Theme.statusWipBg

                                                Label {
                                                    id: statusText

                                                    anchors.centerIn: parent

                                                    text: projectDelegate.status === "done" ? qsTr("DONE") : qsTr("WIP")

                                                    color: Theme.textPrimary
                                                    font.pixelSize: 11
                                                    font.weight: Font.DemiBold
                                                }
                                            }
                                        }

                                        Label {
                                            id: titleLabel

                                            anchors.top: thumbnailContainer.bottom
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom

                                            anchors.topMargin: 8
                                            anchors.leftMargin: projectDelegate.cardPadding
                                            anchors.rightMargin: projectDelegate.cardPadding
                                            anchors.bottomMargin: projectDelegate.cardPadding

                                            text: projectDelegate.title

                                            color: Theme.textPrimary
                                            font.pixelSize: 14
                                            font.weight: Font.Medium

                                            wrapMode: Text.Wrap
                                            maximumLineCount: 2
                                            elide: Text.ElideRight

                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                }
                            }
                        }

                        Component {
                            id: listComponent

                            Item {
                                ListView {
                                    id: memoList
                                    anchors.fill: parent
                                    spacing: Theme.spacingSm
                                    clip: true
                                    model: pageRoot.draftEditorViewModel.memo_block_list

                                    header: ToolButton {
                                        text: "< Back"
                                        onClicked: pageStack.pop()
                                    }

                                    delegate: Rectangle {
                                        id: memoDelegate

                                        required property var memoId
                                        required property string start
                                        required property string end
                                        required property string label
                                        required property string body

                                        width: parent.width
                                        height: contentCol.implicitHeight + Theme.spacingMd * 2
                                        color: Theme.memoSurface
                                        radius: Theme.radiusMd
                                        border.width: 1
                                        border.color: Theme.borderDefault

                                        ColumnLayout {
                                            id: contentCol
                                            anchors.fill: parent
                                            anchors.margins: Theme.spacingMd
                                            spacing: Theme.spacingSm

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: Theme.spacingSm

                                                Label {
                                                    text: memoDelegate.start
                                                    color: Theme.textMuted
                                                }

                                                Label {
                                                    text: "→"
                                                    color: Theme.textMuted
                                                }

                                                Label {
                                                    text: memoDelegate.end
                                                    color: Theme.textMuted
                                                }

                                                Item {
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: Theme.spacingSm

                                                Label {
                                                    text: memoDelegate.label
                                                    visible: text.length > 0
                                                    color: Theme.textSecondary
                                                    font.pointSize: Theme.fontSizeSm
                                                }

                                                Label {
                                                    text: memoDelegate.body
                                                    visible: text.length > 0
                                                    color: Theme.textSecondary
                                                    font.pointSize: Theme.fontSizeSm
                                                }
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent

                                            onDoubleClicked: pageRoot.draftEditorViewModel.addMemo(memoDelegate.memoId, pageRoot.parseTimeString(memoDelegate.start), pageRoot.parseTimeString(memoDelegate.end), memoDelegate.label, memoDelegate.body)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AssetLibraryPage {
                        id: assetLibraryPage
                        viewModel: pageRoot.assetLibraryViewModel

                        onAssetDoubleClicked: (assetId, kind) => {
                            if (pageRoot.draftEditorViewModel.pendingSeTargetId !== -1) {
                                pageRoot.draftEditorViewModel.attachSeToBlock(assetId);
                                assetViewHighlight.visible = false;
                            } else if (pageRoot.draftEditorViewModel.pendingSeTargetId === -1 && kind === "video") {
                                pageRoot.draftEditorViewModel.addVideo(assetId);
                            } else if (pageRoot.draftEditorViewModel.pendingSeTargetId === -1 && kind === "image") {
                                pageRoot.draftEditorViewModel.addImage(assetId);
                            } else {
                                // TODO: se preview
                            }
                        }

                        Rectangle {
                            id: assetViewHighlight
                            anchors.fill: parent
                            color: "transparent"
                            border.color: Theme.borderStrong
                            border.width: 3
                            visible: false
                            z: 999
                        }
                    }

                    IdeaLibraryPage {
                        id: ideaLibraryPage
                        viewModel: pageRoot.ideaLibraryViewModel

                        onIdeaDoubleClicked: ideaId => {
                            pageRoot.draftEditorViewModel.addIdea(ideaId);
                        }
                    }
                }
            }

            Rectangle {
                id: overlay

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: handle.height + (pageRoot.controlsExpanded ? playerControls.implicitHeight : 0)
                clip: true
                color: Qt.rgba(Theme.bgPage.r, Theme.bgPage.g, Theme.bgPage.b, 0.6)
                z: 100

                Behavior on height {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }

                Rectangle {
                    id: handle
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 64
                    height: 22
                    color: "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: pageRoot.controlsExpanded ? "▲" : "▼"
                        color: Theme.textSecondary
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: pageRoot.controlsExpanded = !pageRoot.controlsExpanded
                    }
                }

                PlayerControls {
                    id: playerControls
                    anchors.top: handle.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                }

                Connections {
                    target: playerControls.mediaPlayer
                    function onPositionChanged() {
                        if (pageRoot.previewEndMs < 0) return;

                        if (playerControls.mediaPlayer.position >= pageRoot.previewEndMs) {
                            playerControls.mediaPlayer.pause();
                            playerControls.mediaPlayer.position = pageRoot.previewStartMs;
                        } else if (playerControls.mediaPlayer.position < pageRoot.previewStartMs) {
                            playerControls.mediaPlayer.position = pageRoot.previewStartMs;
                        }
                    }
                }

                Button {
                    text: "test"
                    onClicked: {
                        pageRoot.draftEditorViewModel.requestPreview();
                    }
                }

                Connections {
                    target: pageRoot.draftEditorViewModel
                    function onPreviewReady(path) {
                        pageRoot.previewStartMs = -1;
                        pageRoot.previewEndMs = -1;

                        playerControls.seekSlider.from = 0;
                        playerControls.seekSlider.to = Qt.binding(() => playerControls.mediaPlayer.duration);

                        playerControls.mediaPlayer.source = path;
                        playerControls.durationText = pageRoot.draftEditorViewModel.mediaDurationAsString(path);
                        playerControls.mediaPlayer.play();
                    }
                }
            }
        }

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.preferredWidth: 640
            Layout.fillHeight: true

            clip: true

            model: pageRoot.draftEditorViewModel.draft_block_list

            moveDisplaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: 150
                }
            }

            delegate: Item {
                id: draftDelegate

                required property var draftBlockId
                required property string kind
                required property string sourcePath
                required property string duration
                required property string start
                required property string end
                required property string label
                required property string body
                required property string sePath
                required property int seOffsetMs
                required property int index

                function fileNameOf(path) {
                    return path.substring(path.lastIndexOf("/") + 1);
                }

                width: parent ? parent.width : 0
                height: card.height

                DropArea {
                    anchors.fill: parent
                    onEntered: drag => {
                        pageRoot.draftEditorViewModel.move(drag.source.dragIndex, draftDelegate.index);
                    }
                }

                Rectangle {
                    id: card
                    width: draftDelegate.width
                    height: contentCol.implicitHeight + Theme.spacingMd * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: {
                        draftDelegate.draftBlockId === pageRoot.draftEditorViewModel.pendingSeTargetId ? Theme.memoSurface : Theme.memoSurfacePressed;
                    }
                    radius: Theme.radiusMd
                    border.width: 1
                    border.color: dragHandle.held ? Theme.textSecondary : Theme.borderDefault

                    Drag.active: dragHandle.held
                    Drag.source: dragHandle
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2

                    states: State {
                        when: dragHandle.held
                        ParentChange {
                            target: card
                            parent: listView
                        }
                        AnchorChanges {
                            target: card
                            anchors.horizontalCenter: undefined
                            anchors.verticalCenter: undefined
                        }
                    }

                    MouseArea {
                        id: mouseArea

                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton

                        onClicked: (mouse) => {
                            contextMenu.popup()
                        }
                    }

                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: "Preview block"
                            onTriggered: {
                                pageRoot.controlsExpanded = true

                                const preview = pageRoot.draftEditorViewModel.resolvePreviewSource(draftDelegate.draftBlockId);

                                playerControls.mediaPlayer.source = preview.source;
                                playerControls.seekSlider.from = Qt.binding(() => pageRoot.previewStartMs);
                                playerControls.seekSlider.to = Qt.binding(() => pageRoot.previewEndMs);

                                pageRoot.previewStartMs = preview.start;
                                pageRoot.previewEndMs = preview.end;

                                playerControls.mediaPlayer.position = preview.start;
                                playerControls.durationText = playerControls.convertMsToString(preview.end - preview.start);
                                playerControls.mediaPlayer.play();
                            }
                            enabled: draftDelegate.kind === "memo" || draftDelegate.kind === "video"
                        }
                        MenuItem {
                            text: "Duplicate"
                            onTriggered: pageRoot.draftEditorViewModel.duplicateDraftBlock(draftDelegate.draftBlockId)
                            enabled: draftDelegate.kind === "memo"
                        }
                        MenuItem {
                            text: "Delete"
                            onTriggered: pageRoot.draftEditorViewModel.deleteDraftBlock(draftDelegate.draftBlockId)
                        }
                    }

                    // --- Drag handle ---
                    MouseArea {
                        id: dragHandle
                        width: 24
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        property bool held: false
                        property int dragIndex: draftDelegate.index

                        drag.target: held ? card : undefined
                        drag.axis: Drag.YAxis
                        cursorShape: Qt.SizeVerCursor

                        onPressed: held = true
                        onReleased: {
                            if (held) {
                                held = false;
                                pageRoot.draftEditorViewModel.commit_draft_block_order();
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            text: "⠿"
                            color: Theme.textMuted
                        }
                    }

                    ColumnLayout {
                        id: contentCol
                        anchors.fill: parent
                        anchors.margins: Theme.spacingMd
                        anchors.leftMargin: Theme.spacingMd + dragHandle.width
                        spacing: Theme.spacingSm

                        Loader {
                            Layout.fillWidth: true
                            sourceComponent: {
                                switch (draftDelegate.kind) {
                                case "memo":
                                    return memoContent;
                                case "image":
                                    return imageContent;
                                case "video":
                                    return videoContent;
                                case "idea":
                                    return ideaContent;
                                default:
                                    return null;
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Theme.spacingSm

                            Label {
                                text: draftDelegate.sePath.length > 0 ? "SE: " + draftDelegate.fileNameOf(draftDelegate.sePath) : "Empty"
                                color: Theme.textSecondary
                                font.pointSize: Theme.fontSizeSm
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            ToolButton {
                                id: seSetButton
                                text: draftDelegate.sePath.length > 0 ? "Reset" : "Add"
                                onClicked: {
                                    if (seSetButton.text === "Add") {
                                        tabBar.currentIndex = 1;
                                        assetViewHighlight.visible = true;
                                        pageRoot.draftEditorViewModel.setPendingSeTargetId(draftDelegate.draftBlockId);
                                    } else {
                                        pageRoot.draftEditorViewModel.resetSeFromBlock(draftDelegate.draftBlockId);
                                        pageRoot.draftEditorViewModel.setPendingSeTargetId(-1);
                                    }
                                }
                            }
                        }
                    }
                }

                // --- kind: memo ---
                Component {
                    id: memoContent
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Theme.spacingSm

                            Label {
                                text: draftDelegate.start
                                color: Theme.textMuted
                            }
                            Label {
                                text: "→"
                                color: Theme.textMuted
                            }
                            Label {
                                text: draftDelegate.end
                                color: Theme.textMuted
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ToolButton {
                                text: qsTr("Edit")
                                onClicked: {
                                    rangeDialog.targetId = draftDelegate.draftBlockId;
                                    pageRoot.draftEditorViewModel.openRangeDialog(draftDelegate.draftBlockId);
                                    rangeDialog.open();
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Theme.spacingSm

                            Label {
                                text: draftDelegate.label
                                visible: text.length > 0
                                color: Theme.textSecondary
                                font.pointSize: Theme.fontSizeSm
                            }
                            Label {
                                text: draftDelegate.body
                                visible: text.length > 0
                                color: Theme.textSecondary
                                font.pointSize: Theme.fontSizeSm
                            }
                        }
                    }
                }

                // --- kind: image ---
                Component {
                    id: imageContent
                    RowLayout {
                        spacing: Theme.spacingSm

                        Image {
                            id: img
                            source: draftDelegate.sourcePath
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredHeight: 64
                            Layout.preferredWidth: (img.sourceSize.height > 0) ? img.sourceSize.width * height / img.sourceSize.height : height
                        }

                        ColumnLayout {
                            spacing: Theme.spacingXs

                            Label {
                                text: draftDelegate.fileNameOf(draftDelegate.sourcePath)
                                color: Theme.textSecondary
                                font.pointSize: Theme.fontSizeSm
                            }
                            TextField {
                                text: draftDelegate.duration
                                implicitWidth: 80
                                onEditingFinished: {
                                    pageRoot.draftEditorViewModel.updateBlockDuration(
                                        draftDelegate.draftBlockId, pageRoot.parseTimeString(text));
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }

                // --- kind: video ---
                Component {
                    id: videoContent
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.spacingSm

                        Image {
                            id: videoImg
                            source: "qrc:/qt/qml/Kirimochi/assets/images/asset_video.png"
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredHeight: 64
                            Layout.preferredWidth: (videoImg.sourceSize.height > 0) ? videoImg.sourceSize.width * height / videoImg.sourceSize.height : height
                        }

                        RowLayout {
                            spacing: Theme.spacingXs
                            Layout.preferredWidth: 400
                            Layout.preferredHeight: 64

                            TextField {
                                id: startField
                                implicitWidth: 80
                                text: draftDelegate.start
                                onEditingFinished: {
                                    const newStart = pageRoot.parseTimeString(text);
                                    const newEnd = pageRoot.parseTimeString(endField.text);
                                    if (newStart >= newEnd) {
                                        text = draftDelegate.start;
                                        return;
                                    }
                                    pageRoot.draftEditorViewModel.updateBlockRange(
                                        draftDelegate.draftBlockId, newStart, newEnd);
                                }
                            }
                            Label {
                                text: "→"
                                color: Theme.textMuted
                            }
                            TextField {
                                id: endField
                                implicitWidth: 80
                                text: draftDelegate.end
                                onEditingFinished: {
                                    const newStart = pageRoot.parseTimeString(startField.text);
                                    const newEnd = pageRoot.parseTimeString(text);
                                    if (newStart >= newEnd) {
                                        text = draftDelegate.end;
                                        return;
                                    }
                                    pageRoot.draftEditorViewModel.updateBlockRange(
                                        draftDelegate.draftBlockId, newStart, newEnd);
                                }
                            }
                            Label {
                                text: draftDelegate.fileNameOf(draftDelegate.sourcePath)
                                color: Theme.textSecondary
                                font.pointSize: Theme.fontSizeSm
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }

                // --- kind: idea ---
                Component {
                    id: ideaContent
                    RowLayout {
                        spacing: Theme.spacingSm

                        Label {
                            text: draftDelegate.body
                            visible: text.length > 0
                            color: Theme.textSecondary
                            font.pointSize: Theme.fontSizeSm
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            text: draftDelegate.duration
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
    }
}
