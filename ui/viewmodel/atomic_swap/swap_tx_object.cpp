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

#include "swap_tx_object.h"
#include "wallet/transactions/swaps/common.h"
#include "wallet/transactions/swaps/swap_transaction.h"
#include "core/ecc.h"
#include "viewmodel/qml_globals.h"
#include "viewmodel/ui_helpers.h"
#include "model/app_model.h"

#include <qdebug.h>

using namespace hds;
using namespace hds::wallet;

namespace
{
    constexpr uint32_t kSecondsPerHour = 60 * 60;

    QString getWaitingPeerStr(const hds::wallet::SwapTxDescription& tx, Height currentHeight)
    {
        auto minHeight = tx.getMinHeight();
        auto responseTime = tx.getResponseTime();

        QString time = hdsui::convertHdsHeightDiffToTime(minHeight + responseTime - currentHeight);
        //% "If nobody accepts the offer in %1, the offer will be automatically canceled"
        return qtTrId("swap-tx-state-initial").arg(time);
    }

    QString getInProgressNormalStr(const hds::wallet::SwapTxDescription& tx, Height currentHeight)
    {
        if (tx.isRedeemTxRegistered())
        {
            return "";
        }
        
        QString time = "";
        auto minHeightRefund = tx.getMinRefundTxHeight();
        if (minHeightRefund)
        {
            if (currentHeight < *minHeightRefund)
            {
                time = hdsui::convertHdsHeightDiffToTime(*minHeightRefund - currentHeight);
                //% "The swap is expected to complete in %1 at most."
                return qtTrId("swap-tx-state-in-progress-normal").arg(time);
            }
        }
        else {
            auto maxHeightLockTx = tx.getMaxLockTxHeight();
            if (maxHeightLockTx && currentHeight < *maxHeightLockTx)
            {
                time = hdsui::convertHdsHeightDiffToTime(*maxHeightLockTx - currentHeight);
                //% "If the other side will not sign the transaction in %1, the offer will be canceled automatically."
                return qtTrId("swap-tx-state-in-progress-negotiation").arg(time);
            }
        }
        return "";
    }

    QString getInProgressRefundingStr(const hds::wallet::SwapTxDescription& tx, double blocksPerHour, Height currentHdsHeight)
    {
        
        if (tx.isRefundTxRegistered())
        {
            //% "Swap failed, the money is being released back to your wallet"
            return qtTrId("swap-tx-state-refunding");
        }

        QString time;
        QString coin;
        if (tx.isHdsSide())
        {
            auto refundMinHeight = tx.getMinRefundTxHeight();
            if (refundMinHeight && currentHdsHeight < *refundMinHeight)
            {
                time = hdsui::convertHdsHeightDiffToTime(*refundMinHeight - currentHdsHeight);
                coin = "hds";
            }
        }
        else
        {
            auto currentCoinHeight = tx.getExternalHeight();
            auto lockTime = tx.getExternalLockTime();
            if (lockTime && currentCoinHeight && *currentCoinHeight < *lockTime && blocksPerHour)
            {
                double hdsBlocksPerBlock = (kSecondsPerHour / hds::Rules().DA.Target_s) / blocksPerHour;
                double hdsBlocks = (*lockTime - *currentCoinHeight) * hdsBlocksPerBlock;
                time = hdsui::convertHdsHeightDiffToTime(static_cast<int32_t>(std::round(hdsBlocks)));
                coin = hdsui::toString(hdsui::convertSwapCoinToCurrency(tx.getSwapCoin()));
            }
        }
        if (time.isEmpty() || coin.isEmpty())
        {
            return "";
        }

        //% "Swap failed: the refund of your %2 will start in %1. The refund duration depends on the transaction fee you specified for %2."
        return qtTrId("swap-tx-state-in-progress-refunding").arg(time).arg(coin);
    }
}  // namespace

SwapTxObject::SwapTxObject(const TxDescription& tx, uint32_t minTxConfirmations, double blocksPerHour, QObject* parent/* = nullptr*/)
        : TxObject(tx, parent),
          m_swapTx(tx),
          m_minTxConfirmations(minTxConfirmations),
          m_blocksPerHour(blocksPerHour)
{
}

bool SwapTxObject::operator==(const SwapTxObject& other) const
{
    return getTxID() == other.getTxID();
}

auto SwapTxObject::isHdsSideSwap() const -> bool
{
    return m_swapTx.isHdsSide();
}

bool SwapTxObject::isExpired() const
{
    return m_swapTx.isExpired();
}

bool SwapTxObject::isInProgress() const
{
    return  m_tx.m_status == wallet::TxStatus::Pending ||
            m_tx.m_status == wallet::TxStatus::Registering ||
            m_tx.m_status == wallet::TxStatus::InProgress;
}

bool SwapTxObject::isPending() const
{
    return m_tx.m_status == wallet::TxStatus::Pending;
}

bool SwapTxObject::isCompleted() const
{
    return m_tx.m_status == wallet::TxStatus::Completed;
}

bool SwapTxObject::isCanceled() const
{
    return m_tx.m_status == wallet::TxStatus::Canceled;
}

bool SwapTxObject::isFailed() const
{
    return m_swapTx.isFailed();
}

bool SwapTxObject::isCancelAvailable() const
{
    return m_swapTx.isCancelAvailable();
}

bool SwapTxObject::isDeleteAvailable() const
{
    return  m_tx.m_status == wallet::TxStatus::Completed ||
            m_tx.m_status == wallet::TxStatus::Canceled ||
            m_tx.m_status == wallet::TxStatus::Failed;
}

auto SwapTxObject::getSwapCoinName() const -> QString
{
    switch (m_swapTx.getSwapCoin())
    {
        case AtomicSwapCoin::Bitcoin:   return toString(hdsui::Currencies::Bitcoin);
        case AtomicSwapCoin::Litecoin:  return toString(hdsui::Currencies::Litecoin);
        case AtomicSwapCoin::Qtum:      return toString(hdsui::Currencies::Qtum);
        case AtomicSwapCoin::Unknown:   // no break
        default:                        return toString(hdsui::Currencies::Unknown);
    }
}

QString SwapTxObject::getSentAmountWithCurrency() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountWithCurrency(true);
    }
    return m_tx.m_sender ? getAmountWithCurrency() : "";
}

QString SwapTxObject::getSentAmount() const
{
    QString amount = hdsui::AmountToUIString(getSentAmountValue());
    return amount == "0" ? "" : amount;
}

hds::Amount SwapTxObject::getSentAmountValue() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountValue(true);
    }

    return m_tx.m_sender ? m_tx.m_amount : 0;
}

QString SwapTxObject::getReceivedAmountWithCurrency() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountWithCurrency(false);
    }
    return !m_tx.m_sender ? getAmountWithCurrency() : "";
}

QString SwapTxObject::getReceivedAmount() const
{
    QString amount = hdsui::AmountToUIString(getReceivedAmountValue());
    return amount == "0" ? "" : amount;
}

hds::Amount SwapTxObject::getReceivedAmountValue() const
{
    if (m_type == TxType::AtomicSwap)
    {
        return getSwapAmountValue(false);
    }

    return !m_tx.m_sender ? m_tx.m_amount : 0;
}

QString SwapTxObject::getSwapAmountWithCurrency(bool sent) const
{
    bool isHdsSide = m_swapTx.isHdsSide();
    bool s = sent ? !isHdsSide : isHdsSide;
    if (s)
    {
        return AmountToUIString(m_swapTx.getSwapAmount(), hdsui::convertSwapCoinToCurrency(m_swapTx.getSwapCoin()));
    }
    return getAmountWithCurrency();
}

hds::Amount SwapTxObject::getSwapAmountValue(bool sent) const
{
    bool isHdsSide = m_swapTx.isHdsSide();
    bool s = sent ? !isHdsSide : isHdsSide;
    if (s)
    {
        return m_swapTx.getSwapAmount();
    }
    return m_tx.m_amount;
}

QString SwapTxObject::getFee() const
{
    auto fee = m_swapTx.getFee();
    if (fee)
    {
        return hdsui::AmountInGrothToUIString(*fee);
    }
    return QString();
}

QString SwapTxObject::getSwapCoinFeeRate() const
{
    auto feeRate = m_swapTx.getSwapCoinFeeRate();
    if (feeRate)
    {
        QString value = QString::number(*feeRate);
        QString rateMeasure = hdsui::getFeeRateLabel(hdsui::convertSwapCoinToCurrency(m_swapTx.getSwapCoin()));
        return value + " " + rateMeasure;
    }
    return QString();
}

QString SwapTxObject::getSwapCoinFee() const
{
    auto feeRate = m_swapTx.getSwapCoinFeeRate();
    if (!feeRate)
    {
        return QString();
    }

    Currency coinTypeQt;

    switch (m_swapTx.getSwapCoin())
    {
        case AtomicSwapCoin::Bitcoin:   coinTypeQt = Currency::CurrBtc; break;
        case AtomicSwapCoin::Litecoin:  coinTypeQt = Currency::CurrLtc; break;
        case AtomicSwapCoin::Qtum:      coinTypeQt = Currency::CurrQtum; break;
        default:                        coinTypeQt = Currency::CurrStart; break;
    }
    return QMLGlobals::calcTotalFee(coinTypeQt, *feeRate);
}

QString SwapTxObject::getFailureReason() const
{
    if (m_swapTx.isRefunded())
    {
        //% "Refunded"
        return qtTrId("swap-tx-failure-refunded");
    }
    auto failureReason = m_swapTx.getFailureReason();
    return failureReason ? getReasonString(*failureReason) : QString();
}

QString SwapTxObject::getStateDetails() const
{
    if (getTxDescription().m_txType == hds::wallet::TxType::AtomicSwap)
    {
        switch (getTxDescription().m_status)
        {
        case hds::wallet::TxStatus::Pending:
        case hds::wallet::TxStatus::InProgress:
        {
            Height currentHeight = AppModel::getInstance().getWallet()->getCurrentHeight();
            auto state = m_swapTx.getState();
            if (state)
            {
                switch (*state)
                {
                case wallet::AtomicSwapTransaction::State::Initial:
                    return getWaitingPeerStr(m_swapTx, currentHeight);
                case wallet::AtomicSwapTransaction::State::BuildingHdsLockTX:
                case wallet::AtomicSwapTransaction::State::BuildingHdsRefundTX:
                case wallet::AtomicSwapTransaction::State::BuildingHdsRedeemTX:
                case wallet::AtomicSwapTransaction::State::HandlingContractTX:
                case wallet::AtomicSwapTransaction::State::SendingHdsLockTX:
                    return getInProgressNormalStr(m_swapTx, currentHeight);
                case wallet::AtomicSwapTransaction::State::SendingRedeemTX:
                case wallet::AtomicSwapTransaction::State::SendingHdsRedeemTX:
                    return getInProgressNormalStr(m_swapTx, currentHeight);
                case wallet::AtomicSwapTransaction::State::SendingRefundTX:
                case wallet::AtomicSwapTransaction::State::SendingHdsRefundTX:
                    return getInProgressRefundingStr(m_swapTx, m_blocksPerHour, currentHeight);
                default:
                    break;
                }
            }
            else
            {
                return getWaitingPeerStr(m_swapTx, currentHeight);
            }
            break;
        }
        default:
            break;
        }
    }
    return "";
}

hds::wallet::AtomicSwapCoin SwapTxObject::getSwapCoinType() const
{
    return m_swapTx.getSwapCoin();
}

auto SwapTxObject::getStatus() const -> QString
{
    SwapTxStatusInterpreter interpreter(getTxDescription());
    return interpreter.getStatus().c_str();
}

namespace
{
    template<SubTxIndex SubTxId>
    QString getSwapCoinTxId(const SwapTxDescription& swapTxDescription)
    {
        if (auto res = swapTxDescription.getSwapCoinTxId<SubTxId>(); res)
        {
            return QString::fromStdString(*res);
        }
        else return QString();
    }
    
    template<SubTxIndex SubTxId>
    QString getSwapCoinTxConfirmations(const SwapTxDescription& swapTxDescription, uint32_t minTxConfirmations)
    {
        if (auto res = swapTxDescription.getSwapCoinTxConfirmations<SubTxId>(); res)
        {
            std::string result;
            if (minTxConfirmations)
            {
                result = (*res > minTxConfirmations) ? std::to_string(minTxConfirmations) : std::to_string(*res);
                result += "/" + std::to_string(minTxConfirmations);
            }
            else
            {
                result = std::to_string(*res);
            }
            return QString::fromStdString(result);
        }
        return QString();
    }

    template<SubTxIndex SubTxId>
    QString getHdsTxKernelId(const SwapTxDescription& swapTxDescription)
    {
        if (auto res = swapTxDescription.getHdsTxKernelId<SubTxId>(); res)
        {
            return QString::fromStdString(*res);
        }
        return QString();
    }
}

QString SwapTxObject::getToken() const
{
    auto swapToken = m_swapTx.getToken();
    if (swapToken)
    {
        return QString::fromStdString(*swapToken);
    }
    return QString();
}

bool SwapTxObject::isLockTxProofReceived() const
{
    return m_swapTx.isLockTxProofReceived();
}

bool SwapTxObject::isRefundTxProofReceived() const
{
    return m_swapTx.isRefundTxProofReceived();
}

QString SwapTxObject::getSwapCoinLockTxId() const
{
    return getSwapCoinTxId<SubTxIndex::LOCK_TX>(m_swapTx);
}

QString SwapTxObject::getSwapCoinRedeemTxId() const
{
    return getSwapCoinTxId<SubTxIndex::REDEEM_TX>(m_swapTx);
}

QString SwapTxObject::getSwapCoinRefundTxId() const
{
    return getSwapCoinTxId<SubTxIndex::REFUND_TX>(m_swapTx);
}

QString SwapTxObject::getSwapCoinLockTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::LOCK_TX>(m_swapTx, m_minTxConfirmations);
}

QString SwapTxObject::getSwapCoinRedeemTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::REDEEM_TX>(m_swapTx, m_minTxConfirmations);
}

QString SwapTxObject::getSwapCoinRefundTxConfirmations() const
{
    return getSwapCoinTxConfirmations<SubTxIndex::REFUND_TX>(m_swapTx, m_minTxConfirmations);
}

QString SwapTxObject::getHdsLockTxKernelId() const
{
    return getHdsTxKernelId<SubTxIndex::HDS_LOCK_TX>(m_swapTx);
}

QString SwapTxObject::getHdsRedeemTxKernelId() const
{
    return getHdsTxKernelId<SubTxIndex::REDEEM_TX>(m_swapTx);
}

QString SwapTxObject::getHdsRefundTxKernelId() const
{
    return getHdsTxKernelId<SubTxIndex::REFUND_TX>(m_swapTx);
}
