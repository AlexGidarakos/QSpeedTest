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

#include "hostlist.h"
#include "externs.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

bool Hostlist::_updateIsAvailable()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString remoteVersion;

    emit message(trUtf8("Checking online for an updated hostlist"));
    QTimer::singleShot(UPDATECHECKTIMEOUTSECS * 1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(HOSTLISTURL)));    // From here on, download pointer does not require manual deletion!
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        emit message(trUtf8("Hostlist server unreachable"));
        return false;
    }

    download->readLine();

    if((remoteVersion = download->readLine().trimmed().right(12)) > _version)
    {
        emit message(trUtf8("Update available. Local hostlist version is %1, remote is %2").arg(_version).arg(remoteVersion));
        return true;
    }

    emit message(trUtf8("No hostlist updates available"));
    return false;
}

bool Hostlist::_downloadOk(const QString &filename)
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QFile file(filename);
    QTextStream out;

    emit message(trUtf8("Downloading new hostlist"));

    download = manager.get(QNetworkRequest(QUrl(HOSTLISTURL)));    // From here on, download pointer now does not require manual deletion!
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
        emit message(trUtf8("Error. Download failed"));
        return false;
    }

    emit message(trUtf8("Download complete. Writing to file"));

    if(!file.open(QIODevice::WriteOnly))
    {
        emit message(trUtf8("Error writing downloaded hostlist to temporary file %1").arg(QDir::toNativeSeparators(filename)));
        emit message(trUtf8("Please check directory permissions."));
        return false;
    }

    out.setDevice(&file);
    out << download->readAll();
    file.close();
    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    emit message(trUtf8("Wrote %1 bytes").arg(file.size()));
    return true;
}

bool Hostlist::_isValid(const QString &filename2)
{
    QSettings temp(filename2, QSettings::IniFormat);
    QString filename = QDir::toNativeSeparators(filename2);
    QString string;
    int int1, int2;

    if(!temp.childGroups().contains("HostListInfo")) return false;

    temp.beginGroup("HostListInfo");
    string = temp.value("Version", QString("")).toString();

    if(string.isEmpty()) return false;

    string = temp.value("Comment", QString("")).toString();

    if(string.isEmpty()) return false;

    string = temp.value("ContactURL", QString("")).toString();

    if(string.isEmpty()) return false;

    int1 = temp.value("NumberOfPingGroups", 0).toInt();

    if(int1 < 1) return false;

    temp.endGroup();

    for(int i = 1; i <= int1; i++)
    {
        if(!temp.childGroups().contains(QString("PingGroup%1").arg(i))) return false;

        temp.beginGroup(QString("PingGroup%1").arg(i));
        string = temp.value("Name", QString("")).toString();

        if(string.isEmpty()) return false;

        int2 = temp.value("NumberOfHosts", 0).toInt();

        if(int2 < 1) return false;

        for(int j = 1; j <= int2; j++)
        {
            string = temp.value(QString("%1/Name").arg(j), QString("")).toString();

            if(string.isEmpty()) return false;

            string = temp.value(QString("%1/URL").arg(j), QString("")).toString();

            if(string.isEmpty() || !QUrl(string).isValid()) return false;
        }

        temp.endGroup();
    }

    int1 = temp.value("HostListInfo/NumberOfDownloadGroups", 0).toInt();

    if(int1 < 1) return false;

    for(int i = 1; i <= int1; i++)
    {
        if(!temp.childGroups().contains(QString("DownloadGroup%1").arg(i))) return false;

        temp.beginGroup(QString("DownloadGroup%1").arg(i));
        string = temp.value("Name", QString("")).toString();

        if(string.isEmpty()) return false;

        int2 = temp.value("NumberOfHosts", 0).toInt();

        if(int2 < 1) return false;

        for(int j = 1; j <= int2; j++)
        {
            string = temp.value(QString("%1/Name").arg(j), QString("")).toString();

            if(string.isEmpty()) return false;

            string = temp.value(QString("%1/URL").arg(j), QString("")).toString();

            if(string.isEmpty() || !QUrl(string).isValid()) return false;
        }

        temp.endGroup();
    }

    emit message(trUtf8("%1 is a valid hostlist").arg(filename));

    return true;
}

bool Hostlist::_restoreEmbeddedOk()
{
    QFile file(fileName());
    QFile embedded(":/libspeedtest/default.hostlist.ini");

    if(file.exists()) QFile::rename(fileName(), fileName() + ".wrong");

    emit message(trUtf8("Extracting default hostlist from embedded resources"));

    if(!embedded.copy(fileName()))
    {
        emit message(trUtf8("Error: Could not copy to ") + QDir::toNativeSeparators(fileName()));

        return false;
    }

    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    emit message(trUtf8("Copied to ") + QDir::toNativeSeparators(fileName()));

    return true;
}

void Hostlist::_load()
{
    QString filename = QDir::toNativeSeparators(fileName());
    int tempNumberOfGroups;
    int tempGroupSize;
    QString name;
    QString url;
    PingGroup *newPingGroup;
    DownloadGroup *newDownloadGroup;

    _clear();
    emit message(trUtf8("Loading hosts from %1").arg(filename));
    _version = value("HostListInfo/Version").toString();
    _results._hostlistVersion = _version;
    _comment = value("HostListInfo/Comment").toString();
    _results._hostlistComment = _comment;
    _contactUrl = value("HostListInfo/ContactURL").toString();
    _results._hostlistContactUrl = _contactUrl;
    tempNumberOfGroups = value("HostListInfo/NumberOfPingGroups").toInt();

    for(int i = 1; i <= tempNumberOfGroups; i++)
    {
        name = value(QString("PingGroup%1/Name").arg(i)).toString();
        newPingGroup = new PingGroup(name);
        tempGroupSize = value(QString("PingGroup%1/NumberOfHosts").arg(i)).toInt();

        for(int j = 1; j <= tempGroupSize; j++)
        {
            name = value(QString("PingGroup%1/%2/Name").arg(i).arg(j)).toString();
            url = value(QString("PingGroup%1/%2/URL").arg(i).arg(j)).toString();
            newPingGroup->_hosts.append(PingHost(name, url));
        }

        _results._pingGroups.append(*newPingGroup);
        delete newPingGroup;
    }

    tempNumberOfGroups = value("HostListInfo/NumberOfDownloadGroups").toInt();

    for(int i = 1; i <= tempNumberOfGroups; i++)
    {
        name = value(QString("DownloadGroup%1/Name").arg(i)).toString();
        newDownloadGroup = new DownloadGroup(name);
        tempGroupSize = value(QString("DownloadGroup%1/NumberOfHosts").arg(i)).toInt();

        for(int j = 1; j <= tempGroupSize; j++)
        {
            name = value(QString("DownloadGroup%1/%2/Name").arg(i).arg(j)).toString();
            url = value(QString("DownloadGroup%1/%2/URL").arg(i).arg(j)).toString();
            newDownloadGroup->_hosts.append(DownloadHost(name, url));
        }

        _results._downloadGroups.append(*newDownloadGroup);
        delete newDownloadGroup;
    }

    emit message(trUtf8("Hostlist version: %1").arg(_version));
    emit message(trUtf8("Hostlist comment: %1").arg(_comment));
    emit message(trUtf8("Hostlist contact URL: %1").arg(_contactUrl));
    emit message(trUtf8("%1 ping hosts in %2 groups were loaded").arg(_results._pingHostsTotal()).arg(_results._pingGroups.size()));
    emit message(trUtf8("%1 download hosts in %2 groups were loaded").arg(_results._downloadHostsTotal()).arg(_results._downloadGroups.size()));
}

bool Hostlist::initOk()
{
    QTemporaryFile tempFile;
    QString filename;

    tempFile.open();
    tempFile.close();
    filename = tempFile.fileName();

    if(!_exists())
    {
        emit message(trUtf8("Hostlist not found"));
        _restoreEmbeddedOk();
    }
    else
    {
        if(!_isValid(fileName()))
        {
            emit message(trUtf8("Error parsing %1. Not a valid hostlist").arg(fileName()));
            _restoreEmbeddedOk();
        }
    }

    // At this point, there is a _valid_ hostlist file in place, either preexisting or from embedded resources
    sync();
    _version = value("HostListInfo/Version").toString();

    if(_updateIsAvailable() && _downloadOk(filename) && _isValid(filename))
    {
        QFile::rename(fileName(), fileName() + ".bak");    // create a backup copy of the previous valid hostlist
        sync();
        emit message(trUtf8("Copying temporary downloaded hostlist to program settings directory"));
        tempFile.copy(fileName());
        QFile::setPermissions(fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
        sync();
    }

    // At this point, there is a _valid_ hostlist file in place, either preexisting or from embedded resources or from a download
    _load();

    return true;    // An assumption was made here: That, no matter what, restoring from embedded resources will always succeed and give a valid hostlist file. That _may not_ always be the case!
}

/*
void Hostlist::startPingTest()
{
    int count = pingGroups.size();

    if(!count)
    {
        emit sigPingTestFinished();
        return;
    }

    pingGroupsQueue.clear();

    for(int i = 0; i < count; i++)
    {
        pingGroupsQueue.enqueue(&pingGroups[i]);
    }

    connect(pingGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextPingGroup()));
    pingGroupsQueue.first()->startPing();

}

void Hostlist::stopPingTest()
{
    for(int i = 0; i < pingGroups.size(); i++)
    {
        if(pingGroups[i].isRunning())
        {
            pingGroups[i].stopPing();
            disconnect(&pingGroups[i], SIGNAL(sigFinished()), this, SLOT(slotNextPingGroup()));
        }
    }
}

void Hostlist::startParallelDownloadTest()
{
    int count = downloadGroups.size();

    if(!count)
    {
        emit sigFinished();
        return;
    }

    downloadGroupsQueue.clear();

    for(int i = 0; i < count; i++)
    {
        downloadGroupsQueue.enqueue(&downloadGroups[i]);
    }

    connect(downloadGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextDownloadGroup()));
    downloadGroupsQueue.first()->startParallelDownload();
    emit sigShowProgressBar(0, downloadTestSecs * 1000);
    progress = 0;
    emit sigProgress(0);
    progressTimer.start(downloadTestSecs * 25);    // Arbitrary value, chosen because for the default downloadTestSecs (10) gives an interval of 250msec for the progress bar updates
}

void Hostlist::stopParallelDownloadTest()
{
    progressTimer.stop();
    emit sigHideProgressBar();

    for(int i = 0; i < downloadGroups.size(); i++)
    {
        if(downloadGroups[i].isRunning())
        {
            downloadGroups[i].stopParallelDownload();
            disconnect(&downloadGroups[i], SIGNAL(sigFinished()), this, SLOT(slotNextDownloadGroup()));
        }
    }
}

void Hostlist::slotNewProgress()
{ progress += progressTimer.interval(); emit sigProgress(progress); }

void Hostlist::slotNextPingGroup()
{
    disconnect(pingGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextPingGroup()));
    emit sigResult(trUtf8("Group sum:     %1\n"
                          "Group average: %2\n").arg(pingGroupsQueue.first()->getPingSumAsString()).arg(pingGroupsQueue.first()->getPingAsString()));
    pingGroupsQueue.dequeue()->sort();


    if(pingGroupsQueue.size())
    {
        connect(pingGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextPingGroup()));
        pingGroupsQueue.first()->startPing();

        return;
    }

    if(TESTMODE & TestMode::Download)
    {
        startParallelDownloadTest();

        return;
    }

    emit sigFinished();
}

void Hostlist::slotNextDownloadGroup()
{
    disconnect(downloadGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextDownloadGroup()));
    downloadGroupsQueue.dequeue();

    if(downloadGroupsQueue.size())
    {
        connect(downloadGroupsQueue.first(), SIGNAL(sigFinished()), this, SLOT(slotNextDownloadGroup()));
        downloadGroupsQueue.first()->startParallelDownload();
        progress = 0;
        emit sigProgress(0);
        progressTimer.start(downloadTestSecs * 25);    // Arbitrary value, chosen because for the default downloadTestSecs (10) gives an interval of 250msec for the progress bar updates
    }
    else
    {
        progressTimer.stop();
        emit sigHideProgressBar();
        emit sigFinished();    // The download test is the last test, so we don't need a specific sigDownloadTestFinished signal, the generic one will do
    }
}
*/
