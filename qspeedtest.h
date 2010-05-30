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
#include "externs.h"
#include "mainwindow.h"
#include "targetlist.h"


class QSpeedTest : public QApplication
{
    Q_OBJECT

    public:
        QSpeedTest(int, char**);

    private:
        MainWindow mainWindow;
        TargetList targetList;
        QString testDateTime;
        QProcess winSystemInfo;
        QString ISP;
        QString IP;
        QString BBRAS;
        QTime timer;
        double secondsElapsed;
        void checkForProgramUpdates();
        void printHostAndProgramInfo();
        void printLineInfo();

    signals:
        void initOK();
        void logMessage(QString);
        void newTestResult(QString);
        void newVbCode(QString);
        void newHtmlCode(QString);
        void benchmarkFinished(bool completed);

    private slots:
        void startBenchmark();
};

#endif // QSPEEDTEST_H
