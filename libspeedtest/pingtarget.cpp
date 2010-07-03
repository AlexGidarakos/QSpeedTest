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


#include <QtCore/QProcess>
#include <QtCore/QEventLoop>
#include "pingtarget.h"


quint8 LIBSPEEDTEST_EXPORT PINGSPERTARGET = 4;


PingTarget::PingTarget()
{
    jitterSum = 0.0;
    packetLoss = 100.0;
    replies = 0;
    rttSum = 0.0;
}


PingTarget::PingTarget(const PingTarget &target, QObject *parent) : QObject(parent)
{
    name = target.name;
    address = target.address;
    rtt = target.rtt;
    rttSum = target.rttSum;
    replies = target.replies;
    packetLoss = target.packetLoss;
    jitterSum = target.jitterSum;
    rank = target.rank;
}


PingTarget& PingTarget::operator=(const PingTarget &target)
{
    if(this == &target)
        return *this;

    name = target.name;
    address = target.address;
    rtt = target.rtt;
    rttSum = target.rttSum;
    replies = target.replies;
    packetLoss = target.packetLoss;
    jitterSum = target.jitterSum;
    rank = target.rank;

    return *this;
}


void PingTarget::reset()
{
    rtt.clear();
    rttSum = 0.0;
    jitterSum = 0.0;
    packetLoss = 100.0;
    rank.clear();
    replies = 0;
}


void PingTarget::addRtt(double value)
{
    if(replies)
    {
        jitterSum += rtt.last() - value;
    }

    rtt.append(value);
    rttSum += value;
    replies++;
    packetLoss -= (100.0 / PINGSPERTARGET);
}


double PingTarget::getRttAvg() const
{
    if(replies)
    {
        return rttSum / (replies * 1.0);
    }

    return 0.0;
}


QString PingTarget::getRttAvgAsString() const
{
    if(replies)
    {
        return QString::number(getRttAvg(), 'f', 2) + " msec";
    }

    return "N/A";
}


QString PingTarget::getPacketLossAsString() const
{
    if(replies)
    {
        return QString::number(packetLoss, 'f', 2) + "%";
    }

    return "100.00%";
}


double PingTarget::getJitter() const
{
    if(replies < 2)
    {
        return 0.0;
    }

    return (jitterSum / replies);
}


QString PingTarget::getJitterAsString() const
{
    double jitter;

    if(!replies)
    {
        return "N/A";
    }

    jitter = getJitter();

    return ((jitter >= 0.00)? "+" : NULL) + QString::number(jitter, 'f', 2) + " msec";
}


QString PingTarget::getRank() const
{
    double rttAvg;

    if(!replies)
    {
        return "N/A";
    }

    rttAvg = rttSum / replies;
    if(rttAvg < 30.0) return "A";
    else if(rttAvg < 75.0) return "B";
    else if(rttAvg < 125.0) return "C";
    else if(rttAvg < 200.0) return "D";
    else if(rttAvg < 250.0) return "E";
    else return "F";
}


void PingTarget::ping()
{
    QByteArray contents;
    QString newRtt;
    QStringList list;
    QProcess pingProcess;
    QEventLoop loop;
    QString pingCmd;
    int skipLines;

#ifdef Q_WS_WIN
    pingCmd = QString("ping -n 1 -w %1 %2").arg(PINGTIMEOUT * 1000).arg(address);
    skipLines = 3;
#else
    skipLines = 6;
#ifdef Q_OS_LINUX
    pingCmd = QString("ping -c 1 -W %1 %2").arg(PINGTIMEOUT).arg(address);
#else
    pingCmd = QString("ping -c 1 -W %1 %2").arg(PINGTIMEOUT * 1000).arg(address);
#endif // Q_OS_LINUX
#endif // Q_WS_WIN

    pingProcess.setProcessChannelMode(QProcess::MergedChannels);
    connect(&pingProcess, SIGNAL(finished(int)), &loop, SLOT(quit()));
    reset();

    for(int i = 0; i < PINGSPERTARGET; i++)
    {
        MUTEX.lock();
        if(STOPBENCHMARK)
        {
            MUTEX.unlock();
            return;
        }
        MUTEX.unlock();

        pingProcess.start(pingCmd, QIODevice::ReadOnly);
        loop.exec();

        for(int j = 0; j < skipLines; j++)
        {
            contents = pingProcess.readLine().trimmed();
        }

#ifdef Q_WS_WIN
        if(!contents.contains(QString("=").toAscii()))
        {
            contents = pingProcess.readLine().trimmed();
        }
#endif // Q_WS_WIN

        if(contents.contains(QString("=").toAscii()))
        {
            newRtt = contents;
            list = newRtt.split("=");
#ifdef Q_WS_WIN
            newRtt = list[2];
            newRtt.chop(6);
#else
            list = QString(list[1]).split('/');
            newRtt = list[0];
#endif // Q_WS_WIN
            addRtt(newRtt.toDouble());
        }
    }

    emit newTestResult(name.leftJustified(27, ' ', true) + "    " + getRttAvgAsString().rightJustified(11, ' ', true) + "    " + getPacketLossAsString().rightJustified(9, ' ', true) + "    " + getJitterAsString().rightJustified(12, ' ', true) + "    " + getRank().rightJustified(3, ' ', true));
}
