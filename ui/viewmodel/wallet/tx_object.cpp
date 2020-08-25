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
#include "tx_object.h"
#include "viewmodel/ui_helpers.h"
#include "wallet/core/common.h"
#include "wallet/core/simple_transaction.h"
#include "wallet/core/strings_resources.h"
#include "model/app_model.h"

using namespace hds;
using namespace hds::wallet;
using namespace hdsui;

namespace
{
    QString getWaitingPeerStr(const hds::wallet::TxParameters& txParameters, bool isSender)
    {
        auto minHeight = txParameters.GetParameter<hds::Height>(TxParameterID::MinHeight);
        auto responseTime = txParameters.GetParameter<hds::Height>(TxParameterID::PeerResponseTime);
        QString time = "";
        if (minHeight && responseTime)
        {
            time = convertHdsHeightDiffToTime(*minHeight + *responseTime - AppModel::getInstance().getWallet()->getCurrentHeight());
        }
        if (isSender)
        {
            //% "If the receiver won't get online in %1, the transaction will be canceled automatically"
            return qtTrId("tx-state-initial-sender").arg(time);
        }
        //% "If the sender won't get online in %1, the transaction will be canceled automatically"
        return qtTrId("tx-state-initial-receiver").arg(time);
    }

    QString getInProgressStr(const hds::wallet::TxParameters& txParameters)
    {
        const Height kNormalTxConfirmationDelay = 10;
        auto maxHeight = txParameters.GetParameter<hds::Height>(TxParameterID::MaxHeight);
        QString time = "";
        if (!maxHeight)
        {
            return "";
        }

        auto currentHeight = AppModel::getInstance().getWallet()->getCurrentHeight();
        if (currentHeight >= *maxHeight)
        {
            return "";
        }

        Height delta =  *maxHeight - currentHeight;
        auto lifetime = txParameters.GetParameter<hds::Height>(TxParameterID::Lifetime);
        if (!lifetime || *lifetime < delta)
        {
            return "";
        }

        if (*lifetime - delta <= kNormalTxConfirmationDelay)
        {
            //% "The transaction is usually expected to complete in a few minutes."
            return qtTrId("tx-state-in-progress-normal");
        }

        time = hdsui::convertHdsHeightDiffToTime(delta);
        if (time.isEmpty())
        {
            return "";
        }
        //% "It is taking longer than usual. In case the transaction could not be completed it will be canceled automatically in %1."
        return qtTrId("tx-state-in-progress-long").arg(time);
    }
}


TxObject::TxObject( const TxDescription& tx,
                    QObject* parent/* = nullptr*/)
        : TxObject(tx, hds::wallet::ExchangeRate::Currency::Unknown, parent)
{
}

TxObject::TxObject( const TxDescription& tx,
                    hds::wallet::ExchangeRate::Currency secondCurrency,
                    QObject* parent/* = nullptr*/)
        : QObject(parent)
        , m_tx(tx)
        , m_type(*m_tx.GetParameter<TxType>(TxParameterID::TransactionType))
        , m_secondCurrency(secondCurrency)
{
    auto kernelID = QString::fromStdString(to_hex(m_tx.m_kernelID.m_pData, m_tx.m_kernelID.nBytes));
    setKernelID(kernelID);
}

bool TxObject::operator==(const TxObject& other) const
{
    return getTxID() == other.getTxID();
}

auto TxObject::timeCreated() const -> hds::Timestamp
{
    return m_tx.m_createTime;
}

auto TxObject::getTxID() const -> hds::wallet::TxID
{
    return m_tx.m_txId;
}

bool TxObject::isIncome() const
{
    return m_tx.m_sender == false;
}

QString TxObject::getComment() const
{
    std::string str{ m_tx.m_message.begin(), m_tx.m_message.end() };
    return QString(str.c_str()).trimmed();
}

QString TxObject::getAmountWithCurrency() const
{
    return AmountToUIString(m_tx.m_amount, Currencies::Hds);
}

QString TxObject::getAmount() const
{
    return AmountToUIString(m_tx.m_amount);
}

hds::Amount TxObject::getAmountValue() const
{
    return m_tx.m_amount;
}

QString TxObject::getSecondCurrencyRate() const
{
    auto exchangeRatesOptional = getTxDescription().GetParameter<std::vector<ExchangeRate>>(TxParameterID::ExchangeRates);

    if (exchangeRatesOptional)
    {
        std::vector<ExchangeRate>& rates = *exchangeRatesOptional;
        auto secondCurrency = m_secondCurrency;
        auto search = std::find_if(std::begin(rates),
                                   std::end(rates),
                                   [secondCurrency](const ExchangeRate& r)
                                   {
                                       return r.m_currency == ExchangeRate::Currency::Hds
                                           && r.m_unit == secondCurrency;
                                   });
        if (search != std::cend(rates))
        {
            return AmountToUIString(search->m_rate);
        }
    }
    return "0";
}

QString TxObject::getStatus() const
{
    if (m_tx.m_txType == wallet::TxType::Simple)
    {
        SimpleTxStatusInterpreter interpreter(m_tx);
        return interpreter.getStatus().c_str();
    }
    else if (m_tx.m_txType >= wallet::TxType::AssetIssue && m_tx.m_txType <= wallet::TxType::AssetInfo)
        {
        AssetTxStatusInterpreter interpreter(m_tx);
        return interpreter.getStatus().c_str();
        }
    else
        {
        BOOST_ASSERT_MSG(false, kErrorUnknownTxType);
        return "unknown";
    }
}

bool TxObject::isCancelAvailable() const
{
    return m_tx.canCancel();
}

bool TxObject::isDeleteAvailable() const
{
    return m_tx.canDelete();
}

QString TxObject::getAddressFrom() const
{
    return toString(m_tx.m_sender ? m_tx.m_myId : m_tx.m_peerId);
}

QString TxObject::getAddressTo() const
{
    return toString(!m_tx.m_sender ? m_tx.m_myId : m_tx.m_peerId);
}

QString TxObject::getFee() const
{
    if (m_tx.m_fee)
    {
        return AmountInGrothToUIString(m_tx.m_fee);
    }
    return QString{};
}

const hds::wallet::TxDescription& TxObject::getTxDescription() const
{
    return m_tx;
}

void TxObject::setStatus(hds::wallet::TxStatus status)
{
    if (m_tx.m_status != status)
    {
        m_tx.m_status = status;
        emit statusChanged();
    }
}

QString TxObject::getKernelID() const
{
    return m_kernelID;
}

void TxObject::setKernelID(const QString& value)
{
    if (m_kernelID != value)
    {
        m_kernelID = value;
        emit kernelIDChanged();
    }
}

QString TxObject::getTransactionID() const
{
    return QString::fromStdString(to_hex(m_tx.m_txId.data(), m_tx.m_txId.size()));
}

QString TxObject::getReasonString(hds::wallet::TxFailureReason reason) const
{
    // clang doesn't allow to make 'auto reasons' so for the moment assertions below are a bit pointles
    // let's wait until they fix template arg deduction and restore it back
    static const std::array<QString, TxFailureReason::Count> reasons = {
        //% "Unexpected reason, please send wallet logs to Hds support"
        qtTrId("tx-failure-undefined"),
        //% "Transaction cancelled"
        qtTrId("tx-failure-cancelled"),
        //% "Receiver signature in not valid, please send wallet logs to Hds support"
        qtTrId("tx-failure-receiver-signature-invalid"),
        //% "Failed to register transaction with the blockchain, see node logs for details"
        qtTrId("tx-failure-not-registered-in-blockchain"),
        //% "Transaction is not valid, please send wallet logs to Hds support"
        qtTrId("tx-failure-not-valid"),
        //% "Invalid kernel proof provided"
        qtTrId("tx-failure-kernel-invalid"),
        //% "Failed to send Transaction parameters"
        qtTrId("tx-failure-parameters-not-sended"),
        //% "No inputs"
        qtTrId("tx-failure-no-inputs"),
        //% "Address is expired"
        qtTrId("tx-failure-addr-expired"),
        //% "Failed to get transaction parameters"
        qtTrId("tx-failure-parameters-not-readed"),
        //% "Transaction timed out"
        qtTrId("tx-failure-time-out"),
        //% "Payment not signed by the receiver, please send wallet logs to Hds support"
        qtTrId("tx-failure-not-signed-by-receiver"),
        //% "Kernel maximum height is too high"
        qtTrId("tx-failure-max-height-to-high"),
        //% "Transaction has invalid state"
        qtTrId("tx-failure-invalid-state"),
        //% "Subtransaction has failed"
        qtTrId("tx-failure-subtx-failed"),
        //% "Contract's amount is not valid"
        qtTrId("tx-failure-invalid-contract-amount"),
        //% "Side chain has invalid contract"
        qtTrId("tx-failure-invalid-sidechain-contract"),
        //% "Side chain bridge has internal error"
        qtTrId("tx-failure-sidechain-internal-error"),
        //% "Side chain bridge has network error"
        qtTrId("tx-failure-sidechain-network-error"),
        //% "Side chain bridge has response format error"
        qtTrId("tx-failure-invalid-sidechain-response-format"),
        //% "Invalid credentials of Side chain"
        qtTrId("tx-failure-invalid-side-chain-credentials"),
        //% "Not enough time to finish btc lock transaction"
        qtTrId("tx-failure-not-enough-time-btc-lock"),
        //% "Failed to create multi-signature"
        qtTrId("tx-failure-create-multisig"),
        //% "Fee is too small"
        qtTrId("tx-failure-fee-too-small"),
        //% "Fee is too large"
        qtTrId("tx-failure-fee-too-large"),
        //% "Kernel's min height is unacceptable"
        qtTrId("tx-failure-kernel-min-height"),
        //% "Not a loopback transaction"
        qtTrId("tx-failure-loopback"),
        //% "Key keeper is not initialized"
        qtTrId("tx-failure-key-keeper-no-initialized"),
        //% "No valid asset owner id/asset owner idx"
        qtTrId("tx-failure-invalid-asset-id"),
        //% "No asset info or asset info is not valid"
        qtTrId("tx-failure-asset-invalid-info"),
        //% "No asset metadata or asset metadata is not valid"
        qtTrId("tx-failure-asset-invalid-metadata"),
        //% "Invalid asset id"
        qtTrId("tx-failure-asset-invalid-id"),
        //% "Failed to receive asset confirmation"
        qtTrId("tx-failure-asset-confirmation"),
        //% "Asset is still in use (issued amount > 0)"
        qtTrId("tx-failure-asset-in-use"),
        //% "Asset is still locked"
        qtTrId("tx-failure-asset-locked"),
        //% "Asset registration fee is too small"
        qtTrId("tx-failure-asset-small-fee"),
        //% "Cannot issue/consume more than MAX_INT64 asset groth in one transaction"
        qtTrId("tx-failure-invalid-asset-amount"),
        //% "Some mandatory data for payment proof is missing"
        qtTrId("tx-failure-invalid-data-for-payment-proof"),
        //%  "Master key is needed for this transaction, but unavailable"
        qtTrId("tx-failure-there-is-no-master-key"),
        //% "Key keeper malfunctioned"
        qtTrId("tx-failure-keeper-malfunctioned"),
        //% "Aborted by the user"
        qtTrId("tx-failure-aborted-by-user"),
        //% "Asset has been already registered"
        qtTrId("tx-failure-asset-exists"),
        //% "Invalid asset owner id"
        qtTrId("tx-failure-asset-invalid-owner-id"),
        //% "Assets transactions are disabled"
        qtTrId("tx-failure-assets-disabled"),
        //% "You have no vouchers to insert coins to lelantus"
        qtTrId("tx-failure-no-vouchers"),
        //% "Asset transactions are not available until fork2"
        qtTrId("tx-failure-assets-fork2")
    };

    // ensure QString
    static_assert(std::is_same<decltype(reasons)::value_type, QString>::value);
    // ensure that we have all reasons, otherwise it would be runtime crash
    static_assert(std::tuple_size<decltype(reasons)>::value == static_cast<size_t>(TxFailureReason::Count));

    assert(reasons.size() > static_cast<size_t>(reason));
    if (static_cast<size_t>(reason) >= reasons.size())
    {
        LOG_WARNING()  << "Unknown failure reason code " << reason << ". Defaulting to 0";
        reason = TxFailureReason::Unknown;
    }

    return reasons[reason];
}

QString TxObject::getFailureReason() const
{
    // TODO: add support for other transactions
    if (getTxDescription().m_status == wallet::TxStatus::Failed && getTxDescription().m_txType == hds::wallet::TxType::Simple)
    {
        return getReasonString(getTxDescription().m_failureReason);
    }

    return QString();
}

void TxObject::setFailureReason(hds::wallet::TxFailureReason reason)
{
    if (m_tx.m_failureReason != reason)
    {
        m_tx.m_failureReason = reason;
        emit failureReasonChanged();
    }
}

QString TxObject::getStateDetails() const
{
    auto& tx = getTxDescription();
    if (tx.m_txType == hds::wallet::TxType::Simple)
    {
        switch (tx.m_status)
        {
        case hds::wallet::TxStatus::Pending:
        case hds::wallet::TxStatus::InProgress:
        {
            auto state = getTxDescription().GetParameter<wallet::SimpleTransaction::State>(TxParameterID::State);
            if (state)
            {
                switch (*state)
                {
                case wallet::SimpleTransaction::Initial:
                case wallet::SimpleTransaction::Invitation:
                    return getWaitingPeerStr(tx, tx.m_sender);
                default:
                    break;
                }
            }
            return getWaitingPeerStr(tx, tx.m_sender);
        }
        case hds::wallet::TxStatus::Registering:
            return getInProgressStr(tx);
        default:
            break;
        }
    }
    return "";
}

QString TxObject::getToken() const
{
    const auto& tx = getTxDescription();
    return QString::fromStdString(tx.getToken());
}

QString TxObject::getSenderIdentity() const
{
    return QString::fromStdString(m_tx.getSenderIdentity());
}

QString TxObject::getReceiverIdentity() const
{
    return QString::fromStdString(m_tx.getReceiverIdentity());
}

bool TxObject::hasPaymentProof() const
{
    return !isIncome() && m_tx.m_status == wallet::TxStatus::Completed;
}

void TxObject::update(const hds::wallet::TxDescription& tx)
{
    setStatus(tx.m_status);
    auto kernelID = QString::fromStdString(to_hex(tx.m_kernelID.m_pData, tx.m_kernelID.nBytes));
    setKernelID(kernelID);
    setFailureReason(tx.m_failureReason);
}

bool TxObject::isInProgress() const
{
    switch (m_tx.m_status)
    {
        case wallet::TxStatus::Pending:
        case wallet::TxStatus::InProgress:
        case wallet::TxStatus::Registering:
            return true;
        default:
            return false;
    }
}

bool TxObject::isPending() const
{
    return m_tx.m_status == wallet::TxStatus::Pending;
}

bool TxObject::isCompleted() const
{
    return m_tx.m_status == wallet::TxStatus::Completed;
}

bool TxObject::isSelfTx() const
{
    return m_tx.m_selfTx;
}

bool TxObject::isCanceled() const
{
    return m_tx.m_status == wallet::TxStatus::Canceled;
}

bool TxObject::isFailed() const
{
    return m_tx.m_status == wallet::TxStatus::Failed;
}

bool TxObject::isExpired() const
{
    return isFailed() && m_tx.m_failureReason == TxFailureReason::TransactionExpired;
}
