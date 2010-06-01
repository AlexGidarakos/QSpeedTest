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


#include "filehost.h"
#include "externs.h"
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QTimer>


FileHost::FileHost()
{
}


FileHost::FileHost(QString name, QUrl url)
{
    this->name = name;
    this->url = url;
}


FileHost::FileHost(const FileHost &fileHost)
{
    name = fileHost.name;
    url = fileHost.url;
    bytesDownloaded = fileHost.bytesDownloaded;
}


FileHost& FileHost::operator=(const FileHost &fileHost) {

    if(this == &fileHost)
        return *this;

    name = fileHost.name;
    url = fileHost.url;
    bytesDownloaded = fileHost.bytesDownloaded;

    return *this;
}


void FileHost::downloadTest()
{
    QNetworkAccessManager manager;
    QNetworkReply *file;

    loop = new QEventLoop;
    QTimer::singleShot(DOWNLOADTESTSECS * 1000, loop, SLOT(quit()));
    emit newTestResult(trUtf8("%1").arg(url.toString()));
    file = manager.get(QNetworkRequest(url));
    connect(file, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateBytes(qint64)));
    loop->exec();
    MUTEX.lock();
    BYTESDOWNLOADED += bytesDownloaded;
    MUTEX.unlock();
    emit newTestResult(trUtf8("%1: %2 bytes").arg(name).arg(bytesDownloaded));
    file->abort();
    delete file;
    delete loop;
}


QString FileHost::getName()
{
    return name;
}


void FileHost::setName(QString value)
{
    name = value;
}


QUrl FileHost::getUrl()
{
    return url;
}


void FileHost::setUrl(QUrl value)
{
    url = value;
}


void FileHost::updateBytes(qint64 bytes)
{
    bytesDownloaded = bytes;
}


void FileHost::abortDownload()
{
    loop->quit();
}
