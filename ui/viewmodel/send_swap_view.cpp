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
#include "send_swap_view.h"
#include "model/app_model.h"
#include "qml_globals.h"
#include "wallet/transactions/swaps/common.h"
#include "wallet/transactions/swaps/swap_transaction.h"
#include "ui_helpers.h"

#include <algorithm>
#include <regex>

SendSwapViewModel::SendSwapViewModel()
    : _sendAmountGrothes(0)
    , _sendFeeGrothes(0)
    , _sendCurrency(Currency::CurrHds)
    , _receiveAmountGrothes(0)
    , _receiveFeeGrothes(0)
    , _receiveCurrency(Currency::CurrBtc)
    , _changeGrothes(0)
    , _walletModel(*AppModel::getInstance().getWallet())
    , _isHdsSide(true)
{
    connect(&_walletModel, &WalletModel::changeCalculated,  this,  &SendSwapViewModel::onChangeCalculated);
    connect(&_walletModel, &WalletModel::availableChanged, this, &SendSwapViewModel::recalcAvailable);
    connect(&_exchangeRatesManager, SIGNAL(rateUnitChanged()), SIGNAL(secondCurrencyLabelChanged()));
    connect(&_exchangeRatesManager, SIGNAL(activeRateChanged()), SIGNAL(secondCurrencyRateChanged()));
}

QString SendSwapViewModel::getToken() const
{
    return _token;
}

namespace
{
    Currency convertSwapCoinToCurrency(hds::wallet::AtomicSwapCoin  coin)
    {
        switch (coin)
        {
        case hds::wallet::AtomicSwapCoin::Bitcoin:
            return Currency::CurrBtc;
        case hds::wallet::AtomicSwapCoin::Litecoin:
            return Currency::CurrLtc;
        case hds::wallet::AtomicSwapCoin::Qtum:
            return Currency::CurrQtum;
        default:
            return Currency::CurrHds;
        }
    }
}  // namespace

void SendSwapViewModel::fillParameters(const hds::wallet::TxParameters& parameters)
{
    // Set currency before fee, otherwise it would be reset to default fee
    using namespace hds::wallet;
    using namespace hds;

    auto isHdsSide = parameters.GetParameter<bool>(TxParameterID::AtomicSwapIsHdsSide);
    auto swapCoin = parameters.GetParameter<AtomicSwapCoin>(TxParameterID::AtomicSwapCoin);
    auto hdsAmount = parameters.GetParameter<Amount>(TxParameterID::Amount);
    auto swapAmount = parameters.GetParameter<Amount>(TxParameterID::AtomicSwapAmount);
    auto peerID = parameters.GetParameter<WalletID>(TxParameterID::PeerID);
    auto peerResponseTime = parameters.GetParameter<Height>(TxParameterID::PeerResponseTime);
    auto offeredTime = parameters.GetParameter<Timestamp>(TxParameterID::CreateTime);
    auto minHeight = parameters.GetParameter<Height>(TxParameterID::MinHeight);

    if (peerID && swapAmount && hdsAmount && swapCoin && isHdsSide
        && peerResponseTime && offeredTime && minHeight)
    {
        if (*isHdsSide) // other participant is not a hds side
        {
            // Do not set fee, it is set automatically based on the currency param
            setSendCurrency(Currency::CurrHds);
            setSendAmount(hdsui::AmountToUIString(*hdsAmount));
            setReceiveCurrency(convertSwapCoinToCurrency(*swapCoin));
            setReceiveAmount(hdsui::AmountToUIString(*swapAmount));
        }
        else
        {
            // Do not set fee, it is set automatically based on the currency param
            setSendCurrency(convertSwapCoinToCurrency(*swapCoin));
            setSendAmount(hdsui::AmountToUIString(*swapAmount));
            setReceiveCurrency(Currency::CurrHds);
            setReceiveAmount(hdsui::AmountToUIString(*hdsAmount));
        }
        setOfferedTime(QDateTime::fromSecsSinceEpoch(*offeredTime));

        auto currentHeight = _walletModel.getCurrentHeight();
        assert(currentHeight);
        hds::Timestamp currentHeightTime = _walletModel.getCurrentHeightTimestamp();
        auto expiresHeight = *minHeight + *peerResponseTime;
        setExpiresTime(hdsui::CalculateExpiresTime(currentHeightTime, currentHeight, expiresHeight));

        _txParameters = parameters;
        _isHdsSide = *isHdsSide;
    }

    _tokenGeneratebByNewAppVersionMessage.clear();

#ifdef HDS_LIB_VERSION
    if (auto libVersion = parameters.GetParameter(hds::wallet::TxParameterID::LibraryVersion); libVersion)
    {
        std::string libVersionStr;
        hds::wallet::fromByteBuffer(*libVersion, libVersionStr);
        std::string myLibVersionStr = HDS_LIB_VERSION;
        std::regex libVersionRegex("\\d{1,}\\.\\d{1,}\\.\\d{4,}");
        if (std::regex_match(libVersionStr, libVersionRegex) &&
            std::lexicographical_compare(
                myLibVersionStr.begin(),
                myLibVersionStr.end(),
                libVersionStr.begin(),
                libVersionStr.end(),
                std::less<char>{}))
        {
            _tokenGeneratebByNewAppVersionMessage = qtTrId("token-newer-lib")
                .arg(libVersionStr.c_str())
                .arg(myLibVersionStr.c_str());
            emit tokenGeneratebByNewAppVersion();
        }
    }
#endif // HDS_LIB_VERSION

#ifdef HDS_CLIENT_VERSION
    if (auto clientVersion = parameters.GetParameter(hds::wallet::TxParameterID::ClientVersion); clientVersion)
    {
        std::string clientVersionStr;
        hds::wallet::fromByteBuffer(*clientVersion, clientVersionStr);
        std::string myClientVersionStr = HDS_CLIENT_VERSION;

        auto appName = AppModel::getMyName();
        auto res = clientVersionStr.find(appName + " ");
        if (res != std::string::npos)
        {
            clientVersionStr.erase(0, appName.length() + 1);
            std::regex clientVersionRegex("\\d{1,}\\.\\d{1,}\\.\\d{4,}\\.\\d{4,}");
            if (std::regex_match(clientVersionStr, clientVersionRegex) &&
                std::lexicographical_compare(
                    myClientVersionStr.begin(),
                    myClientVersionStr.end(),
                    clientVersionStr.begin(),
                    clientVersionStr.end(),
                    std::less<char>{}))
            {
                _tokenGeneratebByNewAppVersionMessage = qtTrId("token-newer-client")
                    .arg(clientVersionStr.c_str())
                    .arg(myClientVersionStr.c_str());
                emit tokenGeneratebByNewAppVersion();
            }
        }
    }
#endif // HDS_CLIENT_VERSION
}

void SendSwapViewModel::setParameters(const QVariant& parameters)
{
    if (!parameters.isNull() && parameters.isValid())
    {
        auto p = parameters.value<hds::wallet::TxParameters>();
        fillParameters(p);
    }
}

void SendSwapViewModel::setToken(const QString& value)
{
    if (_token != value)
    {
        _token = value;
        auto parameters = hds::wallet::ParseParameters(_token.toStdString());
        if (getTokenValid() && parameters)
        {
            fillParameters(parameters.value());
        }
        emit tokenChanged();
    }
}

bool SendSwapViewModel::getTokenValid() const
{
    return QMLGlobals::isSwapToken(_token);
}

bool SendSwapViewModel::getParametersValid() const
{
    auto type = _txParameters.GetParameter<hds::wallet::TxType>(hds::wallet::TxParameterID::TransactionType);
    return type && *type == hds::wallet::TxType::AtomicSwap;
}

QString SendSwapViewModel::getSendAmount() const
{
    return hdsui::AmountToUIString(_sendAmountGrothes);
}

void SendSwapViewModel::setSendAmount(QString value)
{
    const auto amount = hdsui::UIStringToAmount(value);
    if (amount != _sendAmountGrothes)
    {
        _sendAmountGrothes = amount;
        emit sendAmountChanged();
        emit isSendFeeOKChanged();
        recalcAvailable();
    }
}

unsigned int SendSwapViewModel::getSendFee() const
{
    return _sendFeeGrothes;
}

void SendSwapViewModel::setSendFee(unsigned int value)
{
    if (value != _sendFeeGrothes)
    {
        _sendFeeGrothes = value;
        emit sendFeeChanged();
        emit isSendFeeOKChanged();
        recalcAvailable();
    }
}

Currency SendSwapViewModel::getSendCurrency() const
{
    return _sendCurrency;
}

void SendSwapViewModel::setSendCurrency(Currency value)
{
    assert(value > Currency::CurrStart && value < Currency::CurrEnd);

    if (value != _sendCurrency)
    {
        _sendCurrency = value;
        emit sendCurrencyChanged();
        emit isSendFeeOKChanged();
        recalcAvailable();
    }
}

QString SendSwapViewModel::getReceiveAmount() const
{
    return hdsui::AmountToUIString(_receiveAmountGrothes);
}

void SendSwapViewModel::setReceiveAmount(QString value)
{
    const auto amount = hdsui::UIStringToAmount(value);
    if (amount != _receiveAmountGrothes)
    {
        _receiveAmountGrothes = amount;
        emit receiveAmountChanged();
        emit isReceiveFeeOKChanged();
    }
}

unsigned int SendSwapViewModel::getReceiveFee() const
{
    return _receiveFeeGrothes;
}

void SendSwapViewModel::setReceiveFee(unsigned int value)
{
    if (value != _receiveFeeGrothes)
    {
        _receiveFeeGrothes = value;
        emit receiveFeeChanged();
        emit canSendChanged();
        emit isReceiveFeeOKChanged();
    }
}

Currency SendSwapViewModel::getReceiveCurrency() const
{
    return _receiveCurrency;
}

void SendSwapViewModel::setReceiveCurrency(Currency value)
{
    assert(value > Currency::CurrStart && value < Currency::CurrEnd);

    if (value != _receiveCurrency)
    {
        _receiveCurrency = value;
        emit receiveCurrencyChanged();
        emit isReceiveFeeOKChanged();
    }
}

QString SendSwapViewModel::getComment() const
{
    return _comment;
}

void SendSwapViewModel::setComment(const QString& value)
{
    if (_comment != value)
    {
        _comment = value;
        emit commentChanged();
    }
}

QDateTime SendSwapViewModel::getOfferedTime() const
{
    return _offeredTime;
}

void SendSwapViewModel::setOfferedTime(const QDateTime& value)
{
    if (_offeredTime != value)
    {
        _offeredTime = value;
        emit offeredTimeChanged();
    }
}

QDateTime SendSwapViewModel::getExpiresTime() const
{
    return _expiresTime;
}

void SendSwapViewModel::setExpiresTime(const QDateTime& value)
{
    if (_expiresTime != value)
    {
        _expiresTime = value;
        emit expiresTimeChanged();
    }
}

void SendSwapViewModel::onChangeCalculated(hds::Amount change)
{
    _changeGrothes = change;
    emit enoughChanged();
    emit canSendChanged();
}

bool SendSwapViewModel::isEnough() const
{
    switch(_sendCurrency)
    {
    case Currency::CurrHds:
    {
        const auto total = _sendAmountGrothes + _sendFeeGrothes + _changeGrothes;
        return _walletModel.getAvailable() >= total;
    }
    case Currency::CurrBtc:
    {
        // TODO sentFee is fee rate. should be corrected
        const hds::Amount total = _sendAmountGrothes + _sendFeeGrothes;
        return AppModel::getInstance().getBitcoinClient()->getAvailable() > total;
    }
    case Currency::CurrLtc:
    {
        const hds::Amount total = _sendAmountGrothes + _sendFeeGrothes;
        return AppModel::getInstance().getLitecoinClient()->getAvailable() > total;
    }
    case Currency::CurrQtum:
    {
        const hds::Amount total = _sendAmountGrothes + _sendFeeGrothes;
        return AppModel::getInstance().getQtumClient()->getAvailable() > total;
    }
    default:
    {
        assert(false);
        return true;
    }
    }
}

void SendSwapViewModel::recalcAvailable()
{
    switch(_sendCurrency)
    {
    case Currency::CurrHds:
        _changeGrothes = 0;
        _walletModel.getAsync()->calcChange(_sendAmountGrothes + _sendFeeGrothes);
        return;
    default:
        // TODO:SWAP implement for all currencies
        _changeGrothes = 0;
    }

    emit enoughChanged();
    emit canSendChanged();
}

QString SendSwapViewModel::getReceiverAddress() const
{
    auto peerID = _txParameters.GetParameter<hds::wallet::WalletID>(hds::wallet::TxParameterID::PeerID);
    if (peerID)
    {
        return hdsui::toString(*peerID);
    }
    return _token;
}

bool SendSwapViewModel::canSend() const
{
    // TODO:SWAP check if correct
    return QMLGlobals::isFeeOK(_sendFeeGrothes, _sendCurrency) &&
           _sendCurrency != _receiveCurrency &&
           isEnough() &&
           QDateTime::currentDateTime() < _expiresTime;
}

void SendSwapViewModel::sendMoney()
{
    using hds::wallet::TxParameterID;
    
    auto txParameters = hds::wallet::TxParameters(_txParameters);
    auto hdsFee = _isHdsSide ? getSendFee() : getReceiveFee();
    auto swapFee = _isHdsSide ? getReceiveFee() : getSendFee();

    hds::wallet::FillSwapFee(
        &txParameters,
        hds::Amount(hdsFee),
        hds::Amount(swapFee),
        _isHdsSide);

    if (!_comment.isEmpty())
    {
        std::string localComment = _comment.toStdString();
        txParameters.SetParameter(TxParameterID::Message, hds::ByteBuffer(localComment.begin(), localComment.end()));
    }

    {
        auto txID = txParameters.GetTxID();
        auto swapCoin = txParameters.GetParameter<hds::wallet::AtomicSwapCoin>(TxParameterID::AtomicSwapCoin);
        auto amount = txParameters.GetParameter<hds::Amount>(TxParameterID::Amount);
        auto swapAmount = txParameters.GetParameter<hds::Amount>(TxParameterID::AtomicSwapAmount);
        auto responseHeight = txParameters.GetParameter<hds::Height>(TxParameterID::PeerResponseTime);
        auto minimalHeight = txParameters.GetParameter<hds::Height>(TxParameterID::MinHeight);

        LOG_INFO() << *txID << " Accept offer.\n\t"
                    << "isHdsSide: " << (_isHdsSide ? "true" : "false") << "\n\t"
                    << "swapCoin: " << std::to_string(*swapCoin) << "\n\t"
                    << "amount: " << *amount << "\n\t"
                    << "swapAmount: " << *swapAmount << "\n\t"
                    << "responseHeight: " << *responseHeight << "\n\t"
                    << "minimalHeight: " << *minimalHeight;
    }

    _walletModel.getAsync()->startTransaction(std::move(txParameters));
}

bool SendSwapViewModel::isSendFeeOK() const
{
    return _sendAmountGrothes == 0 || QMLGlobals::isSwapFeeOK(_sendAmountGrothes, _sendFeeGrothes, _sendCurrency);
}

bool SendSwapViewModel::isReceiveFeeOK() const
{
    return _receiveAmountGrothes == 0 || QMLGlobals::isSwapFeeOK(_receiveAmountGrothes, _receiveFeeGrothes, _receiveCurrency);
}

bool SendSwapViewModel::isSendHds() const
{
    return _isHdsSide;
}

QString SendSwapViewModel::getRate() const
{
    hds::Amount otherCoinAmount =
        isSendHds() ? _receiveAmountGrothes : _sendAmountGrothes;
    hds::Amount hdsAmount =
        isSendHds() ? _sendAmountGrothes : _receiveAmountGrothes;

    if (!hdsAmount) return QString();

    return QMLGlobals::divideWithPrecision8(hdsui::AmountToUIString(otherCoinAmount), hdsui::AmountToUIString(hdsAmount));
}

QString SendSwapViewModel::getSecondCurrencySendRateValue() const
{
    auto sendCurrency = ExchangeRatesManager::convertCurrencyToExchangeCurrency(getSendCurrency());
    auto rate = _exchangeRatesManager.getRate(sendCurrency);
    return hdsui::AmountToUIString(rate);
}

QString SendSwapViewModel::getSecondCurrencyReceiveRateValue() const
{
    auto receiveCurrency = ExchangeRatesManager::convertCurrencyToExchangeCurrency(getReceiveCurrency());
    auto rate = _exchangeRatesManager.getRate(receiveCurrency);
    return hdsui::AmountToUIString(rate);
}

QString SendSwapViewModel::getSecondCurrencyLabel() const
{
    return hdsui::getCurrencyLabel(_exchangeRatesManager.getRateUnitRaw());
}

bool SendSwapViewModel::isTokenGeneratedByNewVersion() const
{
    return !_tokenGeneratebByNewAppVersionMessage.isEmpty();
}

QString SendSwapViewModel::tokenGeneratedByNewVersionMessage() const
{
    return _tokenGeneratebByNewAppVersionMessage;
}
