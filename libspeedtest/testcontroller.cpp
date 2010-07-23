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

#include "testcontroller.h"
#include <QtCore/QtConcurrentMap>
#include <QtGui/QApplication>

void TestController::startPingTest()
{
    QThreadPool::globalInstance()->setMaxThreadCount(_results._pingThreads);
    _connectPingTestSignals();
    _groupNext = 0;
    _slotNextPingGroup();
}

void TestController::startDownloadTest()
{
    _connectDownloadTestSignals();
    _groupNext = 0;
    _slotNextDownloadGroup();
}

void TestController::stopTests()
{
    _disconnectPingTestSignals();
    _disconnectDownloadTestSignals();

    if(_futureWatcher.isRunning())
    {
        _futureWatcher.cancel();
    }

    quint8 size = _results._downloadGroups.size();

    for(int i = 0; i < size; i++)
    {
        _results._downloadGroups[i].cancel();
    }
}

void TestController::_connectPingTestSignals()
{
    connect(this, SIGNAL(pingTestFinished()), this, SLOT(_slotPingTestFinished()));
    connect(&_futureWatcher, SIGNAL(finished()), this, SLOT(_slotNextPingGroup()));
    connect(&_futureWatcher, SIGNAL(progressRangeChanged(int,int)), this, SLOT(_slotProgressRangeChanged(int,int)));
    connect(&_futureWatcher, SIGNAL(progressValueChanged(int)), this, SLOT(_slotProgressValueChanged(int)));
    quint8 size = _results._pingGroups.size();

    for(int i = 0; i < size; i++)
    {
        quint16 size2 = _results._pingGroups[i]._hosts.size();

        for(int j = 0; j < size2; j++)
        {
            connect(&_results._pingGroups[i]._hosts[j], SIGNAL(result(QString)), this, SLOT(_slotResultReceived(QString)));
        }
    }
}

void TestController::_disconnectPingTestSignals()
{
    disconnect(this, SIGNAL(pingTestFinished()), this, SLOT(_slotPingTestFinished()));
    disconnect(&_futureWatcher, SIGNAL(finished()), this, SLOT(_slotNextPingGroup()));
    connect(&_futureWatcher, SIGNAL(progressRangeChanged(int,int)), this, SLOT(_slotProgressRangeChanged(int,int)));
    connect(&_futureWatcher, SIGNAL(progressValueChanged(int)), this, SLOT(_slotProgressValueChanged(int)));
    quint8 size = _results._pingGroups.size();

    for(int i = 0; i < size; i++)
    {
        quint16 size2 = _results._pingGroups[i]._hosts.size();

        for(int j = 0; j < size2; j++)
        {
            disconnect(&_results._pingGroups[i]._hosts[j], SIGNAL(result(QString)), this, SLOT(_slotResultReceived(QString)));
        }
    }
}

void TestController::_connectDownloadTestSignals()
{
    quint8 size = _results._downloadGroups.size();

    for(int i = 0; i < size; i++)
    {
        connect(&_results._downloadGroups[i], SIGNAL(progressRangeChanged(int,int)), this, SLOT(_slotProgressRangeChanged(int,int)));
        connect(&_results._downloadGroups[i], SIGNAL(progressValueChanged(int)), this, SLOT(_slotProgressValueChanged(int)));
        connect(&_results._downloadGroups[i], SIGNAL(result(QString)), this, SLOT(_slotResultReceived(QString)));
        connect(&_results._downloadGroups[i], SIGNAL(finished()), this, SLOT(_slotNextDownloadGroup()));
    }
}

void TestController::_disconnectDownloadTestSignals()
{
    quint8 size = _results._downloadGroups.size();

    for(int i = 0; i < size; i++)
    {
        disconnect(&_results._downloadGroups[i], SIGNAL(progressRangeChanged(int,int)), this, SLOT(_slotProgressRangeChanged(int,int)));
        disconnect(&_results._downloadGroups[i], SIGNAL(progressValueChanged(int)), this, SLOT(_slotProgressValueChanged(int)));
        disconnect(&_results._downloadGroups[i], SIGNAL(result(QString)), this, SLOT(_slotResultReceived(QString)));
        disconnect(&_results._downloadGroups[i], SIGNAL(finished()), this, SLOT(_slotNextDownloadGroup()));
    }
}

void TestController::_slotNextPingGroup()
{
    if(_groupNext)    // Means it's not the first call of this slot
    {
        emit result(trUtf8("Group sum:     %1").arg(_results._pingGroups[_groupNext - 1].rttSumString()));
        emit result(trUtf8("Group average: %1").arg(_results._pingGroups[_groupNext - 1].rttAverageString()) + '\n');
        _results._pingGroups[_groupNext - 1].sort();
    }

    if(_groupNext == _results._pingGroups.size())    // Means we are done with all the ping groups
    {
        emit pingTestFinished();

        return;
    }

    emit result(_results._pingGroups[_groupNext].name().leftJustified(27, ' ', true) + "    " + trUtf8("Avg. ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
    emit result("-------------------------------------------------------------------------------");
    _groupNext++;
    _futureWatcher.setFuture(QtConcurrent::map(_results._pingGroups[_groupNext - 1]._hosts, &PingHost::ping));
}

void TestController::_slotPingTestFinished()
{
    _disconnectPingTestSignals();

    if(_results._testMode & TestMode::Download)
    {
        startDownloadTest();
    }
    else
    {
        emit finished();
    }
}

void TestController::_slotNextDownloadGroup()
{
    if(_groupNext == _results._downloadGroups.size())    // Means we are done with all the download groups, hence all the controller tests too
    {
        _disconnectDownloadTestSignals();
        emit finished();

        return;
    }

    _groupNext++;
    _results._downloadGroups[_groupNext - 1].start();
}
