/*
Copyright 2010 Aleksandros Gidarakos


This file is part of QSpeedTest.

QSpeedTest is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QSpeedTest is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QSpeedTest.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef TARGETLIST_H
#define TARGETLIST_H


#include "libspeedtest_global.h"
#include "pinggroup.h"
#include "downloadtarget.h"
#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtCore/QList>


const QString TARGETLISTURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.targets.ini";
const quint8 UPDATECHECKTIMEOUT = 3;


class LIBSPEEDTEST_EXPORT TargetList : public QObject {
    Q_OBJECT

public:
    explicit TargetList(QString, QString, QObject *parent = 0);
    ~TargetList() { delete iniTargets; }
    bool init();
    qint64 getBytesDownloaded(QList<DownloadTarget>&) const;
    QString getVersion() const { return version; }
    QString getComment() const { return comment; }
    QString getContactUrl() const { return contactUrl; }
    int getNumberOfTargets() const { return numberOfTargets; }
    QList<PingGroup> groups;
    QList<DownloadTarget> fileHostsDomestic;
    QList<DownloadTarget> fileHostsInternational;

private:
    QSettings *iniTargets;
    QString version;
    QString comment;
    QString contactUrl;
    int numberOfTargets;
    void purge();
    bool exists();
    bool updateIsAvailable();
    bool downloadList();
    bool load();
    bool restoreEmbedded();

signals:
    void logMessage(QString);
};


#endif // TARGETLIST_H
