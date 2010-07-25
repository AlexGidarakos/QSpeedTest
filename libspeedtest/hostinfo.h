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
    QProcess _osDetectProc;
    QProcess _bbrasDetectProc;
    QEventLoop _loop;
    QString _bbrasDetectProcCmd;
    QString _bbrasLine;
    QString _winArchitecture;
    QNetworkAccessManager _manager;
    QNetworkReply *_ipDetectDownload;
    QNetworkReply *_ispNetworkDetectDownload;
    QNetworkReply *_ispNameDetectDownload;

signals:
    void osDetectionFinished();
    void ipDetectionFinished();
    void bbrasDetectionFinished();
    void ispNetworkDetectionFinished();
    void ispNameDetectionFinished();
    void finished();

public slots:
    void slotStartInfoTest();

private slots:
    void _slotOsDetectProcFinished();
    void _slotIpDetectDownloadFinished();
    void _slotBbrasDetectProcFinished();
    void _slotIspNetworkDetectDownloadFinished();
    void _slotIspNameDetectDownloadFinished();
};
#endif // HOSTINFO_H
