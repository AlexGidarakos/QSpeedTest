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


TargetGroup::TargetGroup()
{
    size = 0;
}


void TargetGroup::reset()
{
    rttSum = 0.0;
    rttSumAsString.clear();
    rttAvgAsString.clear();
    packetLossAvg = 0.0;
    packetLossAvgAsString.clear();
    rank.clear();
    targetsAlive = 0;
}


void TargetGroup::addTarget(const Target& target)
{
    targets.append(target);
    size++;
}


double TargetGroup::getRttSum()
{
    if(rttSum > 0.0)
    {
        return rttSum;
    }

    for(int i = 0; i < targets.size(); i++)
    {
        rttSum += targets[i].getRttAvg();
    }

    return rttSum;
}


QString TargetGroup::getRttSumAsString()
{
    if(rttSumAsString.isEmpty())
    {
        if(getTargetsAlive())
        {
            return (rttSumAsString = QString::number(getRttSum(), 'f', 2) + " msec");
        }

        return (rttSumAsString = "N/A");
    }

    return rttSumAsString;
}


double TargetGroup::getRttAvg()
{
    if(rttSum > 0.0)
    {
        return (rttSum / targetsAlive);
    }

    if(getTargetsAlive())
    {
        return (getRttSum() / getTargetsAlive());
    }

    return 0.0;
}


QString TargetGroup::getRttAvgAsString()
{
    if(rttAvgAsString.isEmpty())
    {
        if(getTargetsAlive())
        {
            return (rttAvgAsString = QString::number(getRttAvg(), 'f', 2) + " msec");
        }

        return (rttAvgAsString = "N/A");
    }

    return rttAvgAsString;
}


double TargetGroup::getPacketLossAvg()
{
    if(packetLossAvg > 0.0)
    {
        return packetLossAvg;
    }

    if(getTargetsAlive())
    {
        for(int i = 0; i < targets.size(); i++)
        {
            packetLossAvg += targets[i].getPacketLoss();
        }

        packetLossAvg /= targetsAlive;
    }

    return packetLossAvg;
}


QString TargetGroup::getPacketLossAvgAsString()
{
    if(packetLossAvgAsString.isEmpty())
    {
        if(getTargetsAlive())
        {
            return (packetLossAvgAsString = QString::number(getPacketLossAvg(), 'f', 2) + "%");
        }

        return (packetLossAvgAsString = "100%");
    }

    return packetLossAvgAsString;
}


QString TargetGroup::getRank()
{
    if(rank.isEmpty())
    {
        if(getTargetsAlive())
        {
            double rttAvg = getRttAvg();

            if(rttAvg < 30.0) return (rank = "A");
            else if(rttAvg < 75.0) return (rank = "B");
            else if(rttAvg < 125.0) return (rank = "C");
            else if(rttAvg < 200.0) return (rank = "D");
            else if(rttAvg < 250.0) return (rank = "E");
            else return (rank = "F");
        }

        return (rank = "N/A");
    }

    return rank;
}


int TargetGroup::getTargetsAlive()
{
    if(!targetsAlive)
    {
        for(int i = 0; i < targets.size(); i++)
        {
            if(targets[i].isAlive())
            {
                targetsAlive++;
            }
        }
    }

    return targetsAlive;
}


QString TargetGroup::getName()
{
    return name;
}


void TargetGroup::setName(QString value)
{
    name = value;
}


int TargetGroup::getSize()
{
    return size;
}


void TargetGroup::setSize(int value)
{
    size = value;
}
