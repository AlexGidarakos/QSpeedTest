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
#include "results.h"
#include "hostlist.h"
#include "hostinfo.h"
#include "testcontroller.h"
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QTime>

class QSpeedTestCli : public QObject
{
    Q_OBJECT

public:
    QSpeedTestCli();
    ~QSpeedTestCli();

private:
    void _parseArguments();
    void _checkForProgramUpdates();
    void _startTests();
    void _saveReports();

    Results _results;
    Hostlist *_hostlist;
    HostInfo *_hostInfo;
    TestController *_controller;
//    QString testModeAsString;
//    bool pingTestEnabled;
//    bool downloadTestEnabled;
    bool _htmlEnabled;
    bool _bbCodeEnabled;
    QTime _timer;
//    QString _html;
//    QString _bbCode;

signals:
    void message(QString);
    void result(QString);

private slots:
    void _slotStart();
    void _slotTestsFinished();
    inline void _slotLog(QString message) { qDebug() << qPrintable((QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + message)); }
    inline void _slotResult(QString result) { qDebug() << qPrintable(result); }
};
#endif // QSPEEDTESTCLI_H
