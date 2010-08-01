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

#include "externs.h"

const QString PROJECTNAME = "QSpeedTest";
const QString PROJECTVERSION = "r53";
QString PROGRAMNAME = "QSpeedTest";
const QString PROJECTURL = "http://qspeedtest.sourceforge.net/";
const QString PROJECTDOWNLOADURL = "https://sourceforge.net/projects/qspeedtest/files/";
const QString PROJECTDISCUSSURL = "https://sourceforge.net/apps/phpbb/qspeedtest/";
const QString PROJECTVERSIONURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.version";
const QString PROJECTAUTHOR = "parsifal";
const QString PROJECTCOMPANY = "Binary Notion";
const QString PROJECTCOMPANYURL = "http://binarynotion.net/";
const quint8 UPDATECHECKTIMEOUTSECSDEFAULT = 3;
quint8 UPDATECHECKTIMEOUTSECS;
const TestMode::Mode TESTMODEDEFAULT = TestMode::All;
TestMode::Mode TESTMODE;
const quint8 PINGSPERHOSTDEFAULT = 4;
quint8 PINGSPERHOST;
const quint8 PINGTHREADSDEFAULT = 4;
quint8 PINGTHREADS;
const quint8 PINGTIMEOUTSECSDEFAULT = 2;
quint8 PINGTIMEOUTSECS;
const quint8 DOWNLOADTESTSECSDEFAULT = 10;
quint8 DOWNLOADTESTSECS;
const QString HOSTLISTURLDEFAULT = "http://hostlist.narfhosting.net/qspeedtest.hostlist.ini";
QString HOSTLISTURL;
const QString IPDETECTIONSERVICEURL = "http://www.whatismyip.com/automation/n09230945.asp";
