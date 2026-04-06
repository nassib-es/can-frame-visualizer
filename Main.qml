import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "CAN Frame Visualizer"
    color: "#0A0E14"

    QtObject {
        id: colors
        property color bg:          "#0A0E14"
        property color bgPanel:     "#0F1520"
        property color bgInput:     "#141C2B"
        property color border:      "#1E2D45"
        property color accent:      "#00D4FF"
        property color textPrimary: "#F0F6FF"
        property color textMuted:   "#7A9CC0"
        property color sof:         "#FF4444"
        property color fieldId:     "#00D4FF"
        property color rtr:         "#FFB74D"
        property color dlc:         "#81C784"
        property color data:        "#CE93D8"
        property color crc:         "#F06292"
        property color stuffBit:    "#FFD54F"
    }

    QtObject {
        id: frameTypeSelector
        property int current: 0
    }

    ListModel { id: bitStreamModel }
    ListModel { id: frameLogModel }
    ListModel { id: frameBreakdownModel }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // ── Left panel — Frame Builder ──────────────────────────────
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: colors.bgPanel
            radius: 8
            border.color: colors.border
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "FRAME BUILDER"
                    color: colors.accent
                    font.family: "Lucida Console"
                    font.pixelSize: 12
                    font.letterSpacing: 3
                }

                Rectangle { width: parent.width; height: 1; color: colors.border }

                Text {
                    text: "FRAME TYPE"
                    color: colors.textMuted
                    font.family: "Lucida Console"
                    font.pixelSize: 11
                    font.letterSpacing: 2
                }

                Row {
                    spacing: 8
                    Repeater {
                        model: ["STD", "EXT", "RTR"]
                        delegate: Rectangle {
                            width: 70; height: 28; radius: 4
                            color: frameTypeSelector.current === index ? colors.accent : colors.bgInput
                            border.color: frameTypeSelector.current === index ? colors.accent : colors.border
                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: frameTypeSelector.current === index ? colors.bg : colors.textMuted
                                font.family: "Lucida Console"
                                font.pixelSize: 12
                                font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: frameTypeSelector.current = index
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }

                Text {
                    text: "FRAME ID (HEX)"
                    color: colors.textMuted
                    font.family: "Lucida Console"
                    font.pixelSize: 11
                    font.letterSpacing: 2
                }

                Rectangle {
                    width: parent.width; height: 36
                    color: colors.bgInput; radius: 4
                    border.color: idInput.activeFocus ? colors.accent : colors.border
                    TextInput {
                        id: idInput
                        anchors.fill: parent; anchors.margins: 8
                        color: colors.textPrimary
                        font.family: "Lucida Console"; font.pixelSize: 14
                        text: "123"; maximumLength: 8
                        validator: RegularExpressionValidator {
                            regularExpression: /[0-9A-Fa-f]{1,8}/
                        }
                    }
                }

                Text {
                    text: "DLC (0-8)"
                    color: colors.textMuted
                    font.family: "Lucida Console"
                    font.pixelSize: 11
                    font.letterSpacing: 2
                }

                Rectangle {
                    width: parent.width; height: 36
                    color: colors.bgInput; radius: 4
                    border.color: dlcInput.activeFocus ? colors.accent : colors.border
                    TextInput {
                        id: dlcInput
                        anchors.fill: parent; anchors.margins: 8
                        color: colors.textPrimary
                        font.family: "Lucida Console"; font.pixelSize: 14
                        text: "3"; maximumLength: 1
                        validator: IntValidator { bottom: 0; top: 8 }
                    }
                }

                Text {
                    text: "DATA BYTES (HEX)"
                    color: colors.textMuted
                    font.family: "Lucida Console"
                    font.pixelSize: 11
                    font.letterSpacing: 2
                }

                Rectangle {
                    width: parent.width; height: 36
                    color: colors.bgInput; radius: 4
                    border.color: dataInput.activeFocus ? colors.accent : colors.border
                    TextInput {
                        id: dataInput
                        anchors.fill: parent; anchors.margins: 8
                        color: colors.textPrimary
                        font.family: "Lucida Console"; font.pixelSize: 14
                        text: "DE AD BE"; maximumLength: 23
                    }
                }

                Rectangle {
                    width: parent.width; height: 40; radius: 4
                    color: buildArea.containsMouse ? Qt.lighter(colors.accent, 1.2) : colors.accent
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Text {
                        anchors.centerIn: parent
                        text: "BUILD FRAME"
                        color: colors.bg
                        font.family: "Lucida Console"
                        font.pixelSize: 13
                        font.bold: true
                        font.letterSpacing: 2
                    }
                    MouseArea {
                        id: buildArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var result = canBackend.buildFrame(
                                idInput.text,
                                parseInt(dlcInput.text),
                                dataInput.text,
                                ["STD", "EXT", "RTR"][frameTypeSelector.current]
                            )

                            if (!result.valid) {
                                errorCountText.text = canBackend.errorCount
                                return
                            }

                            frameCountText.text    = canBackend.frameCount
                            stuffBitCountText.text = canBackend.stuffBitCount
                            errorCountText.text    = canBackend.errorCount

                            bitStreamModel.clear()
                            for (var i = 0; i < result.bitStream.length; i++)
                                bitStreamModel.append(result.bitStream[i])

                            frameBreakdownModel.clear()
                            for (var j = 0; j < result.breakdown.length; j++)
                                frameBreakdownModel.append(result.breakdown[j])

                            frameLogModel.insert(0, {
                                frameId:   result.frameId,
                                dlc:       result.dlc,
                                data:      result.data,
                                frameType: result.frameType
                            })
                        }
                    }
                }
            }
        }

        // ── Center panel — Bit Visualizer ───────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: colors.bgPanel
            radius: 8
            border.color: colors.border
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "BIT STREAM VISUALIZER"
                    color: colors.accent
                    font.family: "Lucida Console"
                    font.pixelSize: 12
                    font.letterSpacing: 3
                }

                Rectangle { width: parent.width; height: 1; color: colors.border }

                Row {
                    spacing: 16
                    Repeater {
                        model: [
                            {label: "SOF",     color: "#FF4444"},
                            {label: "ID",      color: "#00D4FF"},
                            {label: "RTR/IDE", color: "#FFB74D"},
                            {label: "DLC",     color: "#81C784"},
                            {label: "DATA",    color: "#CE93D8"},
                            {label: "CRC",     color: "#F06292"},
                            {label: "STUFF",   color: "#FFD54F"}
                        ]
                        delegate: Row {
                            spacing: 4
                            Rectangle {
                                width: 10; height: 10; radius: 2
                                color: modelData.color
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.label
                                color: colors.textMuted
                                font.family: "Lucida Console"
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width; height: 120
                    color: colors.bgInput; radius: 4
                    border.color: colors.border; clip: true

                    Flow {
                        anchors.fill: parent; anchors.margins: 8; spacing: 2
                        Repeater {
                            id: bitRepeater
                            model: bitStreamModel
                            delegate: Rectangle {
                                width: 18; height: 28; radius: 2
                                color: model.fieldColor
                                Text {
                                    anchors.centerIn: parent
                                    text: model.bit
                                    color: colors.bg
                                    font.family: "Lucida Console"
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: bitRepeater.count === 0 ? "Build a frame to visualize the bit stream" : ""
                        color: colors.textMuted
                        font.family: "Lucida Console"
                        font.pixelSize: 13
                    }
                }

                Rectangle {
                    width: parent.width
                    height: parent.height - 220
                    color: colors.bgInput; radius: 4
                    border.color: colors.border

                    Column {
                        anchors.fill: parent; anchors.margins: 12; spacing: 8

                        Text {
                            text: "FRAME BREAKDOWN"
                            color: colors.textMuted
                            font.family: "Lucida Console"
                            font.pixelSize: 11
                            font.letterSpacing: 2
                        }

                        Repeater {
                            model: frameBreakdownModel
                            delegate: Row {
                                spacing: 12
                                Rectangle {
                                    width: 8; height: 8; radius: 1
                                    color: model.fieldColor
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: model.fieldName
                                    color: colors.textMuted
                                    font.family: "Lucida Console"
                                    font.pixelSize: 12
                                    width: 80
                                }
                                Text {
                                    text: model.fieldValue
                                    color: colors.textPrimary
                                    font.family: "Lucida Console"
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Right panel — Bus Monitor ────────────────────────────────
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: colors.bgPanel
            radius: 8
            border.color: colors.border
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "BUS MONITOR"
                    color: colors.accent
                    font.family: "Lucida Console"
                    font.pixelSize: 12
                    font.letterSpacing: 3
                }

                Rectangle { width: parent.width; height: 1; color: colors.border }

                Row {
                    spacing: 32
                    Column {
                        spacing: 4
                        Text {
                            text: "FRAMES"
                            color: colors.textMuted
                            font.family: "Lucida Console"
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Text {
                            id: frameCountText
                            text: "0"
                            color: colors.accent
                            font.family: "Lucida Console"
                            font.pixelSize: 22
                            font.bold: true
                        }
                    }
                    Column {
                        spacing: 4
                        Text {
                            text: "ERRORS"
                            color: colors.textMuted
                            font.family: "Lucida Console"
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Text {
                            id: errorCountText
                            text: "0"
                            color: colors.sof
                            font.family: "Lucida Console"
                            font.pixelSize: 22
                            font.bold: true
                        }
                    }
                    Column {
                        spacing: 4
                        Text {
                            text: "STUFF BITS"
                            color: colors.textMuted
                            font.family: "Lucida Console"
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Text {
                            id: stuffBitCountText
                            text: "0"
                            color: colors.stuffBit
                            font.family: "Lucida Console"
                            font.pixelSize: 22
                            font.bold: true
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: colors.border }

                ListView {
                    id: frameListView
                    width: parent.width
                    height: parent.height - 160
                    clip: true
                    model: frameLogModel
                    spacing: 4

                    delegate: Rectangle {
                        width: frameListView.width; height: 52
                        color: colors.bgInput; radius: 4
                        border.color: colors.border

                        Column {
                            anchors.fill: parent; anchors.margins: 8; spacing: 2
                            Row {
                                spacing: 8
                                Rectangle {
                                    width: 32; height: 16; radius: 2
                                    color: colors.fieldId
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        anchors.centerIn: parent
                                        text: model.frameType
                                        color: colors.bg
                                        font.family: "Lucida Console"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }
                                Text {
                                    text: "0x" + model.frameId
                                    color: colors.textPrimary
                                    font.family: "Lucida Console"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    text: "DLC:" + model.dlc
                                    color: colors.dlc
                                    font.family: "Lucida Console"
                                    font.pixelSize: 12
                                }
                            }
                            Text {
                                text: model.data
                                color: colors.textMuted
                                font.family: "Lucida Console"
                                font.pixelSize: 12
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width; height: 32; radius: 4
                    color: colors.bgInput
                    border.color: colors.border
                    Text {
                        anchors.centerIn: parent
                        text: "CLEAR LOG"
                        color: colors.textMuted
                        font.family: "Lucida Console"
                        font.pixelSize: 11
                        font.letterSpacing: 2
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            canBackend.reset()
                            frameLogModel.clear()
                            bitStreamModel.clear()
                            frameBreakdownModel.clear()
                            frameCountText.text    = "0"
                            errorCountText.text    = "0"
                            stuffBitCountText.text = "0"
                        }
                    }
                }
            }
        }
    }
}