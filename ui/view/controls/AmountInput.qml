import QtQuick.Layouts 1.11
import QtQuick 2.11
import Hds.Wallet 1.0
import "../utils.js" as Utils
import Hds.Wallet 1.0

ColumnLayout {
    id: control

    function getFeeTitle() {
        if (control.currency == Currency.CurrHds) {
            return control.currFeeTitle ?
                //% "HDS Transaction fee"
                qsTrId("hds-transaction-fee") :
                //% "Transaction fee"
                qsTrId("general-fee")
        }
        //% "%1 Transaction fee rate"
        return qsTrId("general-fee-rate").arg(control.currencyLabel)
    }

    function getTotalFeeTitle() {
        //% "%1 Transaction fee (est)"
        return qsTrId("general-fee-total").arg(control.currencyLabel)
    }

    function getTotalFeeAmount() {
        return HdsGlobals.calcTotalFee(control.currency, control.fee);
    }

    function getFeeInSecondCurrency(feeValue) {
        return HdsGlobals.calcFeeInSecondCurrency(
            feeValue,
            control.currency,
            control.secondCurrencyRateValue,
            control.secondCurrencyLabel);
    }

    function getAmountInSecondCurrency() {
        return HdsGlobals.calcAmountInSecondCurrency(
            control.amountIn,
            control.secondCurrencyRateValue,
            control.secondCurrencyLabel) + " " + control.secondCurrencyLabel;
    }

    readonly property bool     isValidFee:     hasFee ? feeInput.isValid : true
    readonly property bool     isValid:        error.length == 0 && isValidFee
    readonly property string   currencyLabel:  HdsGlobals.getCurrencyLabel(control.currency)

    property string   title
    property string   color:        Style.accent_incoming
    property string   currColor:    Style.content_main
    property bool     hasFee:       false
    property bool     currFeeTitle: false
    property bool     multi:        false // changing this property in runtime would reset bindings
    property int      currency:     Currency.CurrHds
    property string   amount:       "0"
    property string   amountIn:     "0"  // public property for binding. Use it to avoid binding overriding
    property int      fee:          HdsGlobals.getDefaultFee(control.currency)
    property alias    error:        errmsg.text
    property bool     readOnlyA:    false
    property bool     readOnlyF:    false
    property bool     resetAmount:  true
    property var      amountInput:  ainput
    property bool     showTotalFee: false
    property bool     showAddAll:   false
    property string   secondCurrencyRateValue:  "0"
    property string   secondCurrencyLabel:      ""
    property var      setMaxAvailableAmount:    {} // callback function to set amount from viewmodel
    property bool     showSecondCurrency:       control.secondCurrencyLabel != "" && control.secondCurrencyLabel != control.currencyLabel
    readonly property bool  isExchangeRateAvailable:    control.secondCurrencyRateValue != "0"

    SFText {
        font.pixelSize:   14
        font.styleName:   "Bold"
        font.weight:      Font.Bold
        color:            Style.content_main
        text:             control.title
    }

    RowLayout {
        Layout.fillWidth: true

        SFTextInput {
            id:               ainput
            Layout.fillWidth: true
            font.pixelSize:   36
            font.styleName:   "Light"
            font.weight:      Font.Light
            color:            error.length ? Style.validator_error : control.color
            backgroundColor:  error.length ? Style.validator_error : Style.content_main
            validator:        RegExpValidator {regExp: /^(([1-9][0-9]{0,7})|(1[0-9]{8})|(2[0-4][0-9]{7})|(25[0-3][0-9]{6})|(0))(\.[0-9]{0,7}[1-9])?$/}
            selectByMouse:    true
            text:             formatDisplayedAmount()
            readOnly:         control.readOnlyA

            onTextChanged: {
                // if nothing then "0", remove insignificant zeroes and "." in floats
                if (ainput.focus) {
                    control.amount = text ? text.replace(/\.0*$|(\.\d*[1-9])0+$/,'$1') : "0"
                }
            }

            onFocusChanged: {
                text = formatDisplayedAmount()
                if (focus) cursorPosition = positionAt(ainput.getMousePos().x, ainput.getMousePos().y)
            }

            function formatDisplayedAmount() {
                return control.amountIn == "0" ? "" : (ainput.focus ? control.amountIn : Utils.uiStringToLocale(control.amountIn))
            }

            Connections {
                target: control
                onAmountInChanged: {
                    if (!ainput.focus) {
                        ainput.text = ainput.formatDisplayedAmount()
                    }
                }
            }
        }

        SFText {
            Layout.topMargin:   22
            font.pixelSize:     24
            font.letterSpacing: 0.6
            color:              control.currColor
            text:               control.currencyLabel
            visible:            !multi
        }

        CustomComboBox {
            id:                  currCombo
            Layout.topMargin:    22
            Layout.minimumWidth: 95
            spacing:             0
            fontPixelSize:       24
            fontLetterSpacing:   0.6
            currentIndex:        control.currency
            color:               control.currColor
            visible:             multi
            model:               Utils.currenciesList()

            onActivated: {
                if (multi) control.currency = index
                if (resetAmount) control.amount = 0
            }
        }

        RowLayout {
            id:                  addAllButton
            Layout.alignment:    Qt.AlignBottom
            Layout.bottomMargin: 7
            Layout.leftMargin:   25
            visible:             control.showAddAll

            function addAll(){
                ainput.focus = false;                
                if (control.setMaxAvailableAmount) {
                    control.setMaxAvailableAmount();
                }
            }

            SvgImage {
                Layout.maximumHeight: 16
                Layout.maximumWidth:  16
                source: "qrc:/assets/icon-send-blue-copy-2.svg"
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        addAllButton.addAll();
                    }
                }
            }

            SFText {
                font.pixelSize:   14
                font.styleName:   "Bold";
                font.weight:      Font.Bold
                color:            control.color
                //% "add all"
                text:             qsTrId("amount-input-add-all")
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        addAllButton.addAll();
                    }
                }
            }
        }
    }

    Item {
        Layout.fillWidth: true
        SFText {
            id:              errmsg
            color:           Style.validator_error
            font.pixelSize:  12
            font.styleName:  "Italic"
            width:           parent.width
            visible:         error.length
        }
        SFText {
            id:             amountSecondCurrencyText
            visible:        control.showSecondCurrency && !errmsg.visible && !showTotalFee  // show only on send side
            font.pixelSize: 14
            opacity:        isExchangeRateAvailable ? 0.5 : 0.7
            color:          isExchangeRateAvailable ? Style.content_secondary : Style.accent_fail
            text:           isExchangeRateAvailable
                            ? getAmountInSecondCurrency()
                            //% "Exchange rate to %1 is not available"
                            : qsTrId("general-exchange-rate-not-available").arg(control.secondCurrencyLabel)
        }
    }

    GridLayout {
        columns:       2
        Layout.topMargin: 50
        ColumnLayout {
            Layout.maximumWidth:  198
            Layout.alignment:     Qt.AlignTop
            visible:              control.hasFee
            SFText {
                font.pixelSize:   14
                font.styleName:   "Bold"
                font.weight:      Font.Bold
                color:            Style.content_main
                text:             getFeeTitle()
            }
            FeeInput {
                id:               feeInput
                Layout.fillWidth: true
                fee:              control.fee
                minFee:           HdsGlobals.getMinimalFee(control.currency)
                feeLabel:         HdsGlobals.getFeeRateLabel(control.currency)
                color:            control.color
                readOnly:         control.readOnlyF
                showSecondCurrency:         control.showSecondCurrency
                isExchangeRateAvailable:    control.isExchangeRateAvailable
                secondCurrencyAmount:       getFeeInSecondCurrency(control.fee)
                secondCurrencyLabel:        control.secondCurrencyLabel
                Connections {
                    target: control
                    onFeeChanged: feeInput.fee = control.fee
                    onCurrencyChanged: feeInput.fee = HdsGlobals.getDefaultFee(control.currency)
                }
            }
        }
       
        ColumnLayout {
            Layout.alignment:     Qt.AlignLeft | Qt.AlignTop
            visible:              showTotalFee && control.hasFee && control.currency != Currency.CurrHds
            SFText {
                font.pixelSize:   14
                font.styleName:   "Bold"
                font.weight:      Font.Bold
                color:            Style.content_main
                text:             getTotalFeeTitle()
            }
            SFText {
                id:               totalFeeLabel
                Layout.topMargin: 6
                font.pixelSize:   14
                color:            Style.content_main
                text:             getTotalFeeAmount()
            }
            SFText {
                id:               feeTotalInSecondCurrency
                visible:          control.showSecondCurrency && control.isExchangeRateAvailable
                Layout.topMargin: 6
                font.pixelSize:   14
                opacity:          0.5
                color:            Style.content_secondary
                text:             getFeeInSecondCurrency(parseInt(totalFeeLabel.text, 10))
            }
        }
    }

    SFText {
        enabled:               control.hasFee && control.currency != Currency.CurrHds
        visible:               enabled
        Layout.topMargin:      20
        Layout.preferredWidth: 370
        font.pixelSize:        14
        font.italic:           true
        wrapMode:              Text.WordWrap
        color:                 Style.content_secondary
        lineHeight:            1.1
        //% "Remember to validate the expected fee rate for the blockchain (as it varies with time)."
        text:                  qsTrId("settings-fee-rate-note")
    }

    Binding {
        target:   control
        property: "fee"
        value:    feeInput.fee
    }
}
