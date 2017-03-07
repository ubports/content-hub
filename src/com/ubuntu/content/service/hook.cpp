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
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QVector>
#include <com/ubuntu/content/peer.h>

#include "debug.h"
#include "hook.h"

namespace cuc = com::ubuntu::content;
namespace cucd = com::ubuntu::content::detail;

cucd::Hook::Hook(const QSharedPointer<cucd::PeerRegistry>& registry, QObject *parent) :
    QObject(parent),
    registry(registry)
{
    QTimer::singleShot(200, this, SLOT(run()));
}

cucd::Hook::~Hook()
{
    TRACE() << Q_FUNC_INFO;
}

void cucd::Hook::run()
{
    TRACE() << Q_FUNC_INFO;
    /* Looks for files in ${HOME}/.local/share/content-hub/${id} installed
     * by click packages.  These files are JSON, for example:
     *
     * {
     *     "source": [
     *         "pictures",
     *         "music"
     *     ]
     * }
     *
     * The hook also iterates known peers and removes them if there is
     * no JSON file installed in this path.
     */

    QVector<QDir> contentDirs;

    contentDirs.append(QDir(
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QString("/")
        + QString("content-hub")));
    
    contentDirs.append(QDir("/usr/share/content-hub/peers/"));
    contentDirs.append(QDir("/usr/share/local/content-hub/peers/"));

    QStringList all_peers;
    registry->enumerate_known_peers([&all_peers](const cuc::Peer& peer)
                                    {
                                        all_peers.append(peer.id());
                                    });

    Q_FOREACH(QString p, all_peers)
    {
        TRACE() << Q_FUNC_INFO << "Looking for" << p;
        bool foundPeer = false;
        Q_FOREACH(QDir contentDir, contentDirs)
        {
            QStringList pp = contentDir.entryList(QStringList("*"+ p));
            if (!pp.isEmpty()) {
                foundPeer = true;
            }
        }
        if(!foundPeer) {
            registry->remove_peer(cuc::Peer{p});
        }
    }

    bool peerDirsExist = false;

    Q_FOREACH(QDir contentDir, contentDirs)
    {

        if (contentDir.exists()) 
        {
            peerDirsExist = true;

            Q_FOREACH(QFileInfo f, contentDir.entryInfoList(QDir::Files))
                add_peer(f);
        }

    }

    if(!peerDirsExist)
        return_error("No peer setting directories exist.");

    deleteLater();
}

bool cucd::Hook::add_peer(QFileInfo result)
{
    TRACE() << Q_FUNC_INFO << "Hook:" << result.filePath();

    QStringList knownTypes;
    knownTypes << "all" << "pictures" << "music" << "contacts" << "documents" << "videos" << "links" << "ebooks" << "text" << "events";
    QString app_id = result.fileName();
    auto peer = cuc::Peer(app_id);

    QFile contentJson(result.absoluteFilePath());
    if (!contentJson.open(QIODevice::ReadOnly | QIODevice::Text))
        return_error("couldn't open " + result.absoluteFilePath());

    QJsonParseError *e = new QJsonParseError();
    QJsonDocument contentDoc = QJsonDocument::fromJson(contentJson.readAll(), e);

    if (e->error != 0)
        return return_error(e->errorString());

    if (not contentDoc.isObject())
        return return_error("invalid JSON object");

    QJsonObject contentObj = contentDoc.object();
    QVariant sources = contentObj.toVariantMap()["source"];
    Q_FOREACH(QString k, sources.toStringList())
    {
        if (knownTypes.contains(k))
        {
            if (registry->install_source_for_type(cuc::Type{k}, peer))
                TRACE() << "Installed source:" << peer.id() << "for type:" << k;
        }
        else
            qWarning() << "Failed to install" << peer.id() << "unknown type:" << k;
    }

    QVariant dests = contentObj.toVariantMap()["destination"];
    Q_FOREACH(QString k, dests.toStringList())
    {
        if (knownTypes.contains(k))
        {
            if (registry->install_destination_for_type(cuc::Type{k}, peer))
                TRACE() << "Installed destination:" << peer.id() << "for type:" << k;
        }
        else
            qWarning() << "Failed to install" << peer.id() << "unknown type:" << k;
    }

    QVariant shares = contentObj.toVariantMap()["share"];
    Q_FOREACH(QString k, shares.toStringList())
    {
        if (knownTypes.contains(k))
        {
            if (registry->install_share_for_type(cuc::Type{k}, peer))
                TRACE() << "Installed share:" << peer.id() << "for type:" << k;
        }
        else
            qWarning() << "Failed to install" << peer.id() << "unknown type:" << k;
    }
    return true;
}

bool cucd::Hook::return_error(QString err)
{
    qWarning() << "Failed to install peer" << err;
    return false;
}
