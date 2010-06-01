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


#include <QProcess>
#include <QEventLoop>
#include <QApplication>
#include "target.h"
#include "externs.h"


Target::Target()
{
    jitterSum = 0.0;
    packetLoss = 100.0;
    replies = 0;
    rttSum = 0.0;
}


Target::Target(const Target &target)
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


Target& Target::operator=(const Target &target) {

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


void Target::reset()
{
    rtt.clear();
    rttSum = 0.0;
    rttAvgAsString.clear();
    jitterSum = 0.0;
    jitterAsString.clear();
    packetLoss = 100.0;
    packetLossAsString.clear();
    rank.clear();
    replies = 0;
}


QString Target::getName()
{
    return name;
}


void Target::setName(QString value)
{
    name = value;
}


QString Target::getAddress()
{
    return address;
}


void Target::setAddress(QString value)
{
    address = value;
}


void Target::addRtt(double value)
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


double Target::getRttAvg()
{
    if(replies)
    {
        return rttSum / (replies * 1.0);
    }

    return 0.0;
}


QString Target::getRttAvgAsString()
{
    if(rttAvgAsString.isEmpty())
    {
        if(replies)
        {
            return (rttAvgAsString = QString::number(getRttAvg(), 'f', 2) + " msec");
        }

        return (rttAvgAsString = "N/A");
    }

    return rttAvgAsString;
}



double Target::getPacketLoss()
{
        return packetLoss;
}


QString Target::getPacketLossAsString()
{
    if(packetLossAsString.isEmpty())
    {
        if(replies)
        {
            return (packetLossAsString = QString::number(packetLoss, 'f', 2) + "%");
        }

        return (packetLossAsString = "100.00%");
    }

    return packetLossAsString;
}


double Target::getJitter()
{
    if(replies < 2)
    {
        return 0.0;
    }

    return (jitterSum / replies);
}


QString Target::getJitterAsString()
{
    if(jitterAsString.isEmpty())
    {
        if(!replies)
        {
            return (jitterAsString = "N/A");
        }

        double jitter = getJitter();

        return (jitterAsString = ((jitter >= 0.00)? "+" : NULL) + QString::number(jitter, 'f', 2) + " msec");
    }

    return jitterAsString;
}


QString Target::getRank()
{
    if(rank.isEmpty())
    {
        if(!replies)
        {
            return (rank = "N/A");
        }

        double rttAvg = rttSum / replies;
        if(rttAvg < 30.0) return (rank = "A");
        else if(rttAvg < 75.0) return (rank = "B");
        else if(rttAvg < 125.0) return (rank = "C");
        else if(rttAvg < 200.0) return (rank = "D");
        else if(rttAvg < 250.0) return (rank = "E");
        else return (rank = "F");
    }

    return rank;
}


bool Target::isAlive()
{
    return replies;
}


void Target::ping()
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
        if(STOPBENCHMARK)
        {
            return;
        }

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
    QApplication::processEvents();
}
