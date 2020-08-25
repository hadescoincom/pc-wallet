// Copyright 2020 The Hds Team
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

#include "push_notification_manager.h"

#include "viewmodel/ui_helpers.h"

PushNotificationManager::PushNotificationManager()
    : m_walletModel(*AppModel::getInstance().getWallet())
{
    connect(&m_walletModel,
            SIGNAL(notificationsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Notification>&)),
            SLOT(onNotificationsChanged(hds::wallet::ChangeAction, const std::vector<hds::wallet::Notification>&)));

    m_walletModel.getAsync()->getNotifications();
}

// TODO(sergey.zavarza): deprecated 
void PushNotificationManager::onNewSoftwareUpdateAvailable(
    const hds::wallet::VersionInfo& info, const ECC::uintBig& notificationID, bool showPopup)
{
    if (info.m_application == hds::wallet::VersionInfo::Application::DesktopWallet &&
        hdsui::getCurrentLibVersion() < info.m_version)
    {
        m_hasNewerVersion = true;
        if (showPopup)
        {
            QString newVersion = QString::fromStdString(info.m_version.to_string());
            QString currentVersion = QString::fromStdString(hdsui::getCurrentLibVersion().to_string());
            QVariant id = QVariant::fromValue(notificationID);
        
            emit showUpdateNotification(newVersion, currentVersion, id);
        }
    }
}

void PushNotificationManager::onNewSoftwareUpdateAvailable(
        const hds::wallet::WalletImplVerInfo& info, const ECC::uintBig& notificationID, bool showPopup)
{
    if (info.m_application == hds::wallet::VersionInfo::Application::DesktopWallet &&
        ((hdsui::getCurrentLibVersion() < info.m_version) ||
         (hdsui::getCurrentLibVersion() == info.m_version && hdsui::getCurrentUIRevision() < info.m_UIrevision)))
    {
        m_hasNewerVersion = true;
        if (showPopup)
        {
            QString newVersion = QString::fromStdString(
                info.m_version.to_string() + "." + std::to_string(info.m_UIrevision));
            QString currentVersion = QString::fromStdString(
                hdsui::getCurrentLibVersion().to_string() + "." + std::to_string(hdsui::getCurrentUIRevision()));
            QVariant id = QVariant::fromValue(notificationID);
        
            emit showUpdateNotification(newVersion, currentVersion, id);
        }
    }
}

void PushNotificationManager::onNotificationsChanged(hds::wallet::ChangeAction action, const std::vector<hds::wallet::Notification>& notifications)
{
    if ((m_firstNotification && action == hds::wallet::ChangeAction::Reset)
        || action == hds::wallet::ChangeAction::Added)
    {
        for (const auto& n : notifications)
        {
            // TODO(sergey.zavarza): deprecated 
            if (n.m_type == hds::wallet::Notification::Type::SoftwareUpdateAvailable)
            {
                hds::wallet::VersionInfo info;
                if (hds::wallet::fromByteBuffer(n.m_content, info))
                {
                    onNewSoftwareUpdateAvailable(info, n.m_ID, n.m_state == hds::wallet::Notification::State::Unread);
                }
            }
            if (n.m_type == hds::wallet::Notification::Type::WalletImplUpdateAvailable)
            {
                hds::wallet::WalletImplVerInfo info;
                if (hds::wallet::fromByteBuffer(n.m_content, info))
                {
                    onNewSoftwareUpdateAvailable(info, n.m_ID, n.m_state == hds::wallet::Notification::State::Unread);
                }
            }
        }
        m_firstNotification = false;
    }
}

void PushNotificationManager::onCancelPopup(const QVariant& variantID)
{
    auto id = variantID.value<ECC::uintBig>();
    m_walletModel.getAsync()->markNotificationAsRead(id);
}

bool PushNotificationManager::hasNewerVersion() const
{
    return m_hasNewerVersion;
}