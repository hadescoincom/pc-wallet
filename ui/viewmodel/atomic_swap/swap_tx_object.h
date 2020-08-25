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

#include "viewmodel/wallet/tx_object.h"
#include "wallet/transactions/swaps/swap_tx_description.h"

class SwapTxObject : public TxObject
{
    // TODO: consider remove inheritance of TxObject
    Q_OBJECT

public:
    SwapTxObject(const hds::wallet::TxDescription& tx, uint32_t minTxConfirmations, double blocksPerHour, QObject* parent = nullptr);
    bool operator==(const SwapTxObject& other) const;

    auto getSentAmountWithCurrency() const -> QString;
    auto getSentAmount() const-> QString;
    auto getSentAmountValue() const -> hds::Amount;
    auto getReceivedAmountWithCurrency() const-> QString;
    auto getReceivedAmount() const -> QString;
    auto getReceivedAmountValue() const -> hds::Amount;
    auto getToken() const -> QString;
    auto getSwapCoinLockTxId() const -> QString;
    auto getSwapCoinLockTxConfirmations() const -> QString;
    auto getSwapCoinRedeemTxId() const -> QString;
    auto getSwapCoinRedeemTxConfirmations() const -> QString;
    auto getSwapCoinRefundTxId() const -> QString;
    auto getSwapCoinRefundTxConfirmations() const -> QString;
    auto getHdsLockTxKernelId() const -> QString;
    auto getHdsRedeemTxKernelId() const -> QString;
    auto getHdsRefundTxKernelId() const -> QString;
    auto getSwapCoinName() const -> QString;
    auto getSwapCoinFeeRate() const -> QString;
    auto getSwapCoinFee() const -> QString;
    auto getFee() const -> QString override;
    auto getFailureReason() const -> QString override;
    QString getStateDetails() const override;
    hds::wallet::AtomicSwapCoin getSwapCoinType() const;
    auto getStatus() const -> QString override;

    bool isLockTxProofReceived() const;
    bool isRefundTxProofReceived() const;
    bool isHdsSideSwap() const;
    
    bool isCancelAvailable() const override;
    bool isDeleteAvailable() const override;
    bool isInProgress() const override;
    bool isPending() const override;
    bool isExpired() const override;
    bool isCompleted() const override;
    bool isCanceled() const override;
    bool isFailed() const override;

signals:

private:
    auto getSwapAmountValue(bool sent) const -> hds::Amount;
    auto getSwapAmountWithCurrency(bool sent) const -> QString;

    hds::wallet::SwapTxDescription m_swapTx;
    uint32_t m_minTxConfirmations = 0;
    double m_blocksPerHour = 0;
};
