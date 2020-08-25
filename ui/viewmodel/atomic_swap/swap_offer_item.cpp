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

#include "swap_offer_item.h"
#include "utility/helpers.h"
#include "wallet/core/common.h"
#include "viewmodel/ui_helpers.h"
#include "viewmodel/qml_globals.h"

using namespace hds::wallet;

SwapOfferItem::SwapOfferItem(const SwapOffer& offer,
                             const QDateTime& timeExpiration)
    : m_offer{offer}
    , m_isHdsSide{offer.isHdsSide()}
    , m_timeExpiration{timeExpiration} {}

bool SwapOfferItem::operator==(const SwapOfferItem& other) const
{
    return getTxID() == other.getTxID();
}

auto SwapOfferItem::timeCreated() const -> QDateTime
{
    QDateTime datetime;
    datetime.setTime_t(m_offer.timeCreated());
    return datetime;
}

auto SwapOfferItem::timeExpiration() const -> QDateTime
{
    return m_timeExpiration;
}

auto SwapOfferItem::rawAmountSend() const -> hds::Amount
{
    return isSendHds() ? m_offer.amountHds() : m_offer.amountSwapCoin();
}

auto SwapOfferItem::rawAmountReceive() const -> hds::Amount
{
    return isSendHds() ? m_offer.amountSwapCoin() : m_offer.amountHds();
}

auto SwapOfferItem::rate() const -> QString
{
    hds::Amount otherCoinAmount =
        isSendHds() ? rawAmountReceive() : rawAmountSend();
    hds::Amount hdsAmount =
        isSendHds() ? rawAmountSend() : rawAmountReceive();

    if (!hdsAmount) return QString();

    return QMLGlobals::divideWithPrecision8(hdsui::AmountToUIString(otherCoinAmount), hdsui::AmountToUIString(hdsAmount));
}

auto SwapOfferItem::amountSend() const -> QString
{
    auto coinType = isSendHds() ? hdsui::Currencies::Hds : getSwapCoinType();
    return hdsui::AmountToUIString(rawAmountSend(), coinType);
}

auto SwapOfferItem::amountReceive() const -> QString
{
    auto coinType = isSendHds() ? getSwapCoinType() : hdsui::Currencies::Hds;
    return hdsui::AmountToUIString(rawAmountReceive(), coinType);
}

auto SwapOfferItem::isOwnOffer() const -> bool
{
    return m_offer.m_isOwn;
}

auto SwapOfferItem::isSendHds() const -> bool
{
    return m_offer.m_isOwn ? !m_isHdsSide : m_isHdsSide;
}

auto SwapOfferItem::getTxParameters() const -> hds::wallet::TxParameters
{
    return m_offer;
}

auto SwapOfferItem::getTxID() const -> TxID
{
    return m_offer.m_txId;    
}

auto SwapOfferItem::getSwapCoinType() const -> hdsui::Currencies
{
    return hdsui::convertSwapCoinToCurrency(m_offer.swapCoinType());
}

auto SwapOfferItem::getSwapCoinName() const -> QString
{
    return toString(getSwapCoinType());
}
