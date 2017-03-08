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
#ifndef COM_UBUNTU_CONTENT_TYPE_H_
#define COM_UBUNTU_CONTENT_TYPE_H_

#include <QObject>
#include <QSharedPointer>

namespace com
{
namespace ubuntu
{
namespace content
{
namespace detail
{
class Service;
class RegistryUpdater;
}
class Type : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id())
  public:
    
    static const Type& unknown();
    static const Type& all();

    struct Known
    {
        static const Type& documents();
        static const Type& pictures();
        static const Type& music();
        static const Type& contacts();
        static const Type& videos();
        static const Type& links();
        static const Type& ebooks();
        static const Type& text();
        static const Type& events();
    };

    virtual ~Type();
    Type(const Type&);
    
    Type& operator=(const Type&);    
    bool operator==(const Type&) const;
    bool operator!=(const Type&) const;
    bool operator<(const Type&) const;

    Q_INVOKABLE virtual const QString& id() const;

  protected:
    friend struct Known;
    friend class detail::Service;
    friend class detail::RegistryUpdater;

    explicit Type(const QString&, QObject* = nullptr);
    
  private:
    struct Private;
    QSharedPointer<Private> d;
};
}
}
}

#endif // COM_UBUNTU_CONTENT_TYPE_H_
