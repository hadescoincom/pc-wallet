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

#include "swap_tx_object_list.h"

SwapTxObjectList::SwapTxObjectList()
{
}

auto SwapTxObjectList::roleNames() const -> QHash<int, QByteArray>
{
    static const auto roles = QHash<int, QByteArray>
    {
        { static_cast<int>(Roles::TimeCreated), "timeCreated" },
        { static_cast<int>(Roles::TimeCreatedSort), "timeCreatedSort" },
        { static_cast<int>(Roles::AmountGeneralWithCurrency), "amountGeneralWithCurrency" },
        { static_cast<int>(Roles::AmountGeneralWithCurrencySort), "amountGeneralWithCurrencySort" },
        { static_cast<int>(Roles::AmountGeneral), "amountGeneral" },
        { static_cast<int>(Roles::AmountGeneralSort), "amountGeneralSort" },
        { static_cast<int>(Roles::AddressFrom), "addressFrom" },
        { static_cast<int>(Roles::AddressFromSort), "addressFromSort" },
        { static_cast<int>(Roles::AddressTo), "addressTo" },
        { static_cast<int>(Roles::AddressToSort), "addressToSort" },
        { static_cast<int>(Roles::Status), "status" },
        { static_cast<int>(Roles::StatusSort), "statusSort" },
        { static_cast<int>(Roles::Fee), "fee" },
        { static_cast<int>(Roles::Comment), "comment" },
        { static_cast<int>(Roles::TxID), "txID" },
        { static_cast<int>(Roles::KernelID), "kernelID" },
        { static_cast<int>(Roles::FailureReason), "failureReason" },
        { static_cast<int>(Roles::IsCancelAvailable), "isCancelAvailable" },
        { static_cast<int>(Roles::IsDeleteAvailable), "isDeleteAvailable" },
        { static_cast<int>(Roles::IsSelfTransaction), "isSelfTransaction" },
        { static_cast<int>(Roles::IsIncome), "isIncome" },
        { static_cast<int>(Roles::IsInProgress), "isInProgress" },
        { static_cast<int>(Roles::IsPending), "isPending" },
        { static_cast<int>(Roles::IsCompleted), "isCompleted" },
        { static_cast<int>(Roles::IsCanceled), "isCanceled" },
        { static_cast<int>(Roles::IsFailed), "isFailed" },
        { static_cast<int>(Roles::IsExpired), "isExpired" },
        { static_cast<int>(Roles::HasPaymentProof), "hasPaymentProof" },
        { static_cast<int>(Roles::RawTxID), "rawTxID" },
        { static_cast<int>(Roles::Search), "search" },
        // atomic swap only roles
        { static_cast<int>(Roles::IsHdsSideSwap), "isHdsSideSwap" },
        { static_cast<int>(Roles::IsLockTxProofReceived), "isLockTxProofReceived" },
        { static_cast<int>(Roles::IsRefundTxProofReceived), "isRefundTxProofReceived" },
        { static_cast<int>(Roles::AmountSendWithCurrency), "amountSendWithCurrency" },
        { static_cast<int>(Roles::AmountSendWithCurrencySort), "amountSendWithCurrencySort" },
        { static_cast<int>(Roles::AmountSend), "amountSend" },
        { static_cast<int>(Roles::AmountSendSort), "amountSendSort" },
        { static_cast<int>(Roles::AmountReceiveWithCurrency), "amountReceiveWithCurrency" },
        { static_cast<int>(Roles::AmountReceiveWithCurrencySort), "amountReceiveWithCurrencySort" },
        { static_cast<int>(Roles::AmountReceive), "amountReceive" },
        { static_cast<int>(Roles::AmountReceiveSort), "amountReceiveSort" },
        { static_cast<int>(Roles::Token), "token" },
        { static_cast<int>(Roles::SwapCoin), "swapCoin" },
        { static_cast<int>(Roles::SwapCoinFeeRate), "swapCoinFeeRate" },
        { static_cast<int>(Roles::SwapCoinFee), "swapCoinFee" },
        { static_cast<int>(Roles::SwapCoinLockTxId), "swapCoinLockTxId" },
        { static_cast<int>(Roles::SwapCoinLockTxConfirmations), "swapCoinLockTxConfirmations" },
        { static_cast<int>(Roles::SwapCoinRedeemTxId), "swapCoinRedeemTxId" },
        { static_cast<int>(Roles::SwapCoinRedeemTxConfirmations), "swapCoinRedeemTxConfirmations" },
        { static_cast<int>(Roles::SwapCoinRefundTxId), "swapCoinRefundTxId" },
        { static_cast<int>(Roles::SwapCoinRefundTxConfirmations), "swapCoinRefundTxConfirmations" },
        { static_cast<int>(Roles::HdsLockTxKernelId), "hdsLockTxKernelId" },
        { static_cast<int>(Roles::HdsRedeemTxKernelId), "hdsRedeemTxKernelId" },
        { static_cast<int>(Roles::HdsRefundTxKernelId), "hdsRefundTxKernelId" },
        { static_cast<int>(Roles::StateDetails), "stateDetails" }
    };
    return roles;
}

auto SwapTxObjectList::data(const QModelIndex &index, int role) const -> QVariant
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_list.size())
    {
       return QVariant();
    }
    
    auto& value = m_list[index.row()];
    switch (static_cast<Roles>(role))
    {
        case Roles::TimeCreated:
        {
            QDateTime datetime;
            datetime.setTime_t(value->timeCreated());
            return datetime.toString(Qt::SystemLocaleShortDate);
        }
        case Roles::TimeCreatedSort:
        {
            return static_cast<qulonglong>(value->timeCreated());
        }

        case Roles::AmountGeneralWithCurrency:
            return value->getAmountWithCurrency();
        case Roles::AmountGeneralWithCurrencySort:
            return static_cast<qulonglong>(value->getAmountValue());
        case Roles::AmountGeneral:
            return value->getAmount();
        case Roles::AmountGeneralSort:
            return static_cast<qulonglong>(value->getAmountValue());

        case Roles::AddressFrom:
        case Roles::AddressFromSort:
            return value->getAddressFrom();

        case Roles::AddressTo:
        case Roles::AddressToSort:
            return value->getAddressTo();

        case Roles::Status:
        case Roles::StatusSort:
            return value->getStatus();

        case Roles::Fee:
            return value->getFee();

        case Roles::Comment:
            return value->getComment();

        case Roles::TxID:
            return value->getTransactionID();

        case Roles::KernelID:
            return value->getKernelID();

        case Roles::FailureReason:
            return value->getFailureReason();

        case Roles::IsCancelAvailable:
            return value->isCancelAvailable();

        case Roles::IsDeleteAvailable:
            return value->isDeleteAvailable();

        case Roles::IsSelfTransaction:
            return value->isSelfTx();

        case Roles::IsIncome:
            return value->isIncome();

        case Roles::IsInProgress:
            return value->isInProgress();

        case Roles::IsPending:
            return value->isPending();

        case Roles::IsCompleted:
            return value->isCompleted();

        case Roles::IsCanceled:
            return value->isCanceled();

        case Roles::IsFailed:
            return value->isFailed();

        case Roles::IsExpired:
            return value->isExpired();

        case Roles::HasPaymentProof:
            return value->hasPaymentProof();

        case Roles::RawTxID:
            return QVariant::fromValue(value->getTxID());

        case Roles::Search: 
        {
            QString r = value->getTransactionID();
            r.append(" ");
            r.append(value->getKernelID());
            r.append(" ");
            r.append(value->getAddressFrom());
            r.append(" ");
            r.append(value->getAddressTo());
            r.append(" ");
            r.append(value->getComment());
            return r;
        }
            
        // atomic swap only roles

        case Roles::IsHdsSideSwap:
            return value->isHdsSideSwap();

        case Roles::IsLockTxProofReceived:
            return value->isLockTxProofReceived();

        case Roles::IsRefundTxProofReceived:
            return value->isRefundTxProofReceived();
            
        case Roles::AmountSendWithCurrency:
            return value->getSentAmountWithCurrency();
        case Roles::AmountSendWithCurrencySort:
            return value->isHdsSideSwap()
                ? static_cast<qulonglong>(value->getSentAmountValue())
                : static_cast<qulonglong>(value->getReceivedAmountValue());
        case Roles::AmountSend:
            return value->getSentAmount();
        case Roles::AmountSendSort:
            return static_cast<qulonglong>(value->getSentAmountValue());

        case Roles::AmountReceiveWithCurrency:
            return value->getReceivedAmountWithCurrency();
        case Roles::AmountReceiveWithCurrencySort:
            return value->isHdsSideSwap()
                    ? static_cast<qulonglong>(value->getSentAmountValue())
                    : static_cast<qulonglong>(value->getReceivedAmountValue());
        case Roles::AmountReceive:
            return value->getReceivedAmount();
        case Roles::AmountReceiveSort:
            return static_cast<qulonglong>(value->getReceivedAmountValue());

        case Roles::Token:
            return value->getToken();

        case Roles::SwapCoin:
            return value->getSwapCoinName();

        case Roles::SwapCoinFeeRate:
            return value->getSwapCoinFeeRate();

        case Roles::SwapCoinFee:
            return value->getSwapCoinFee();

        case Roles::SwapCoinLockTxId:
            return value->getSwapCoinLockTxId();

        case Roles::SwapCoinLockTxConfirmations:
            return value->getSwapCoinLockTxConfirmations();

        case Roles::SwapCoinRedeemTxId:
            return value->getSwapCoinRedeemTxId();

        case Roles::SwapCoinRedeemTxConfirmations:
            return value->getSwapCoinRedeemTxConfirmations();

        case Roles::SwapCoinRefundTxId:
            return value->getSwapCoinRefundTxId();

        case Roles::SwapCoinRefundTxConfirmations:
            return value->getSwapCoinRefundTxConfirmations();

        case Roles::HdsLockTxKernelId:
            return value->getHdsLockTxKernelId();

        case Roles::HdsRedeemTxKernelId:
            return value->getHdsRedeemTxKernelId();

        case Roles::HdsRefundTxKernelId:
            return value->getHdsRefundTxKernelId();

        case Roles::StateDetails:
            return value->getStateDetails();

        default:
            return QVariant();
    }
}
