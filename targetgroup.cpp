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


#include "targetgroup.h"
#include "externs.h"
#include <QEventLoop>
#include <QProcess>


Target::Target()
{
    address = QString(NULL);
    jitter = 0.0;
    name = QString(NULL);
    packetLoss = 0.0;
    rank = "N/A";
    replies = 0;
    rttAvg = 0.0;
}


Target::Target(const Target &target)
{
    name = target.name;
    address = target.address;
    rtt = target.rtt;
    rttAvg = target.rttAvg;
    replies = target.replies;
    packetLoss = target.packetLoss;
    jitter = target.jitter;
    rank = target.rank;
}


Target& Target::operator=(const Target &target) {

    if(this == &target)
        return *this;

    name = target.name;
    address = target.address;
    rtt = target.rtt;
    rttAvg = target.rttAvg;
    replies = target.replies;
    packetLoss = target.packetLoss;
    jitter = target.jitter;
    rank = target.rank;

    return *this;
}


QString Target::getName()
{
    return name;
}


void Target::setName(QString name)
{
    this->name = name;
}


QString Target::getAddress()
{
    return address;
}


void Target::setAddress(QString address)
{
    this->address = address;
}


void Target::addRtt(double rtt)
{
    this->rtt.append(rtt);
    replies++;
}


double Target::getRttAvg()
{
    rttAvg = 0.0;

    if(!replies)
    {
        rttAvg = 0.0;
        return rttAvg;
    }

    foreach(double i, rtt)
    {
        rttAvg += i;
    }

    return (rttAvg /= (replies * 1.0));
}


double Target::getPacketLoss()
{
    return (packetLoss = ((PINGSPERTARGET - replies) * 100.0) / PINGSPERTARGET);
}


double Target::getJitter()
{
    double jitter = 0.0;

    if(replies < 2)
    {
        return 0.0;
    }

    for(int i = 1; i < rtt.size(); i++)
    {
        jitter += rtt.at(i - 1) - rtt.at(i);
    }

    return (jitter /= (replies * 1.0));
}


QString Target::getRank()
{
    if(!replies)
    {
        return (rank = "N/A");
    }

    if(rttAvg < 30.0) return (rank = "A");
    else if(rttAvg < 75.0) return (rank = "B");
    else if(rttAvg < 125.0) return (rank = "C");
    else if(rttAvg < 200.0) return (rank = "D");
    else if(rttAvg < 250.0) return (rank = "E");
    else return (rank = "F");
}


bool Target::wasAlive()
{
    return replies;
}


void Target::reset()
{
    rtt.clear();
    replies = 0;
    packetLoss = 0.0;
    jitter = 0.0;
}


void Target::ping()
{
    QByteArray contents;
    QString rttString;
    QStringList list;
    QProcess pingProcess;
    QEventLoop loop;
#ifdef Q_WS_WIN
    QString pingCmd = QString("ping -n 1 -w %1 %2").arg(PINGTIMEOUT).arg(address);
    int skipLines = 3;
#else
    QString pingCmd = QString("ping -c 1 -W %1 %2").arg(PINGTIMEOUT / 1000).arg(address);
    int skipLines = 6;
#endif
    QString packetLossString;
    QString jitterString;
    double jitterDouble;

    pingProcess.setProcessChannelMode(QProcess::MergedChannels);
    connect(&pingProcess, SIGNAL(finished(int)), &loop, SLOT(quit()));

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
#endif
        if(contents.contains(QString("=").toAscii()))
        {
            rttString = contents;
            list = rttString.split("=");
#ifdef Q_WS_WIN
            rttString = list[2];
            rttString.chop(6);
#else
            list = QString(list[1]).split('/');
            rttString = list[0];
#endif
            addRtt(rttString.toDouble());
        }
    }

    if(!wasAlive())
    {
        rttString = trUtf8("N/A");
        packetLossString = "100.00%";
        jitterString = trUtf8("N/A");
    }
    else
    {
        rttString = QString::number(getRttAvg(), 'f', 2) + " msec";
        packetLossString = QString::number(getPacketLoss(), 'f', 2) + "%";
        jitterDouble = getJitter();
        jitterString = ((jitterDouble >= 0.00)? "+" : "") + QString::number(getJitter(), 'f', 2) + " msec";
    }

    emit message(name.leftJustified(27, ' ', true) + "    " + rttString.rightJustified(11, ' ', true) + "    " + packetLossString.rightJustified(9, ' ', true) + "    " + jitterString.rightJustified(12, ' ', true) + "    " + getRank().rightJustified(3, ' ', true));
    changevBulletinCode(QString("[url=%1]%2[/url] | [right]%3[/right] | [right]%4[/right] | [right]%5[/right] | [center]%6[/center] |\n").arg(address).arg(name).arg(rttString).arg(packetLossString).arg(jitterString).arg(getRank()));
    changeHTML(QString("            <tr><td><a href=\"%1\">%2</a></td><td><div align=\"right\">%3</div></td><td><div align=\"right\">%4</div></td><td><div align=\"right\">%5</div></td><td><div align=\"center\">%6</div></td></tr>\n").arg(address).arg(name).arg(rttString).arg(packetLossString).arg(jitterString).arg(getRank()));
}


TargetGroup::TargetGroup()
{
    name = QString(NULL);
    size = 0;
}


double TargetGroup::rttGroupSum()
{
    double sum = 0.0;
    Target target;

    foreach(target, targets)
    {
        sum += target.getRttAvg();
    }

    return sum;
}


int TargetGroup::countTargetsAlive()
{
    int count = 0;
    Target target;

    foreach(target, targets)
    {
        if(target.wasAlive())
        {
            count++;
        }
    }

    return count;
}
