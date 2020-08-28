import QtQuick 2.11

AbstractColors {
    // Color definitions
    property color content_main:          "#000000"
    property color accent_outgoing:       "#da68f5"  // heliotrope
    property color accent_incoming:       "#0bccf7"  // bright-sky-blue
    property color accent_swap:           "#41b37d"
    property color accent_fail:           "#d74e5a"
    property color content_secondary:     "#333333"  // bluey-grey
    property color content_disabled:      "#8d8d8d"
    property color content_opposite:      "#000000" // marine
    property color validator_warning:     "#f4ce4a"
    property color validator_error:       "#ff625c"
    property color section:               "#000000"

    property color navigation_background: "#000000" 
    property color background_main:       "#f1f1f1"
    property color background_main_top:   "#f1f1f1"
    property color background_second:     "#f1f1f1"
    property color background_row_even:   "#fafafa"
    property color background_row_odd:    "#f1f1f1"
    property color background_details:    "#ffffff"
    property color background_button:     "#000000"
    property color background_popup:      "#ffffff"
    property color row_selected:          "#e8e8e8"
    property color separator:             "#000000"
    property color table_header:          "#e8e8e8"
    property color grayBg:                "#e8e8e8"
    property color gray3:                 "#333333"
    property color grayC:                 "#cccccc"
    property color gray7:                 "#777777"

    property color active :               "#2483ff" // bright-teal
    property color passive:               "#ffffff"  // silver
        
    property color caps_warning:          "#000000"

    property string linkStyle: "<style>a:link {color: '#2483ff'; text-decoration: none;}</style>"
    property string explorerUrl: "https://ex.hadescoin.com/"

    property color swapCurrencyPaneGrRight:     "#e8e8e8"
    property color swapCurrencyPaneGrLeftHDS:  "#eaeaea"
    property color swapCurrencyPaneGrLeftBTC:   "#fcaf38"
    property color swapCurrencyPaneGrLeftLTC:   "#bebebe"
    property color swapCurrencyPaneGrLeftQTUM:  "#2e9ad0"
    property color swapCurrencyPaneGrLeftOther: Qt.rgba(0, 246, 210, 0.1)
    property color swapCurrencyStateIndicator:  "#eeeeee"
    property color swapCurrencyOptionsBorder:   "#eeeeee"
}
