// Copyright 2020 The Hds Team
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

#include "model/wallet_model.h"
#include "model/settings.h"
#include "wallet/client/extensions/news_channels/interface.h"
#include "viewmodel/currencies.h"   // WalletCurrency::Currency enum used in UI

class ExchangeRatesManager : public QObject
{
    Q_OBJECT
public:
    ExchangeRatesManager();

    hds::Amount getRate(hds::wallet::ExchangeRate::Currency) const;
    hds::wallet::ExchangeRate::Currency getRateUnitRaw() const;

    static hds::wallet::ExchangeRate::Currency convertCurrencyToExchangeCurrency(WalletCurrency::Currency uiCurrency);

public slots:
    void onExchangeRatesUpdate(const std::vector<hds::wallet::ExchangeRate>& rates);
    void onRateUnitChanged();

signals:
    void rateUnitChanged();
    void activeRateChanged();

private:
    void setRateUnit();

    WalletModel& m_walletModel;
    WalletSettings& m_settings;

    hds::wallet::ExchangeRate::Currency m_rateUnit;
    std::map<hds::wallet::ExchangeRate::Currency, hds::Amount> m_rates;
};
