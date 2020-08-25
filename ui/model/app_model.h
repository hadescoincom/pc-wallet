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

#include "wallet_model.h"
#include "swap_coin_client_model.h"
#include "settings.h"
#include "messages.h"
#include "node_model.h"
#include "helpers.h"
#include "wallet/core/secstring.h"
#include "wallet/core/private_key_keeper.h"
#include "wallet/transactions/swaps/bridges/bitcoin/bridge_holder.h"
#include <memory>

#if defined(HDS_HW_WALLET)
namespace hds::wallet
{
    class HWWallet;
}
#endif

class AppModel final: public QObject
{
    Q_OBJECT
public:
    static AppModel& getInstance();
    static std::string getMyName();

    AppModel(WalletSettings& settings);
    ~AppModel() override;

    bool createWallet(const hds::SecString& seed, const hds::SecString& pass);

#if defined(HDS_HW_WALLET)
    bool createTrezorWallet(const hds::SecString& pass, hds::wallet::IPrivateKeyKeeper2::Ptr keyKeeper);
    std::shared_ptr<hds::wallet::HWWallet> getHardwareWalletClient() const;
#endif

    bool openWallet(const hds::SecString& pass, hds::wallet::IPrivateKeyKeeper2::Ptr keyKeeper = {});
    bool checkWalletPassword(const hds::SecString& pass) const;
    void changeWalletPassword(const std::string& pass);

    void applySettingsChanges();
    void nodeSettingsChanged();
    void resetWallet();

    WalletModel::Ptr getWallet() const;
    WalletSettings& getSettings() const;
    MessageManager& getMessages();
    NodeModel& getNode();
    SwapCoinClientModel::Ptr getBitcoinClient() const;
    SwapCoinClientModel::Ptr getLitecoinClient() const;
    SwapCoinClientModel::Ptr getQtumClient() const;

public slots:
    void onStartedNode();
    void onFailedToStartNode(hds::wallet::ErrorType errorCode);
    void onResetWallet();

signals:
    void walletReset();
    void walletResetCompleted();

private:
    void start();
    void startNode();
    void startWallet();
    void InitBtcClient();
    void InitLtcClient();
    void InitQtumClient();
    void onWalledOpened(const hds::SecString& pass);
    void backupDB(const std::string& dbFilePath);
    void restoreDBFromBackup(const std::string& dbFilePath);

private:
    // SwapCoinClientModels must be destroyed after WalletModel
    SwapCoinClientModel::Ptr m_bitcoinClient;
    SwapCoinClientModel::Ptr m_litecoinClient;
    SwapCoinClientModel::Ptr m_qtumClient;

    hds::bitcoin::IBridgeHolder::Ptr m_btcBridgeHolder;
    hds::bitcoin::IBridgeHolder::Ptr m_ltcBridgeHolder;
    hds::bitcoin::IBridgeHolder::Ptr m_qtumBridgeHolder;

    WalletModel::Ptr m_wallet;
    NodeModel m_nodeModel;
    WalletSettings& m_settings;
    MessageManager m_messages;
    ECC::NoLeak<ECC::uintBig> m_passwordHash;
    hds::io::Reactor::Ptr m_walletReactor;
    hds::wallet::IWalletDB::Ptr m_db;
    Connections m_nsc; // [n]ode [s]tarting [c]onnections
    Connections m_walletConnections;
    static AppModel* s_instance;
    std::string m_walletDBBackupPath;

#if defined(HDS_HW_WALLET)
    mutable std::shared_ptr<hds::wallet::HWWallet> m_hwWallet;
#endif
};
