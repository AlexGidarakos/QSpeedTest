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


#ifndef QSPEEDTEST_H
#define QSPEEDTEST_H


#include <QtGui/QApplication>
#include <QtCore/QProcess>
#include <QtNetwork/QNetworkReply>
#include "mainwindow.h"
#include "targetlist.h"
#include "testresults.h"
#include "hostinfo.h"
#include "externs.h"


class QSpeedTest : public QApplication
{
    Q_OBJECT

public:
    QSpeedTest(int, char**);
    ~QSpeedTest();

private:
    MainWindow mainWindow;
    TargetList *targetList;
    TestResults results;
    HostInfo *hostInfo;
    bool pingTestEnabled;
    bool downloadTestEnabled;
    QString htmlCode;
    QString vbCode;

    void checkForProgramUpdates();
    void connectPingGroup(PingGroup&);
    void generateHtmlCode();
    void generateVbCode();

signals:
    void initOK();
    void logMessage(QString);
    void newTestResult(QString);
    void benchmarkFinished(bool completed);

private slots:
    void runBenchmark();
    void showReport(bool);
};


#endif // QSPEEDTEST_H
