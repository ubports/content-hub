/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Ken VanDine <ken.vandine@canonical.com>
 */

#include <QCoreApplication>
#include <QProcessEnvironment>
#include <csignal>
#include <com/ubuntu/content/item.h>

#include "detail/app_manager.h"
#include "debug.h"
#include "common.h"
#include "registry.h"
#include "registry-updater.h"
#include "detail/i18n.h"
#include "detail/service.h"
#include "detail/peer_registry.h"
#include "serviceadaptor.h"

namespace cuca = com::ubuntu::ApplicationManager;
namespace cucd = com::ubuntu::content::detail;
namespace cuc = com::ubuntu::content;

namespace {
    void shutdown(int sig)
    {
        TRACE() << Q_FUNC_INFO << sig;
        QCoreApplication::instance()->quit();
    }
}

int main(int argc, char** argv)
{
    int ret = 0;
    QCoreApplication *app = new QCoreApplication(argc, argv);

    cucd::initTr(I18N_DOMAIN, NULL);

    /* read environment variables */
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    if (environment.contains(QLatin1String("CONTENT_HUB_LOGGING_LEVEL"))) {
        bool isOk;
        int value = environment.value(
            QLatin1String("CONTENT_HUB_LOGGING_LEVEL")).toInt(&isOk);
        if (isOk)
            setLoggingLevel(value);
    }

    auto connection = QDBusConnection::sessionBus();

    auto registry = QSharedPointer<cucd::PeerRegistry>(new Registry());

    auto updater = new cuc::detail::RegistryUpdater(registry);

    auto app_manager = QSharedPointer<cuca::ApplicationManager>(new cucd::AppManager());

    auto server = new cucd::Service(connection, registry, app_manager, app->parent());
    new ServiceAdaptor(server);

    if (not connection.registerService(HUB_SERVICE_NAME))
    {
        qWarning() << "Failed to register" << HUB_SERVICE_NAME;
        ret = 1;
    }
    if (not connection.registerObject(HUB_SERVICE_PATH,
                                      server,
                                      QDBusConnection::ExportAdaptors))
    {
        qWarning() << "Failed to register object on" << HUB_SERVICE_PATH;
        ret = 1;
    }

    std::signal(SIGTERM, shutdown);
    std::signal(SIGHUP, shutdown);
    std::signal(SIGKILL, shutdown);
    std::signal(SIGINT, shutdown);

    if (ret == 1)
        app->exit(ret);
    else        
        ret = app->exec();

    TRACE() << "Server exiting, cleaning up";
    delete server;
    return ret;
}
