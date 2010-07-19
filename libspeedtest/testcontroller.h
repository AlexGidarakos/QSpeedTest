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

#ifndef TESTCONTROLLER_H
#define TESTCONTROLLER_H
#include <QtCore/QObject>
#include <QtCore/QFutureWatcher>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTemporaryFile>
#include "results.h"

class LIBSPEEDTEST_EXPORT TestController : public QObject
{
    Q_OBJECT

public:
    explicit TestController(Results &results, QObject *parent) : QObject(parent), _results(results) {}
    void startPingTest();
    void startDownloadTest();
    void stopTests();

private:
    void _connectPingTestSignals();
    void _disconnectPingTestSignals();
    void _connectDownloadTestSignals();
    void _disconnectDownloadTestSignals();

    QFutureWatcher<void> _futureWatcher;
    Results &_results;
    quint8 _groupNext;

signals:
    void progressRangeChanged(int, int);
    void progressValueChanged(int);
    void result(QString);
    void pingTestFinished();
    void finished();

private slots:
    inline void _slotProgressRangeChanged(int min, int max) { emit progressRangeChanged(min, max); }
    inline void _slotProgressValueChanged(int value) { emit progressValueChanged(value); }
    inline void _slotResultReceived(QString text) { emit result(text); }
    void _slotNextPingGroup();
    void _slotPingTestFinished();
    void _slotNextDownloadGroup();
};
#endif // TESTCONTROLLER_H
