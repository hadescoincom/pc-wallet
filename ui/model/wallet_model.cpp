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

#include "wallet_model.h"
#include "app_model.h"
#include "utility/logger.h"
#include "utility/bridge.h"
#include "utility/io/asyncevent.h"
#include "utility/helpers.h"
#include "version.h"

using namespace hds;
using namespace hds::wallet;
using namespace hds::io;
using namespace std;

WalletModel::WalletModel(IWalletDB::Ptr walletDB, const std::string& nodeAddr, hds::io::Reactor::Ptr reactor)
    : WalletClient(walletDB, nodeAddr, reactor)
{
    qRegisterMetaType<hds::ByteBuffer>("hds::ByteBuffer");
    qRegisterMetaType<hds::wallet::WalletStatus>("hds::wallet::WalletStatus");
    qRegisterMetaType<hds::wallet::ChangeAction>("hds::wallet::ChangeAction");
    qRegisterMetaType<vector<hds::wallet::TxDescription>>("std::vector<hds::wallet::TxDescription>");
    qRegisterMetaType<vector<hds::wallet::SwapOffer>>("std::vector<hds::wallet::SwapOffer>");
    qRegisterMetaType<hds::Amount>("hds::Amount");
    qRegisterMetaType<vector<hds::wallet::Coin>>("std::vector<hds::wallet::Coin>");
    qRegisterMetaType<vector<hds::wallet::WalletAddress>>("std::vector<hds::wallet::WalletAddress>");
    qRegisterMetaType<hds::wallet::WalletID>("hds::wallet::WalletID");
    qRegisterMetaType<hds::wallet::WalletAddress>("hds::wallet::WalletAddress");
    qRegisterMetaType<hds::wallet::ErrorType>("hds::wallet::ErrorType");
    qRegisterMetaType<hds::wallet::TxID>("hds::wallet::TxID");
    qRegisterMetaType<hds::wallet::TxParameters>("hds::wallet::TxParameters");
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    qRegisterMetaType<std::vector<hds::wallet::Notification>>("std::vector<hds::wallet::Notification>");
    qRegisterMetaType<hds::wallet::VersionInfo>("hds::wallet::VersionInfo");
    qRegisterMetaType<hds::wallet::WalletImplVerInfo>("hds::wallet::WalletImplVerInfo");
    qRegisterMetaType<ECC::uintBig>("ECC::uintBig");

    connect(this, SIGNAL(walletStatus(const hds::wallet::WalletStatus&)), this, SLOT(setStatus(const hds::wallet::WalletStatus&)));
    connect(this, SIGNAL(addressesChanged(bool, const std::vector<hds::wallet::WalletAddress>&)),
            this, SLOT(setAddresses(bool, const std::vector<hds::wallet::WalletAddress>&)));
    connect(this, SIGNAL(functionPosted(const std::function<void()>&)), this, SLOT(doFunction(const std::function<void()>&)));

    getAsync()->getAddresses(true);
}

WalletModel::~WalletModel()
{
    stopReactor();
}

QString WalletModel::GetErrorString(hds::wallet::ErrorType type)
{
    // TODO: add more detailed error description
    switch (type)
    {
    case wallet::ErrorType::NodeProtocolBase:
        //% "Node protocol error!"
        return qtTrId("wallet-model-node-protocol-error");
    case wallet::ErrorType::NodeProtocolIncompatible:
        //% "You are trying to connect to incompatible peer."
        return qtTrId("wallet-model-incompatible-peer-error");
    case wallet::ErrorType::ConnectionBase:
        //% "Connection error"
        return qtTrId("wallet-model-connection-base-error");
    case wallet::ErrorType::ConnectionTimedOut:
        //% "Connection timed out"
        return qtTrId("wallet-model-connection-time-out-error");
    case wallet::ErrorType::ConnectionRefused:
        //% "Cannot connect to node"
        return qtTrId("wallet-model-connection-refused-error") + ": " +  getNodeAddress().c_str();
    case wallet::ErrorType::ConnectionHostUnreach:
        //% "Node is unreachable"
        return qtTrId("wallet-model-connection-host-unreach-error") + ": " + getNodeAddress().c_str();
    case wallet::ErrorType::ConnectionAddrInUse:
    {
        auto localNodePort = AppModel::getInstance().getSettings().getLocalNodePort();
        //% "The port %1 is already in use. Check if a wallet is already running on this machine or change the port settings."
        return qtTrId("wallet-model-connection-addr-in-use-error").arg(QString::number(localNodePort));
    }
    case wallet::ErrorType::TimeOutOfSync:
        //% "System time not synchronized"
        return qtTrId("wallet-model-time-sync-error");
    case wallet::ErrorType::HostResolvedError:
        //% "Incorrect node name or no Internet connection."
        return qtTrId("wallet-model-host-unresolved-error");
    default:
        //% "Unexpected error!"
        return qtTrId("wallet-model-undefined-error");
    }
}

bool WalletModel::isOwnAddress(const WalletID& walletID) const
{
    return m_myWalletIds.count(walletID);
}

bool WalletModel::isAddressWithCommentExist(const std::string& comment) const
{
    if (comment.empty())
    {
        return false;
    }
    return m_myAddrLabels.find(comment) != m_myAddrLabels.end();
}

void WalletModel::onStatus(const hds::wallet::WalletStatus& status)
{
    emit walletStatus(status);
}

void WalletModel::onTxStatus(hds::wallet::ChangeAction action, const std::vector<hds::wallet::TxDescription>& items)
{
    emit transactionsChanged(action, items);
}

void WalletModel::onSyncProgressUpdated(int done, int total)
{
    emit syncProgressUpdated(done, total);
}

void WalletModel::onChangeCalculated(hds::Amount change)
{
    emit changeCalculated(change);
}

void WalletModel::onAllUtxoChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::Coin>& utxos)
{
    emit allUtxoChanged(action, utxos);
}

void WalletModel::onAddressesChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::WalletAddress>& items)
{
    emit addressesChanged(action, items);
    for (const auto& item : items)
    {
        if (item.isOwn())
        {
            if (action == ChangeAction::Removed)
            {
                m_myWalletIds.erase(item.m_walletID);
                m_myAddrLabels.erase(item.m_label);
            }
            else
            {
                m_myWalletIds.emplace(item.m_walletID);
                m_myAddrLabels.emplace(item.m_label);
            }
        }
    }
}

void WalletModel::onAddresses(bool own, const std::vector<hds::wallet::WalletAddress>& addrs)
{
    emit addressesChanged(own, addrs);
}

#ifdef HDS_ATOMIC_SWAP_SUPPORT
void WalletModel::onSwapOffersChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::SwapOffer>& offers)
{
    emit swapOffersChanged(action, offers);
}
#endif  // HDS_ATOMIC_SWAP_SUPPORT

void WalletModel::onCoinsByTx(const std::vector<hds::wallet::Coin>& coins)
{
}

void WalletModel::onAddressChecked(const std::string& addr, bool isValid)
{
    emit addressChecked(QString::fromStdString(addr), isValid);
}

void WalletModel::onImportRecoveryProgress(uint64_t done, uint64_t total)
{
}

void WalletModel::onShowKeyKeeperMessage()
{
#if defined(HDS_HW_WALLET)
    emit showTrezorMessage();
#endif
}

void WalletModel::onHideKeyKeeperMessage()
{
#if defined(HDS_HW_WALLET)
    emit hideTrezorMessage();
#endif
}

void WalletModel::onShowKeyKeeperError(const std::string& error)
{
#if defined(HDS_HW_WALLET)
    emit showTrezorError(QString::fromStdString(error));
#endif
}

void WalletModel::onSwapParamsLoaded(const hds::ByteBuffer& params)
{
    emit swapParamsLoaded(params);
}

void WalletModel::onGeneratedNewAddress(const hds::wallet::WalletAddress& walletAddr)
{
    emit generatedNewAddress(walletAddr);
}

void WalletModel::onNewAddressFailed()
{
    emit newAddressFailed();
}

void WalletModel::onNoDeviceConnected()
{
#if defined(HDS_HW_WALLET)
    //% "There is no Trezor device connected. Please, connect and try again."
    showTrezorError(qtTrId("wallet-model-device-not-connected"));
#endif
}

void WalletModel::onImportDataFromJson(bool isOk)
{
}

void WalletModel::onExportDataToJson(const std::string& data)
{
}

void WalletModel::onExportTxHistoryToCsv(const std::string& data)
{
    emit txHistoryExportedToCsv(QString::fromStdString(data));
}

void WalletModel::onNodeConnectionChanged(bool isNodeConnected)
{
    emit nodeConnectionChanged(isNodeConnected);
}

void WalletModel::onWalletError(hds::wallet::ErrorType error)
{
    emit walletError(error);
}

void WalletModel::FailedToStartWallet()
{
    //% "Failed to start wallet. Please check your wallet data location"
    AppModel::getInstance().getMessages().addMessage(qtTrId("wallet-model-data-location-error"));
}

void WalletModel::onSendMoneyVerified()
{
    emit sendMoneyVerified();
}

void WalletModel::onCantSendToExpired()
{
    emit cantSendToExpired();
}

void WalletModel::onPaymentProofExported(const hds::wallet::TxID& txID, const hds::ByteBuffer& proof)
{
    string str;
    str.resize(proof.size() * 2);

    hds::to_hex(str.data(), proof.data(), proof.size());
    emit paymentProofExported(txID, QString::fromStdString(str));
}

void WalletModel::onPostFunctionToClientContext(MessageFunction&& func)
{
    emit functionPosted(func);
}

void WalletModel::onExchangeRates(const std::vector<hds::wallet::ExchangeRate>& rates)
{
    emit exchangeRatesUpdate(rates);
}

void WalletModel::onNotificationsChanged(hds::wallet::ChangeAction action, const std::vector<Notification>& notifications)
{
    emit notificationsChanged(action, notifications);
}

hds::Version WalletModel::getLibVersion() const
{
    hds::Version ver;
    return ver.from_string(HDS_VERSION) ? ver : hds::Version();
}

uint32_t WalletModel::getClientRevision() const
{
    return VERSION_REVISION;
}

hds::Amount WalletModel::getAvailable() const
{
    return m_status.available;
}

hds::Amount WalletModel::getReceiving() const
{
    return m_status.receiving;
}

hds::Amount WalletModel::getReceivingIncoming() const
{
    return m_status.receivingIncoming;
}

hds::Amount WalletModel::getReceivingChange() const
{
    return m_status.receivingChange;
}

hds::Amount WalletModel::getSending() const
{
    return m_status.sending;
}

hds::Amount WalletModel::getMaturing() const
{
    return m_status.maturing;
}

hds::Height WalletModel::getCurrentHeight() const
{
    return m_status.stateID.m_Height;
}

hds::Height WalletModel::getCurrentHeightTimestamp() const
{
    return m_status.update.lastTime;
}

hds::Block::SystemState::ID WalletModel::getCurrentStateID() const
{
    return m_status.stateID;
}

void WalletModel::setStatus(const hds::wallet::WalletStatus& status)
{
    if (m_status.available != status.available)
    {
        m_status.available = status.available;
        emit availableChanged();
    }

    if (m_status.receiving != status.receiving)
    {
        m_status.receiving = status.receiving;
        emit receivingChanged();
    }

    if (m_status.receivingIncoming != status.receivingIncoming)
    {
        m_status.receivingIncoming = status.receivingIncoming;
        emit receivingIncomingChanged();
    }

    if (m_status.receivingChange != status.receivingChange)
    {
        m_status.receivingChange = status.receivingChange;
        emit receivingChangeChanged();
    }

    if (m_status.sending != status.sending)
    {
        m_status.sending = status.sending;
        emit sendingChanged();
    }

    if (m_status.maturing != status.maturing)
    {
        m_status.maturing = status.maturing;
        emit maturingChanged();
    }

    if (m_status.stateID != status.stateID)
    {
        m_status.stateID = status.stateID;
        m_status.update = status.update;
        emit stateIDChanged();
    }
}

void WalletModel::setAddresses(bool own, const std::vector<hds::wallet::WalletAddress>& addrs)
{
    if (own)
    {
        m_myWalletIds.clear();
        m_myAddrLabels.clear();

        for (const auto& addr : addrs)
        {
            m_myWalletIds.emplace(addr.m_walletID);
            m_myAddrLabels.emplace(addr.m_label);
        }
    }
}

void WalletModel::doFunction(const std::function<void()>& func)
{
    func();
}
