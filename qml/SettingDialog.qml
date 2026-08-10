import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kirimochi 1.0

Dialog {
    id: settingDialog
    property SettingViewModel viewModel

    title: qsTr("Setting")
    modal: true
    anchors.centerIn: Overlay.overlay

    standardButtons: Dialog.Ok

    onOpened: {
        reset();
        archiveDirField.forceActiveFocus();
    }

    onAccepted: {
        viewModel.setArchiveDir(archiveDirField.text.trim());
    }

    onRejected: archiveDirField.clear()

    Component.onCompleted: {
        standardButton(Dialog.Ok).text = qsTr("Save");
    }

    contentItem: ColumnLayout {
        spacing: 12

        Label {
            Layout.fillWidth: true
            text: qsTr("Archive Dir")
            color: Theme.textSecondary
            font.pixelSize: 14
        }

        TextField {
            id: archiveDirField

            Layout.fillWidth: true
            selectByMouse: true
            activeFocusOnTab: true
        }
    }
}
