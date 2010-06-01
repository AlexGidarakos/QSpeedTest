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
#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTextStream>


TargetList::TargetList(QObject *parent) : QObject(parent)
{
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, PROGRAMNAME, PROGRAMNAME.toLower());
    version.clear();
    numberOfGroups = 0;
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
}


TargetList::~TargetList()
{
    delete settings;
}


void TargetList::purge()
{
    version.clear();
    numberOfGroups = 0;
    comment.clear();
    contactUrl.clear();
    numberOfTargets = 0;
    groups.clear();
}


bool TargetList::isUpdateAvailable()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString remoteVersion;

    emit logMessage(trUtf8("Checking online for an updated version of the target list"));
    download = manager.get(QNetworkRequest(QUrl(TARGETLISTUPDATECHECKURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
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
    }
    else
    {
        if(file.exists())
        {
            QFile::remove(settings->fileName());
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
    tempNumberOfGroups = settings->value("TargetListInfo/NumberOfGroups", QString("")).toInt();

    if(tempNumberOfGroups < 1)
    {
        emit logMessage(trUtf8("Error: Key \"NumberOfGroups\" missing from section [TargetListInfo] or has incompatible value"));
        return false;
    }

    for(int i = 1; i <= tempNumberOfGroups; i++)
    {
        newGroup.setSize(0);
        newGroup.targets.clear();

        if(!settings->childGroups().contains(QString("Group%1").arg(i)))
        {
            emit logMessage(trUtf8("Error: Section [Group%1] missing").arg(i));
            return false;
        }

        newGroup.setName(settings->value(QString("Group%1/Name").arg(i), QString("")).toString());

        if(newGroup.getName().isEmpty())
        {
            emit logMessage(trUtf8("Error: Key \"Name\" missing from section [Group%1] or has incompatible value").arg(i));
            return false;
        }

        tempGroupSize = settings->value(QString("Group%1/NumberOfTargets").arg(i), QString("")).toInt();

        if(tempGroupSize < 1)
        {
            emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [Group%1] or has incompatible value").arg(i));
            return false;
        }

        for(int j = 1; j <= tempGroupSize; j++)
        {
            newTarget.setName(settings->value(QString("Group%1/%2/Name").arg(i).arg(j), QString("")).toString());

            if(newTarget.getName().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Name\" missing from section [Group%1] or has incompatible value").arg(i).arg(j));
                return false;
            }

            newTarget.setAddress(settings->value(QString("Group%1/%2/Address").arg(i).arg(j), QString("")).toString());

            if(newTarget.getAddress().isEmpty())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" missing from section [Group%1] or is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            if(!QUrl(newTarget.getAddress()).isValid())
            {
                emit logMessage(trUtf8("Error: Key \"%2\\Address\" in section [Group%1] is not a proper IP address or URL").arg(i).arg(j));
                return false;
            }

            newGroup.addTarget(newTarget);
        }

        groups.append(newGroup);
        numberOfGroups++;
        numberOfTargets += newGroup.getSize();
    }

    if(!settings->childGroups().contains("DownloadTest"))
    {
        emit logMessage(trUtf8("Error: Section [DownloadTest] missing"));
        return false;
    }

    tempGroupSize = settings->value(QString("DownloadTest/NumberOfTargets"), QString("")).toInt();

    if(tempGroupSize < 1)
    {
        emit logMessage(trUtf8("Error: Key \"NumberOfTargets\" missing from section [DownloadTest] or has incompatible value"));
        return false;
    }

    for(int i = 1; i <= tempGroupSize; i++)
    {
        newFileHost.setName(settings->value(QString("DownloadTest/%1/Name").arg(i), QString("")).toString());

        if(newFileHost.getName().isEmpty())
        {
            emit logMessage(trUtf8("Error: Key \"%1\\Name\" missing from section [DownloadTest] or has incompatible value").arg(i));
            return false;
        }

        newFileHost.setUrl(QUrl(settings->value(QString("DownloadTest/%1/URL").arg(i), QString("")).toString()));

        if(newFileHost.getUrl().isEmpty())
        {
            emit logMessage(trUtf8("Error: Key \"%1\\URL\" missing from section [DownloadTest] or is not a proper file URL").arg(i));
            return false;
        }

        if(!newFileHost.getUrl().isValid())
        {
            emit logMessage(trUtf8("Error: Key \"%1\\URL\" in section [DownloadTest] is not a proper file URL").arg(i));
            return false;
        }

        fileHosts.append(newFileHost);
    }

    emit logMessage(trUtf8("Target list version: %1").arg(version));
    emit logMessage(trUtf8("Target list comment: %1").arg(comment));
    emit logMessage(trUtf8("Target list contact URL: %1").arg(contactUrl));
    emit logMessage(trUtf8("%1 ping targets in %2 groups were successfully loaded").arg(numberOfTargets).arg(numberOfGroups));
    emit logMessage(trUtf8("%1 download test targets were successfully loaded").arg(fileHosts.size()));
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
            QFile::remove(settings->fileName());
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
    if(load())
    {
        if(isUpdateAvailable())
        {
            int reply = QMessageBox::question(NULL, trUtf8("Target list update available"), trUtf8("An updated version of the target list is available online.\n\nWould you like to update now?"), QMessageBox::Yes, QMessageBox::No);

            if(reply == QMessageBox::Yes)
            {
                if(downloadList())
                {
                    purge();
                    settings->sync();

                    if(load())
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                emit logMessage(trUtf8("Update aborted"));
            }
        }

        return true;
    }

    if(downloadList())
    {
        purge();
        settings->sync();

        if(load())
        {
            return true;
        }
    }

    if(restoreEmbedded())
    {
        purge();
        settings->sync();

        if(load())
        {
            return true;
        }
    }

    return false;
}
