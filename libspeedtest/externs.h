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
along with QSpeedTest. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EXTERNS_H
#define EXTERNS_H
#include "libspeedtest_global.h"
#include <QtCore/QString>

namespace TestMode
{
    enum Mode
    {
        Info = 0x1,
        Ping = 0x2,
        Download = 0x4,
        All = 0x7
    };
}

namespace ReportFormat
{
    enum Format
    {
        PlainText = 0x1,
        BbCode,
        Html
    };
}

namespace SpeedUnit
{
    enum Unit
    {
        bps = 0x1,
        Bps,
        Kbps,
        KBps,
        Mbps,
        MBps
    };
}

extern const QString LIBSPEEDTEST_EXPORT PROJECTNAME;
extern const QString LIBSPEEDTEST_EXPORT PROJECTVERSION;
extern QString LIBSPEEDTEST_EXPORT PROGRAMNAME;
extern const QString LIBSPEEDTEST_EXPORT PROJECTURL;
extern const QString LIBSPEEDTEST_EXPORT PROJECTDOWNLOADURL;
extern const QString LIBSPEEDTEST_EXPORT PROJECTDISCUSSURL;
extern const QString LIBSPEEDTEST_EXPORT PROJECTVERSIONURL;
extern const QString LIBSPEEDTEST_EXPORT PROJECTAUTHOR;
extern const QString LIBSPEEDTEST_EXPORT PROJECTCOMPANY;
extern const QString LIBSPEEDTEST_EXPORT PROJECTCOMPANYURL;
extern const quint8 LIBSPEEDTEST_EXPORT UPDATECHECKTIMEOUTSECSDEFAULT;
extern quint8 LIBSPEEDTEST_EXPORT UPDATECHECKTIMEOUTSECS;
extern const TestMode::Mode LIBSPEEDTEST_EXPORT TESTMODEDEFAULT;
extern TestMode::Mode LIBSPEEDTEST_EXPORT TESTMODE;
extern const quint8 LIBSPEEDTEST_EXPORT PINGSPERHOSTDEFAULT;
extern quint8 LIBSPEEDTEST_EXPORT PINGSPERHOST;
extern const quint8 LIBSPEEDTEST_EXPORT PINGTHREADSDEFAULT;
extern quint8 LIBSPEEDTEST_EXPORT PINGTHREADS;
extern const quint8 LIBSPEEDTEST_EXPORT PINGTIMEOUTSECSDEFAULT;
extern quint8 LIBSPEEDTEST_EXPORT PINGTIMEOUTSECS;
extern const quint8 LIBSPEEDTEST_EXPORT DOWNLOADTESTSECSDEFAULT;
extern quint8 LIBSPEEDTEST_EXPORT DOWNLOADTESTSECS;
extern const QString LIBSPEEDTEST_EXPORT HOSTLISTURLDEFAULT;
extern QString LIBSPEEDTEST_EXPORT HOSTLISTURL;
extern const QString LIBSPEEDTEST_EXPORT IPDETECTIONSERVICEURL;
#endif // EXTERNS_H
