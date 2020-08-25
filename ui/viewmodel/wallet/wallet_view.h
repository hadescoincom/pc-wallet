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
#include <QQmlListProperty>
#include <QQueue>
#include <QString>
#include "wallet/transactions/swaps/bridges/bitcoin/client.h"
#include "model/wallet_model.h"
#include "model/settings.h"
#include "viewmodel/messages_view.h"
#include "viewmodel/notifications/exchange_rates_manager.h"
#include "tx_object_list.h"

class WalletViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hdsAvailable                 READ hdsAvailable              NOTIFY hdsAvailableChanged)
    Q_PROPERTY(QString hdsReceiving                 READ hdsReceiving              NOTIFY hdsReceivingChanged)
    Q_PROPERTY(QString hdsSending                   READ hdsSending                NOTIFY hdsSendingChanged)
    Q_PROPERTY(QString hdsLocked                    READ hdsLocked                 NOTIFY hdsLockedChanged)
    Q_PROPERTY(QString hdsLockedMaturing            READ hdsLockedMaturing         NOTIFY hdsLockedChanged)
    Q_PROPERTY(QString hdsReceivingChange           READ hdsReceivingChange        NOTIFY hdsReceivingChanged)
    Q_PROPERTY(QString hdsReceivingIncoming         READ hdsReceivingIncoming      NOTIFY hdsReceivingChanged)
    Q_PROPERTY(QString secondCurrencyLabel           READ getSecondCurrencyLabel     NOTIFY secondCurrencyLabelChanged)
    Q_PROPERTY(QString secondCurrencyRateValue       READ getSecondCurrencyRateValue NOTIFY secondCurrencyRateChanged)
    Q_PROPERTY(bool isAllowedHdsCOMLinks             READ isAllowedHdsCOMLinks       WRITE allowHdsCOMLinks      NOTIFY hdsMWLinksAllowed)
    Q_PROPERTY(QAbstractItemModel* transactions      READ getTransactions            NOTIFY transactionsChanged)

public:
    WalletViewModel();

    QString hdsAvailable() const;
    QString hdsReceiving() const;
    QString hdsSending() const;
    QString hdsLocked() const;
    QString hdsLockedMaturing() const;
    QString hdsReceivingChange() const;
    QString hdsReceivingIncoming() const;

    QString getSecondCurrencyLabel() const;
    QString getSecondCurrencyRateValue() const;

    QAbstractItemModel* getTransactions();
    bool getIsOfflineStatus() const;
    bool getIsFailedStatus() const;
    QString getWalletStatusErrorMsg() const;
    void allowHdsCOMLinks(bool value);

    Q_INVOKABLE void cancelTx(const QVariant& variantTxID);
    Q_INVOKABLE void deleteTx(const QVariant& variantTxID);
    Q_INVOKABLE PaymentInfoItem* getPaymentInfo(const QVariant& variantTxID);
    Q_INVOKABLE bool isAllowedHdsCOMLinks() const;
    Q_INVOKABLE void exportTxHistoryToCsv();

public slots:
    void onTransactionsChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::TxDescription>& items);
    void onTxHistoryExportedToCsv(const QString& data);

signals:
    void hdsAvailableChanged();
    void hdsReceivingChanged();
    void hdsSendingChanged();
    void hdsLockedChanged();

    void secondCurrencyLabelChanged();
    void secondCurrencyRateChanged();

    void transactionsChanged();
    void hdsMWLinksAllowed();

private:
    WalletModel& _model;
    WalletSettings& _settings;
    ExchangeRatesManager _exchangeRatesManager;
    TxObjectList _transactionsList;
    QQueue<QString> _txHistoryToCsvPaths;
};
