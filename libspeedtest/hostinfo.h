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


#ifndef HOSTINFO_H
#define HOSTINFO_H


#include "libspeedtest_global.h"
#include "testresults.h"
#include <QtCore/QProcess>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


class LIBSPEEDTEST_EXPORT HostInfo {
public:
    HostInfo(TestResults*);
    void init();
    void retrieve();

private:
    TestResults *results;
    QProcess osDetectProc, traceroute;
    QEventLoop loop;
    QString tracerouteCmd;
    QString bbrasLine;
    QString winArch;
    QNetworkAccessManager manager;
    QNetworkReply *download;
};


#endif // HOSTINFO_H
