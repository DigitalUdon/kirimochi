pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtWebEngine

import Kirimochi 1.0

Page {
    id: pageRoot
    required property ArchivePlayerViewModel viewModel

    padding: 20

    Component.onDestruction: viewModel.setMediaPosition(playbackControls.mediaPlayer.position)

    Timer {
        interval: 60000
        running: playbackControls.mediaPlayer.playing
        repeat: true
        onTriggered: pageRoot.viewModel.setMediaPosition(playbackControls.mediaPlayer.position)
    }

    Connections {
        target: Qt.application
        function onAboutToQuit() {
            pageRoot.viewModel.setMediaPosition(playbackControls.mediaPlayer.position);
        }
    }

    function parseTimeString(timeString) {
        var parts = timeString.split(":");
        var hours = parseInt(parts[0], 10);
        var minutes = parseInt(parts[1], 10);
        var seconds = parseInt(parts[2], 10);

        return (hours * 3600 + minutes * 60 + seconds) * 1000;
    }

    readonly property bool textEditing: Window.activeFocusItem instanceof TextInput

    Shortcut {
        sequence: "Left"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position -= 5000
    }
    Shortcut {
        sequence: "Right"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position += 5000
    }
    Shortcut {
        sequence: "Shift+Left"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position -= 1000
    }
    Shortcut {
        sequence: "Shift+Right"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position += 1000
    }
    Shortcut {
        sequence: "Ctrl+Left"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position -= 10000
    }
    Shortcut {
        sequence: "Ctrl+Right"
        enabled: !pageRoot.textEditing
        onActivated: playbackControls.mediaPlayer.position += 10000
    }

    property bool focusNewMemo: false

    component TimeChip: Label {
        id: chip

        signal clicked

        font.pointSize: Theme.fontSizeMd
        leftPadding: Theme.spacingSm
        rightPadding: Theme.spacingSm
        topPadding: Theme.spacingXs
        bottomPadding: Theme.spacingXs

        background: Rectangle {
            radius: Theme.radiusSm
            color: chipArea.containsMouse ? Theme.memoSurfaceHover : Theme.bgSurfaceRaised
            border.width: 1
            border.color: chipArea.containsMouse ? Theme.borderAccent : Theme.borderDefault
        }

        MouseArea {
            id: chipArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: chip.clicked()
        }
    }

    component MemoActionButton: Button {
        focusPolicy: Qt.NoFocus
        flat: true
        font.pointSize: Theme.fontSizeSm
        leftPadding: Theme.spacingSm
        rightPadding: Theme.spacingSm
        topPadding: Theme.spacingXs
        bottomPadding: Theme.spacingXs
        ToolTip.visible: hovered
        ToolTip.delay: 400
    }

    header: RowLayout {
        width: parent.width
        spacing: 0

        ToolButton {
            text: qsTr("< Home")
            onClicked: AppNavigator.goToHomePage()
        }
    }

    RowLayout {
        anchors.fill: parent

        Component.onCompleted: {
            playbackControls.mediaPlayer.source = pageRoot.viewModel.videoPath();
            playbackControls.mediaPlayer.play();
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 640
            Layout.fillHeight: true

            PlayerControls {
                id: playbackControls
                durationText: pageRoot.viewModel.mediaDurationAsString()

                Connections {
                    target: playbackControls.mediaPlayer
                    function onDurationChanged() {
                        pageRoot.viewModel.setMediaDuration(playbackControls.mediaPlayer.duration);
                    }
                    function onMediaStatusChanged() {
                        if (playbackControls.mediaPlayer.mediaStatus === MediaPlayer.LoadedMedia) {
                            if (!playbackControls.hasLoadedMedia) {
                                playbackControls.mediaPlayer.position = pageRoot.viewModel.mediaPositionAsMs();
                                playbackControls.mediaPlayer.play();
                                playbackControls.hasLoadedMedia = true;
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSm

                Button {
                    text: "+ Best"
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        pageRoot.focusNewMemo = true;
                        pageRoot.viewModel.addMemoBlock(playbackControls.mediaPlayer.position, "Best");
                    }
                }
                Button {
                    text: "+ Good"
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        pageRoot.focusNewMemo = true;
                        pageRoot.viewModel.addMemoBlock(playbackControls.mediaPlayer.position, "Good");
                    }
                }
                Button {
                    text: "+ Memo"
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        pageRoot.focusNewMemo = true;
                        pageRoot.viewModel.addMemoBlock(playbackControls.mediaPlayer.position, "Memo");
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true
                sourceComponent: pageRoot.viewModel.chatVolumeDataPath() === "" ? rectComponent : webViewComponent
            }

            Component {
                id: webViewComponent
                WebEngineView {
                    url: Qt.resolvedUrl(pageRoot.viewModel.chatVolumeDataPath())
                }
            }

            Component {
                id: rectComponent
                Rectangle {
                    color: "transparent"
                }
            }
        }

        ListView {
            id: memoList
            Layout.fillWidth: true
            Layout.preferredWidth: 640
            Layout.fillHeight: true
            spacing: Theme.spacingSm
            clip: true
            model: pageRoot.viewModel.memo_block_list

            onCountChanged: {
                if (pageRoot.focusNewMemo) {
                    pageRoot.focusNewMemo = false;
                    memoList.positionViewAtEnd();
                    Qt.callLater(function () {
                        var item = memoList.itemAtIndex(memoList.count - 1);
                        if (item) {
                            item.focusBody();
                        }
                    });
                }
            }

            delegate: Rectangle {
                id: memoDelegate

                required property var memoId
                required property string start
                required property string end
                required property string label
                required property string body

                function focusBody() {
                    bodyField.forceActiveFocus();
                    bodyField.selectAll();
                }

                width: parent.width
                height: contentCol.implicitHeight + Theme.spacingMd * 2
                color: Theme.memoSurface
                radius: Theme.radiusMd
                border.width: 1
                border.color: Theme.borderDefault

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton

                    onClicked: {
                        contextMenu.popup()
                    }
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: "Delete"
                        onTriggered: pageRoot.viewModel.deleteMemoBlock(memoDelegate.memoId)
                    }
                }

                ColumnLayout {
                    id: contentCol
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMd
                    spacing: Theme.spacingSm

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.spacingSm

                        TimeChip {
                            text: memoDelegate.start
                            onClicked: playbackControls.mediaPlayer.position = pageRoot.parseTimeString(memoDelegate.start)
                        }

                        Label {
                            text: "→"
                            color: Theme.textMuted
                        }

                        TimeChip {
                            text: memoDelegate.end
                            onClicked: playbackControls.mediaPlayer.position = pageRoot.parseTimeString(memoDelegate.end)
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        MemoActionButton {
                            text: "⇤"
                            onClicked: pageRoot.viewModel.updateMemoStart(memoDelegate.memoId, playbackControls.mediaPlayer.position)
                        }

                        MemoActionButton {
                            text: "⇥"
                            onClicked: pageRoot.viewModel.updateMemoEnd(memoDelegate.memoId, playbackControls.mediaPlayer.position)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.spacingSm

                        Label {
                            text: {
                                let icon = "🏷️";
                                if (memoDelegate.label === "Best") icon = "⭐";
                                else if (memoDelegate.label === "Good") icon = "✨";
                                return icon + " " + memoDelegate.label;
                            }
                            color: Theme.textSecondary
                            font.pointSize: Theme.fontSizeSm
                            Layout.preferredWidth: 60
                            elide: Text.ElideRight
                        }

                        TextField {
                            id: bodyField
                            Layout.fillWidth: true
                            font.pointSize: Theme.fontSizeMd
                            text: memoDelegate.body
                            onEditingFinished: pageRoot.viewModel.updateMemoComment(memoDelegate.memoId, text)
                        }
                    }
                }
            }
        }
    }
}
