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

#ifndef HOSTINFO_H
#define HOSTINFO_H

#include "libspeedtest_global.h"
#include "results.h"
#include <QtCore/QProcess>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class LIBSPEEDTEST_EXPORT HostInfo : public QObject
{
    Q_OBJECT

public:
    explicit HostInfo(Results &, QObject *parent = 0);
    ~HostInfo();
    void startDetection();
    void stopInfoTest();

private:
    void _retrieve();
    Results &_results;
    QProcess _osDetectProc, _tracerouteProc;
    QEventLoop _loop;
    QString _tracerouteCmd;
    QString _bbrasLine;
    QString _winArch;
    QNetworkAccessManager _manager;
    QNetworkReply *_download;

signals:
    void osDetectionFinished();
    void finished();

public slots:
    void startInfoTest();

private slots:
    void _slotOsDetectProcFinished();
};
#endif // HOSTINFO_H
