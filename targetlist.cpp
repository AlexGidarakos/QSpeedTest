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
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, PROGRAMCOMPANY, PROGRAMNAME);
    version = QString(NULL);
    numberOfGroups = 0;
    comment = QString(NULL);
    contactURL = QString(NULL);
    numberOfTargets = 0;
}


TargetList::~TargetList()
{
    delete settings;
}


void TargetList::purge()
{
    version = QString(NULL);
    numberOfGroups = 0;
    comment = QString(NULL);
    contactURL = QString(NULL);
    numberOfTargets = 0;
    groups.clear();
}


bool TargetList::isUpdateAvailable()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    QString remoteVersion;

    emit message(trUtf8("Checking online for an updated version of the target list..."));

    download = manager.get(QNetworkRequest(QUrl(TARGETLISTVERSIONURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
        emit message(trUtf8("Update site unreachable."));
        delete download;
        return false;
    }

    if((remoteVersion = download->readLine()) > version)
    {
        emit message(trUtf8("Update available, remote target list version: v%1").arg(remoteVersion));
        delete download;
        return true;
    }

    emit message(trUtf8("Your local target list is already up to date."));
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

    emit message(trUtf8("Downloading new target list..."));

    download = manager.get(QNetworkRequest(QUrl(TARGETLISTURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
        emit message(trUtf8("Error: Download failed."));
        delete download;
        return false;
    }

    emit message(trUtf8("Download complete. Writing to file..."));

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
        emit message(trUtf8("Wrote %1 bytes.").arg(file.size()));
        delete download;
        return true;
    }

    emit message(trUtf8("Error: Could not open file %1 for write operation. Please check your folder access permissions.").arg(settings->fileName()));
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

    emit message(trUtf8("Loading target list from %1 ...").arg(targetListPath));

    if(!QFile::exists(targetListPath))
    {
        emit message(trUtf8("Error: File not found."));
        return false;
    }

    if(!settings->childGroups().contains("TargetListInfo"))
    {
        emit message(trUtf8("Error: Section [TargetListInfo] missing."));
        return false;
    }

    version = settings->value("TargetListInfo/Version", NULL).toString();

    if(version == QString(NULL))
    {
        emit message(trUtf8("Error: Key \"Version\" missing from section [TargetListInfo]."));
        return false;
    }

    comment = settings->value("TargetListInfo/Comment", NULL).toString();
    contactURL = settings->value("TargetListInfo/ContactURL", NULL).toString();
    tempNumberOfGroups = settings->value("TargetListInfo/NumberOfGroups", NULL).toInt();

    if(tempNumberOfGroups < 1)
    {
        emit message(trUtf8("Error: Key \"NumberOfGroups\" missing from section [TargetListInfo] or has incompatible value."));
        return false;
    }

    for(int i = 1; i <= tempNumberOfGroups; i++)
    {
        newGroup.size = 0;
        newGroup.targets.clear();

        if(!settings->childGroups().contains(QString("Group%1").arg(i)))
        {
            emit message(trUtf8("Error: Section [Group%1] missing").arg(i));
            return false;
        }

        newGroup.name = settings->value(QString("Group%1/Name").arg(i), NULL).toString();

        if(newGroup.name == QString(NULL))
        {
            emit message(trUtf8("Error: Key \"Name\" missing from section [Group%1] or has incompatible value.").arg(i));
            return false;
        }

        tempGroupSize = settings->value(QString("Group%1/NumberOfTargets").arg(i), NULL).toInt();

        if(tempGroupSize < 1)
        {
            emit message(trUtf8("Error: Key \"NumberOfTargets\" missing from section [Group%1] or has incompatible value.").arg(i));
            return false;
        }

        for(int j = 1; j <= tempGroupSize; j++)
        {
            newTarget.setName(settings->value(QString("Group%1/%2/Name").arg(i).arg(j), NULL).toString());

            if(newTarget.getName() == QString(NULL))
            {
                emit message(trUtf8("Error: Key \"%2\\Name\" missing from section [Group%1] or has incompatible value.").arg(i).arg(j));
                return false;
            }

            newTarget.setAddress(settings->value(QString("Group%1/%2/Address").arg(i).arg(j), NULL).toString());

            if(newTarget.getAddress() == QString(NULL))
            {
                emit message(trUtf8("Error: Key \"%2\\Address\" missing from section [Group%1] or is not a proper IP or URL address.").arg(i).arg(j));
                return false;
            }

            if(!QUrl(newTarget.getAddress()).isValid())
            {
                emit message(trUtf8("Error: Key \"%2\\Address\" in section [Group%1] is not a proper IP or URL address.").arg(i).arg(j));
                return false;
            }

            newGroup.targets.append(newTarget);
            newGroup.size++;
        }

        groups.append(newGroup);
        numberOfGroups++;
        numberOfTargets += newGroup.size;
    }

    emit message(trUtf8("%1 target in %2 groups were successfully loaded.\nTarget list version: %3\nTarget list comment: %4\nTarget list contact URL: %5\n").arg(numberOfTargets).arg(numberOfGroups).arg(version).arg(comment).arg(contactURL));
    return true;
}


bool TargetList::restoreEmbedded()
{
    QString filename(settings->fileName());
    QFileInfo info(filename);
    QFile file(filename);
    QFile embedded(":/QSpeedTest.ini");

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

    emit message(trUtf8("Extracting a default target list from embedded resources ..."));

    if(embedded.copy(filename))
    {
        emit message(trUtf8("Successfully copied to %1.").arg(QDir::toNativeSeparators(filename)));
        return true;
    }

    emit message(trUtf8("Error: Could not create file %1.").arg(QDir::toNativeSeparators(filename)));
    return false;
}


bool TargetList::init()
{
    if(load())
    {
        if(isUpdateAvailable())
        {
            int reply = QMessageBox::question(NULL, trUtf8("Update available"), trUtf8("An updated version of the target list is available online.\n\nWould you like to update now?"), QMessageBox::Yes, QMessageBox::No);

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
                emit message(trUtf8("Update aborted."));
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
