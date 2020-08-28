import QtQuick 2.11

AbstractColors {
    property color content_main:          "#ffffff"  // white
    property color accent_outgoing:       "#da68f5"  // heliotrope
    property color accent_incoming:       "#0bccf7"  // bright-sky-blue
    property color accent_swap:           "#000000"
    property color accent_fail:           "#ff746b"
    property color content_secondary:     "#ffffff"  // bluey-grey
    property color content_disabled:      "#889da9"
    property color content_opposite:      "#171717"
    property color validator_warning:     "#f4ce4a"
    property color validator_error:       "#ff625c"
    property color section:               "#2c5066"

    property color navigation_background: "#000000"
    property color background_main:       "#1a132d"
    property color background_main_top:   "#4c3677"
    property color background_second:     Qt.rgba(255, 255, 255, 0.05)
    property color background_row_even:   "#07ffffff"
    property color background_row_odd:    "#0cffffff"
    property color background_details:    "#3e384a"
    property color background_button:     "#708090"
    property color background_popup:      "#342e41"
    property color row_selected:          "#534e5e"
    property color separator:             "#342e41"
    property color table_header:          "#3e384a"

    property color active :               "#333333" // bright-teal
    property color passive:               "#d6d9e0"  // silver
        
    property color caps_warning:          "#000000"

    property string linkStyle: "<style>a:link {color: '#333333'; text-decoration: none;}</style>"
    property string explorerUrl: "https://testnet.explorer.hadescoin.com/"

    property color swapCurrencyPaneGrRight:     "#45316c"
    property color swapCurrencyPaneGrLeftHDS:  "#333333"
    property color swapCurrencyPaneGrLeftBTC:   "#fcaf38"
    property color swapCurrencyPaneGrLeftLTC:   "#bebebe"
    property color swapCurrencyPaneGrLeftQTUM:  "#2e9ad0"
    property color swapCurrencyPaneGrLeftOther: Qt.rgba(0, 246, 210, 0.1)
    property color swapCurrencyStateIndicator:  "#ff746b"
    property color swapCurrencyOptionsBorder:   Qt.rgba(0, 246, 210, 0.15)
}
