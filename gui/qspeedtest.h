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
#include <QtCore/QTime>
#include "mainwindow.h"
#include "results.h"
#include "preferences.h"
#include "hostlist.h"
#include "hostinfo.h"
#include "testcontroller.h"

class QSpeedTest : public QApplication
{
    Q_OBJECT

public:
    QSpeedTest(int, char**);

private:
    void _connectSignalsSlots();
    void _checkForProgramUpdates();

    Results _results;
    Preferences *_preferences;
    Hostlist *_hostlist;
    HostInfo *_hostInfo;
    TestController *_controller;
    MainWindow _w;
    QTime _timer;

signals:
    void hostlistOk(bool);    // Connect to appropriate MainWindow's slot to enable or disable Start button

private slots:
    inline void _slotLoadHostlist() { emit hostlistOk(_hostlist->initOk()); }
    void _slotCopyReport(ReportFormat::Format);
    void _slotSaveReport(ReportFormat::Format);
    void _slotSavePreferences();
    void _slotStartTests();
    void _slotStopTests();
    void _slotTestsFinished();
};
#endif // QSPEEDTEST_H
