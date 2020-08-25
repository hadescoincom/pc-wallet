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

#include <QApplication>
#include <QtQuick>
#include <QQmlApplicationEngine>

#include <QInputDialog>
#include <QMessageBox>
// uncomment for QML profiling
//#include <QQmlDebuggingEnabler>
//QQmlDebuggingEnabler enabler;
#include <qqmlcontext.h>
#include "viewmodel/start_view.h"
#include "viewmodel/loading_view.h"
#include "viewmodel/main_view.h"
#include "viewmodel/utxo/utxo_view.h"
#include "viewmodel/utxo/utxo_view_status.h"
#include "viewmodel/utxo/utxo_view_type.h"
#include "viewmodel/atomic_swap/swap_offers_view.h"
#include "viewmodel/address_book_view.h"
#include "viewmodel/wallet/wallet_view.h"
#include "viewmodel/help_view.h"
#include "viewmodel/settings_view.h"
#include "viewmodel/messages_view.h"
#include "viewmodel/statusbar_view.h"
#include "viewmodel/theme.h"
#include "viewmodel/receive_view.h"
#include "viewmodel/receive_swap_view.h"
#include "viewmodel/send_view.h"
#include "viewmodel/send_swap_view.h"
#include "viewmodel/el_seed_validator.h"
#include "viewmodel/currencies.h"
#include "model/app_model.h"
#include "viewmodel/qml_globals.h"
#include "viewmodel/helpers/list_model.h"
#include "viewmodel/helpers/sortfilterproxymodel.h"
#include "viewmodel/helpers/token_bootstrap_manager.h"
#include "viewmodel/notifications/notifications_view.h"
#include "viewmodel/notifications/push_notification_manager.h"
#include "viewmodel/notifications/exchange_rates_manager.h"
#include "wallet/core/wallet_db.h"
#include "utility/log_rotation.h"
#include "core/ecc_native.h"
#include "utility/cli/options.h"
#include <QtCore/QtPlugin>
#include "version.h"
#include "utility/string_helpers.h"
#include "utility/helpers.h"
#include "model/translator.h"

#if defined(HDS_USE_STATIC)

#if defined Q_OS_WIN
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsPrinterSupportPlugin)
#elif defined Q_OS_MAC
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
Q_IMPORT_PLUGIN(QCocoaPrinterSupportPlugin)
#elif defined Q_OS_LINUX
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin)
Q_IMPORT_PLUGIN(QCupsPrinterSupportPlugin)
#endif

Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickControls1Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2Plugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPlugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPrivatePlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
Q_IMPORT_PLUGIN(QtQuickTemplates2Plugin)


#endif

using namespace hds;
using namespace std;
using namespace ECC;

#ifdef APP_NAME
static const char* AppName = APP_NAME;
#else
static const char* AppName = "Hds Wallet Masternet";
#endif

int main (int argc, char* argv[])
{
#if defined Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    block_sigpipe();

    QApplication app(argc, argv);

	app.setWindowIcon(QIcon(Theme::iconPath()));

    QApplication::setApplicationName(AppName);

    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    try
    {

        // TODO: ugly temporary fix for unused variable, GCC only
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

        auto [options, visibleOptions] = createOptionsDescription(GENERAL_OPTIONS | UI_OPTIONS | WALLET_OPTIONS);

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

        po::variables_map vm;

        try
        {
            vm = getOptions(argc, argv, WalletSettings::WalletCfg, options, true);
        }
        catch (const po::error& e)
        {
            QMessageBox msgBox;
            msgBox.setText(e.what());
            msgBox.exec();
            return -1;
        }

        if (vm.count(cli::VERSION))
        {
            QMessageBox msgBox;
            msgBox.setText(PROJECT_VERSION.c_str());
            msgBox.exec();
            return 0;
        }

        if (vm.count(cli::GIT_COMMIT_HASH))
        {
            QMessageBox msgBox;
            msgBox.setText(GIT_COMMIT_HASH.c_str());
            msgBox.exec();
            return 0;
        }

        if (vm.count(cli::APPDATA_PATH))
        {
            const auto newPath = QString::fromStdString(vm[cli::APPDATA_PATH].as<string>());
            appDataDir.setPath(newPath);
        }

        int logLevel = getLogLevel(cli::LOG_LEVEL, vm, LOG_LEVEL_DEBUG);
        int fileLogLevel = getLogLevel(cli::FILE_LOG_LEVEL, vm, LOG_LEVEL_DEBUG);

        hds::Crash::InstallHandler(appDataDir.filePath(AppName).toStdString().c_str());

#define LOG_FILES_PREFIX "hds_ui_"

        const auto logFilesPath = appDataDir.filePath(WalletSettings::LogsFolder).toStdString();
        auto logger = hds::Logger::create(logLevel, logLevel, fileLogLevel, LOG_FILES_PREFIX, logFilesPath);

        unsigned logCleanupPeriod = vm[cli::LOG_CLEANUP_DAYS].as<uint32_t>() * 24 * 3600;

        clean_old_logfiles(logFilesPath, LOG_FILES_PREFIX, logCleanupPeriod);

        try
        {
            Rules::get().UpdateChecksum();
            LOG_INFO() << "Hds Wallet UI " << PROJECT_VERSION << " (" << BRANCH_NAME << ")";
            LOG_INFO() << "Hds Core " << HDS_VERSION << " (" << HDS_BRANCH_NAME << ")";
            LOG_INFO() << "Rules signature: " << Rules::get().get_SignatureStr();

            // AppModel Model MUST BE created before the UI engine and destroyed after.
            // AppModel serves the UI and UI should be able to access AppModel at any time
            // even while being destroyed. Do not move engine above AppModel
            WalletSettings settings(appDataDir);
            AppModel appModel(settings);
            QQmlApplicationEngine engine;
            Translator translator(settings, engine);
            
            if (settings.getNodeAddress().isEmpty())
            {
                if (vm.count(cli::NODE_ADDR))
                {
                    string nodeAddr = vm[cli::NODE_ADDR].as<string>();
                    settings.setNodeAddress(nodeAddr.c_str());
                }
            }

            qmlRegisterSingletonType<Theme>(
                    "Hds.Wallet", 1, 0, "Theme",
                    [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
                        Q_UNUSED(engine)
                        Q_UNUSED(scriptEngine)
                        return new Theme;
                    });

            qmlRegisterSingletonType<QMLGlobals>(
                    "Hds.Wallet", 1, 0, "HdsGlobals",
                    [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
                        Q_UNUSED(engine)
                        Q_UNUSED(scriptEngine)
                        return new QMLGlobals(*engine);
                    });

            qRegisterMetaType<Currency>("Currency");
            qmlRegisterUncreatableType<WalletCurrency>("Hds.Wallet", 1, 0, "Currency", "Not creatable as it is an enum type.");
            qmlRegisterType<StartViewModel>("Hds.Wallet", 1, 0, "StartViewModel");
            qmlRegisterType<LoadingViewModel>("Hds.Wallet", 1, 0, "LoadingViewModel");
            qmlRegisterType<MainViewModel>("Hds.Wallet", 1, 0, "MainViewModel");
            qmlRegisterType<WalletViewModel>("Hds.Wallet", 1, 0, "WalletViewModel");
            qmlRegisterUncreatableType<UtxoViewStatus>("Hds.Wallet", 1, 0, "UtxoStatus", "Not creatable as it is an enum type.");
            qmlRegisterUncreatableType<UtxoViewType>("Hds.Wallet", 1, 0, "UtxoType", "Not creatable as it is an enum type.");
            qmlRegisterType<UtxoViewModel>("Hds.Wallet", 1, 0, "UtxoViewModel");
            qmlRegisterType<SettingsViewModel>("Hds.Wallet", 1, 0, "SettingsViewModel");
            qmlRegisterType<AddressBookViewModel>("Hds.Wallet", 1, 0, "AddressBookViewModel");
            qmlRegisterType<SwapOffersViewModel>("Hds.Wallet", 1, 0, "SwapOffersViewModel");
            qmlRegisterType<NotificationsViewModel>("Hds.Wallet", 1, 0, "NotificationsViewModel");
            qmlRegisterType<HelpViewModel>("Hds.Wallet", 1, 0, "HelpViewModel");
            qmlRegisterType<MessagesViewModel>("Hds.Wallet", 1, 0, "MessagesViewModel");
            qmlRegisterType<StatusbarViewModel>("Hds.Wallet", 1, 0, "StatusbarViewModel");
            qmlRegisterType<ReceiveViewModel>("Hds.Wallet", 1, 0, "ReceiveViewModel");
            qmlRegisterType<ReceiveSwapViewModel>("Hds.Wallet", 1, 0, "ReceiveSwapViewModel");
            qmlRegisterType<SendViewModel>("Hds.Wallet", 1, 0, "SendViewModel");
            qmlRegisterType<SendSwapViewModel>("Hds.Wallet", 1, 0, "SendSwapViewModel");
            qmlRegisterType<ELSeedValidator>("Hds.Wallet", 1, 0, "ELSeedValidator");

            qmlRegisterType<AddressItem>("Hds.Wallet", 1, 0, "AddressItem");
            qmlRegisterType<ContactItem>("Hds.Wallet", 1, 0, "ContactItem");
            qmlRegisterType<UtxoItem>("Hds.Wallet", 1, 0, "UtxoItem");
            qmlRegisterType<PaymentInfoItem>("Hds.Wallet", 1, 0, "PaymentInfoItem");
            qmlRegisterType<WalletDBPathItem>("Hds.Wallet", 1, 0, "WalletDBPathItem");
            qmlRegisterType<SwapOfferItem>("Hds.Wallet", 1, 0, "SwapOfferItem");
            qmlRegisterType<SwapOffersList>("Hds.Wallet", 1, 0, "SwapOffersList");
            qmlRegisterType<SwapTxObjectList>("Hds.Wallet", 1, 0, "SwapTxObjectList");
            qmlRegisterType<TxObjectList>("Hds.Wallet", 1, 0, "TxObjectList");
            
            qmlRegisterType<TokenBootstrapManager>("Hds.Wallet", 1, 0, "TokenBootstrapManager");
            qmlRegisterType<PushNotificationManager>("Hds.Wallet", 1, 0, "PushNotificationManager");
            qmlRegisterType<ExchangeRatesManager>("Hds.Wallet", 1, 0, "ExchangeRatesManager");
            
            qmlRegisterType<SortFilterProxyModel>("Hds.Wallet", 1, 0, "SortFilterProxyModel");

            engine.load(QUrl("qrc:/root.qml"));

            if (engine.rootObjects().count() < 1)
            {
                LOG_ERROR() << "Probmlem with QT";
                return -1;
            }

            QObject* topLevel = engine.rootObjects().value(0);
            QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);

            if (!window)
            {
                LOG_ERROR() << "Probmlem with QT";
                return -1;
            }

            //window->setMinimumSize(QSize(768, 540));
            window->setFlag(Qt::WindowFullscreenButtonHint);
            window->show();

            return app.exec();
        }
        catch (const po::error& e)
        {
            LOG_ERROR() << e.what();
            return -1;
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
        return -1;
    }
}
