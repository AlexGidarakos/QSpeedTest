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

#include <QApplication>
#include <QTime>
#include <QProcess>
#include <QNetworkAccessManager>
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
        QProcess winSystemInfo;
        MainWindow mainWindow;
        TargetList targetList;
        QString testDateTime;
        QProcess proc;
        QEventLoop loop1, loop2;
        QString winArch;
        bool foundFlag;
        QString tracerouteCommand;
        QString bbrasLine;
        QProcess traceroute;
        QNetworkReply *download;
        QNetworkAccessManager manager;
        QByteArray contents;
        QStringList list;
        QTime timer;
        bool pingTestEnabledFlag;
        bool downloadTestEnabledFlag;
        TestResults results;
        QString vbCode;
        QString htmlCode;
        void checkForProgramUpdates();
        void printHostAndProgramInfo();
        void printLineInfo();

    signals:
        void initOK();
        void logMessage(QString);
        void newTestResult(QString);
        void benchmarkFinished(bool completed);

    private slots:
        void startBenchmark();
        void copyVbCode();
        void copyHtmlCode();
};

#endif // QSPEEDTEST_H
