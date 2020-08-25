#pragma once
#include <QObject>
#include "wallet/core/common.h"
#ifdef HDS_ATOMIC_SWAP_SUPPORT
#include "wallet/transactions/swaps/common.h"
#endif  // HDS_ATOMIC_SWAP_SUPPORT
#include "wallet/client/extensions/news_channels/interface.h"

Q_DECLARE_METATYPE(hds::wallet::TxID)
Q_DECLARE_METATYPE(hds::wallet::TxParameters)
Q_DECLARE_METATYPE(ECC::uintBig)

namespace hdsui
{
    // UI labels all for Currencies elements
    constexpr std::string_view currencyHdsLabel =      "HDS";
    constexpr std::string_view currencyBitcoinLabel =   "BTC";
    constexpr std::string_view currencyLitecoinLabel =  "LTC";
    constexpr std::string_view currencyQtumLabel =      "QTUM";
    constexpr std::string_view currencyUsdLabel =       "USD";
    constexpr std::string_view currencyUnknownLabel =   "";

    constexpr std::string_view currencyHdsFeeRateLabel =       "GROTH";
    constexpr std::string_view currencyBitcoinFeeRateLabel =    "sat/kB";
    constexpr std::string_view currencyLitecoinFeeRateLabel =   "ph/kB";
    constexpr std::string_view currencyQtumFeeRateLabel =       "qsat/kB";
    constexpr std::string_view currencyUnknownFeeRateLabel =    "";

    enum class Currencies
    {
        Hds,
        Bitcoin,
        Litecoin,
        Qtum,
        Usd,
        Unknown
    };

    QString toString(Currencies currency);
    std::string toStdString(Currencies currency);

    QString getCurrencyLabel(Currencies);
    QString getCurrencyLabel(hds::wallet::ExchangeRate::Currency);
    QString getFeeRateLabel(Currencies);

    /// Convert amount to ui string with "." as a separator. With the default @coinType, no currency label added.
    QString AmountToUIString(const hds::Amount& value, Currencies coinType = Currencies::Unknown);
    QString AmountInGrothToUIString(const hds::Amount& value);

    /// expects ui string with a "." as a separator
    hds::Amount UIStringToAmount(const QString& value);

    Currencies convertExchangeRateCurrencyToUiCurrency(hds::wallet::ExchangeRate::Currency);
#ifdef HDS_ATOMIC_SWAP_SUPPORT
    Currencies convertSwapCoinToCurrency(hds::wallet::AtomicSwapCoin coin);
#endif

    QString toString(const hds::wallet::WalletID&);
    QString toString(const hds::wallet::PeerID&);
    QString toString(const hds::Merkle::Hash&);
    QString toString(const hds::Timestamp& ts);

    class Filter
    {
    public:
        Filter(size_t size = 12);
        void addSample(double value);
        double getAverage() const;
        double getMedian() const;
    private:
        std::vector<double> _samples;
        size_t _index;
        bool _is_poor;
    };
    QDateTime CalculateExpiresTime(hds::Timestamp currentHeightTime, hds::Height currentHeight, hds::Height expiresHeight);
    QString getEstimateTimeStr(int estimate);
    QString convertHdsHeightDiffToTime(int32_t dt);

    hds::Version getCurrentLibVersion();
    quint32 getCurrentUIRevision();

}  // namespace hdsui
