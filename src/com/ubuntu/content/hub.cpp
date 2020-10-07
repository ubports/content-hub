/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include "common.h"
#include "debug.h"
#include "ContentServiceInterface.h"
#include "ContentHandlerInterface.h"
#include "handleradaptor.h"
#include "paste_p.h"
#include "transfer_p.h"
#include "utils.cpp"

#include <com/ubuntu/content/hub.h>
#include <com/ubuntu/content/import_export_handler.h>
#include <com/ubuntu/content/peer.h>
#include <com/ubuntu/content/scope.h>
#include <com/ubuntu/content/store.h>
#include <com/ubuntu/content/type.h>

#ifdef WITH_LIBERTINE
#include <libertine.h>
#endif

#include <QDBusPendingCallWatcher>
#include <QIcon>
#include <QStandardPaths>
#include <QStringList>
#include <QProcessEnvironment>
#include <map>

namespace cuc = com::ubuntu::content;

struct cuc::Hub::Private
{
    Private(QObject* parent) : service(
        new com::ubuntu::content::dbus::Service(
            HUB_SERVICE_NAME,
            HUB_SERVICE_PATH,
            QDBusConnection::sessionBus(),
            parent))
    {
    }

    com::ubuntu::content::dbus::Service* service;
    QStringList pasteFormats;
};

cuc::Hub::Hub(QObject* parent) : QObject(parent), d{new cuc::Hub::Private{this}}
{
    /* read environment variables */
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    if (environment.contains(QLatin1String("CONTENT_HUB_LOGGING_LEVEL"))) {
        bool isOk;
        int value = environment.value(
            QLatin1String("CONTENT_HUB_LOGGING_LEVEL")).toInt(&isOk);
        if (isOk)
            setLoggingLevel(value);
    }

    qDBusRegisterMetaType<cuc::Item>();

    if (qApp)
        qApp->installEventFilter(this);

#ifdef WITH_LIBERTINE
    /* Append icon paths from the libertine containers */
    QStringList iconPaths = QIcon::themeSearchPaths();
    gchar ** containers = libertine_list_containers();
    for (int i = 0; containers[i]; i++) {
        QString path = libertine_container_path(containers[i]);
        iconPaths << QString(path + "/usr/share/icons/");
    }
    QIcon::setThemeSearchPaths(iconPaths);
#endif

    QObject::connect(d->service, &com::ubuntu::content::dbus::Service::PasteFormatsChanged,
            this, &cuc::Hub::Hub::onPasteFormatsChanged);
    requestPasteFormats();
    QObject::connect(d->service, SIGNAL(PasteboardChanged()),
            this,
            SIGNAL(pasteboardChanged()));
}

cuc::Hub::~Hub()
{
}

cuc::Hub* cuc::Hub::Client::instance()
{
    static cuc::Hub* hub = new cuc::Hub(nullptr);
    return hub;
}

void cuc::Hub::requestPasteFormats()
{
    auto reply = d->service->PasteFormats();

    auto replyWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(replyWatcher, &QDBusPendingCallWatcher::finished,
            this, [this, replyWatcher]() {
        QDBusPendingReply<QStringList> reply = *replyWatcher;
        replyWatcher->deleteLater();
        if (!reply.isError()) {
            d->pasteFormats = reply.value();
            Q_EMIT(pasteFormatsChanged());
        }
    });
}

void cuc::Hub::onPasteFormatsChanged(const QStringList &formats)
{
    TRACE() << Q_FUNC_INFO;

    d->pasteFormats = formats;
    TRACE() << Q_FUNC_INFO << d->pasteFormats;
    Q_EMIT(pasteFormatsChanged());
}

bool cuc::Hub::eventFilter(QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::ApplicationActivate)
   {
       QString id = app_id();
       if (!id.isEmpty())
       {
           TRACE() << Q_FUNC_INFO << id << "Activated";
           d->service->HandlerActive(id);
       } else
       {
           qWarning() << "APP_ID isn't set, the handler ignored";
       }
   }
   return QObject::eventFilter(obj, event);
}

void cuc::Hub::register_import_export_handler(cuc::ImportExportHandler* handler)
{
    TRACE() << Q_FUNC_INFO;
    QString id = app_id();

    if (id.isEmpty())
    {
        qWarning() << "APP_ID isn't set, the handler can not be registered";
        return;
    }

    auto c = QDBusConnection::sessionBus();
    auto h = new cuc::detail::Handler(c, id, handler);

    new HandlerAdaptor(h);


    if (not c.registerObject(handler_path(id), h))
    {
        qWarning() << Q_FUNC_INFO << "Failed to register object for:" << id;
        return;
    }

    d->service->RegisterImportExportHandler(
                id,
                QDBusObjectPath{handler_path(id)});
}

const cuc::Store* cuc::Hub::store_for_scope_and_type(cuc::Scope scope, cuc::Type type)
{
    static const std::map<std::pair<cuc::Scope, cuc::Type>, cuc::Store*> lut =
            {
                {{cuc::system, cuc::Type::Known::pictures()}, new cuc::Store{"/content/Pictures", this}},
                {{cuc::system, cuc::Type::Known::music()}, new cuc::Store{"/content/Music", this}},
                {{cuc::system, cuc::Type::Known::documents()}, new cuc::Store{"/content/Documents", this}},
                {{cuc::user, cuc::Type::Known::pictures()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), this}},
                {{cuc::user, cuc::Type::Known::music()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::MusicLocation), this}},
                {{cuc::user, cuc::Type::Known::documents()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), this}},
                {{cuc::app, cuc::Type::Known::pictures()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Pictures", this}},
                {{cuc::app, cuc::Type::Known::music()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Music", this}},
                {{cuc::app, cuc::Type::Known::documents()}, new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/Documents", this}},
            };

    auto it = lut.find(std::make_pair(scope, type));

    if (it == lut.end())
        return nullptr;

    return it->second;
}

cuc::Peer cuc::Hub::default_source_for_type(cuc::Type t)
{
    TRACE() << Q_FUNC_INFO;
    auto reply = d->service->DefaultSourceForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return cuc::Peer::unknown();

    auto peer = reply.value();
    return qdbus_cast<cuc::Peer>(peer.variant());
}

QVector<cuc::Peer> cuc::Hub::known_sources_for_type(cuc::Type t)
{
    QVector<cuc::Peer> result;

    auto reply = d->service->KnownSourcesForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return result;

    auto peers = reply.value();
    QString id = app_id();

    Q_FOREACH(const QVariant& p, peers)
    {
        auto peer = qdbus_cast<cuc::Peer>(p);
        if (peer.id() != id)
            result << peer;
    }
    return result;
}

QVector<cuc::Peer> cuc::Hub::known_destinations_for_type(cuc::Type t)
{
    QVector<cuc::Peer> result;

    auto reply = d->service->KnownDestinationsForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return result;

    auto peers = reply.value();
    QString id = app_id();

    Q_FOREACH(const QVariant& p, peers)
    {
        auto peer = qdbus_cast<cuc::Peer>(p);
        if (peer.id() != id)
            result << peer;
    }
    return result;
}

QVector<cuc::Peer> cuc::Hub::known_shares_for_type(cuc::Type t)
{
    QVector<cuc::Peer> result;

    auto reply = d->service->KnownSharesForType(t.id());
    reply.waitForFinished();

    if (reply.isError())
        return result;

    auto peers = reply.value();
    QString id = app_id();

    Q_FOREACH(const QVariant& p, peers)
    {
        auto peer = qdbus_cast<cuc::Peer>(p);
        if (peer.id() != id)
            result << peer;
    }
    return result;
}

cuc::Transfer* cuc::Hub::create_import_from_peer(cuc::Peer peer)
{
    return this->create_import_from_peer_for_type(peer, cuc::Type::unknown());
}

cuc::Transfer* cuc::Hub::create_import_from_peer_for_type(cuc::Peer peer, cuc::Type type)
{
    /* This needs to be replaced with a better way to get the APP_ID */
    QString id = app_id();

    auto reply = d->service->CreateImportFromPeer(peer.id(), id, type.id());
    reply.waitForFinished();

    if (reply.isError())
        return nullptr;

    cuc::Transfer *transfer = cuc::Transfer::Private::make_transfer(reply.value(), this);
    const cuc::Store *store = new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/HubIncoming/" + QString::number(transfer->id()), this};
    transfer->setStore(store);
    return transfer;
}

cuc::Transfer* cuc::Hub::create_export_to_peer(cuc::Peer peer)
{
    return this->create_export_to_peer_for_type(peer, cuc::Type::unknown());
}

cuc::Transfer* cuc::Hub::create_export_to_peer_for_type(cuc::Peer peer, cuc::Type type)
{
    /* This needs to be replaced with a better way to get the APP_ID */
    QString id = app_id();

    auto reply = d->service->CreateExportToPeer(peer.id(), id, type.id());
    reply.waitForFinished();

    if (reply.isError())
        return nullptr;

    cuc::Transfer *transfer = cuc::Transfer::Private::make_transfer(reply.value(), this);

    QString peerName = peer.id().split("_")[0];
    TRACE() << Q_FUNC_INFO << "peerName: " << peerName;
    const cuc::Store *store = new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + peerName + "/HubIncoming/" + QString::number(transfer->id()), this};
    TRACE() << Q_FUNC_INFO << "STORE:" << store->uri();
    transfer->setStore(store);
    transfer->start();
    return transfer;
}

cuc::Transfer* cuc::Hub::create_share_to_peer(cuc::Peer peer)
{
    return this->create_share_to_peer_for_type(peer, cuc::Type::unknown());
}

cuc::Transfer* cuc::Hub::create_share_to_peer_for_type(cuc::Peer peer, cuc::Type type)
{
    /* This needs to be replaced with a better way to get the APP_ID */
    QString id = app_id();

    auto reply = d->service->CreateShareToPeer(peer.id(), id, type.id());
    reply.waitForFinished();

    if (reply.isError())
        return nullptr;

    cuc::Transfer *transfer = cuc::Transfer::Private::make_transfer(reply.value(), this);
    QString peerName = peer.id().split("_")[0];
    TRACE() << Q_FUNC_INFO << "peerName: " << peerName;
    const cuc::Store *store = new cuc::Store{QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + peerName + "/HubIncoming/" + QString::number(transfer->id()), this};
    TRACE() << Q_FUNC_INFO << "STORE:" << store->uri();
    transfer->setStore(store);
    transfer->start();
    return transfer;
}

void cuc::Hub::quit()
{
    d->service->Quit();
}

bool cuc::Hub::has_pending(QString peer_id)
{
    auto reply = d->service->HasPending(peer_id);
    reply.waitForFinished();

    if (reply.isError())
        return false;

    return reply.value();
}

cuc::Peer cuc::Hub::peer_for_app_id(QString app_id)
{
    auto reply = d->service->PeerForId(app_id);
    reply.waitForFinished();

    if (reply.isError())
        return cuc::Peer::unknown();

    auto peer = reply.value();
    return qdbus_cast<cuc::Peer>(peer.variant());
}

QDBusPendingCall cuc::Hub::createPaste(const QString &surfaceId, const QMimeData& mimeData)
{
    /* This needs to be replaced with a better way to get the APP_ID */
    QString appId = app_id();
    TRACE() << Q_FUNC_INFO << appId;

    auto serializedMimeData = serializeMimeData(mimeData);
    if (serializedMimeData.isEmpty()) {
        return QDBusPendingCall::fromCompletedCall(
                QDBusMessage::createError("Data serialization failed","Could not serialize mimeData"));
    }

    return d->service->CreatePaste(appId, surfaceId, serializedMimeData, mimeData.formats());
}

bool cuc::Hub::createPasteSync(const QString &surfaceId, const QMimeData& data)
{
    QDBusPendingCall reply = createPaste(surfaceId, data);
    reply.waitForFinished();
    return !reply.isError();
}

QDBusPendingCall cuc::Hub::requestLatestPaste(const QString &surfaceId)
{
    TRACE() << Q_FUNC_INFO;
    return d->service->GetLatestPasteData(surfaceId);
}

QDBusPendingCall cuc::Hub::requestPasteById(const QString &surfaceId, int pasteId)
{
    TRACE() << Q_FUNC_INFO;
    return d->service->GetPasteData(surfaceId, QString::number(pasteId));
}

QMimeData* cuc::Hub::paste(QDBusPendingCall pendingCall)
{
    auto reply = QDBusPendingReply<QByteArray>(pendingCall);
    reply.waitForFinished();

    if (reply.isError())
        return nullptr;

    QByteArray serializedMimeData = qdbus_cast<QByteArray>(reply.value());
    return deserializeMimeData(serializedMimeData);
}

QMimeData* cuc::Hub::latestPaste(const QString &surfaceId)
{
    return paste(requestLatestPaste(surfaceId));
}

QMimeData* cuc::Hub::pasteById(const QString &surfaceId, int pasteId)
{
    return paste(requestPasteById(surfaceId, pasteId));
}

QStringList cuc::Hub::pasteFormats() {
    TRACE() << Q_FUNC_INFO;
    return d->pasteFormats;
}
