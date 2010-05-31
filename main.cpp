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


#include <QString>
#include <QMutex>
#include "qspeedtest.h"
const QString PROGRAMCOMPANY = "Binary Notion";
const QString PROGRAMAUTHOR = "parsifal";
const QString PROGRAMNAME = "QSpeedTest";
const QString PROGRAMVERSION = "r16";
const QString PROGRAMURL = "https://sourceforge.net/projects/qspeedtest/files/";
const QString PROGRAMDISCUSSURL = "https://sourceforge.net/apps/phpbb/qspeedtest";
const QString PROGRAMUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.version";
const QString TARGETLISTURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.ini";
const QString TARGETLISTUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.ini.version";
const int PINGTIMEOUT = 1;
const int DOWNLOADTESTSECS = 15;
int PINGSPERTARGET = 4;
QMutex MUTEX;
qint64 BYTESDOWNLOADED;
bool STOPBENCHMARK = false;


int main(int argc, char *argv[])
{
    QSpeedTest qSpeedTest(argc, argv);

    return qSpeedTest.exec();
}
