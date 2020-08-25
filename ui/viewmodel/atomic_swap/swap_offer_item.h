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
#include <QDateTime>
#include "model/wallet_model.h"
#include "viewmodel/ui_helpers.h"

using namespace hds::wallet;

class SwapOfferItem : public QObject
{
    Q_OBJECT

public:
    SwapOfferItem() = default;
    SwapOfferItem(const SwapOffer& offer, const QDateTime& timeExpiration);
    bool operator==(const SwapOfferItem& other) const;

    auto timeCreated() const -> QDateTime;
    auto timeExpiration() const -> QDateTime;
    auto amountSend() const -> QString;
    auto amountReceive() const -> QString;
    auto rate() const -> QString;
    auto isOwnOffer() const -> bool;
    auto isSendHds() const -> bool;

    auto rawAmountSend() const -> hds::Amount;
    auto rawAmountReceive() const -> hds::Amount;

    auto getTxParameters() const -> TxParameters;
    auto getTxID() const -> TxID;
    auto getSwapCoinName() const -> QString;

signals:

private:
    auto getSwapCoinType() const -> hdsui::Currencies;

    hds::wallet::SwapOffer m_offer;          /// TxParameters subclass
    bool m_isHdsSide;          /// pay hds to receive other coin
    QDateTime m_timeExpiration;
};
