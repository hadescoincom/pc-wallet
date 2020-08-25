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
#include "model/wallet_model.h"
#include "notifications/exchange_rates_manager.h"

class QR;
class ReceiveViewModel: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString  amountToReceive    READ getAmountToReceive    WRITE  setAmountToReceive  NOTIFY  amountReceiveChanged)
    Q_PROPERTY(int      addressExpires     READ getAddressExpires     WRITE  setAddressExpires   NOTIFY  addressExpiresChanged)
    Q_PROPERTY(QString  addressComment     READ getAddressComment     WRITE  setAddressComment   NOTIFY  addressCommentChanged)
    Q_PROPERTY(QString  receiverAddress    READ getReceiverAddress                               NOTIFY  receiverAddressChanged)
    Q_PROPERTY(QString  receiverAddressQR  READ getReceiverAddressQR                             NOTIFY  receiverAddressChanged)
    Q_PROPERTY(QString  transactionToken   READ getTransactionToken   WRITE  setTranasctionToken NOTIFY  transactionTokenChanged)
    Q_PROPERTY(bool     commentValid       READ getCommentValid                                  NOTIFY  commentValidChanged)
    Q_PROPERTY(bool     hasIdentity        READ getHasIdentity        WRITE  setHasIdentity      NOTIFY  hasIdentityChanged)
    Q_PROPERTY(QString  secondCurrencyLabel         READ getSecondCurrencyLabel                  NOTIFY secondCurrencyLabelChanged)
    Q_PROPERTY(QString  secondCurrencyRateValue     READ getSecondCurrencyRateValue              NOTIFY secondCurrencyRateChanged)

public:
    ReceiveViewModel();
    ~ReceiveViewModel() override;

signals:
    void amountReceiveChanged();
    void addressExpiresChanged();
    void receiverAddressChanged();
    void addressCommentChanged();
    void transactionTokenChanged();
    void newAddressFailed();
    void commentValidChanged();
    void hasIdentityChanged();
    void secondCurrencyLabelChanged();
    void secondCurrencyRateChanged();

public:
    Q_INVOKABLE void generateNewAddress();
    Q_INVOKABLE void saveAddress();

private:
    QString getAmountToReceive() const;
    void    setAmountToReceive(QString value);

    void setAddressExpires(int value);
    int  getAddressExpires() const;

    QString getReceiverAddress() const;
    QString getReceiverAddressQR() const;

    void setAddressComment(const QString& value);
    QString getAddressComment() const;

    void setTranasctionToken(const QString& value);
    QString getTransactionToken() const;

    bool getCommentValid() const;

    bool getHasIdentity() const;
    void setHasIdentity(bool value);

    void updateTransactionToken();

    QString getSecondCurrencyLabel() const;
    QString getSecondCurrencyRateValue() const;

private slots:
    void onGeneratedNewAddress(const hds::wallet::WalletAddress& walletAddr);
    void onReceiverQRChanged();

private:
    hds::Amount _amountToReceiveGrothes;
    int          _addressExpires;
    QString      _addressComment;
    QString      _token;
    hds::wallet::WalletAddress _receiverAddress;

    std::unique_ptr<QR> _qr;
    WalletModel& _walletModel;
    ExchangeRatesManager _exchangeRatesManager;
    hds::wallet::TxParameters _txParameters;
    bool         _hasIdentity;
};
