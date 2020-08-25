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

#include "wallet_view.h"

#include <iomanip>

#include <QApplication>
#include <QtGui/qimage.h>
#include <QtCore/qbuffer.h>
#include <QFileDialog>
#include <QUrlQuery>
#include <QClipboard>

#include "qrcode/QRCodeGenerator.h"
#include "utility/helpers.h"
#include "model/app_model.h"
#include "model/qr.h"
#include "viewmodel/ui_helpers.h"

using namespace hds;
using namespace hds::wallet;
using namespace std;
using namespace hdsui;

namespace
{
const char kTxHistoryFileNamePrefix[] = "transactions_history_";
const char kTxHistoryFileFormatDesc[] = "Comma-Separated Values (*.csv)";
const char kTxHistoryFileNameFormat[] = "yyyy_MM_dd_HH_mm_ss";
}  // namespace

WalletViewModel::WalletViewModel()
    : _model(*AppModel::getInstance().getWallet())
    , _settings(AppModel::getInstance().getSettings())
{
    connect(&_model, SIGNAL(transactionsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>&)),
        SLOT(onTransactionsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>&)));

    connect(&_model, SIGNAL(availableChanged()), this, SIGNAL(hdsAvailableChanged()));
    connect(&_model, SIGNAL(receivingChanged()), this, SIGNAL(hdsReceivingChanged()));
    connect(&_model, SIGNAL(sendingChanged()), this, SIGNAL(hdsSendingChanged()));
    connect(&_model, SIGNAL(maturingChanged()), this, SIGNAL(hdsLockedChanged()));
    connect(&_model, SIGNAL(receivingChangeChanged()), this, SIGNAL(hdsReceivingChanged()));
    connect(&_model, SIGNAL(receivingIncomingChanged()), this, SIGNAL(hdsReceivingChanged()));
    connect(&_model, SIGNAL(txHistoryExportedToCsv(const QString&)),
            this, SLOT(onTxHistoryExportedToCsv(const QString&)));

    connect(&_exchangeRatesManager, SIGNAL(rateUnitChanged()), SIGNAL(secondCurrencyLabelChanged()));
    connect(&_exchangeRatesManager, SIGNAL(activeRateChanged()), SIGNAL(secondCurrencyRateChanged()));

    _model.getAsync()->getTransactions();
}

QAbstractItemModel* WalletViewModel::getTransactions()
{
    return &_transactionsList;
}

void WalletViewModel::cancelTx(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        _model.getAsync()->cancelTx(txId);
    }
}

void WalletViewModel::deleteTx(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        _model.getAsync()->deleteTx(txId);
    }
}

PaymentInfoItem* WalletViewModel::getPaymentInfo(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        return new MyPaymentInfoItem(txId, this);
    }
    else return Q_NULLPTR;
}

void WalletViewModel::onTransactionsChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::TxDescription>& transactions)
{
    vector<shared_ptr<TxObject>> modifiedTransactions;
    modifiedTransactions.reserve(transactions.size());
    ExchangeRate::Currency secondCurrency = _exchangeRatesManager.getRateUnitRaw();

    for (const auto& t : transactions)
    {
        if(const auto txType = t.GetParameter<TxType>(TxParameterID::TransactionType))
        {
            switch(*txType)
            {
            case TxType::AtomicSwap:
            case TxType::AssetIssue:
            case TxType::AssetConsume:
            case TxType::AssetReg:
            case TxType::AssetUnreg:
            case TxType::AssetInfo:
            case TxType::PushTransaction:
            case TxType::PullTransaction:
            case TxType::VoucherRequest:
            case TxType::VoucherResponse:
                continue;
            case TxType::ALL:
                assert(!"This should not happen");
                continue;
            case TxType::Simple:
                break;
            }

            // Even simple transactions can be on assets, we do not support these in UI at the moment
            if(const auto assetId = t.GetParameter<Asset::ID>(TxParameterID::AssetID))
            {
                if (*assetId != Asset::s_InvalidID)
                {
                    continue;
                }
            }

            modifiedTransactions.push_back(make_shared<TxObject>(t, secondCurrency));
        }
    }

    switch (action)
    {
        case ChangeAction::Reset:
            {
                _transactionsList.reset(modifiedTransactions);
                break;
            }

        case ChangeAction::Removed:
            {
                _transactionsList.remove(modifiedTransactions);
                break;
            }

        case ChangeAction::Added:
            {
                _transactionsList.insert(modifiedTransactions);
                break;
            }
        
        case ChangeAction::Updated:
            {
                _transactionsList.update(modifiedTransactions);
                break;
            }

        default:
            assert(false && "Unexpected action");
            break;
    }

    emit transactionsChanged();
}

void WalletViewModel::onTxHistoryExportedToCsv(const QString& data)
{
    if (!_txHistoryToCsvPaths.isEmpty())
    {
        const auto& path = _txHistoryToCsvPaths.dequeue();
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextCodec *codec = QTextCodec::codecForName("UTF8");
            QTextStream out(&file);
            out.setCodec(codec);
            out << data;
        }
    }
}

QString WalletViewModel::hdsAvailable() const
{
    return hdsui::AmountToUIString(_model.getAvailable());
}

QString WalletViewModel::hdsReceiving() const
{
    return hdsui::AmountToUIString(_model.getReceivingChange() + _model.getReceivingIncoming());
}

QString WalletViewModel::hdsSending() const
{
    return hdsui::AmountToUIString(_model.getSending());
}

QString WalletViewModel::hdsReceivingChange() const
{
     return hdsui::AmountToUIString(_model.getReceivingChange());
}

QString WalletViewModel::hdsReceivingIncoming() const
{
    return hdsui::AmountToUIString(_model.getReceivingIncoming());
}

QString WalletViewModel::hdsLocked() const
{
    return hdsLockedMaturing();
}

QString WalletViewModel::hdsLockedMaturing() const
{
    return hdsui::AmountToUIString(_model.getMaturing());
}

QString WalletViewModel::getSecondCurrencyLabel() const
{
    return hdsui::getCurrencyLabel(_exchangeRatesManager.getRateUnitRaw());
}

QString WalletViewModel::getSecondCurrencyRateValue() const
{
    auto rate = _exchangeRatesManager.getRate(ExchangeRate::Currency::Hds);
    return hdsui::AmountToUIString(rate);
}

bool WalletViewModel::isAllowedHdsCOMLinks() const
{
    return _settings.isAllowedHdsCOMLinks();
}

void WalletViewModel::exportTxHistoryToCsv()
{
    QDateTime now = QDateTime::currentDateTime();
    QString path = QFileDialog::getSaveFileName(
        nullptr,
        //: transactions history screen, export button tooltip and open file dialog
        //% "Export transactions history"
        qtTrId("wallet-export-tx-history"),
        QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
            .filePath(kTxHistoryFileNamePrefix +
                      now.toString(kTxHistoryFileNameFormat)),
        kTxHistoryFileFormatDesc);

    
    if (!path.isEmpty())
    {
        _txHistoryToCsvPaths.enqueue(path);
        _model.getAsync()->exportTxHistoryToCsv();
    }
}

void WalletViewModel::allowHdsCOMLinks(bool value)
{
    _settings.setAllowedHdsCOMLinks(value);
}
