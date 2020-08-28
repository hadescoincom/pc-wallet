import QtQuick 2.11
import QtQuick.Controls 1.2
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import Hds.Wallet 1.0
import "controls"
import "utils.js" as Utils

Item {
    id: root
    anchors.fill: parent

    property string openedTxID: ""

    function onAccepted() { walletStackView.pop(); }
    function onClosed() { walletStackView.pop(); }
    function onSwapToken(token) {
        tokenDuplicateChecker.checkTokenForDuplicate(token);
    }

    WalletViewModel {
        id: viewModel
    }

    property bool toSend: false

    ConfirmationDialog {
        id: deleteTransactionDialog
        //% "Delete"
        okButtonText: qsTrId("general-delete")
    }

    PaymentInfoDialog {
        id: paymentInfoDialog
        onTextCopied: function(text){
            HdsGlobals.copyToClipboard(text);
        }
    }

    PaymentInfoItem {
        id: verifyInfo
    }

    PaymentInfoDialog {
        id: paymentInfoVerifyDialog
        shouldVerify: true
        
        model:verifyInfo 
        onTextCopied: function(text){
            HdsGlobals.copyToClipboard(text);
        }
    }

    TokenDuplicateChecker {
        id: tokenDuplicateChecker
        onAccepted: {
            walletStackView.pop();
            main.openSwapActiveTransactionsList()
        }
        Connections {
            target: tokenDuplicateChecker.model
            onTokenPreviousAccepted: function(token) {
                tokenDuplicateChecker.isOwn = false;
                tokenDuplicateChecker.open();
            }
            onTokenFirstTimeAccepted: function(token) {
                walletStackView.pop();
                walletStackView.push(Qt.createComponent("send_swap.qml"),
                                     {
                                         "onAccepted": tokenDuplicateChecker.onAccepted,
                                         "onClosed": onClosed
                                     });
                walletStackView.currentItem.setToken(token);
            }
            onTokenOwnGenerated: function(token) {
                tokenDuplicateChecker.isOwn = true;
                tokenDuplicateChecker.open();
            }
        }
    }
    
    Title {
        x: 0
        //% "Wallet"
        text: qsTrId("wallet-title")
    }

    StatusBar {
        id: status_bar
        model: statusbarModel
    }

    Component {
        id: walletLayout

        ColumnLayout {
            id: transactionsLayout
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            state: "all"

            Row {
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                Layout.topMargin: 30
                spacing: 20

                CustomButton {
                    id: sendButton
                    height: 32
                    icon.source: "qrc:/assets/icon-send-blue.svg"
                    //% "Send"
                    text: qsTrId("general-send")
                    font.pixelSize: 12
                    //font.capitalization: Font.AllUppercase

                    onClicked: {
                        walletStackView.push(Qt.createComponent("send_regular.qml"),
                                             {"onAccepted":  onAccepted,
                                              "onClosed":    onClosed,
                                              "onSwapToken": onSwapToken})
                    }
                }

                CustomButton {
                    height: 32
                    icon.source: "qrc:/assets/icon-receive-blue.svg"
                    //% "Receive"
                    text: qsTrId("wallet-receive-button")
                    font.pixelSize: 12
                    //font.capitalization: Font.AllUppercase

                    onClicked: {
                        walletStackView.push(Qt.createComponent("receive_regular.qml"), {"onClosed": onClosed});
                    }
                }
            }

            AvailablePanel {
                Layout.topMargin: 29
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.maximumHeight: 80
                Layout.minimumHeight: 80

                Layout.preferredWidth: parseFloat(viewModel.hdsSending) > 0 || parseFloat(viewModel.hdsReceiving) > 0 ? parent.width : (parent.width / 2)

                available:         viewModel.hdsAvailable
                locked:            viewModel.hdsLocked
                lockedMaturing:    viewModel.hdsLockedMaturing
                sending:           viewModel.hdsSending
                receiving:         viewModel.hdsReceiving
                receivingChange:   viewModel.hdsReceivingChange
                receivingIncoming: viewModel.hdsReceivingIncoming
                secondCurrencyLabel:        viewModel.secondCurrencyLabel
                secondCurrencyRateValue:    viewModel.secondCurrencyRateValue
            }

            Item {
                Layout.topMargin: 45
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth : true

                SFText {

                    font {
                        pixelSize: 14
                        letterSpacing: 4
                        styleName: "Bold"; weight: Font.Bold
                        capitalization: Font.AllUppercase
                    }

                    opacity: 0.5
                    color: Style.content_main

                    //% "Transactions"
                    text: qsTrId("wallet-transactions-title")
                }
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.topMargin: 30
                Layout.preferredHeight: 32
                Layout.bottomMargin: 10

                TxFilter {
                    id: allTabSelector
                    Layout.alignment: Qt.AlignVCenter
                    //% "All"
                    label: qsTrId("wallet-transactions-all-tab")
                    onClicked: transactionsLayout.state = "all"
                    capitalization: Font.AllUppercase
                }
                TxFilter {
                    id: inProgressTabSelector
                    Layout.alignment: Qt.AlignVCenter
                    //% "In progress"
                    label: qsTrId("wallet-transactions-in-progress-tab")
                    onClicked: transactionsLayout.state = "inProgress"
                    capitalization: Font.AllUppercase
                }
                TxFilter {
                    id: sentTabSelector
                    Layout.alignment: Qt.AlignVCenter
                    //% "Sent"
                    label: qsTrId("wallet-transactions-sent-tab")
                    onClicked: transactionsLayout.state = "sent"
                    capitalization: Font.AllUppercase
                }
                TxFilter {
                    id: receivedTabSelector
                    Layout.alignment: Qt.AlignVCenter
                    //% "Received"
                    label: qsTrId("wallet-transactions-received-tab")
                    onClicked: transactionsLayout.state = "received"
                    capitalization: Font.AllUppercase
                }
                Item {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                }
                SearchBox {
                    id: searchBox
                    Layout.preferredWidth: 400
                    Layout.alignment: Qt.AlignVCenter
                    //% "Transaction or kernel ID, comment, address or contact"
                    placeholderText: qsTrId("wallet-search-transactions-placeholder")
                }
                CustomToolButton {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    icon.source: "qrc:/assets/icon-export.svg"
                    //: transactions history screen, export button tooltip and open file dialog
                    //% "Export transactions history"
                    ToolTip.text: qsTrId("wallet-export-tx-history")
                    onClicked: {
                        viewModel.exportTxHistoryToCsv();
                    }
                }
                CustomToolButton {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    icon.source: "qrc:/assets/icon-proof.svg"
                    //% "Verify payment"
                    ToolTip.text: qsTrId("wallet-verify-payment")
                    onClicked: {
                        paymentInfoVerifyDialog.model.reset();
                        paymentInfoVerifyDialog.open();
                    }
                }
            }
            
            states: [
                State {
                    name: "all"
                    PropertyChanges { target: allTabSelector; state: "active" }
                    PropertyChanges { target: txProxyModel; filterRole: "status" }
                    PropertyChanges { target: txProxyModel; filterString: "*" }
                },
                State {
                    name: "inProgress"
                    PropertyChanges { target: inProgressTabSelector; state: "active" }
                    PropertyChanges { target: txProxyModel; filterRole: "isInProgress" }
                    PropertyChanges { target: txProxyModel; filterString: "true" }
                },
                State {
                    name: "sent"
                    PropertyChanges { target: sentTabSelector; state: "active" }
                    PropertyChanges { target: txProxyModel; filterRole: "status" }
                    PropertyChanges { target: txProxyModel; filterString: "sent" }
                },
                State {
                    name: "received"
                    PropertyChanges { target: receivedTabSelector; state: "active" }
                    PropertyChanges { target: txProxyModel; filterRole: "status" }
                    PropertyChanges { target: txProxyModel; filterString: "received" }
                }
            ]

            CustomTableView {
                id: transactionsTable

                Component.onCompleted: {
                    transactionsTable.model.modelReset.connect(function(){
                        if (root.openedTxID != "") {
                            var index = viewModel.transactions.index(0, 0);
                            var indexList = viewModel.transactions.match(index, TxObjectList.Roles.TxID, root.openedTxID);
                            if (indexList.length > 0) {
                                index = searchProxyModel.mapFromSource(indexList[0]);
                                index = txProxyModel.mapFromSource(index);
                                transactionsTable.positionViewAtRow(index.row, ListView.Beginning)
                               // var item = transactionsTable.getItemAt(index.row, ListView.Beginning)
                            }
                        }
                    })
                }

                Layout.alignment: Qt.AlignTop
                Layout.fillWidth : true
                Layout.fillHeight : true
                Layout.bottomMargin: 9

                property int rowHeight: 56

                property double resizableWidth: transactionsTable.width - actionsColumn.width
                property double columnResizeRatio: resizableWidth / 810

                selectionMode: SelectionMode.NoSelection
                sortIndicatorVisible: true
                sortIndicatorColumn: 0
                sortIndicatorOrder: Qt.DescendingOrder

                onSortIndicatorColumnChanged: {
                    sortIndicatorOrder = sortIndicatorColumn != 0
                        ? Qt.AscendingOrder
                        : Qt.DescendingOrder;
                }

                model: SortFilterProxyModel {
                    id: txProxyModel
                    source: SortFilterProxyModel {
                        id: searchProxyModel
                        source: viewModel.transactions
                        filterRole: "search"
                        filterString: searchBox.text
                        filterSyntax: SortFilterProxyModel.Wildcard
                        filterCaseSensitivity: Qt.CaseInsensitive
                    }

                    sortOrder: transactionsTable.sortIndicatorOrder
                    sortCaseSensitivity: Qt.CaseInsensitive
                    sortRole: transactionsTable.getColumn(transactionsTable.sortIndicatorColumn).role + "Sort"

                    filterSyntax: SortFilterProxyModel.Wildcard
                    filterCaseSensitivity: Qt.CaseInsensitive
                }

                rowDelegate: ExpandableRowDelegate {
                    id: rowItem
                    collapsed: true
                    rowInModel: styleData.row !== undefined && styleData.row >= 0 &&  styleData.row < txProxyModel.count
                    rowHeight: transactionsTable.rowHeight
                    backgroundColor: styleData.selected ? Style.row_selected : (styleData.alternate ? Style.background_row_even : Style.background_row_odd)
                    property var myModel: parent.model
                    property bool hideFiltered: true
                    onLeftClick: function() {
                        if (!collapsed && searchBox.text.length && hideFiltered) {
                            hideFiltered = false;
                            return false;
                        }
                        return true;
                    }

                    delegate: TransactionDetails {
                        id: detailsPanel
                        width: transactionsTable.width
                        property var        txRolesMap: myModel
                        sendAddress:        txRolesMap && txRolesMap.addressFrom ? txRolesMap.addressFrom : ""
                        receiveAddress:     txRolesMap && txRolesMap.addressTo ? txRolesMap.addressTo : ""
                        senderIdentity:     txRolesMap && txRolesMap.senderIdentity ? txRolesMap.senderIdentity : ""
                        receiverIdentity:   txRolesMap && txRolesMap.receiverIdentity ? txRolesMap.receiverIdentity : ""
                        fee:                txRolesMap && txRolesMap.fee ? txRolesMap.fee : ""
                        comment:            txRolesMap && txRolesMap.comment ? txRolesMap.comment : ""
                        txID:               txRolesMap && txRolesMap.txID ? txRolesMap.txID : ""
                        kernelID:           txRolesMap && txRolesMap.kernelID ? txRolesMap.kernelID : ""
                        status:             txRolesMap && txRolesMap.status ? txRolesMap.status : ""
                        failureReason:      txRolesMap && txRolesMap.failureReason ? txRolesMap.failureReason : ""
                        isIncome:           txRolesMap && txRolesMap.isIncome ? txRolesMap.isIncome : false
                        hasPaymentProof:    txRolesMap && txRolesMap.hasPaymentProof ? txRolesMap.hasPaymentProof : false
                        isSelfTx:           txRolesMap && txRolesMap.isSelfTransaction ? txRolesMap.isSelfTransaction : false
                        rawTxID:            txRolesMap && txRolesMap.rawTxID ? txRolesMap.rawTxID : null
                        stateDetails:       txRolesMap && txRolesMap.stateDetails ? txRolesMap.stateDetails : ""
                        amount:             txRolesMap && txRolesMap.amountGeneral ? txRolesMap.amountGeneral : ""
                        secondCurrencyRate: txRolesMap && txRolesMap.secondCurrencyRate ? txRolesMap.secondCurrencyRate : ""
                        secondCurrencyLabel: viewModel.secondCurrencyLabel
                        searchFilter:       searchBox.text
                        hideFiltered:       rowItem.hideFiltered
                        token:              txRolesMap ? txRolesMap.token : ""

                        onSearchFilterChanged: function(text) {
                            rowItem.collapsed = searchBox.text.length == 0;
                            rowItem.hideFiltered = true;
                        }

                        onOpenExternal : function() {
                            var url = Style.explorerUrl + "block?kernel_id=" + detailsPanel.kernelID;
                            Utils.openExternalWithConfirmation(url);
                        }

                        onTextCopied: function (text) {
                            HdsGlobals.copyToClipboard(text);
                        }

                        onCopyPaymentProof: function() {
                            if (detailsPanel.rawTxID)
                            {
                                var paymentInfo = viewModel.getPaymentInfo(detailsPanel.rawTxID);
                                if (paymentInfo.paymentProof.length === 0)
                                {
                                    paymentInfo.paymentProofChanged.connect(function() {
                                        textCopied(paymentInfo.paymentProof);
                                    });
                                }
                                else
                                {
                                    textCopied(paymentInfo.paymentProof);
                                }
                            }
                        }
                        onShowPaymentProof: {
                            if (detailsPanel.rawTxID)
                            {
                                paymentInfoDialog.model = viewModel.getPaymentInfo(detailsPanel.rawTxID);
                                paymentInfoDialog.open();
                            }
                        }
                    }
                }

                itemDelegate: Item {
                    Item {
                        width: parent.width
                        height: transactionsTable.rowHeight

                        TableItem {
                            text:  styleData.value
                            elide: styleData.elideMode
                            onCopyText: HdsGlobals.copyToClipboard(styleData.value)
                        }
                    }
                }

                TableViewColumn {
                    role: "timeCreated"
                    //% "Created on"
                    title: qsTrId("wallet-txs-date-time")
                    elideMode: Text.ElideRight
                    width: 120 * transactionsTable.columnResizeRatio
                    movable: false
                    resizable: false
                }
                TableViewColumn {
                    role: "addressFrom"
                    //% "From"
                    title: qsTrId("general-address-from")
                    elideMode: Text.ElideMiddle
                    width: 200 * transactionsTable.columnResizeRatio
                    movable: false
                    resizable: false
                }
                TableViewColumn {
                    role: "addressTo"
                    //% "To"
                    title: qsTrId("general-address-to")
                    elideMode: Text.ElideMiddle
                    width: 200 * transactionsTable.columnResizeRatio
                    movable: false
                    resizable: false
                }
                TableViewColumn {
                    role: "amountGeneralWithCurrency"
                    //% "Amount"
                    title: qsTrId("general-amount")
                    elideMode: Text.ElideRight
                    width: 130 * transactionsTable.columnResizeRatio
                    movable: false
                    resizable: false
                    delegate: Item {
                        Item {
                            width: parent.width
                            height: transactionsTable.rowHeight
                            property var isIncome: !!styleData.value && !!model && model.isIncome
                            TableItem {
                                text: (parent.isIncome ? "+ " : "- ") + styleData.value
                                fontWeight: Font.Bold
                                fontStyleName: "Bold"
                                fontSizeMode: Text.Fit
                                color: parent.isIncome ? Style.accent_incoming : Style.accent_outgoing
                                onCopyText: HdsGlobals.copyToClipboard(!!model ? model.amountGeneral : "")
                            }
                        }
                    }
                }
                TableViewColumn {
                    id: statusColumn
                    role: "status"
                    //% "Status"
                    title: qsTrId("general-status")
                    width: transactionsTable.getAdjustedColumnWidth(statusColumn)//150 * transactionsTable.columnResizeRatio
                    movable: false
                    resizable: false
                    delegate: Item {
                        property var myModel: model
                        Item {
                            width: parent.width
                            height: transactionsTable.rowHeight

                            RowLayout {
                                id: statusRow
                                Layout.alignment: Qt.AlignLeft
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 10

                                SvgImage {
                                    id: statusIcon;
                                    Layout.alignment: Qt.AlignLeft
                                    
                                    sourceSize: Qt.size(20, 20)
                                    source: getIconSource()
                                    function getIconSource() {
                                        if (!model) {
                                            return "";
                                        }
                                        if (model.isInProgress) {
                                            if (model.isSelfTransaction) {
                                                return "qrc:/assets/icon-sending-own.svg";
                                            }
                                            return model.isIncome
                                                ? "qrc:/assets/icon-receiving.svg"
                                                : "qrc:/assets/icon-sending.svg";
                                        }
                                        else if (model.isCompleted) {
                                            if (model.isSelfTransaction) {
                                                return "qrc:/assets/icon-sent-own.svg";
                                            }
                                            return model.isIncome
                                                ? "qrc:/assets/icon-received.svg"
                                                : "qrc:/assets/icon-sent.svg";
                                        }
                                        else if (model.isExpired) {
                                            return "qrc:/assets/icon-expired.svg" 
                                        }
                                        else if (model.isFailed) {
                                            return model.isIncome
                                                ? "qrc:/assets/icon-receive-failed.svg"
                                                : "qrc:/assets/icon-send-failed.svg";
                                        }
                                        else {
                                            return model.isIncome
                                                ? "qrc:/assets/icon-receive-canceled.svg"
                                                : "qrc:/assets/icon-send-canceled.svg";
                                        }
                                    }
                                }
                                SFLabel {
                                    Layout.alignment: Qt.AlignLeft
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    font.italic: true
                                    wrapMode: Text.WordWrap
                                    text: styleData.value
                                    verticalAlignment: Text.AlignBottom
                                    color: {
                                        if (!model || model.isExpired) {
                                            return Style.content_secondary;
                                        }
                                        if (model.isInProgress || model.isCompleted) {
                                            if (model.isSelfTransaction) {
                                                return Style.content_main;
                                            }
                                            return model.isIncome ? Style.accent_incoming : Style.accent_outgoing;
                                        } else if (model.isFailed) {
                                            return Style.accent_fail;
                                        }
                                        else {
                                            return Style.content_secondary;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                TableViewColumn {
                    id: actionsColumn
                    elideMode: Text.ElideRight
                    width: 40
                    movable: false
                    resizable: false
                    delegate: txActions
                }

                function showContextMenu(row) {
                    txContextMenu.cancelEnabled = transactionsTable.model.getRoleValue(row, "isCancelAvailable");
                    txContextMenu.deleteEnabled = transactionsTable.model.getRoleValue(row, "isDeleteAvailable");
                    txContextMenu.txID = transactionsTable.model.getRoleValue(row, "rawTxID");
                    txContextMenu.popup();
                }

                Component {
                    id: txActions
                    Item {
                        Item {
                            width: parent.width
                            height: transactionsTable.rowHeight
                            CustomToolButton {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 12
                                icon.source: "qrc:/assets/icon-actions.svg"
                                //% "Actions"
                                ToolTip.text: qsTrId("general-actions")
                                onClicked: {
                                    transactionsTable.showContextMenu(styleData.row);
                                }
                            }
                        }
                    }
                }

                ContextMenu {
                    id: txContextMenu
                    modal: true
                    dim: false
                    property bool cancelEnabled
                    property bool deleteEnabled
                    property var txID

                    Action {
                        //% "Cancel"
                        text: qsTrId("general-cancel")
                        icon.source: "qrc:/assets/icon-cancel.svg"
                        enabled: txContextMenu.cancelEnabled
                        onTriggered: {
                            viewModel.cancelTx(txContextMenu.txID);
                        }
                    }
                    Action {
                        //% "Delete"
                        text: qsTrId("general-delete")
                        icon.source: "qrc:/assets/icon-delete.svg"
                        enabled: txContextMenu.deleteEnabled
                        onTriggered: {
                            //% "The transaction will be deleted. This operation can not be undone"
                            deleteTransactionDialog.text = qsTrId("wallet-txs-delete-message");
                            deleteTransactionDialog.open();
                        }
                    }
                    Connections {
                        target: deleteTransactionDialog
                        onAccepted: {
                            viewModel.deleteTx(txContextMenu.txID);
                        }
                    }
                }

                ConfirmationDialog {
                    id: deleteTransactionDialog
                    //% "Delete"
                    okButtonText: qsTrId("general-delete")
                }
            }
        }
    }

    StackView {
        id: walletStackView
        anchors.fill: parent
        initialItem: walletLayout

        pushEnter: Transition {
            enabled: false
        }
        pushExit: Transition {
            enabled: false
        }
        popEnter: Transition {
            enabled: false
        }
        popExit: Transition {
            enabled: false
        }

        onCurrentItemChanged: {
            if (currentItem && currentItem.defaultFocusItem) {
                walletStackView.currentItem.defaultFocusItem.forceActiveFocus();
            }
        }
    }

    Component.onCompleted: {
        if (root.toSend) {
            sendButton.clicked();
            root.toSend = false;
        }
    }

    Component.onDestruction: {
        var item = walletStackView.currentItem;
        if (item && item.saveAddress && typeof item.saveAddress == "function") {
            item.saveAddress();
        }
    }
}
