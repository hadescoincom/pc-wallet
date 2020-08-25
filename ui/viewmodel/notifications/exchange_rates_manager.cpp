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

#include "exchange_rates_manager.h"

#include "model/app_model.h"
#include "viewmodel/ui_helpers.h"
#include "viewmodel/qml_globals.h"

// test
#include "utility/logger.h"

using namespace hds::wallet;

ExchangeRatesManager::ExchangeRatesManager()
    : m_walletModel(*AppModel::getInstance().getWallet())
    , m_settings(AppModel::getInstance().getSettings())
{

    qRegisterMetaType<std::vector<hds::wallet::ExchangeRate>>("std::vector<hds::wallet::ExchangeRate>");

    connect(&m_walletModel,
            SIGNAL(exchangeRatesUpdate(const std::vector<hds::wallet::ExchangeRate>&)),
            SLOT(onExchangeRatesUpdate(const std::vector<hds::wallet::ExchangeRate>&)));

    connect(&m_settings,
            SIGNAL(secondCurrencyChanged()),
            SLOT(onRateUnitChanged()));

    m_rateUnit = ExchangeRate::from_string(m_settings.getSecondCurrency().toStdString());
    if (m_rateUnit != ExchangeRate::Currency::Unknown)
    {
        m_walletModel.getAsync()->getExchangeRates();
    }
}

void ExchangeRatesManager::setRateUnit()
{
    auto newCurrency = ExchangeRate::from_string(m_settings.getSecondCurrency().toStdString());

    if (newCurrency == ExchangeRate::Currency::Unknown && m_rateUnit != newCurrency)
    {
        m_walletModel.getAsync()->switchOnOffExchangeRates(false);
    }
    else
    {
        if (m_rateUnit == ExchangeRate::Currency::Unknown)
        {
            m_walletModel.getAsync()->switchOnOffExchangeRates(true);
        }
        if (m_rateUnit != newCurrency)
        {
            m_walletModel.getAsync()->getExchangeRates();
        }
    }
    m_rateUnit = newCurrency;
}

void ExchangeRatesManager::onExchangeRatesUpdate(const std::vector<hds::wallet::ExchangeRate>& rates)
{
    if (m_rateUnit == ExchangeRate::Currency::Unknown) return;  /// Second currency is turned OFF

    bool isActiveRateChanged = false;

    for (const auto& rate : rates)
    {
        {
            PrintableAmount amount(rate.m_rate, true /*show decimal point*/);
            LOG_DEBUG() << "Exchange rate: 1 " << hds::wallet::ExchangeRate::to_string(rate.m_currency) << " = "
                        << amount << " " << hds::wallet::ExchangeRate::to_string(rate.m_unit);
        }

        if (rate.m_unit != m_rateUnit) continue;
        m_rates[rate.m_currency] = rate.m_rate;
        if (!isActiveRateChanged) isActiveRateChanged = true;
    }
    if (isActiveRateChanged) emit activeRateChanged();
}

void ExchangeRatesManager::onRateUnitChanged()
{
    setRateUnit();
    emit rateUnitChanged();
}

hds::wallet::ExchangeRate::Currency ExchangeRatesManager::getRateUnitRaw() const
{
    return m_rateUnit;
}

/**
 *  Get an exchange rate for specific @currency.
 */
hds::Amount ExchangeRatesManager::getRate(ExchangeRate::Currency currency) const
{
    const auto it = m_rates.find(currency);
    return (it == std::cend(m_rates)) ? 0 : it->second;
}

ExchangeRate::Currency ExchangeRatesManager::convertCurrencyToExchangeCurrency(WalletCurrency::Currency uiCurrency)
{
    switch (uiCurrency)
    {
    case Currency::CurrHds:
        return ExchangeRate::Currency::Hds;
    case Currency::CurrBtc:
        return ExchangeRate::Currency::Bitcoin;
    case Currency::CurrLtc:
        return ExchangeRate::Currency::Litecoin;
    case Currency::CurrQtum:
        return ExchangeRate::Currency::Qtum;
    default:
        return ExchangeRate::Currency::Unknown;
    }
}

