import QtQuick 2.11
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.4
import Hds.Wallet 1.0
import "controls"
import "./utils.js" as Utils

ColumnLayout {
    id: sendRegularView

    property var defaultFocusItem: receiverTAInput
    spacing: 0
    // callbacks set by parent
    property var onAccepted: undefined
    property var onClosed: undefined
    property var onSwapToken: undefined

    readonly property bool showInsufficientBalanceWarning:
        !viewModel.isEnough &&
        !(viewModel.isZeroBalance && (viewModel.sendAmount == "" || viewModel.sendAmount == "0"))  // not shown if available is 0 and no value entered to send

    TopGradient {
        mainRoot: main
        topColor: Style.passive
    }

    SendViewModel {
        id: viewModel

        onSendMoneyVerified: {
            onAccepted();
        }

        onCantSendToExpired: {
            Qt.createComponent("send_expired.qml")
                .createObject(sendRegularView)
                .open();
        }
    }

    function isTAInputValid() {
        return viewModel.receiverTA.length == 0 || viewModel.receiverTAValid
    }

    Row {
        Layout.alignment:    Qt.AlignHCenter
        Layout.topMargin:    75
        //Layout.bottomMargin: 40

        SFText {
            font.pixelSize:  18
            font.styleName:  "Bold"; font.weight: Font.Bold
            color:           Style.content_main
            //% "Send"
            text:            qsTrId("send-title")
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.maximumHeight: 40
        Layout.minimumHeight: 20
    }

    RowLayout  {
        Layout.fillWidth: true
        spacing:    70

        //
        // Left column
        //
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.leftMargin: 25
            Layout.fillWidth: true
            Layout.preferredWidth: 100

            SFText {
                font.pixelSize:  14
                font.styleName:  "Bold"; font.weight: Font.Bold
                color:           Style.content_main
                //% "Transaction token or contact"
                text:            qsTrId("send-send-to-label")
                bottomPadding:   26
            }

            SFTextInput {
                Layout.fillWidth: true
                id:               receiverTAInput
                font.pixelSize:   14
                color:            isTAInputValid() ? Style.content_main : Style.validator_error
                backgroundColor:  isTAInputValid() ? Style.content_main : Style.validator_error
                font.italic :     !isTAInputValid()
                text:             viewModel.receiverTA
                validator:        RegExpValidator { regExp: /[0-9a-zA-Z]{1,}/ }
                selectByMouse:    true
                //% "Please specify contact or transaction token"
                placeholderText:  qsTrId("send-contact-placeholder")

                onTextChanged: {
                    if (HdsGlobals.isSwapToken(text)&&
                        typeof onSwapToken == "function") {
                        onSwapToken(text);
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                SFText {
                    property bool isTokenOrAddressValid: !isTAInputValid()
                    Layout.alignment: Qt.AlignTop
                    id:               receiverTAError
                    color:            isTokenOrAddressValid ? Style.validator_error : Style.content_secondary
                    font.italic:      !isTokenOrAddressValid
                    font.pixelSize:   12
                    text:             isTokenOrAddressValid
                        //% "Invalid wallet address"
                        ? qsTrId("wallet-send-invalid-address-or-token")
                        : viewModel.tokenGeneratebByNewAppVersionMessage
                    visible:          isTokenOrAddressValid || viewModel.isTokenGeneratebByNewAppVersion
                }
            }

            Binding {
                target:   viewModel
                property: "receiverTA"
                value:    receiverTAInput.text
            }

            SFText {
                Layout.topMargin: 45
                font.pixelSize:   14
                font.styleName:   "Bold"; font.weight: Font.Bold
                color:            Style.content_main
                text:             qsTrId(qsTrId("general-address"))
                topPadding:       5
                visible:          viewModel.isToken
            }

            SFTextArea {
                id:               addressInput
                Layout.fillWidth: true
                font.pixelSize:   14
                color:            Style.content_main
                //enabled:          false
                text:             viewModel.receiverAddress
                visible:          viewModel.isToken
            }

            SFText {
                Layout.topMargin: 45
                font.pixelSize:   14
                font.styleName:   "Bold"; font.weight: Font.Bold
                color:            Style.content_main
                //% "Comment"
                text:             qsTrId("general-comment")
                topPadding:       5
            }

            SFTextInput {
                id:               comment_input
                Layout.fillWidth: true
                font.pixelSize:   14
                color:            Style.content_main
                selectByMouse:    true
                maximumLength:    HdsGlobals.maxCommentLength()
            }

            Item {
                Layout.fillWidth: true
                SFText {
                    Layout.alignment: Qt.AlignTop
                    color:            Style.content_secondary
                    font.italic:      true
                    font.pixelSize:   12
                    //% "Comments are local and won't be shared"
                    text:             qsTrId("general-comment-local")
                }
            }

            Binding {
                target:   viewModel
                property: "comment"
                value:    comment_input.text
            }
        }

        //
        // Right column
        //
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.rightMargin: 35
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 100

            AmountInput {
                //% "Send"
                title:            qsTrId("send-title")
                id:               sendAmountInput
                amountIn:         viewModel.sendAmount
                secondCurrencyRateValue:    viewModel.secondCurrencyRateValue
                secondCurrencyLabel:        viewModel.secondCurrencyLabel
                setMaxAvailableAmount:      function() { viewModel.setMaxAvailableAmount(); }
                hasFee:           true
                showAddAll:       true
                color:            Style.active
                error:            showInsufficientBalanceWarning
                                  //% "Insufficient funds: you would need %1 to complete the transaction"
                                  ? qsTrId("send-founds-fail").arg(Utils.uiStringToLocale(viewModel.missing))
                                  : ""
            }

            Binding {
                target:   viewModel
                property: "sendAmount"
                value:    sendAmountInput.amount
            }

            Binding {
                target:   viewModel
                property: "feeGrothes"
                value:    sendAmountInput.fee
            }

            Item {
                Layout.fillHeight: true
                Layout.maximumHeight: 50
                Layout.minimumHeight: 25
            }

            GridLayout {
                //Layout.topMargin:    50
                Layout.alignment:    Qt.AlignTop
                Layout.minimumWidth: 400
                columnSpacing:       20
                rowSpacing:          10
                columns:             2

                Rectangle {
                    x:      0
                    y:      0
                    width:  parent.width
                    height: parent.height
                    radius: 10
                    color:  Style.background_second
                }

                SFText {
                    Layout.alignment:       Qt.AlignTop
                    Layout.topMargin:       30
                    Layout.leftMargin:      25
                    font.pixelSize:         14
                    color:                  Style.content_secondary
                    //% "Total UTXO value"
                    text:                   qsTrId("send-total-label") + ":"
                }

                HdsAmount
                {
                    Layout.alignment:       Qt.AlignTop
                    Layout.topMargin:       30
                    Layout.rightMargin:     25
                    error:                  showInsufficientBalanceWarning
                    amount:                 viewModel.totalUTXO
                    currencySymbol:         HdsGlobals.getCurrencyLabel(Currency.CurrHds)
                    secondCurrencyLabel:    viewModel.secondCurrencyLabel
                    secondCurrencyRateValue: viewModel.secondCurrencyRateValue
                }

                SFText {
                    Layout.alignment:       Qt.AlignTop
                    Layout.leftMargin:      25
                    font.pixelSize:         14
                    color:                  Style.content_secondary
                    //% "Amount to send"
                    text:                   qsTrId("send-amount-label") + ":"
                }

                HdsAmount
                {
                    Layout.alignment:       Qt.AlignTop
                    Layout.rightMargin:     25
                    error:                  showInsufficientBalanceWarning
                    amount:                 viewModel.sendAmount
                    currencySymbol:         HdsGlobals.getCurrencyLabel(Currency.CurrHds)
                    secondCurrencyLabel:    viewModel.secondCurrencyLabel
                    secondCurrencyRateValue: viewModel.secondCurrencyRateValue
                }

                SFText {
                    Layout.alignment:       Qt.AlignTop
                    Layout.leftMargin:      25
                    font.pixelSize:         14
                    color:                  Style.content_secondary
                    text:                   qsTrId("general-change") + ":"
                }

                HdsAmount
                {
                    Layout.alignment:       Qt.AlignTop
                    Layout.rightMargin:     25
                    error:                  showInsufficientBalanceWarning
                    amount:                 viewModel.change
                    currencySymbol:         HdsGlobals.getCurrencyLabel(Currency.CurrHds)
                    secondCurrencyLabel:    viewModel.secondCurrencyLabel
                    secondCurrencyRateValue: viewModel.secondCurrencyRateValue
                }

                SFText {
                    Layout.alignment:       Qt.AlignTop
                    Layout.leftMargin:      25
                    Layout.bottomMargin:    30
                    font.pixelSize:         14
                    color:                  Style.content_secondary
                    //% "Remaining"
                    text:                   qsTrId("send-remaining-label") + ":"
                }

                HdsAmount
                {
                    Layout.alignment:       Qt.AlignTop
                    Layout.rightMargin:     25
                    Layout.bottomMargin:    30
                    error:                  showInsufficientBalanceWarning
                    amount:                 viewModel.available
                    currencySymbol:         HdsGlobals.getCurrencyLabel(Currency.CurrHds)
                    secondCurrencyLabel:    viewModel.secondCurrencyLabel
                    secondCurrencyRateValue: viewModel.secondCurrencyRateValue
                }
            }
        }
    }
    Item {
        Layout.fillHeight: true
        Layout.maximumHeight: 40
        Layout.minimumHeight: 20
    }
    Row {
        Layout.alignment: Qt.AlignHCenter
        //Layout.topMargin: 40
        spacing:          25

        CustomButton {
            //% "Back"
            text:        qsTrId("general-back")
            icon.source: "qrc:/assets/icon-back.svg"
            onClicked:   onClosed();
        }

        CustomButton {
            //% "Send"
            text:               qsTrId("general-send")
            palette.buttonText: Style.content_opposite
            palette.button:     Style.active
            icon.source:        "qrc:/assets/icon-send-blue.svg"
            enabled:            viewModel.canSend
            onClicked: {                
                const dialogComponent = Qt.createComponent("send_confirm.qml");
                const dialogObject = dialogComponent.createObject(sendRegularView,
                    {
                        addressText: viewModel.receiverAddress,
                        currency: Currency.CurrHds,
                        amount: viewModel.sendAmount,
                        fee: viewModel.feeGrothes,
                        onAcceptedCallback: acceptedCallback,
                        secondCurrencyRate: viewModel.secondCurrencyRateValue,
                        secondCurrencyLabel: viewModel.secondCurrencyLabel
                    }).open();

                function acceptedCallback() {
                    viewModel.sendMoney();
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 20
    }
}
