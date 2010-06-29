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


#include <QtCore/QString>
#include <QtCore/QMutex>
#include "qspeedtestcli.h"


const QString PROJECTNAME = "QSpeedTest";
const QString PROGRAMNAME = "QSpeedTestCLI";
const QString PROGRAMAUTHOR = "parsifal";
const QString PROGRAMVERSION = "r2";
const QString PROGRAMURL = "https://sourceforge.net/projects/qspeedtest/files/";
const QString PROGRAMDISCUSSURL = "https://sourceforge.net/apps/phpbb/qspeedtest";
const QString PROGRAMUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtestcli.version";
const QString TARGETLISTURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.ini";
const QString TARGETLISTUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.ini.version";
const int UPDATECHECKTIMEOUT = 3;
const int PINGTIMEOUT = 1;
const int DOWNLOADTESTSECS = 15;
int PINGSPERTARGET = 4;
QMutex MUTEX;
qint64 BYTESDOWNLOADED;
bool STOPBENCHMARK = false;
int PARALLELPINGS = 4;


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QSpeedTestCli qSpeedTestCli;

    QMetaObject::invokeMethod(&qSpeedTestCli, "start", Qt::QueuedConnection);

    return app.exec();
}
