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

#include <qdebug.h>
#include "model/app_model.h"
#include "model/settings.h"
#include "swap_offers_view.h"
#include "viewmodel/ui_helpers.h"

using namespace hds;
using namespace hds::wallet;
using namespace std;
using namespace hds::bitcoin;

void SwapOffersViewModel::ActiveTxCounters::increment(AtomicSwapCoin swapCoinType)
{
    ++getCounter(swapCoinType);
}

void SwapOffersViewModel::ActiveTxCounters::decrement(AtomicSwapCoin swapCoinType)
{
    --getCounter(swapCoinType);
}

int& SwapOffersViewModel::ActiveTxCounters::getCounter(AtomicSwapCoin swapCoinType)
{
    switch(swapCoinType)
    {
    case wallet::AtomicSwapCoin::Bitcoin:
        return btc;
    case wallet::AtomicSwapCoin::Litecoin:
        return ltc;
    case wallet::AtomicSwapCoin::Qtum:
        return qtum;
    default:
        throw std::runtime_error("Unexpected coin type");
    }
}

void SwapOffersViewModel::ActiveTxCounters::clear()
{
    btc = ltc = qtum = 0;
}

SwapOffersViewModel::SwapOffersViewModel()
    :   m_walletModel{*AppModel::getInstance().getWallet()},
        m_btcClient(AppModel::getInstance().getBitcoinClient()),
        m_ltcClient(AppModel::getInstance().getLitecoinClient()),
        m_qtumClient(AppModel::getInstance().getQtumClient())
{
    connect(&m_walletModel, SIGNAL(availableChanged()), this, SIGNAL(hdsAvailableChanged()));
    connect(&m_walletModel,
            SIGNAL(transactionsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>&)),
            SLOT(onTransactionsDataModelChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::TxDescription>&)));

    connect(&m_walletModel,
            SIGNAL(swapOffersChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::SwapOffer>&)),
            SLOT(onSwapOffersDataModelChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::SwapOffer>&)));

    connect(m_btcClient.get(),  SIGNAL(balanceChanged()), this, SIGNAL(btcAvailableChanged()));
    connect(m_ltcClient.get(), SIGNAL(balanceChanged()), this, SIGNAL(ltcAvailableChanged()));
    connect(m_qtumClient.get(), SIGNAL(balanceChanged()), this, SIGNAL(qtumAvailableChanged()));
    connect(m_btcClient.get(), SIGNAL(statusChanged()), this, SIGNAL(btcOKChanged()));
    connect(m_ltcClient.get(), SIGNAL(statusChanged()), this, SIGNAL(ltcOKChanged()));
    connect(m_qtumClient.get(), SIGNAL(statusChanged()), this, SIGNAL(qtumOKChanged()));

    monitorAllOffersFitBalance();

    m_walletModel.getAsync()->getSwapOffers();
    m_walletModel.getAsync()->getTransactions();
    
    m_minTxConfirmations.emplace(AtomicSwapCoin::Bitcoin, m_btcClient->GetSettings().GetTxMinConfirmations());
    m_minTxConfirmations.emplace(AtomicSwapCoin::Litecoin, m_ltcClient->GetSettings().GetTxMinConfirmations());
    m_minTxConfirmations.emplace(AtomicSwapCoin::Qtum, m_qtumClient->GetSettings().GetTxMinConfirmations());

    m_blocksPerHour.emplace(AtomicSwapCoin::Bitcoin, m_btcClient->GetSettings().GetBlocksPerHour());
    m_blocksPerHour.emplace(AtomicSwapCoin::Litecoin, m_ltcClient->GetSettings().GetBlocksPerHour());
    m_blocksPerHour.emplace(AtomicSwapCoin::Qtum, m_qtumClient->GetSettings().GetBlocksPerHour());
}

QAbstractItemModel* SwapOffersViewModel::getAllOffers()
{
    return &m_offersList;
}

QAbstractItemModel* SwapOffersViewModel::getAllOffersFitBalance()
{
    return &m_offersListFitBalance;
}

QString SwapOffersViewModel::hdsAvailable() const
{
    return hdsui::AmountToUIString(m_walletModel.getAvailable());
}

QString SwapOffersViewModel::btcAvailable() const
{
    return hdsui::AmountToUIString(m_btcClient->getAvailable());
}

QString SwapOffersViewModel::ltcAvailable() const
{
    return hdsui::AmountToUIString(m_ltcClient->getAvailable());
}

QString SwapOffersViewModel::qtumAvailable() const
{
    return hdsui::AmountToUIString(m_qtumClient->getAvailable());
}

bool SwapOffersViewModel::btcOK()  const
{
    return m_btcClient->getStatus() == Client::Status::Connected;
}

bool SwapOffersViewModel::ltcOK()  const
{
    return m_ltcClient->getStatus() == Client::Status::Connected;
}

bool SwapOffersViewModel::qtumOK() const
{
    return m_qtumClient->getStatus() == Client::Status::Connected;
}

bool SwapOffersViewModel::btcConnecting()  const
{
    return m_btcClient->getStatus() == Client::Status::Connecting;
}

bool SwapOffersViewModel::ltcConnecting()  const
{
    return m_ltcClient->getStatus() == Client::Status::Connecting;
}

bool SwapOffersViewModel::qtumConnecting()  const
{
    return m_qtumClient->getStatus() == Client::Status::Connecting;
}

QAbstractItemModel* SwapOffersViewModel::getTransactions()
{
    return &m_transactionsList;
}

void SwapOffersViewModel::cancelOffer(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        LOG_INFO() << txId << " Cancel offer";
        m_walletModel.getAsync()->cancelTx(txId);
    }
}

void SwapOffersViewModel::cancelTx(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        m_walletModel.getAsync()->cancelTx(txId);
    }
}

void SwapOffersViewModel::deleteTx(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        m_walletModel.getAsync()->deleteTx(txId);
    }
}

PaymentInfoItem* SwapOffersViewModel::getPaymentInfo(const QVariant& variantTxID)
{
    if (!variantTxID.isNull() && variantTxID.isValid())
    {
        auto txId = variantTxID.value<hds::wallet::TxID>();
        return new MyPaymentInfoItem(txId, this);
    }
    else return Q_NULLPTR;
}

void SwapOffersViewModel::onTransactionsDataModelChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::TxDescription>& transactions)
{
    vector<shared_ptr<SwapTxObject>> swapTransactions;
    vector<shared_ptr<SwapTxObject>> activeTransactions;
    vector<shared_ptr<SwapTxObject>> inactiveTransactions;
    swapTransactions.reserve(transactions.size());

    for (const auto& t : transactions)
    {
        if (t.GetParameter<TxType>(TxParameterID::TransactionType) == TxType::AtomicSwap)
        {
            auto swapCoinType = t.GetParameter<AtomicSwapCoin>(TxParameterID::AtomicSwapCoin);
            uint32_t minTxConfirmations = swapCoinType ? getTxMinConfirmations(*swapCoinType) : 0;
            double blocksPerHour = swapCoinType ? getBlocksPerHour(*swapCoinType) : 0;
            auto newItem = make_shared<SwapTxObject>(t, minTxConfirmations, blocksPerHour);
            swapTransactions.push_back(newItem);
            if (!newItem->isPending() && newItem->isInProgress())
            {
                activeTransactions.push_back(newItem);
            }
            else
            {
                inactiveTransactions.push_back(newItem);
            }
        }
    }

    if (swapTransactions.empty())
    {
        return;
    }
    
    auto eraseActive = [this](auto tx)
    {
        if (m_activeTx.erase(tx->getTxID()) > 0) // item was erased
        {
            auto swapCoinType = tx->getSwapCoinType();
            m_activeTxCounters.decrement(swapCoinType);
        }
    };

    auto insertActive = [this](auto tx)
    {
        auto swapCoinType = tx->getSwapCoinType();
        auto p = m_activeTx.emplace(tx->getTxID(), swapCoinType);
        if (p.second) // new item was inserted
        {
            m_activeTxCounters.increment(swapCoinType);
        }
    };

    switch (action)
    {
        case ChangeAction::Reset:
            {
                m_transactionsList.reset(swapTransactions);
                m_activeTx.clear();
                m_activeTxCounters.clear();
                for (auto tx : activeTransactions)
                {
                    auto swapCoinType = tx->getSwapCoinType();
                    m_activeTx.emplace(tx->getTxID(), swapCoinType);
                    m_activeTxCounters.increment(swapCoinType);
                }
                break;
            }

        case ChangeAction::Removed:
            {
                m_transactionsList.remove(swapTransactions);
                for (auto tx : swapTransactions)
                {
                    eraseActive(tx);
                }
                break;
            }

        case ChangeAction::Added:
            {
                m_transactionsList.insert(swapTransactions);
                for (auto tx : activeTransactions)
                {
                    insertActive(tx);
                }
                break;
            }
        
        case ChangeAction::Updated:
            {
                m_transactionsList.update(swapTransactions);
                for (auto tx : activeTransactions)
                {
                    insertActive(tx);
                }
                for (auto tx : inactiveTransactions)
                {
                    eraseActive(tx);
                }

                break;
            }

        default:
            assert(false && "Unexpected action");
            break;
    }
    m_activeTxCount = static_cast<int>(m_activeTx.size());
    emit allTransactionsChanged();
}

void SwapOffersViewModel::onSwapOffersDataModelChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::SwapOffer>& offers)
{
    vector<shared_ptr<SwapOfferItem>> modifiedOffers;
    modifiedOffers.reserve(offers.size());

    for (const auto& offer : offers)
    {

        // Offers without publisherID don't pass validation
        auto peerResponseTime = offer.peerResponseHeight();
        auto minHeight = offer.minHeight();
        auto currentHeight = m_walletModel.getCurrentHeight();
        auto currentHeightTimestamp = m_walletModel.getCurrentHeightTimestamp();

        QDateTime timeExpiration;
        if (currentHeight && peerResponseTime && minHeight)
        {
            auto expiresHeight = minHeight + peerResponseTime;
            timeExpiration = hdsui::CalculateExpiresTime(currentHeightTimestamp, currentHeight, expiresHeight);
        }

        modifiedOffers.push_back(make_shared<SwapOfferItem>(offer, timeExpiration));
    }

    switch (action)
    {
        case ChangeAction::Reset:
            {
                m_offersList.reset(modifiedOffers);
                resetAllOffersFitBalance();
                break;
            }

        case ChangeAction::Added:
            {
                m_offersList.insert(modifiedOffers);
                insertAllOffersFitBalance(modifiedOffers);
                break;
            }

        case ChangeAction::Removed:
            {
                for (const auto& modifiedOffer: modifiedOffers)
                {
                    emit offerRemovedFromTable(
                        QVariant::fromValue(modifiedOffer->getTxID()));
                }
                m_offersList.remove(modifiedOffers);
                removeAllOffersFitBalance(modifiedOffers);
                break;
            }
        
        default:
            assert(false && "Unexpected action");
            break;
    }
    
    emit allOffersChanged();
}

void SwapOffersViewModel::resetAllOffersFitBalance()
{
    auto offersCount = m_offersList.rowCount();
    std::vector<std::shared_ptr<SwapOfferItem>> offersListFitBalance;
    offersListFitBalance.reserve(offersCount);

    for(int i = 0; i < offersCount; ++i)
    {
        const auto&it = m_offersList.get(i);
        if (isOfferFitBalance(*it))
            offersListFitBalance.push_back(it);
    }
    m_offersListFitBalance.reset(offersListFitBalance);
    emit allOffersFitBalanceChanged();
}

bool SwapOffersViewModel::showBetaWarning() const
{
    auto& settings = AppModel::getInstance().getSettings();
    bool showWarning = settings.showSwapBetaWarning();
    if (showWarning)
    {
        settings.setShowSwapBetaWarning(false);
    }
    return showWarning;
}

int SwapOffersViewModel::getActiveTxCount() const
{
    return m_activeTxCount;
}

bool SwapOffersViewModel::hasBtcTx() const
{
    return m_activeTxCounters.btc > 0;
}

bool SwapOffersViewModel::hasLtcTx() const
{
    return m_activeTxCounters.ltc > 0;
}

bool SwapOffersViewModel::hasQtumTx() const
{
    return m_activeTxCounters.qtum > 0;
}

void SwapOffersViewModel::monitorAllOffersFitBalance()
{
    connect(this, SIGNAL(hdsAvailableChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(btcAvailableChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(ltcAvailableChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(qtumAvailableChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(btcOKChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(ltcOKChanged()), SLOT(resetAllOffersFitBalance()));
    connect(this, SIGNAL(qtumOKChanged()), SLOT(resetAllOffersFitBalance()));
}

bool SwapOffersViewModel::isOfferFitBalance(const SwapOfferItem& offer)
{
    if (offer.isOwnOffer())
        return true;

    bool isSendHds = offer.isSendHds();

    auto hdsOfferAmount =
        isSendHds ? offer.rawAmountSend() : offer.rawAmountReceive();
    if (hdsOfferAmount > m_walletModel.getAvailable())
        return false;
    
    auto swapCoinOfferAmount =
        isSendHds ? offer.rawAmountReceive() : offer.rawAmountSend();
    auto swapCoinName = offer.getSwapCoinName();

    
    if (swapCoinName == toString(hdsui::Currencies::Bitcoin))
    {
        auto myBtcAmount = btcOK() ? m_btcClient->getAvailable() : 0;
        return isSendHds ? btcOK() : swapCoinOfferAmount <= myBtcAmount;
    } 
    else if (swapCoinName == toString(hdsui::Currencies::Litecoin))
    {
        auto myLtcAmount = ltcOK() ? m_ltcClient->getAvailable() : 0;
        return isSendHds ? ltcOK() : swapCoinOfferAmount <= myLtcAmount;
    }
    else if (swapCoinName == toString(hdsui::Currencies::Qtum))
    {
        auto myQtumAmount = qtumOK() ? m_qtumClient->getAvailable() : 0;
        return isSendHds ? qtumOK() : swapCoinOfferAmount <= myQtumAmount;
    }
    
    return false;
}

void SwapOffersViewModel::insertAllOffersFitBalance(
    const std::vector<std::shared_ptr<SwapOfferItem>>& offers)
{
    std::vector<std::shared_ptr<SwapOfferItem>> fitBalanceOffers;
    fitBalanceOffers.reserve(offers.size());

    std::copy_if(offers.begin(), offers.end(),
                 std::back_inserter(fitBalanceOffers),
                 [this] (const std::shared_ptr<SwapOfferItem>& it)
                 {
                     return isOfferFitBalance(*it);
                 });
    m_offersListFitBalance.insert(fitBalanceOffers);
    emit allOffersFitBalanceChanged();
}

void SwapOffersViewModel::removeAllOffersFitBalance(
    const std::vector<std::shared_ptr<SwapOfferItem>>& offers)
{
    std::vector<std::shared_ptr<SwapOfferItem>> fitBalanceOffers;
    fitBalanceOffers.reserve(offers.size());

    std::copy_if(offers.begin(), offers.end(),
                 std::back_inserter(fitBalanceOffers),
                 [this] (const std::shared_ptr<SwapOfferItem>& it)
                 {
                     return isOfferFitBalance(*it);
                 });
    m_offersListFitBalance.remove(fitBalanceOffers);
    emit allOffersFitBalanceChanged();    
}

bool SwapOffersViewModel::hasActiveTx(const std::string& swapCoin) const
{
    for (int i = 0; i < m_transactionsList.rowCount(); ++i)
    {
        auto index = m_transactionsList.index(i, 0);
        try
        {
            bool isPending = m_transactionsList.data(index, static_cast<int>(SwapTxObjectList::Roles::IsPending)).toBool();
            if (!isPending)
            {
                bool isInProgress = m_transactionsList.data(index, static_cast<int>(SwapTxObjectList::Roles::IsInProgress)).toBool();
                if (isInProgress)
                {
                    auto mySwapCoin = m_transactionsList.data(index, static_cast<int>(SwapTxObjectList::Roles::SwapCoin)).toString().toStdString();
                    if (mySwapCoin == swapCoin)
                    {
                        return true;
                    }
                }
            }
        }
        catch(...)
        {
            qDebug() << "Wrong ROLE data";
        }
    }

    return false;
}

uint32_t SwapOffersViewModel::getTxMinConfirmations(hds::wallet::AtomicSwapCoin swapCoinType)
{
    auto it = m_minTxConfirmations.find(swapCoinType);
    if (it != m_minTxConfirmations.end())
    {
        return it->second;
    }
    return 0;
}

double SwapOffersViewModel::getBlocksPerHour(AtomicSwapCoin swapCoinType)
{
    auto it = m_blocksPerHour.find(swapCoinType);
    if (it != m_blocksPerHour.end())
    {
        return it->second;
    }
    return 0;
}
