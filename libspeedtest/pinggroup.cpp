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

#include "pinggroup.h"

void PingGroup::reset()
{
    quint16 size = _hosts.size();

    for(int i = 0; i < size; i++) _hosts[i].reset();
}

quint16 PingGroup::hostsAlive() const
{
    quint16 sum = 0;
    quint16 size = _hosts.size();

    for(int i = 0; i < size; i++)
    {
        if(_hosts[i].isAlive()) sum++;
    }

    return sum;
}

double PingGroup::rttSum() const
{
    double sum = 0.0;
    quint16 size = _hosts.size();

    for(int i = 0; i < size; i++) sum += _hosts[i].rtt();

    return sum;
}

QString PingGroup::rttSumString() const
{
    QString string;

    if(!hostsAlive()) string = "N/A";
    else string = QString::number(rttSum(), 'f', 2) + " msec";

    return string;
}

double PingGroup::rttAverage() const
{
    quint16 alive = hostsAlive();
    double average = 0.0;

    if(alive)
    {
        quint16 size = _hosts.size();
        double sum = 0.0;

        for(int i = 0; i < size; i++) if(_hosts[i].isAlive()) sum += _hosts[i].rtt();

        average = sum / alive;
    }

    return average;
}

QString PingGroup::rttAverageString() const
{
    QString string;

    if(!hostsAlive()) string = "N/A";
    else string = QString::number(rttAverage(), 'f', 2) + " msec";

    return string;
}

double PingGroup::packetLossAverage() const
{
    quint16 alive = hostsAlive();
    double average = 100.0;

    if(alive)
    {
        quint16 size = _hosts.size();
        double sum = 0.0;

        for(int i = 0; i < size; i++) if(_hosts[i].isAlive()) sum += _hosts[i].packetLoss();

        average = sum / alive;
    }

    return average;
}

QString PingGroup::packetLossAverageString() const
{
    return QString::number(packetLossAverage(), 'f', 2) + "%";
}

QString PingGroup::rank() const
{
    if(!hostsAlive()) return "N/A";

    double average = rttAverage();

    if(average <  30.0) return "A";
    if(average <  75.0) return "B";
    if(average < 125.0) return "C";
    if(average < 200.0) return "D";
    if(average < 250.0) return "E";
    return "F";
}
