// Copyright 2018 The Hds Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <QObject>

#include "wallet/client/wallet_client.h"
#include <set>

class WalletModel
    : public QObject
    , public hds::wallet::WalletClient
{
    Q_OBJECT
public:

    using Ptr = std::shared_ptr<WalletModel>;

    WalletModel(hds::wallet::IWalletDB::Ptr walletDB, const std::string& nodeAddr, hds::io::Reactor::Ptr reactor);
    ~WalletModel() override;

    QString GetErrorString(hds::wallet::ErrorType type);
    bool isOwnAddress(const hds::wallet::WalletID& walletID) const;
    bool isAddressWithCommentExist(const std::string& comment) const;

    hds::Amount getAvailable() const;
    hds::Amount getReceiving() const;
    hds::Amount getReceivingIncoming() const;
    hds::Amount getReceivingChange() const;
    hds::Amount getSending() const;
    hds::Amount getMaturing() const;
    hds::Height getCurrentHeight() const;
    hds::Timestamp getCurrentHeightTimestamp() const;
    hds::Block::SystemState::ID getCurrentStateID() const;

signals:
    void walletStatus(const hds::wallet::WalletStatus& status);
    void transactionsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>& items);
    void syncProgressUpdated(int done, int total);
    void changeCalculated(hds::Amount change);
    void allUtxoChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Coin>& utxos);
    void addressesChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::WalletAddress>& addresses);
    void addressesChanged(bool own, const std::vector<hds::wallet::WalletAddress>& addresses);
    void swapOffersChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::SwapOffer>& offers);
    void generatedNewAddress(const hds::wallet::WalletAddress& walletAddr);
    void swapParamsLoaded(const hds::ByteBuffer& params);
    void newAddressFailed();
    void nodeConnectionChanged(bool isNodeConnected);
    void walletError(hds::wallet::ErrorType error);
    void sendMoneyVerified();
    void cantSendToExpired();
    void paymentProofExported(const hds::wallet::TxID& txID, const QString& proof);
    void addressChecked(const QString& addr, bool isValid);

    void availableChanged();
    void receivingChanged();
    void receivingIncomingChanged();
    void receivingChangeChanged();
    void sendingChanged();
    void maturingChanged();
    void stateIDChanged();
    void functionPosted(const std::function<void()>&);
#if defined(HDS_HW_WALLET)
    void showTrezorMessage();
    void hideTrezorMessage();
    void showTrezorError(const QString& error);
#endif
    void txHistoryExportedToCsv(const QString& data);
    
    void exchangeRatesUpdate(const std::vector<hds::wallet::ExchangeRate>&);
    void notificationsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Notification>&);

private:
    void onStatus(const hds::wallet::WalletStatus& status) override;
    void onTxStatus(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>& items) override;
    void onSyncProgressUpdated(int done, int total) override;
    void onChangeCalculated(hds::Amount change) override;
    void onAllUtxoChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Coin>& utxos) override;
    void onAddressesChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::WalletAddress>& items) override;
    void onAddresses(bool own, const std::vector<hds::wallet::WalletAddress>& addrs) override;
#ifdef HDS_ATOMIC_SWAP_SUPPORT
    void onSwapOffersChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::SwapOffer>& offers) override;
#endif  // HDS_ATOMIC_SWAP_SUPPORT
    void onGeneratedNewAddress(const hds::wallet::WalletAddress& walletAddr) override;
    void onSwapParamsLoaded(const hds::ByteBuffer& token) override;
    void onNewAddressFailed() override;
    void onNodeConnectionChanged(bool isNodeConnected) override;
    void onWalletError(hds::wallet::ErrorType error) override;
    void FailedToStartWallet() override;
    void onSendMoneyVerified() override;
    void onCantSendToExpired() override;
    void onPaymentProofExported(const hds::wallet::TxID& txID, const hds::ByteBuffer& proof) override;
    void onCoinsByTx(const std::vector<hds::wallet::Coin>& coins) override;
    void onAddressChecked(const std::string& addr, bool isValid) override;
    void onImportRecoveryProgress(uint64_t done, uint64_t total) override;
    void onNoDeviceConnected() override;
    void onImportDataFromJson(bool isOk) override;
    void onExportDataToJson(const std::string& data) override;
    void onExportTxHistoryToCsv(const std::string& data) override;
    void onExchangeRates(const std::vector<hds::wallet::ExchangeRate>&) override;
    void onNotificationsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Notification>&) override;

    void onShowKeyKeeperMessage() override;
    void onHideKeyKeeperMessage() override;
    void onShowKeyKeeperError(const std::string&) override;

    void onPostFunctionToClientContext(MessageFunction&& func) override;
    hds::Version getLibVersion() const override;
    uint32_t getClientRevision() const override;

private slots:
    void setStatus(const hds::wallet::WalletStatus& status);
    void setAddresses(bool own, const std::vector<hds::wallet::WalletAddress>& addrs);
    void doFunction(const std::function<void()>& func);

private:
    std::set<hds::wallet::WalletID> m_myWalletIds;
    std::set<std::string> m_myAddrLabels;
    hds::wallet::WalletStatus m_status;
};
