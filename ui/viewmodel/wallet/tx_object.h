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
#pragma once

#include <QObject>
#include <QDateTime>
#include "viewmodel/payment_item.h"
#include "viewmodel/ui_helpers.h"
#include "wallet/client/extensions/news_channels/interface.h"

class TxObject : public QObject
{
    Q_OBJECT

public:
    TxObject(const hds::wallet::TxDescription& tx,
             QObject* parent = nullptr);
    TxObject(const hds::wallet::TxDescription& tx,
             hds::wallet::ExchangeRate::Currency secondCurrency,
             QObject* parent = nullptr);
    bool operator==(const TxObject& other) const;

    hds::Timestamp timeCreated() const;
    hds::wallet::TxID getTxID() const;
    QString getAmountWithCurrency() const;
    QString getAmount() const;
    hds::Amount getAmountValue() const;
    QString getSecondCurrencyRate() const;
    QString getComment() const;
    QString getAddressFrom() const;
    QString getAddressTo() const;
    virtual QString getFee() const;
    QString getKernelID() const;
    QString getTransactionID() const ;
    bool hasPaymentProof() const;
    virtual QString getStatus() const;
    virtual QString getFailureReason() const;
    virtual QString getStateDetails() const;
    QString getToken() const;
    QString getSenderIdentity() const;
    QString getReceiverIdentity() const;

    bool isIncome() const;
    bool isSelfTx() const;
    virtual bool isCancelAvailable() const;
    virtual bool isDeleteAvailable() const;
    virtual bool isInProgress() const;
    virtual bool isPending() const;
    virtual bool isExpired() const;
    virtual bool isCompleted() const;
    virtual bool isCanceled() const;
    virtual bool isFailed() const;

    void setKernelID(const QString& value);
    void setStatus(hds::wallet::TxStatus status);
    void setFailureReason(hds::wallet::TxFailureReason reason);
    void update(const hds::wallet::TxDescription& tx);

signals:
    void statusChanged();
    void kernelIDChanged();
    void failureReasonChanged();

protected:
    const hds::wallet::TxDescription& getTxDescription() const;
    QString getReasonString(hds::wallet::TxFailureReason reason) const;
    QString getIdentity(bool isSender) const;
 
    hds::wallet::TxDescription m_tx;
    QString m_kernelID;
    hds::wallet::TxType m_type;
    hds::wallet::ExchangeRate::Currency m_secondCurrency;
};
