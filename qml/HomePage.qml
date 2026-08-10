pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Kirimochi 1.0

Page {
    id: pageRoot
    property HomeViewModel viewModel

    background: Rectangle {
        color: Theme.bgPage
    }

    Connections {
        target: AppNavigator

        function onNavigateToSetting(settingViewModel) {
            settingDialog.viewModel = settingViewModel;
            settingDialog.open();
        }
    }

    Dialog {
        id: newMemoDialog

        title: qsTr("Create New Memo")
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(460, pageRoot.width - 48)

        standardButtons: Dialog.Ok | Dialog.Cancel

        property string selectedVideoId: ""

        onOpened: reset();

        onAccepted: {
            AppNavigator.goToArchivePlayerPage(newMemoDialog.selectedVideoId);
            pageRoot.viewModel.loadProjects();
        }

        onRejected: reset()

        Component.onCompleted: {
            standardButton(Dialog.Ok).text = qsTr("Create Memo");
        }

        function reset() {
            selectedVideoId = "";
        }

        contentItem: ColumnLayout {
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: qsTr("Select the archive folder")
                color: Theme.textSecondary
                font.pixelSize: 14
            }

            Button {
                Layout.fillWidth: true
                text: newMemoDialog.selectedVideoId === ""
                    ? qsTr("Choose Folder...")
                    : qsTr("Video ID: %1").arg(newMemoDialog.selectedVideoId)
                onClicked: folderDialog.open()
            }
        }

        FolderDialog {
            id: folderDialog
            title: qsTr("Select archive folder")

            currentFolder: pageRoot.viewModel.archiveDir()

            onAccepted: {
                const folderName = folderDialog.selectedFolder.toString().split("/").pop();
                newMemoDialog.selectedVideoId = folderName.slice(-11);
            }
        }
    }

    Dialog {
        id: newDraftDialog

        title: qsTr("Create New Draft")
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(460, pageRoot.width - 48)

        standardButtons: Dialog.Ok | Dialog.Cancel

        onOpened: {
            reset();
            draftTitleField.forceActiveFocus();
        }

        onAccepted: {
            AppNavigator.goToDraftEditorPage(draftTitleField.text.trim(), draftConceptField.text.trim());
            pageRoot.viewModel.loadProjects();
        }

        onRejected: draftTitleField.clear()

        Component.onCompleted: {
            standardButton(Dialog.Ok).text = qsTr("Create Draft");
        }

        contentItem: ColumnLayout {
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: qsTr("Enter the Project Title")
                color: Theme.textSecondary
                font.pixelSize: 14
            }

            TextField {
                id: draftTitleField

                Layout.fillWidth: true
                placeholderText: qsTr("Title")
                selectByMouse: true
                activeFocusOnTab: true
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Enter the Project Concept")
                color: Theme.textSecondary
                font.pixelSize: 14
            }

            TextField {
                id: draftConceptField

                Layout.fillWidth: true
                placeholderText: qsTr("Concept")
                selectByMouse: true
                activeFocusOnTab: true
            }
        }
    }

    SettingDialog {
        id: settingDialog

        width: Math.min(460, pageRoot.width - 48)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: pageHeader

            Layout.fillWidth: true
            implicitHeight: headerLayout.implicitHeight + 32
            color: Theme.bgSurface

            RowLayout {
                id: headerLayout

                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 16
                anchors.bottomMargin: 16
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Projects")
                    color: Theme.textPrimary
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                }

                Button {
                    id: newMemoButton
                    text: qsTr("+  New Memo")
                    activeFocusOnTab: true
                    onClicked: newMemoDialog.open()
                }

                Button {
                    id: newDraftButton
                    text: qsTr("+  New Draft")
                    activeFocusOnTab: true
                    onClicked: newDraftDialog.open()
                }

                Button {
                    id: goToLibraryButton
                    text: qsTr("+  To Library")
                    activeFocusOnTab: true
                    onClicked: AppNavigator.goToLibraryPage()
                }

                Button {
                    id: settingsButton
                    text: qsTr("Settings")
                    activeFocusOnTab: true
                    onClicked: AppNavigator.goToSettingDialog()
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.borderDefault
            }
        }

        CardGridView {
            id: projectGrid

            model: pageRoot.viewModel.project_list

            delegate: Rectangle {
                id: projectDelegate

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

                    onOpened: {
                        editTitleField.text = projectDelegate.title;
                    }

                    contentItem: ColumnLayout {
                        spacing: Theme.spacingMd

                        Label {
                            text: "title"
                            color: Theme.textPrimary
                            font.pointSize: Theme.fontSizeMd
                        }
                        TextField {
                            id: editTitleField
                            Layout.fillWidth: true
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
                            enabled: editTitleField.text !== ""
                            onClicked: {
                                pageRoot.viewModel.updateProject(projectDelegate.projectId, projectDelegate.kind, editTitleField.text);
                                editDialog.close();
                            }
                        }
                    }
                }

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

                color: mouseArea.pressed ? pressedColor : hoverHandler.hovered ? hoverColor : normalColor

                border.width: activeFocus ? 2 : 1
                border.color: activeFocus ? Theme.borderAccent : hoverHandler.hovered ? Theme.borderStrong : Theme.borderDefault

                scale: mouseArea.pressed ? 0.985 : 1.0
                transformOrigin: Item.Center

                Accessible.role: Accessible.Button
                Accessible.name: title
                Accessible.description: kind + ", " + status

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
                            projectDelegate.forceActiveFocus();
                            if (projectDelegate.kind === "Memo") {
                                AppNavigator.goToArchivePlayerPage(projectDelegate.projectId);
                            } else if (projectDelegate.kind === "Draft") {
                                AppNavigator.goToDraftEditorPage("", "", projectDelegate.projectId);
                            }
                        } else if (mouse.button === Qt.RightButton) {
                            contextMenu.popup()
                        }
                    }
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: "Edit"
                        onTriggered: editDialog.open()
                    }
                    MenuItem {
                        text: "Delete"
                        onTriggered: pageRoot.viewModel.deleteProject(projectDelegate.kind, projectDelegate.projectId)
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
