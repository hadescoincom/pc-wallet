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

#include "swap_coin_client_model.h"

#include "model/app_model.h"
#include "wallet/core/common.h"
#include "wallet/transactions/swaps/common.h"
#include "wallet/transactions/swaps/bridges/bitcoin/bitcoin_core_017.h"
#include "wallet/transactions/swaps/bridges/bitcoin/settings_provider.h"

using namespace hds;

namespace
{
    const int kUpdateInterval = 10000;
}

SwapCoinClientModel::SwapCoinClientModel(hds::bitcoin::IBridgeHolder::Ptr bridgeHolder,
    std::unique_ptr<hds::bitcoin::SettingsProvider> settingsProvider,
    io::Reactor& reactor)
    : bitcoin::Client(bridgeHolder, std::move(settingsProvider), reactor)
    , m_timer(this)
{
    qRegisterMetaType<hds::bitcoin::Client::Status>("hds::bitcoin::Client::Status");
    qRegisterMetaType<hds::bitcoin::Client::Balance>("hds::bitcoin::Client::Balance");
    qRegisterMetaType<hds::bitcoin::IBridge::ErrorType>("hds::bitcoin::IBridge::ErrorType");

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(requestBalance()));

    // connect to myself for save values in UI(main) thread
    connect(this, SIGNAL(gotBalance(const hds::bitcoin::Client::Balance&)), this, SLOT(setBalance(const hds::bitcoin::Client::Balance&)));
    connect(this, SIGNAL(gotStatus(hds::bitcoin::Client::Status)), this, SLOT(setStatus(hds::bitcoin::Client::Status)));
    connect(this, SIGNAL(gotCanModifySettings(bool)), this, SLOT(setCanModifySettings(bool)));
    connect(this, SIGNAL(gotConnectionError(hds::bitcoin::IBridge::ErrorType)), this, SLOT(setConnectionError(hds::bitcoin::IBridge::ErrorType)));

    requestBalance();

    m_timer.start(kUpdateInterval);

    GetAsync()->GetStatus();
}

hds::Amount SwapCoinClientModel::getAvailable()
{
    return m_balance.m_available;
}

void SwapCoinClientModel::OnStatus(Status status)
{
    emit gotStatus(status);
}

hds::bitcoin::Client::Status SwapCoinClientModel::getStatus() const
{
    return m_status;
}

bool SwapCoinClientModel::canModifySettings() const
{
    return m_canModifySettings;
}

hds::bitcoin::IBridge::ErrorType SwapCoinClientModel::getConnectionError() const
{
    return m_connectionError;
}

void SwapCoinClientModel::OnBalance(const bitcoin::Client::Balance& balance)
{
    emit gotBalance(balance);
}

void SwapCoinClientModel::OnCanModifySettingsChanged(bool canModify)
{
    emit gotCanModifySettings(canModify);
}

void SwapCoinClientModel::OnChangedSettings()
{
    requestBalance();
}

void SwapCoinClientModel::OnConnectionError(hds::bitcoin::IBridge::ErrorType error)
{
    emit gotConnectionError(error);
}

void SwapCoinClientModel::requestBalance()
{
    if (GetSettings().IsActivated())
    {
        // update balance
        GetAsync()->GetBalance();
    }
}

void SwapCoinClientModel::setBalance(const hds::bitcoin::Client::Balance& balance)
{
    if (m_balance != balance)
    {
        m_balance = balance;
        emit balanceChanged();
    }
}

void SwapCoinClientModel::setStatus(hds::bitcoin::Client::Status status)
{
    if (m_status != status)
    {
        m_status = status;
        emit statusChanged();
    }
}

void SwapCoinClientModel::setCanModifySettings(bool canModify)
{
    if (m_canModifySettings != canModify)
    {
        m_canModifySettings = canModify;
        emit canModifySettingsChanged();
    }
}

void SwapCoinClientModel::setConnectionError(hds::bitcoin::IBridge::ErrorType error)
{
    if (m_connectionError != error)
    {
        m_connectionError = error;
        emit connectionErrorChanged();
    }
}
