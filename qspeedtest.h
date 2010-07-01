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
#include "externs.h"
#include "mainwindow.h"
#include "targetlist.h"
#include "testresults.h"


class QSpeedTest : public QApplication
{
    Q_OBJECT

public:
    QSpeedTest(int, char**);

private:
    MainWindow mainWindow;
    TargetList targetList;
    bool pingTestEnabled;
    bool downloadTestEnabled;
    TestResults results;
    QProcess traceroute;
    QString tracerouteCommand;
    QString bbrasLine;
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QString htmlCode;
    QString vbCode;

    void checkForProgramUpdates();
    void printHostAndProgramInfo();
    void getLineInfo();    // Only starts downloading necessary files without blocking. The actual parsing (and blocking, if necessary) is performed in printLineInfo
    void printLineInfo();
    void generateHtmlCode();
    void generateVbCode();

signals:
    void initOK();
    void logMessage(QString);
    void newTestResult(QString);
    void benchmarkFinished(bool completed);

private slots:
    void startBenchmark();
    void showReport(bool);
};


#endif // QSPEEDTEST_H
