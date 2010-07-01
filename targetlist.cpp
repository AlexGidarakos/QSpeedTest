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
#include "externs.h"
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QEventLoop>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>


TargetList::TargetList(QObject *parent) : QObject(parent)
{
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, PROJECTNAME, PROGRAMNAME.toLower());
    version.clear();
    numberOfGroups = 0;
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
}


void TargetList::purge()
{
    version.clear();
    numberOfGroups = 0;
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
    groups.clear();
    fileHostsDomestic.clear();
    fileHostsInternational.clear();
    settings->sync();
}


bool TargetList::isUpdateAvailable()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString remoteVersion;

    emit logMessage(trUtf8("Checking online for an updated version of the target list"));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(TARGETLISTUPDATECHECKURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        emit logMessage(trUtf8("Update site unreachable"));
        delete download;
        return false;
    }

    if((remoteVersion = download->readLine()) > version)
    {
        emit logMessage(trUtf8("Update available, remote target list version: %1").arg(remoteVersion));
        delete download;
        return true;
    }

    emit logMessage(trUtf8("Your local target list is already up to date"));
    delete download;
    return false;
}


bool TargetList::downloadList()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString fileName(settings->fileName());
    QFile file(fileName);
    QFileInfo info(fileName);
    QTextStream out;

    emit logMessage(trUtf8("Downloading new target list"));

    download = manager.get(QNetworkRequest(QUrl(TARGETLISTURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
        emit logMessage(trUtf8("Error: Download failed"));
        delete download;
        return false;
    }

    emit logMessage(trUtf8("Download complete. Writing to file"));

    if(!QDir().exists(info.absolutePath()))
    {
        QDir().mkdir(info.absolutePath());
        QFile::setPermissions(info.absolutePath(), QFile::WriteUser);
    }
    else
    {
        if(file.exists())
        {
            QFile::rename(settings->fileName(), settings->fileName() + ".bak");
        }
    }

    if(file.open(QFile::WriteOnly))
    {
        out.setDevice(&file);
        out << download->readAll();
        file.close();
        emit logMessage(trUtf8("Wrote %1 bytes").arg(file.size()));
        delete download;
        return true;
    }

    emit logMessage(trUtf8("Error: Could not open file %1 for write operation").arg(settings->fileName()));
    emit logMessage(trUtf8("Please check your folder access permissions."));
    delete download;
    return false;
}


bool TargetList::load()
{
    QString targetListPath = QDir::toNativeSeparators(settings->fileName());
    int tempNumberOfGroups;
    int tempGroupSize;
    TargetGroup newGroup;
    Target newTarget;
    FileHost newFileHost;

    purge();
    emit logMessage(trUtf8("Loading targets from %1").arg(targetListPath));

    if(!QFile::exists(targetListPath))
    {
        emit logMessage(trUtf8("Error: File not found"));
        return false;
    }

    if(!settings->childGroups().contains("TargetListInfo"))
    {
        emit logMessage(trUtf8("Error: Section [TargetListInfo] missing"));
        return false;
    }

    version = settings->value("TargetListInfo/Version", QString("")).toString();

    if(version.isEmpty())
    {
        emit logMessage(trUtf8("Error: Key \"Version\" missing from section [TargetListInfo]"));
        return false;
    }

    comment = settings->value("TargetListInfo/Comment", QString("")).toString();
    contactUrl = settings->value("TargetListInfo/ContactURL", QString("")).toString();
    tempNumberOfGroups = settings->value("TargetListInfo/NumberOfPingGroups", QString("")).toInt();

    if(tempNumberOfGroups < 1)
    {
        emit logMessage(trUtf8("Error: Key \"NumberOfPingGroups\" missing from section [TargetListInfo] or has incompatible value"));
        return false;
    }

    for(int i = 1; i <= tempNumberOfGroups; i++)
    {
        newGroup.setSize(0);
        newGroup.targets.clear();

        if(!settings->childGroups().contains(QString("PingGroup%1").arg(i)))
        {
            emit logMessage(trUtf8("Error: Section [PingGroup%1] missing").arg(i));
            return false;
        }

        newGroup.setName(settings->value(QString("PingGroup%1/Name").arg(i), QString("")).toString());

        if(newGroup.getName().isEmpty())
        {
            emit logMessage(trUtf8("Error: Key \"Name\" missing from section [PingGroup%1] or has incompatible value").arg(i));
            return false;
        }

        tempGroupSize = settings->value(QString("PingGroup%1/NumberOfTargets").arg(i), QString("")).toInt();

        if(tempGroupSize < 1)
        {
            emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [PingGroup%1] or has incompatible value").arg(i));
            return false;
        }

        for(int j = 1; j <= tempGroupSize; j++)
        {
            newTarget.setName(settings->value(QString("PingGroup%1/%2/Name").arg(i).arg(j), QString("")).toString());

            if(newTarget.getName().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Name\" missing from section [PingGroup%1] or has incompatible value").arg(i).arg(j));
                return false;
            }

            newTarget.setAddress(settings->value(QString("PingGroup%1/%2/Address").arg(i).arg(j), QString("")).toString());

            if(newTarget.getAddress().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" missing from section [PingGroup%1] or is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            if(!QUrl(newTarget.getAddress()).isValid())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" in section [PingGroup%1] is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            newGroup.addTarget(newTarget);
        }

        groups.append(newGroup);
        numberOfGroups++;
        numberOfTargets += newGroup.getSize();
    }

    foreach(QString location, QList<QString>() << "Domestic" << "International")
    {
        if(!settings->childGroups().contains("FileHosts" + location))
        {
            emit logMessage(trUtf8("Error: Section [FileHosts%1] missing").arg(location));
            return false;
        }

        tempGroupSize = settings->value(QString("FileHosts%1/NumberOfTargets").arg(location), QString("")).toInt();

        if(tempGroupSize < 1)
        {
            emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [FileHosts%1] or has incompatible value").arg(location));
            return false;
        }

        for(int i = 1; i <= tempGroupSize; i++)
        {
            newFileHost.setName(settings->value(QString("FileHosts%1/%2/Name").arg(location).arg(i), QString("")).toString());

            if(newFileHost.getName().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%1\\Name\" missing from section [FileHosts%2] or has incompatible value").arg(i).arg(location));
                return false;
            }

            newFileHost.setUrl(QUrl(settings->value(QString("FileHosts%1/%2/URL").arg(location).arg(i), QString("")).toString()));

            if(newFileHost.getUrl().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%1\\URL\" missing from section [FileHosts%2] or is not a proper file URL").arg(i).arg(location));
                return false;
            }

            if(!newFileHost.getUrl().isValid())
            {
                emit logMessage(trUtf8("Error: Key \"%1\\URL\" in section [FileHosts%2] is not a proper file URL").arg(i).arg(location));
                return false;
            }

            if(location == QString("Domestic"))
            {
                fileHostsDomestic.append(newFileHost);
            }
            else
            {
                fileHostsInternational.append(newFileHost);
            }
        }
    }

    emit logMessage(trUtf8("Target list version: %1").arg(version));
    emit logMessage(trUtf8("Target list comment: %1").arg(comment));
    emit logMessage(trUtf8("Target list contact URL: %1").arg(contactUrl));
    emit logMessage(trUtf8("%1 ping targets in %2 groups were loaded").arg(numberOfTargets).arg(numberOfGroups));
    emit logMessage(trUtf8("%1 domestic download test targets were loaded").arg(fileHostsDomestic.size()));
    emit logMessage(trUtf8("%1 international download test targets were loaded").arg(fileHostsInternational.size()));
    return true;
}


bool TargetList::restoreEmbedded()
{
    QString filename(settings->fileName());
    QFileInfo info(filename);
    QFile file(filename);
    QFile embedded(":/qspeedtest.ini");

    if(!QDir().exists(info.absolutePath()))
    {
        QDir().mkdir(info.absolutePath());
    }
    else
    {
        if(file.exists())
        {
            QFile::rename(settings->fileName(), settings->fileName() + ".wrong");
        }
    }

    emit logMessage(trUtf8("Extracting a default target list from embedded resources"));

    if(embedded.copy(filename))
    {
        emit logMessage(trUtf8("Successfully copied to %1").arg(QDir::toNativeSeparators(filename)));
        return true;
    }

    emit logMessage(trUtf8("Error: Could not create file %1").arg(QDir::toNativeSeparators(filename)));
    return false;
}


bool TargetList::init()
{
    load();    // Try to load current target list
    if(isUpdateAvailable() && downloadList()) {}    // Check online for updated target list and if found, download it

    if(load())    // If current target list loads with no errors...
    {
        return true;    // Enable Start button
    }

    if(restoreEmbedded())    // No correct target list file loaded up to this point. Try to restore a target list from embedded resources. If successful...
    {
        if(load())    // Try to load it and return true if successful
        {
            return true;
        }
    }

    return false;    // No method gave a readable target list, return false so Start button stays disabled
}
