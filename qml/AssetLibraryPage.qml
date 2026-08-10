pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtMultimedia

import Kirimochi 1.0

Page {
    id: pageRoot
    property AssetLibraryViewModel viewModel

    signal assetDoubleClicked(var assetId, string kind)

    background: Rectangle {
        color: Theme.bgPage
    }

    Dialog {
        id: registerDialog

        title: "Register"
        modal: true
        implicitWidth: 420
        padding: Theme.spacingLg

        function updateMediaInfo() {
            if (!fileChosen) {
                return;
            }

            const kind = kindCombo.currentText;
            const info = assetLibraryViewModel.getMediaInfo(filePath, kind);

            if (kind === "video") {
                durationField.text = info.duration_ms !== undefined ? info.duration_ms.toString() : "";
                widthField.text = info.width !== undefined ? info.width.toString() : "";
                heightField.text = info.height !== undefined ? info.height.toString() : "";
            } else if (kind === "image") {
                widthField.text = info.width !== undefined ? info.width.toString() : "";
                heightField.text = info.height !== undefined ? info.height.toString() : "";
            } else if (kind === "se" || kind === "bgm") {
                durationField.text = info.duration_ms !== undefined ? info.duration_ms.toString() : "";
            }
        }

        background: Rectangle {
            color: Theme.bgSurface
            radius: Theme.radiusMd
            border.color: Theme.borderDefault
        }

        property string filePath: ""
        property bool fileChosen: filePath !== ""

        onOpened: {
            filePath = "";
            nameField.text = "";
            kindCombo.currentIndex = 0;
            durationField.text = "";
            widthField.text = "";
            heightField.text = "";
            providerField.text = "";
            tagsField.text = "";
            fileDialog.open();
        }

        FileDialog {
            id: fileDialog
            title: "Select asset file"
            onAccepted: {
                registerDialog.filePath = selectedFile.toString();

                const fileName = selectedFile.toString().substring(selectedFile.toString().lastIndexOf("/") + 1);
                const dotIndex = fileName.lastIndexOf(".");
                nameField.text = dotIndex > 0 ? fileName.substring(0, dotIndex) : fileName;

                registerDialog.updateMediaInfo();
            }
            onRejected: registerDialog.close()
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingMd
            visible: registerDialog.fileChosen

            Label {
                text: registerDialog.filePath
                color: Theme.textSecondary
                font.pointSize: Theme.fontSizeSm
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            Label {
                text: "name"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: nameField
                Layout.fillWidth: true
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "kind"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            ComboBox {
                id: kindCombo
                Layout.fillWidth: true
                model: ["se", "bgm", "image", "video", "font", "other"]
                currentIndex: 0
                onActivated: (index) => registerDialog.updateMediaInfo(model[index])
            }

            Label {
                text: "duration_ms"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
                visible: ["se", "bgm", "video"].includes(kindCombo.currentText)
            }
            TextField {
                id: durationField
                Layout.fillWidth: true
                visible: ["se", "bgm", "video"].includes(kindCombo.currentText)
                validator: IntValidator {
                    bottom: 0
                }
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "width / height"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
                visible: ["image", "video"].includes(kindCombo.currentText)
            }
            RowLayout {
                Layout.fillWidth: true
                visible: ["image", "video"].includes(kindCombo.currentText)
                spacing: Theme.spacingSm

                TextField {
                    id: widthField
                    Layout.fillWidth: true
                    placeholderText: "width"
                    validator: IntValidator {
                        bottom: 0
                    }
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
                }
                TextField {
                    id: heightField
                    Layout.fillWidth: true
                    placeholderText: "height"
                    validator: IntValidator {
                        bottom: 0
                    }
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
                }
            }

            Label {
                text: "provider"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: providerField
                Layout.fillWidth: true
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "tags"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: tagsField
                Layout.fillWidth: true
                placeholderText: "tag1,tag2,tag3"
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }
        }

        footer: RowLayout {
            visible: registerDialog.fileChosen
            Layout.margins: Theme.spacingMd
            spacing: Theme.spacingSm

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Cancel"
                onClicked: registerDialog.close()
            }

            Button {
                text: "Register"
                enabled: nameField.text !== "" && kindCombo.currentText !== ""
                onClicked: {
                    pageRoot.viewModel.registerAsset(nameField.text, kindCombo.currentText, registerDialog.filePath, durationField.text !== "" ? parseInt(durationField.text, 10) : 0, widthField.text !== "" ? parseInt(widthField.text, 10) : 0, heightField.text !== "" ? parseInt(heightField.text, 10) : 0, providerField.text, tagsField.text);
                    registerDialog.close();
                }
            }
        }
    }

    Dialog {
        id: editDialog

        title: "Edit"
        modal: true
        implicitWidth: 420
        padding: Theme.spacingLg

        background: Rectangle {
            color: Theme.bgSurface
            radius: Theme.radiusMd
            border.color: Theme.borderDefault
        }

        property var editingAssetId: -1
        property string filePath: ""

        onOpened: {
            const asset = pageRoot.viewModel.getAsset(editingAssetId);

            filePath = asset.file_path;
            editNameField.text = asset.name;
            editKindCombo.currentIndex = editKindCombo.model.indexOf(asset.kind);
            editDurationField.text = asset.duration_ms > 0 ? String(asset.duration_ms) : "";
            editWidthField.text = asset.width > 0 ? String(asset.width) : "";
            editHeightField.text = asset.height > 0 ? String(asset.height) : "";
            editProviderField.text = asset.provider;
            editTagsField.text = asset.tags;
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingMd

            Label {
                text: editDialog.filePath
                color: Theme.textSecondary
                font.pointSize: Theme.fontSizeSm
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            Label {
                text: "name"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: editNameField
                Layout.fillWidth: true
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "kind"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            ComboBox {
                id: editKindCombo
                Layout.fillWidth: true
                model: ["se", "bgm", "image", "video", "font", "other"]
            }

            Label {
                text: "duration_ms"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
                visible: ["se", "bgm", "video"].includes(editKindCombo.currentText)
            }
            TextField {
                id: editDurationField
                Layout.fillWidth: true
                visible: ["se", "bgm", "video"].includes(editKindCombo.currentText)
                validator: IntValidator {
                    bottom: 0
                }
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "width / height"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
                visible: ["image", "video"].includes(editKindCombo.currentText)
            }
            RowLayout {
                Layout.fillWidth: true
                visible: ["image", "video"].includes(editKindCombo.currentText)
                spacing: Theme.spacingSm

                TextField {
                    id: editWidthField
                    Layout.fillWidth: true
                    placeholderText: "width"
                    validator: IntValidator {
                        bottom: 0
                    }
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
                }
                TextField {
                    id: editHeightField
                    Layout.fillWidth: true
                    placeholderText: "height"
                    validator: IntValidator {
                        bottom: 0
                    }
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
                }
            }

            Label {
                text: "provider"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: editProviderField
                Layout.fillWidth: true
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }

            Label {
                text: "tags"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            TextField {
                id: editTagsField
                Layout.fillWidth: true
                placeholderText: "tag1,tag2,tag3"
                color: Theme.textPrimary
                background: Rectangle {
                    color: Theme.bgSurfaceRaised
                    radius: Theme.radiusSm
                    border.color: Theme.borderDefault
                }
            }
        }

        footer: RowLayout {
            Layout.margins: Theme.spacingMd
            spacing: Theme.spacingSm

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Cancel"
                onClicked: editDialog.close()
            }

            Button {
                text: "Update"
                enabled: editNameField.text !== "" && editKindCombo.currentText !== ""
                onClicked: {
                    pageRoot.viewModel.updateAsset(editDialog.editingAssetId, editNameField.text, editKindCombo.currentText, editDialog.filePath, editDurationField.text !== "" ? parseInt(editDurationField.text, 10) : 0, editWidthField.text !== "" ? parseInt(editWidthField.text, 10) : 0, editHeightField.text !== "" ? parseInt(editHeightField.text, 10) : 0, editProviderField.text, editTagsField.text);
                    editDialog.close();
                }
            }
        }
    }

    MediaPlayer {
        id: mediaPlayer
        playbackRate: 1.0
        videoOutput: videoOutput
        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Button {
            text: "Register"
            onClicked: registerDialog.open()
        }

        CardGridView {
            id: assetGrid

            model: pageRoot.viewModel.asset_list

            delegate: Rectangle {
                id: assetDelegate

                required property var assetId
                required property string name
                required property string kind        // "se" | "bgm" | "image" | "video" | "font" | "other"
                required property string filePath
                required property int durationMs

                readonly property int cardPadding: assetGrid.cardPadding
                readonly property int thumbnailWidth: width - cardPadding * 2
                readonly property real thumbnailHeight: thumbnailWidth * 9 / 16

                readonly property bool hasDuration: kind === "se" || kind === "bgm" || kind === "video"

                readonly property var kindPlaceholderSource: ({
                        se: "qrc:/qt/qml/Kirimochi/assets/images/asset_note.png",
                        bgm: "qrc:/qt/qml/Kirimochi/assets/images/asset_note.png",
                        video: "qrc:/qt/qml/Kirimochi/assets/images/asset_video.png",
                        font: "qrc:/qt/qml/Kirimochi/assets/images/asset_font.png",
                        other: "qrc:/qt/qml/Kirimochi/assets/images/asset_other.png"
                    })

                function formatDuration(ms) {
                    var totalSec = Math.floor(ms / 1000);
                    var min = Math.floor(totalSec / 60);
                    var sec = totalSec % 60;
                    return min + ":" + (sec < 10 ? "0" + sec : sec);
                }

                width: assetGrid.cardWidth
                height: assetGrid.cardHeight

                radius: Theme.radiusMd
                activeFocusOnTab: true

                color: mouseArea.pressed ? Theme.bgSurface : hoverHandler.hovered ? Theme.bgSurfaceRaised : Theme.bgSurfacePressed

                border.width: activeFocus ? 2 : 1
                border.color: activeFocus ? Theme.borderAccent : hoverHandler.hovered ? Theme.borderStrong : Theme.borderDefault

                scale: mouseArea.pressed ? 0.985 : 1.0
                transformOrigin: Item.Center

                Accessible.role: Accessible.Button
                Accessible.name: name
                Accessible.description: kind

                Drag.active: dragHandler.active
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.CopyAction
                Drag.mimeData: {
                    "text/uri-list": assetDelegate.filePath
                }

                DragHandler {
                    id: dragHandler
                    target: null

                    onActiveChanged: {
                        assetDelegate.Drag.active = dragHandler.active;
                    }
                }

                HoverHandler {
                    id: hoverHandler
                    cursorShape: Qt.PointingHandCursor
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton | Qt.LeftButton

                    onClicked: (mouse) => {
                        if (mouse.button === Qt.LeftButton) {
                            if (assetDelegate.kind === "se" || assetDelegate.kind === "bgm") {
                                if (!mediaPlayer.playing) {
                                    mediaPlayer.source = assetDelegate.filePath;
                                    mediaPlayer.play();
                                } else {
                                    mediaPlayer.stop();
                                }
                            }
                        } else if (mouse.button === Qt.RightButton) {
                            contextMenu.popup()
                        }
                    }

                    onDoubleClicked: {
                        assetDelegate.forceActiveFocus();
                        pageRoot.assetDoubleClicked(assetDelegate.assetId, assetDelegate.kind);
                    }
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: "Edit"
                        onTriggered: {
                            editDialog.editingAssetId = assetDelegate.assetId
                            editDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Delete"
                        onTriggered: pageRoot.viewModel.deleteAsset(assetDelegate.assetId)
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

                    anchors.topMargin: assetDelegate.cardPadding
                    anchors.leftMargin: assetDelegate.cardPadding
                    anchors.rightMargin: assetDelegate.cardPadding

                    height: assetDelegate.thumbnailHeight

                    radius: Theme.radiusSm
                    color: Theme.bgPage
                    clip: true

                    Image {
                        id: thumbnailImage

                        anchors.fill: parent

                        source: assetDelegate.kind === "image" ? assetDelegate.filePath : assetDelegate.kindPlaceholderSource[assetDelegate.kind]

                        fillMode: assetDelegate.kind === "image" ? Image.PreserveAspectCrop : Image.PreserveAspectFit

                        asynchronous: true
                        smooth: true

                        sourceSize.width: thumbnailContainer.width * 2
                        sourceSize.height: thumbnailContainer.height * 2
                    }

                    Rectangle {
                        id: durationBadge

                        visible: assetDelegate.hasDuration

                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.rightMargin: 8
                        anchors.bottomMargin: 8

                        width: durationText.implicitWidth + 16
                        height: 24
                        radius: height / 2

                        color: Theme.statusWipBg

                        Label {
                            id: durationText

                            anchors.centerIn: parent

                            text: assetDelegate.formatDuration(assetDelegate.durationMs)

                            color: Theme.textPrimary
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                    }
                }

                Label {
                    id: nameLabel

                    anchors.top: thumbnailContainer.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    anchors.topMargin: 8
                    anchors.leftMargin: assetDelegate.cardPadding
                    anchors.rightMargin: assetDelegate.cardPadding
                    anchors.bottomMargin: assetDelegate.cardPadding

                    text: assetDelegate.name

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
