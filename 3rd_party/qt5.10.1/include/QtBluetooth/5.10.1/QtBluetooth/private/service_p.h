/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -p service_p.h:service.cpp org.bluez.all.xml org.bluez.Service
 *
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef SERVICE_P_H_1277421939
#define SERVICE_P_H_1277421939

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.bluez.Service
 */
class OrgBluezServiceInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.bluez.Service"; }

public:
    OrgBluezServiceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgBluezServiceInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<uint> AddRecord(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("AddRecord"), argumentList);
    }

    inline QDBusPendingReply<> CancelAuthorization()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("CancelAuthorization"), argumentList);
    }

    inline QDBusPendingReply<> RemoveRecord(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("RemoveRecord"), argumentList);
    }

    inline QDBusPendingReply<> RequestAuthorization(const QString &in0, uint in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("RequestAuthorization"), argumentList);
    }

    inline QDBusPendingReply<> UpdateRecord(uint in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("UpdateRecord"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace org {
  namespace bluez {
    typedef ::OrgBluezServiceInterface Service;
  }
}
#endif
