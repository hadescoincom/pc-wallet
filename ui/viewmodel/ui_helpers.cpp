#include "ui_helpers.h"
#include <QDateTime>
#include <QLocale>
#include <QTextStream>
#include <numeric>
#include "3rdparty/libbitcoin/include/bitcoin/bitcoin/formats/base_10.hpp"
#include "version.h"

using namespace std;
using namespace hds;

namespace hdsui
{
    QString toString(const hds::wallet::WalletID& walletID)
    {
        if (walletID != Zero)
        {
            auto id = std::to_string(walletID);
            return QString::fromStdString(id);
        }
        return "";
    }

    QString toString(const hds::wallet::PeerID& peerID)
    {
        if (peerID != Zero)
        {
            auto id = std::to_string(peerID);
            return QString::fromStdString(id);
        }
        return "";
    }

    QString toString(const hds::Merkle::Hash& walletID)
    {
        auto id = std::to_string(walletID);
        return QString::fromStdString(id);
    }
    
    QString getCurrencyLabel(Currencies currency)
    {
        switch (currency)
        {
            case Currencies::Hds:
                return QString(currencyHdsLabel.data());

            case Currencies::Bitcoin:
                return QString(currencyBitcoinLabel.data());

            case Currencies::Litecoin:
                return QString(currencyLitecoinLabel.data());

            case Currencies::Qtum:
                return QString(currencyQtumLabel.data());

            case Currencies::Usd:
                return QString(currencyUsdLabel.data());

            case Currencies::Unknown:
            default:
                return QString(currencyUnknownLabel.data());
        }
    }

    QString getCurrencyLabel(hds::wallet::ExchangeRate::Currency currency)
    {
        return getCurrencyLabel(convertExchangeRateCurrencyToUiCurrency(currency));
    }

    QString getFeeRateLabel(Currencies currency)
    {
        switch (currency)
        {
            case Currencies::Hds:
                return QString(currencyHdsFeeRateLabel.data());

            case Currencies::Bitcoin:
                return QString(currencyBitcoinFeeRateLabel.data());

            case Currencies::Litecoin:
                return QString(currencyLitecoinFeeRateLabel.data());

            case Currencies::Qtum:
                return QString(currencyQtumFeeRateLabel.data());

            case Currencies::Usd:
            case Currencies::Unknown:
            default:
                return QString(currencyUnknownFeeRateLabel.data());
        }
    }
    
    /**
     *  Convert amount value to printable format.
     *  @value      Value in coin quants (satoshi, groth and s.o.). 
     *              Unsigned integer with the fixed decimal point.
     *              Decimal point position depends on @coinType.
     *  @coinType   Specify coint type.
     */
    QString AmountToUIString(const Amount& value, Currencies coinType)
    {
        static uint8_t hdsDecimals = static_cast<uint8_t>(std::log10(Rules::Coin));
        std::string amountString;
        switch (coinType)
        {
            case Currencies::Usd:
            case Currencies::Hds:
                amountString = libbitcoin::encode_base10(value, hdsDecimals);
                break;
            default:
                amountString = libbitcoin::satoshi_to_btc(value);
        }

        QString amount = QString::fromStdString(amountString);
        QString coinLabel = getCurrencyLabel(coinType);

        if (coinLabel.isEmpty())
        {
            return amount;
        }
        else
        {
            return amount + " " + coinLabel;
        }
    }

    QString AmountInGrothToUIString(const hds::Amount& value)
    {
        //% "GROTH"
        return QString("%1 %2").arg(value).arg(qtTrId("general-groth"));
    }

    hds::Amount UIStringToAmount(const QString& value)
    {
        hds::Amount amount = 0;
        libbitcoin::btc_to_satoshi(amount, value.toStdString());
        return amount;
    }

    QString toString(const hds::Timestamp& ts)
    {
        QDateTime datetime;
        datetime.setTime_t(ts);

        return datetime.toString(Qt::SystemLocaleShortDate);
    }

#ifdef HDS_ATOMIC_SWAP_SUPPORT
    Currencies convertSwapCoinToCurrency(wallet::AtomicSwapCoin coin)
    {
        switch (coin)
        {
        case wallet::AtomicSwapCoin::Bitcoin:
            return hdsui::Currencies::Bitcoin;
        case wallet::AtomicSwapCoin::Litecoin:
            return hdsui::Currencies::Litecoin;
        case wallet::AtomicSwapCoin::Qtum:
            return hdsui::Currencies::Qtum;
        case wallet::AtomicSwapCoin::Unknown:
        default:
            return hdsui::Currencies::Unknown;
        }
    }
#endif  // HDS_ATOMIC_SWAP_SUPPORT

    Currencies convertExchangeRateCurrencyToUiCurrency(hds::wallet::ExchangeRate::Currency currency)
    {
        switch (currency)
        {
        case wallet::ExchangeRate::Currency::Hds:
            return hdsui::Currencies::Hds;
        case wallet::ExchangeRate::Currency::Bitcoin:
            return hdsui::Currencies::Bitcoin;
        case wallet::ExchangeRate::Currency::Litecoin:
            return hdsui::Currencies::Litecoin;
        case wallet::ExchangeRate::Currency::Qtum:
            return hdsui::Currencies::Qtum;
        case wallet::ExchangeRate::Currency::Usd:
            return hdsui::Currencies::Usd;
        case wallet::ExchangeRate::Currency::Unknown:
        default:
            return hdsui::Currencies::Unknown;
        }
    }

    Filter::Filter(size_t size)
        : _samples(size, 0.0)
        , _index{0}
        , _is_poor{true}
    {
    }
    
    void Filter::addSample(double value)
    {
        _samples[_index] = value;
        _index = (_index + 1) % _samples.size();
        if (_is_poor)
        {
            _is_poor = _index + 1 < _samples.size();
        }
    }

    double Filter::getAverage() const
    {
        double sum = accumulate(_samples.begin(), _samples.end(), 0.0);
        return sum / (_is_poor ? _index : _samples.size());
    }

    double Filter::getMedian() const
    {
        vector<double> temp(_samples.begin(), _samples.end());
        size_t medianPos = (_is_poor ? _index : temp.size()) / 2;
        nth_element(temp.begin(),
                    temp.begin() + medianPos,
                    _is_poor ? temp.begin() + _index : temp.end());
        return temp[medianPos];
    }

    QDateTime CalculateExpiresTime(hds::Timestamp currentHeightTime, hds::Height currentHeight, hds::Height expiresHeight)
    {
        QDateTime expiresTime = QDateTime();

        if (currentHeight <= expiresHeight)
        {
            expiresTime.setTime_t(currentHeightTime + (expiresHeight - currentHeight) * 60);
        }
        else
        {
            expiresTime.setTime_t(currentHeightTime - (currentHeight - expiresHeight) * 60);
        }
        
        return expiresTime;
    }

    QString getEstimateTimeStr(int estimate)
    {
        const int kSecondsInMinute = 60;
        const int kSecondsInHour = 60 * kSecondsInMinute;
        int value = 0;
        QString res;
        QTextStream ss(&res);
        QString units;
        auto writeTime = [&ss](const auto& value, const auto& units)
        { 
            ss << value << " " << units;
        };
        if (estimate >= kSecondsInHour)
        {
            value = estimate / kSecondsInHour;
            //% "h"
            units = qtTrId("loading-view-estimate-hours");
            writeTime(value, units);

            estimate %= kSecondsInHour;
            value = estimate / kSecondsInMinute;

            estimate %= kSecondsInMinute;
            if (estimate)
            {
                ++value;
            }

            if (value >= 1)
            {
                //% "min"
                units = qtTrId("loading-view-estimate-minutes");
                ss << " ";
                writeTime(value, units);
            }

            return res;
        }
        else if (estimate > 100)
        {
            value = estimate / kSecondsInMinute;
            estimate %= kSecondsInMinute;
            if (estimate)
            {
                ++value;
            }
            units = qtTrId("loading-view-estimate-minutes");
        }
        else if (estimate > kSecondsInMinute)
        {
            value = estimate / kSecondsInMinute;
            units = qtTrId("loading-view-estimate-minutes");
            writeTime(value, units);
            value = estimate - kSecondsInMinute;
            //% "sec"
            units = qtTrId("loading-view-estimate-seconds");
            ss << " ";
            writeTime(value, units);
            return res;
        }
        else
        {
            value = estimate > 0 ? estimate : 1;
            units = qtTrId("loading-view-estimate-seconds");
        }
        writeTime(value, units);
        return res;
    }

    QString toString(Currencies currency)
    {
        switch(currency)
        {
            case Currencies::Hds: return "hds";
            case Currencies::Bitcoin: return "btc";
            case Currencies::Litecoin: return "ltc";
            case Currencies::Qtum: return "qtum";
            case Currencies::Usd: return "usd";
            default: return "unknown";
        }
    }

    std::string toStdString(Currencies currency)
    {
        return toString(currency).toStdString();
    }

    QString convertHdsHeightDiffToTime(int32_t dt)
    {
        if (dt <= 0)
        {
            return "";
        }
        const int32_t minute_s = 60;
        const int32_t quantum_s = 5 * minute_s;
        int32_t time_s = dt * hds::Rules().DA.Target_s;
        time_s = (time_s + (quantum_s >> 1)) / quantum_s;
        time_s *= quantum_s;
        return hdsui::getEstimateTimeStr(time_s);
    }

    hds::Version getCurrentLibVersion()
    {
        hds::Version ver;
        return ver.from_string(HDS_VERSION) ? ver : hds::Version();
    }

    quint32 getCurrentUIRevision()
    {
        return VERSION_REVISION;
    }

}  // namespace hdsui
