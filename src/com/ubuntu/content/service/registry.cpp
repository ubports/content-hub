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

#include "debug.h"
#include "registry.h"
#include "utils.cpp"
#include <QMap>
#include <QVector>
#include <com/ubuntu/content/type.h>
#include <gio/gdesktopappinfo.h>
//#include <libertine.h>
#include <ubuntu-app-launch.h>

// Begin anonymous namespace
namespace {

cuc::Type mime_to_wellknown_type (const char * type)
{
    if (QString(type).contains("document"))
        return cuc::Type::Known::documents();
    else if (g_str_has_prefix (type, "x-scheme-handler/http"))
        return cuc::Type::Known::links();
    else if (g_str_has_prefix (type, "audio"))
        return cuc::Type::Known::music();
    else if (g_str_has_prefix (type, "image"))
        return cuc::Type::Known::pictures();
    else if (g_str_has_prefix (type, "video"))
        return cuc::Type::Known::videos();
    return cuc::Type::unknown();
}

/*
QMap<QString, QVector<QString>> libertine_apps()
{
    TRACE() << Q_FUNC_INFO;
    QStringList containers;
    QStringList appIds;
    char * dir = nullptr;
    char * file = nullptr;
    bool ret;
    gchar ** raw_containers = libertine_list_containers();

    for (int i = 0; raw_containers[i]; i++) {
        containers << raw_containers[i];
    }

    g_strfreev(raw_containers);

    gchar ** app_ids = NULL;
    Q_FOREACH(QString container, containers) {
        gchar ** app_ids = libertine_list_apps_for_container(container.toStdString().c_str());
        for (int i = 0; app_ids[i]; i++) {
            appIds << app_ids[i];
        }
    }
    g_strfreev(app_ids);

    QMap<QString, QVector<QString>> appMap;

    Q_FOREACH(QString app, appIds)
    {
        ret = ubuntu_app_launch_application_info(app.toStdString().c_str(), &dir, &file);
        if (ret)
        {
            GKeyFile *key_file = g_key_file_new();
            GError *error = NULL;
            if (!g_key_file_load_from_file(key_file,
                                           g_strjoin("/", dir, file, NULL),
                                           G_KEY_FILE_NONE,
                                           &error)) {
                qWarning() << "ERROR:" <<error->message;
            } else {
                /* FIXME: we should hide NoDisplay and honor ShownIn
                if (g_desktop_app_info_get_nodisplay (appInfo))
                    continue;

                if (not g_app_info_should_show (G_APP_INFO(appInfo)))
                    continue;


                gchar ** types = g_key_file_get_locale_string_list(key_file,
                                                                   G_KEY_FILE_DESKTOP_GROUP,
                                                                   G_KEY_FILE_DESKTOP_KEY_MIME_TYPE,
                                                                   NULL,
                                                                   NULL,
                                                                   &error);

                if (types == NULL)
                    continue;
                cuc::Type type = cuc::Type::unknown();
                for (int i = 0; types[i]; i++) {
                    type = mime_to_wellknown_type(types[i]);
                    if (type != cuc::Type::unknown()) {
                        QVector<QString> t;
                        t << type.id();
                        if (not appMap.keys().contains(app)) {
                            appMap.insert(app, t);
                        } else if (not appMap.value(app).contains(type.id())){
                            t << appMap.value(app);
                            appMap.insert(app, t);
                        }
                    }
                }
            }
        }
    }

    g_free(dir);
    g_free(file);
    return appMap;
}

QStringList libertine_app_ids(QString type)
{
    TRACE() << Q_FUNC_INFO << "TYPE:" << type;
    QStringList results;

    auto appMap = libertine_apps();

    Q_FOREACH (QString a, appMap.keys())
    {
        if (appMap.value(a).contains(type) || type == "all")
            results << a;
    }

    return results;
}
*/
} // End anonymous namespace

Registry::Registry() :
    m_defaultSources(new QGSettings("com.ubuntu.content.hub.default",
                                  "/com/ubuntu/content/hub/peers/")),
    m_sources(new QGSettings("com.ubuntu.content.hub.source",
                           "/com/ubuntu/content/hub/source/")),
    m_dests(new QGSettings("com.ubuntu.content.hub.destination",
                           "/com/ubuntu/content/hub/destination/")),
    m_shares(new QGSettings("com.ubuntu.content.hub.share",
                           "/com/ubuntu/content/hub/share/"))
{
    /* ensure all default sources are registered as available sources */
    QList<cuc::Type> types = known_types();
    Q_FOREACH (cuc::Type type, types)
    {
        if (m_defaultSources->keys().contains(type.id()))
        {
            QVariant peer_v = m_defaultSources->get(type.id());
            // If default isn't a StringList, attempt to reset
            if (peer_v.type() != QVariant::StringList)
            {
                TRACE() << Q_FUNC_INFO << "Default type for" << type.id() << "is incorrect, resetting";
                m_defaultSources->reset(type.id());
                // After reset, lets get a new QVariant
                peer_v = m_defaultSources->get(type.id());
            }

            /* Only assume the default is correct if the type is a StringList
             * The reset above should have ensured it was reset, lets double
             * check anyway to prevent crashes
             */
            if (peer_v.type() == QVariant::StringList)
            {
                QStringList as(peer_v.toStringList());
                if (!as.isEmpty())
                {
                    std::string pkg = as[0].toStdString();
                    std::string app = as[1].toStdString();
                    std::string ver = as[2].toStdString();
                    cuc::Peer peer;
                    if (app.empty() || ver.empty())
                        peer = QString::fromStdString(pkg);
                    else
                        peer = QString::fromLocal8Bit(ubuntu_app_launch_triplet_to_app_id(pkg.c_str(), app.c_str(), ver.c_str()));
                    install_source_for_type(type, cuc::Peer{peer.id(), true});
                }
            }
        }
    }
}

Registry::~Registry()
{
    TRACE() << Q_FUNC_INFO;
}

cuc::Peer Registry::default_source_for_type(cuc::Type type)
{
    TRACE() << Q_FUNC_INFO << type.id();
    if (m_defaultSources->keys().contains(type.id()))
    {
        QVariant peer_v = m_defaultSources->get(type.id());
        if (peer_v.type() != QVariant::StringList)
            return cuc::Peer(peer_v.toString());
        QStringList as(peer_v.toStringList());
        if (!as.isEmpty())
        {
            std::string pkg = as[0].toStdString();
            std::string app = as[1].toStdString();
            std::string ver = as[2].toStdString();
            if (app.empty() || ver.empty())
                return cuc::Peer(QString::fromStdString(pkg));
            return cuc::Peer(QString::fromLocal8Bit(ubuntu_app_launch_triplet_to_app_id(pkg.c_str(), app.c_str(), ver.c_str())), true);
        }
    }

    return cuc::Peer();
}

void Registry::enumerate_known_peers(const std::function<void(const cuc::Peer&)>&for_each)
{
    TRACE() << Q_FUNC_INFO;

    Q_FOREACH (QString type_id, m_sources->keys())
    {
        TRACE() << Q_FUNC_INFO << type_id;
        Q_FOREACH (QString k, m_sources->get(type_id).toStringList())
        {
            TRACE() << Q_FUNC_INFO << k;
            for_each(cuc::Peer{k});
        }
    }
    Q_FOREACH (QString type_id, m_dests->keys())
    {
        TRACE() << Q_FUNC_INFO << type_id;
        Q_FOREACH (QString k, m_dests->get(type_id).toStringList())
        {
            TRACE() << Q_FUNC_INFO << k;
            for_each(cuc::Peer{k});
        }
    }
    Q_FOREACH (QString type_id, m_shares->keys())
    {
        TRACE() << Q_FUNC_INFO << type_id;
        Q_FOREACH (QString k, m_shares->get(type_id).toStringList())
        {
            TRACE() << Q_FUNC_INFO << k;
            for_each(cuc::Peer{k});
        }
    }
}

void Registry::enumerate_known_sources_for_type(cuc::Type type, const std::function<void(const cuc::Peer&)>&for_each)
{
    TRACE() << Q_FUNC_INFO << type.id();

    QStringList peers;
    peers << m_sources->get("all").toStringList();
    if (type != cuc::Type::unknown() && valid_type(type))
        peers << m_sources->get(type.id()).toStringList();

    Q_FOREACH (QString k, peers)
    {
        TRACE() << Q_FUNC_INFO << k;
        bool defaultPeer = false;
        QVariant peer_v;
        if (type != cuc::Type::unknown())
            peer_v = m_defaultSources->get(type.id());
        if (peer_v.type() == QVariant::StringList)
        {
            QStringList as(peer_v.toStringList());
            if (!as.isEmpty())
            {
                std::string pkg = as[0].toStdString();
                std::string app = as[1].toStdString();
                std::string ver = as[2].toStdString();
                if (app.empty() || ver.empty())
                    defaultPeer = QString::fromStdString(pkg) == k;
                else
                    defaultPeer = QString::fromLocal8Bit(ubuntu_app_launch_triplet_to_app_id(pkg.c_str(), app.c_str(), ver.c_str())) == k;
            }
        }
        for_each(cuc::Peer{k, defaultPeer});
    }
}

void Registry::enumerate_known_destinations_for_type(cuc::Type type, const std::function<void(const cuc::Peer&)>&for_each)
{
    TRACE() << Q_FUNC_INFO << type.id();

    QStringList peers;
    peers << m_dests->get("all").toStringList();
    if (type != cuc::Type::unknown() && valid_type(type))
        peers << m_dests->get(type.id()).toStringList();

    //peers << libertine_app_ids(type.id());

    Q_FOREACH (QString k, peers)
    {
        TRACE() << Q_FUNC_INFO << k;
        for_each(cuc::Peer{k});
    }
}

void Registry::enumerate_known_shares_for_type(cuc::Type type, const std::function<void(const cuc::Peer&)>&for_each)
{
    TRACE() << Q_FUNC_INFO << type.id();

    if (type == cuc::Type::unknown() || !valid_type(type))
        return;

    QStringList peers;
    peers << m_shares->get(type.id()).toStringList();

    //peers << libertine_app_ids(type.id());

    Q_FOREACH (QString k, peers)
    {
        TRACE() << Q_FUNC_INFO << k;
        for_each(cuc::Peer{k});
    }
}

bool Registry::install_default_source_for_type(cuc::Type type, cuc::Peer peer)
{
    TRACE() << Q_FUNC_INFO << "type:" << type.id() << "peer:" << peer.id();
    if (m_defaultSources->keys().contains(type.id()))
    {
        TRACE() << Q_FUNC_INFO << "Default peer for" << type.id() << "already installed.";
        return false;
    }

    this->install_source_for_type(type, peer);
    return m_defaultSources->trySet(type.id(), QVariant(peer.id()));
}

bool Registry::install_source_for_type(cuc::Type type, cuc::Peer peer)
{
    TRACE() << Q_FUNC_INFO << "type:" << type.id() << "peer:" << peer.id();
    QStringList l = m_sources->get(type.id()).toStringList();
    if (not l.contains(peer.id()))
    {
        l.append(peer.id());
        return m_sources->trySet(type.id(), QVariant(l));
    }
    return false;
}

bool Registry::install_destination_for_type(cuc::Type type, cuc::Peer peer)
{
    TRACE() << Q_FUNC_INFO << "type:" << type.id() << "peer:" << peer.id();
    QStringList l = m_dests->get(type.id()).toStringList();
    if (not l.contains(peer.id()))
    {
        l.append(peer.id());
        return m_dests->trySet(type.id(), QVariant(l));
    }
    return false;
}

bool Registry::install_share_for_type(cuc::Type type, cuc::Peer peer)
{
    TRACE() << Q_FUNC_INFO << "type:" << type.id() << "peer:" << peer.id();
    QStringList l = m_shares->get(type.id()).toStringList();
    if (not l.contains(peer.id()))
    {
        l.append(peer.id());
        return m_shares->trySet(type.id(), QVariant(l));
    }
    return false;
}

bool Registry::remove_peer(cuc::Peer peer)
{
    TRACE() << Q_FUNC_INFO << "peer:" << peer.id();
    bool ret = false;
    Q_FOREACH (QString type_id, m_sources->keys())
    {
        QStringList l = m_sources->get(type_id).toStringList();
        if (l.contains(peer.id()))
        {
            l.removeAll(peer.id());
            ret = m_sources->trySet(type_id, QVariant(l));
        }
    }
    Q_FOREACH (QString type_id, m_dests->keys())
    {
        QStringList l = m_dests->get(type_id).toStringList();
        if (l.contains(peer.id()))
        {
            l.removeAll(peer.id());
            ret = m_dests->trySet(type_id, QVariant(l));
        }
    }
    Q_FOREACH (QString type_id, m_shares->keys())
    {
        QStringList l = m_shares->get(type_id).toStringList();
        if (l.contains(peer.id()))
        {
            l.removeAll(peer.id());
            ret = m_shares->trySet(type_id, QVariant(l));
        }
    }
    return ret;
}
bool Registry::peer_is_legacy(QString peer_id)
{
    //return libertine_app_ids("all").contains(peer_id);
    return false;
}
