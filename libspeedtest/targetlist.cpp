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


#include "targetlist.h"
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QEventLoop>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>


QMutex LIBSPEEDTEST_EXPORT MUTEX;
bool LIBSPEEDTEST_EXPORT STOPBENCHMARK = false;


TargetList::TargetList(QString projectName, QString programName, QObject *parent) : QObject(parent) {
    iniTargets = new QSettings(QSettings::IniFormat, QSettings::UserScope, projectName, programName.toLower() + ".targets");
    version.clear();
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
}


void TargetList::purge() {
    version.clear();
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
    groups.clear();
    fileHostsDomestic.clear();
    fileHostsInternational.clear();
    iniTargets->sync();
}


bool TargetList::exists() {
    if(QFile::exists(iniTargets->fileName()))
        return true;

    return false;
}


bool TargetList::updateIsAvailable() {
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString remoteVersion;

    emit logMessage(trUtf8("Checking online for an updated version of the target list"));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(TARGETLISTURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error()) {
        download->abort();
        emit logMessage(trUtf8("Update site unreachable"));
        delete download;
        return false;
    }

    download->readLine();

    if((remoteVersion = download->readLine().trimmed().right(12)) > version) {
        emit logMessage(trUtf8("Update available: local target list version is %1, remote is %2").arg(version).arg(remoteVersion));
        delete download;
        return true;
    }

    emit logMessage(trUtf8("Your local target list is already up to date"));
    delete download;
    return false;
}


bool TargetList::downloadList() {
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QFile file(iniTargets->fileName());
    QTextStream out;

    emit logMessage(trUtf8("Downloading new target list"));

    download = manager.get(QNetworkRequest(QUrl(TARGETLISTURL)));    // download pointer now does not require manual deleting!
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error()) {
        emit logMessage(trUtf8("Error: Download failed"));
        return false;
    }

    emit logMessage(trUtf8("Download complete. Writing to file"));

    if(file.exists())
        QFile::rename(iniTargets->fileName(), iniTargets->fileName() + ".bak");

    if(file.open(QIODevice::WriteOnly)) {
        out.setDevice(&file);
        out << download->readAll();
        file.close();
        emit logMessage(trUtf8("Wrote %1 bytes").arg(file.size()));
        return true;
    }

    emit logMessage(trUtf8("Error: Could not open file %1 for write operation").arg(iniTargets->fileName()));
    emit logMessage(trUtf8("Please check your folder access permissions."));
    return false;
}


bool TargetList::load() {
    QString iniTargetsPath = QDir::toNativeSeparators(iniTargets->fileName());
    int tempNumberOfGroups;
    int tempGroupSize;
    PingGroup newGroup;
    PingTarget newPingTarget;
    DownloadTarget newDownloadTarget;

    purge();
    emit logMessage(trUtf8("Loading targets from %1").arg(iniTargetsPath));

    if(!QFile::exists(iniTargets->fileName())) {
        emit logMessage(trUtf8("Error: File not found"));
        return false;
    }

    if(!iniTargets->childGroups().contains("TargetListInfo")) {
        emit logMessage(trUtf8("Error: Section [TargetListInfo] missing"));
        return false;
    }

    version = iniTargets->value("TargetListInfo/Version", QString("")).toString();

    if(version.isEmpty()) {
        emit logMessage(trUtf8("Error: Key \"Version\" missing from section [TargetListInfo]"));
        return false;
    }

    comment = iniTargets->value("TargetListInfo/Comment", QString("")).toString();
    contactUrl = iniTargets->value("TargetListInfo/ContactURL", QString("")).toString();
    tempNumberOfGroups = iniTargets->value("TargetListInfo/NumberOfPingGroups", QString("")).toInt();

    if(tempNumberOfGroups < 1) {
        emit logMessage(trUtf8("Error: Key \"NumberOfPingGroups\" missing from section [TargetListInfo] or has incompatible value"));
        return false;
    }

    for(int i = 1; i <= tempNumberOfGroups; i++) {
        newGroup.setSize(0);
        newGroup.targets.clear();

        if(!iniTargets->childGroups().contains(QString("PingGroup%1").arg(i))) {
            emit logMessage(trUtf8("Error: Section [PingGroup%1] missing").arg(i));
            return false;
        }

        newGroup.setName(iniTargets->value(QString("PingGroup%1/Name").arg(i), QString("")).toString());

        if(newGroup.getName().isEmpty()) {
            emit logMessage(trUtf8("Error: Key \"Name\" missing from section [PingGroup%1] or has incompatible value").arg(i));
            return false;
        }

        tempGroupSize = iniTargets->value(QString("PingGroup%1/NumberOfTargets").arg(i), QString("")).toInt();

        if(tempGroupSize < 1) {
            emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [PingGroup%1] or has incompatible value").arg(i));
            return false;
        }

        for(int j = 1; j <= tempGroupSize; j++) {
            newPingTarget.setName(iniTargets->value(QString("PingGroup%1/%2/Name").arg(i).arg(j), QString("")).toString());

            if(newPingTarget.getName().isEmpty()) {
                emit logMessage(trUtf8("Error: Key \"%2\\Name\" missing from section [PingGroup%1] or has incompatible value").arg(i).arg(j));
                return false;
            }

            newPingTarget.setAddress(iniTargets->value(QString("PingGroup%1/%2/Address").arg(i).arg(j), QString("")).toString());

            if(newPingTarget.getAddress().isEmpty()) {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" missing from section [PingGroup%1] or is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            if(!QUrl(newPingTarget.getAddress()).isValid()) {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" in section [PingGroup%1] is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            newGroup.addTarget(newPingTarget);
        }

        groups.append(newGroup);
        numberOfTargets += newGroup.getSize();
    }

    foreach(QString location, QList<QString>() << "Domestic" << "International") {
        if(!iniTargets->childGroups().contains("FileHosts" + location)) {
            emit logMessage(trUtf8("Error: Section [FileHosts%1] missing").arg(location));
            return false;
        }

        tempGroupSize = iniTargets->value(QString("FileHosts%1/NumberOfTargets").arg(location), QString("")).toInt();

        if(tempGroupSize < 1) {
            emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [FileHosts%1] or has incompatible value").arg(location));
            return false;
        }

        for(int i = 1; i <= tempGroupSize; i++) {
            newDownloadTarget.setName(iniTargets->value(QString("FileHosts%1/%2/Name").arg(location).arg(i), QString("")).toString());

            if(newDownloadTarget.getName().isEmpty()) {
                emit logMessage(trUtf8("Error: Key \"%1\\Name\" missing from section [FileHosts%2] or has incompatible value").arg(i).arg(location));
                return false;
            }

            newDownloadTarget.setUrl(QUrl(iniTargets->value(QString("FileHosts%1/%2/URL").arg(location).arg(i), QString("")).toString()));

            if(newDownloadTarget.getUrl().isEmpty()) {
                emit logMessage(trUtf8("Error: Key \"%1\\URL\" missing from section [FileHosts%2] or is not a proper file URL").arg(i).arg(location));
                return false;
            }

            if(!newDownloadTarget.getUrl().isValid()) {
                emit logMessage(trUtf8("Error: Key \"%1\\URL\" in section [FileHosts%2] is not a proper file URL").arg(i).arg(location));
                return false;
            }

            if(location == QString("Domestic"))
                fileHostsDomestic.append(newDownloadTarget);
            else
                fileHostsInternational.append(newDownloadTarget);
        }
    }

    emit logMessage(trUtf8("Target list version: %1").arg(version));
    emit logMessage(trUtf8("Target list comment: %1").arg(comment));
    emit logMessage(trUtf8("Target list contact URL: %1").arg(contactUrl));
    emit logMessage(trUtf8("%1 ping targets in %2 groups were loaded").arg(numberOfTargets).arg(groups.size()));
    emit logMessage(trUtf8("%1 domestic download test targets were loaded").arg(fileHostsDomestic.size()));
    emit logMessage(trUtf8("%1 international download test targets were loaded").arg(fileHostsInternational.size()));
    return true;
}


bool TargetList::restoreEmbedded() {
    QFile file(iniTargets->fileName());
    QFile embedded(":/default.targets.ini");

    if(file.exists())
        QFile::rename(iniTargets->fileName(), iniTargets->fileName() + ".wrong");

    emit logMessage(trUtf8("Extracting a default target list from embedded resources"));

    if(embedded.copy(iniTargets->fileName())) {
        emit logMessage(trUtf8("Successfully copied to %1").arg(QDir::toNativeSeparators(iniTargets->fileName())));
        return true;
    }

    emit logMessage(trUtf8("Error: Could not create file %1").arg(QDir::toNativeSeparators(iniTargets->fileName())));
    return false;
}


bool TargetList::init() {
    QDir dir(QFileInfo(iniTargets->fileName()).absoluteDir());

    if(exists())
        load();    // Try to load current target list
    else
        if(!dir.exists())
            dir.mkpath(dir.path());

    if(updateIsAvailable())
        downloadList();    // Check online for updated target list and if found, download it

    if(load())    // If current target list loads with no errors...
        return true;    // Enable Start button

    if(restoreEmbedded())    // No correct target list file loaded up to this point. Try to restore a target list from embedded resources. If successful...
        if(load())    // Try to load it and return true if successful
            return true;

    return false;    // No method gave a readable target list, return false so Start button stays disabled
}


qint64 TargetList::getBytesDownloaded(QList<DownloadTarget> &targets) const {
    qint64 sum = 0;

    foreach(DownloadTarget target, targets)
        sum += target.getBytesDownloaded();

    return sum;
}
