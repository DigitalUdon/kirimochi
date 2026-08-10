pragma Singleton

import QtQuick

QtObject {
    // Surfaces
    readonly property color bgPage: "#17181B"
    readonly property color bgSurface: "#1E2024"
    readonly property color bgSurfaceRaised: "#24262B"
    readonly property color bgSurfacePressed: "#181A1E"

    // Accent
    readonly property color accent: "#6C9BFF"

    // Borders
    readonly property color borderDefault: "#34373D"
    readonly property color borderStrong: "#4A4E56"
    readonly property color borderAccent: accent

    // Text
    readonly property color textPrimary: "#EDEEF0"
    readonly property color textSecondary: "#A7ACB4"
    readonly property color textMuted: "#6B707A"

    // Memo card (blue hue)
    readonly property color memoSurface: "#212832"
    readonly property color memoSurfaceHover: "#28303C"
    readonly property color memoSurfacePressed: "#1B212A"

    // Draft card (violet hue)
    readonly property color draftSurface: "#26222D"
    readonly property color draftSurfaceHover: "#2F2838"
    readonly property color draftSurfacePressed: "#1F1B24"

    // Status badges (alpha baked in, overlaid on thumbnail images)
    readonly property color statusDoneBg: "#CC215E45"
    readonly property color statusWipBg: "#CC8A6223"

    // Spacing (4px grid)
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
    readonly property int spacingLg: 16
    readonly property int spacingXl: 24
    readonly property int spacingXxl: 32
    readonly property int spacingXxxl: 48

    // Radius
    readonly property int radiusSm: 6
    readonly property int radiusMd: 8

    // Font sizes (pointSize)
    readonly property int fontSizeSm: 10
    readonly property int fontSizeMd: 12
    readonly property int fontSizeLg: 14
}
