import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Kirimochi 1.0

Page {
    id: pageRoot

    property AssetLibraryViewModel assetLibraryViewModel
    property IdeaLibraryViewModel ideaLibraryViewModel

    header: RowLayout {
        width: parent.width
        spacing: 0

        ToolButton {
            text: qsTr("< Home")
            onClicked: AppNavigator.goToHomePage()
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Asset")
            }
            TabButton {
                text: qsTr("Idea")
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        AssetLibraryPage {
            id: assetLibraryPage
            viewModel: pageRoot.assetLibraryViewModel
        }

        IdeaLibraryPage {
            id: ideaLibraryPage
            viewModel: pageRoot.ideaLibraryViewModel
        }
    }
}
