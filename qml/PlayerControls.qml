import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia

import Kirimochi 1.0

ColumnLayout {
    id: root
    Layout.fillWidth: true
    Layout.preferredWidth: 640
    Layout.fillHeight: true

    property string durationText: ""
    property bool hasLoadedMedia: false

    property alias mediaPlayer: mediaPlayer
    property alias seekSlider: seekSlider
    property alias volumeSlider: volumeSlider

    function togglePlayback() {
        if (mediaPlayer.playing) {
            mediaPlayer.pause();
        } else {
            mediaPlayer.play();
        }
    }

    function convertMsToString(milliseconds) {
        var hours = Math.floor(milliseconds / 3600000);
        var minutes = Math.floor((milliseconds - hours * 3600000) / 60000);
        var seconds = Math.floor((milliseconds - hours * 3600000 - minutes * 60000) / 1000);

        return hours + ":" + minutes.toString().padStart(2, '0') + ":" + seconds.toString().padStart(2, '0');
    }

    MediaPlayer {
        id: mediaPlayer
        playbackRate: 1.0
        videoOutput: videoOutput
        audioOutput: AudioOutput {
            volume: volumeSlider.value
        }
    }

    VideoOutput {
        id: videoOutput
        Layout.fillWidth: true
        Layout.preferredHeight: parent.width * 9 / 16
    }

    Slider {
        id: seekSlider
        Layout.fillWidth: true
        from: 0
        to: mediaPlayer.duration
        onMoved: mediaPlayer.position = value

        Binding {
            seekSlider.value: mediaPlayer.position
            when: !seekSlider.pressed
            restoreMode: Binding.RestoreNone
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spacingSm

        Label {
            text: root.convertMsToString(seekSlider.pressed ? seekSlider.value : mediaPlayer.position) + " / " + root.durationText
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            text: "-10"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position -= 10000
        }
        Button {
            text: "-5"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position -= 5000
        }
        Button {
            text: "-1"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position -= 1000
        }

        Button {
            text: mediaPlayer.playing ? "⏸" : "▶"
            focusPolicy: Qt.NoFocus
            onClicked: root.togglePlayback()
        }

        Button {
            text: "+1"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position += 1000
        }
        Button {
            text: "+5"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position += 5000
        }
        Button {
            text: "+10"
            focusPolicy: Qt.NoFocus
            onClicked: mediaPlayer.position += 10000
        }

        Item {
            Layout.fillWidth: true
        }

        ComboBox {
            id: rateBox
            Layout.preferredWidth: 90
            focusPolicy: Qt.NoFocus
            model: [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
            currentIndex: 2
            displayText: currentValue + "x"
            onActivated: mediaPlayer.playbackRate = currentValue
        }

        Label {
            text: "vol"
            color: Theme.textMuted
        }
        Slider {
            id: volumeSlider
            Layout.preferredWidth: 120
            from: 0
            to: 1
            value: 1.0
        }
    }
}
