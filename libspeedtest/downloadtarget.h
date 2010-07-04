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


#ifndef DOWNLOADTARGET_H
#define DOWNLOADTARGET_H


#include "libspeedtest_global.h"
#include <QtCore/QUrl>
#include <QtCore/QEventLoop>


const quint8 DOWNLOADTESTSECS = 15;


class LIBSPEEDTEST_EXPORT DownloadTarget : public QObject {
    Q_OBJECT

public:
    DownloadTarget();
    DownloadTarget(QString, QUrl);
    DownloadTarget(const DownloadTarget&, QObject *parent = 0);
    DownloadTarget& operator=(const DownloadTarget&);
    void downloadTest();
    QString getName() const { return name; }
    void setName(QString value) { name = value; }
    QUrl getUrl() const { return url; }
    void setUrl(QUrl value) { url = value; }
    qint64 getBytesDownloaded() const { return bytesDownloaded; }

private:
    QString name;
    QUrl url;
    QEventLoop *loop;
    bool *stopBenchmark;
    qint64 bytesDownloaded;

signals:
    void newTestResult(QString);
    void sigProgress(quint8);

public slots:
    void abortDownload() { *stopBenchmark = true; loop->quit(); }

private slots:
    void updateBytes(qint64 value) { bytesDownloaded = value; }
};


#endif // DOWNLOADTARGET_H
