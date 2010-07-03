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


#include "downloadtarget.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>


DownloadTarget::DownloadTarget()
{
}


DownloadTarget::DownloadTarget(QString name, QUrl url)
{
    this->name = name;
    this->url = url;
}


DownloadTarget::DownloadTarget(const DownloadTarget &target, QObject *parent) : QObject(parent)
{
    name = target.name;
    url = target.url;
    bytesDownloaded = target.bytesDownloaded;
}


DownloadTarget& DownloadTarget::operator=(const DownloadTarget &target) {

    if(this == &target)
        return *this;

    name = target.name;
    url = target.url;
    bytesDownloaded = target.bytesDownloaded;

    return *this;
}


void DownloadTarget::downloadTest()
{
    QNetworkAccessManager manager;
    QNetworkReply *file;

    loop = new QEventLoop;
    bytesDownloaded = 0;
    QTimer::singleShot(DOWNLOADTESTSECS * 1000, loop, SLOT(quit()));
    emit newTestResult(trUtf8("%1").arg(url.toString()));
    file = manager.get(QNetworkRequest(url));
    connect(file, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateBytes(qint64)));
    loop->exec();
    disconnect(file, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateBytes(qint64)));
    file->abort();
    emit newTestResult(trUtf8("%1: %2 bytes").arg(name).arg(bytesDownloaded));
    qApp->processEvents();
    delete file;
    delete loop;
}
