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

#include "preferences.h"
#include "externs.h"
#include <QtCore/QDir>
#include <QtCore/QUrl>

void Preferences::init()
{
    QDir dir(QFileInfo(fileName()).absoluteDir());

    TESTMODE = TESTMODEDEFAULT;
    UPDATECHECKTIMEOUTSECS = UPDATECHECKTIMEOUTSECSDEFAULT;
    HOSTLISTURL = HOSTLISTURLDEFAULT;
    PINGSPERHOST = PINGSPERHOSTDEFAULT;
    PINGTHREADS = PINGTHREADSDEFAULT;
    DOWNLOADTESTSECS = DOWNLOADTESTSECSDEFAULT;
    PINGTIMEOUTSECS = PINGTIMEOUTSECSDEFAULT;

    if(!dir.exists())    // If necessary, create subdir for the program's INI files
    {
        emit message(trUtf8("Creating program preferences directory ") + QDir::toNativeSeparators(dir.path()));
        dir.mkpath(dir.path());
    }

    if(_exists() || _restoreEmbeddedOk())    // First, check for a local existing preferences file. If not found, then try to restore one from the library's embedded resources. If either method succeeds, try to load the preferences file
    {
        if(!_loadOk())
        {
            emit message(trUtf8("Error parsing %1. Not a valid preferences file").arg(QDir::toNativeSeparators(fileName())));
        }

        return;
    }

    // At this point, no method gave a readable preferences file. The application should use the default global values provided by the library as starting values for preferences
}

bool Preferences::_loadOk()
{
    TestMode::Mode testMode;
    quint8 pingsPerHost;
    quint8 pingThreads;
    quint8 pingTimeoutSecs;
    quint8 downloadTestSecs;
    QString hostlistUrl;

    sync();

    if(!childGroups().contains("Preferences")) return false;

    beginGroup("Preferences");
    testMode = (TestMode::Mode) value("TestMode", 0).toUInt();

    if(!(testMode == TestMode::Info || testMode == TestMode::Ping || testMode == TestMode::Download || testMode == TestMode::All)) return false;

    pingsPerHost = value("PingsPerHost", 0).toUInt();

    if(pingsPerHost < 1 || pingsPerHost > 10) return false;

    pingThreads = value("PingThreads", 0).toUInt();

    if(pingThreads < 1 || pingThreads > 8) return false;

    pingTimeoutSecs = value("PingTimeoutSecs", 0).toUInt();

    if(pingTimeoutSecs < 1 || pingTimeoutSecs > 4) return false;

    downloadTestSecs = value("DownloadTestSecs", 0).toUInt();

    if(downloadTestSecs < 5 || downloadTestSecs > 30) return false;

    hostlistUrl = value("HostlistUrl", QString("")).toString();

    if(hostlistUrl.isEmpty() || !QUrl(hostlistUrl).isValid()) return false;

    // At this point, preferences INI file was valid. Load its preferences
    TESTMODE = testMode;
    PINGSPERHOST = pingsPerHost;
    PINGTHREADS = pingThreads;
    PINGTIMEOUTSECS = pingTimeoutSecs;
    DOWNLOADTESTSECS = downloadTestSecs;
    HOSTLISTURL = hostlistUrl;
    emit message(trUtf8("Loaded preferences from ") + QDir::toNativeSeparators(fileName()));

    return true;
}

bool Preferences::_restoreEmbeddedOk()
{
    QFile file(fileName());
    QFile embedded(":/libspeedtest/default.preferences.ini");

    if(file.exists()) QFile::rename(fileName(), fileName() + ".wrong");

    emit message(trUtf8("Extracting default preferences file from embedded resources"));

    if(!embedded.copy(fileName()))
    {
        emit message(trUtf8("Error copying to ") + QDir::toNativeSeparators(fileName()));
        emit message(trUtf8("Check directory access permissions"));

        return false;
    }

    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    emit message(trUtf8("Copied to ") + QDir::toNativeSeparators(fileName()));
    sync();

    return true;
}
