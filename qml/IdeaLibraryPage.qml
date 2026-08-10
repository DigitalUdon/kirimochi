pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kirimochi 1.0

Page {
    id: pageRoot
    property IdeaLibraryViewModel viewModel

    signal ideaDoubleClicked(var ideaId)

    background: Rectangle {
        color: Theme.bgPage
    }

    Dialog {
        id: registerDialog

        title: "Register"
        modal: true
        implicitWidth: 420
        padding: Theme.spacingLg

        background: Rectangle {
            color: Theme.bgSurface
            radius: Theme.radiusMd
            border.color: Theme.borderDefault
        }

        onOpened: {
            nameField.text = "";
            descriptionField.text = "";
            tagsField.text = "";
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingMd

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
                text: "description"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120

                TextArea {
                    id: descriptionField
                    wrapMode: TextArea.Wrap
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
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
                enabled: nameField.text !== ""
                onClicked: {
                    pageRoot.viewModel.registerIdea(nameField.text, descriptionField.text, tagsField.text);
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

        property var editingIdeaId: -1

        onOpened: {
            const idea = pageRoot.viewModel.getIdea(editingIdeaId);

            editNameField.text = idea.name;
            editDescriptionField.text = idea.description;
            editTagsField.text = idea.tags;
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingMd

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
                text: "description"
                color: Theme.textPrimary
                font.pointSize: Theme.fontSizeMd
            }
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120

                TextArea {
                    id: editDescriptionField
                    wrapMode: TextArea.Wrap
                    color: Theme.textPrimary
                    background: Rectangle {
                        color: Theme.bgSurfaceRaised
                        radius: Theme.radiusSm
                        border.color: Theme.borderDefault
                    }
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
                enabled: editNameField.text !== ""
                onClicked: {
                    pageRoot.viewModel.updateIdea(editDialog.editingIdeaId, editNameField.text, editDescriptionField.text, editTagsField.text);
                    editDialog.close();
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Button {
            text: "Register"
            onClicked: registerDialog.open()
        }

        CardGridView {
            id: ideaGrid

            cardPadding: 12
            titleAreaHeight: 60

            model: pageRoot.viewModel.idea_list

            delegate: Rectangle {
                id: ideaDelegate

                required property var ideaId
                required property string name
                required property string description
                required property string tags

                readonly property int cardPadding: ideaGrid.cardPadding

                width: ideaGrid.cardWidth
                height: ideaGrid.cardHeight

                radius: Theme.radiusMd
                activeFocusOnTab: true

                color: mouseArea.pressed ? Theme.bgSurface : hoverHandler.hovered ? Theme.bgSurfaceRaised : Theme.bgSurfacePressed

                border.width: activeFocus ? 2 : 1
                border.color: activeFocus ? Theme.borderAccent : hoverHandler.hovered ? Theme.borderStrong : Theme.borderDefault

                scale: mouseArea.pressed ? 0.985 : 1.0
                transformOrigin: Item.Center

                Accessible.role: Accessible.Button
                Accessible.name: name
                Accessible.description: description

                HoverHandler {
                    id: hoverHandler
                    cursorShape: Qt.PointingHandCursor
                }

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton | Qt.LeftButton

                    onClicked: (mouse) => {
                        if (mouse.button === Qt.RightButton) {
                            contextMenu.popup()
                        }
                    }

                    onDoubleClicked: (mouse) => {
                        if (mouse.button === Qt.LeftButton) {                        
                            ideaDelegate.forceActiveFocus();
                            pageRoot.ideaDoubleClicked(ideaDelegate.ideaId);
                        }
                    }
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: "Edit"
                        onTriggered: {
                            editDialog.editingIdeaId = ideaDelegate.ideaId
                            editDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Delete"
                        onTriggered: pageRoot.viewModel.deleteIdea(ideaDelegate.ideaId)
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

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: ideaDelegate.cardPadding
                    spacing: 6

                    Label {
                        id: nameLabel

                        Layout.fillWidth: true

                        text: ideaDelegate.name

                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.weight: Font.Medium

                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }

                    Label {
                        id: descriptionLabel

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: ideaDelegate.description

                        color: Theme.textSecondary
                        font.pixelSize: 12

                        wrapMode: Text.Wrap
                        maximumLineCount: 4
                        elide: Text.ElideRight

                        verticalAlignment: Text.AlignTop
                    }
                }
            }
        }
    }
}
