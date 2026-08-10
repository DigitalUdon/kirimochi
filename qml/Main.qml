import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

import Kirimochi 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: "Kirimochi - v0.1.0"

    Universal.accent: Theme.accent
    Universal.background: Theme.bgPage
    Universal.foreground: Theme.textPrimary

    Dialog {
        id: fatalErrorDialog
        title: "Fatal Error"

        onAccepted: Qt.quit()
        onRejected: Qt.quit()
    }

    Dialog {
        id: warningDialog
        title: "Warning"

        //TODO: Display invalid value messages
    }

    StackView {
        id: stackView
        anchors.fill: parent

        Component.onCompleted: {
            if (AppNavigator.hasDBError()) {
                fatalErrorDialog.open();
            }

            if (AppNavigator.hasInvalidValue()) {
                warningDialog.open();
            }

            AppNavigator.goToHomePage();
        }
    }

    Connections {
        target: AppNavigator

        function onNavigateToHome(viewModel) {
            stackView.push("HomePage.qml", {
                viewModel: viewModel
            });
        }

        function onNavigateToArchivePlayer(viewModel) {
            stackView.push("ArchivePlayerPage.qml", {
                viewModel: viewModel
            });
        }

        function onNavigateToDraftEditor(draftEditorViewModel, assetLibraryViewModel, ideaLibraryViewModel) {
            stackView.push("DraftEditorPage.qml", {
                draftEditorViewModel: draftEditorViewModel,
                assetLibraryViewModel: assetLibraryViewModel,
                ideaLibraryViewModel: ideaLibraryViewModel
            });
        }

        function onNavigateToLibrary(assetLibraryViewModel, ideaLibraryViewModel) {
            stackView.push("LibraryPage.qml", {
                assetLibraryViewModel: assetLibraryViewModel,
                ideaLibraryViewModel: ideaLibraryViewModel
            });
        }
    }
}
