/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#ifndef REGISTRY_UPDATER_H
#define REGISTRY_UPDATER_H

#include <QObject>
#include <QFileInfo>
#include <com/ubuntu/content/peer.h>

#include "registry.h"

namespace com
{
namespace ubuntu
{
namespace content
{
namespace detail
{
class RegistryUpdater : public QObject
{
    Q_OBJECT
public:
    RegistryUpdater(const QSharedPointer<com::ubuntu::content::detail::PeerRegistry>& registry, QObject *parent = 0);
    ~RegistryUpdater();

public Q_SLOTS:
    bool return_error(QString err = "");
    void refresh(const QString&);
    void run();
    bool add_peer(QFileInfo);

private:
    const QSharedPointer<com::ubuntu::content::detail::PeerRegistry>& registry;
    QVector<QDir> contentDirs;
    QScopedPointer<QFileSystemWatcher> watcher;
    
};
}
}
}
}

#endif // REGISTRY_UPDATER_H
