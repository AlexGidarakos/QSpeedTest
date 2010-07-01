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


#ifndef QSPEEDTESTCLI_H
#define QSPEEDTESTCLI_H


#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtNetwork/QNetworkReply>
#include "externs.h"
#include "targetlist.h"
#include "testresults.h"


namespace TestMode
{
    enum { Info = 0, Ping, Download, All };
}


class QSpeedTestCli : public QObject
{
    Q_OBJECT

public:
    QSpeedTestCli();

private:
    TargetList targetList;
    TestResults results;
    int testMode;
    QString testModeAsString;
    bool pingTestEnabled;
    bool downloadTestEnabled;
    bool htmlOutputEnabled;
    bool vbOutputEnabled;
    QProcess traceroute;
    QString tracerouteCommand;
    QString bbrasLine;
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QString htmlCode;
    QString vbCode;

    void parseArguments();
    void checkForProgramUpdates();
    void startBenchmark();
    void saveReports();
    void printHostAndProgramInfo();
    void getLineInfo();    // Only starts downloading necessary files without blocking. The actual parsing (and blocking, if necessary) is performed in printLineInfo
    void printLineInfo();
    void generateHtmlCode();
    void generateVbCode();

signals:
    void logMessage(QString);
    void newTestResult(QString);

private slots:
    void start();
    void updateLogMessages(QString);
    void updateTestResults(QString);
};


#endif // QSPEEDTESTCLI_H
