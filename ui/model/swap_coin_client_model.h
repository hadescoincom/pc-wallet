// Copyright 2019 The Hds Team
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
#include <QTimer>
#include "wallet/transactions/swaps/bridges/bitcoin/client.h"

class SwapCoinClientModel
    : public QObject
    , public hds::bitcoin::Client
{
    Q_OBJECT
public:
    using Ptr = std::shared_ptr<SwapCoinClientModel>;

    SwapCoinClientModel(hds::bitcoin::IBridgeHolder::Ptr bridgeHolder,
        std::unique_ptr<hds::bitcoin::SettingsProvider> settingsProvider,
        hds::io::Reactor& reactor);

    hds::Amount getAvailable();
    hds::bitcoin::Client::Status getStatus() const;
    bool canModifySettings() const;
    hds::bitcoin::IBridge::ErrorType getConnectionError() const;

signals:
    void gotStatus(hds::bitcoin::Client::Status status);
    void gotBalance(const hds::bitcoin::Client::Balance& balance);
    void gotCanModifySettings(bool canModify);
    void gotConnectionError(const hds::bitcoin::IBridge::ErrorType& error);

    void canModifySettingsChanged();
    void balanceChanged();
    void statusChanged();
    void connectionErrorChanged();

private:
    void OnStatus(Status status) override;
    void OnBalance(const Client::Balance& balance) override;
    void OnCanModifySettingsChanged(bool canModify) override;
    void OnChangedSettings() override;
    void OnConnectionError(hds::bitcoin::IBridge::ErrorType error) override;

private slots:
    void requestBalance();
    void setBalance(const hds::bitcoin::Client::Balance& balance);
    void setStatus(hds::bitcoin::Client::Status status);
    void setCanModifySettings(bool canModify);
    void setConnectionError(hds::bitcoin::IBridge::ErrorType error);

private:
    QTimer m_timer;
    Client::Balance m_balance;
    Status m_status = Status::Unknown;
    bool m_canModifySettings = true;
    hds::bitcoin::IBridge::ErrorType m_connectionError = hds::bitcoin::IBridge::ErrorType::None;
};