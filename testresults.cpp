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

#include "testresults.h"
#include "externs.h"


TestResults::TestResults()
{
    programName = PROGRAMNAME;
    programVersion = PROGRAMVERSION;
    programUrl = PROGRAMURL;
    programDiscussUrl = PROGRAMDISCUSSURL;
    pingsPerTarget = PINGSPERTARGET;
}


void TestResults::reset()
{
    rttSum = 0.0;
    targetsAlive = 0;
    speedInKbps = 0.0;
    speedInMBps = 0.0;
    testDuration = 0.0;
}


QString TestResults::getRttSumAsString()
{
    if(targetsAlive)
    {
        return QString::number(rttSum, 'f', 2) + " msec";
    }

    return "N/A";
}


QString TestResults::getRttAvgAsString()
{
    if(targetsAlive)
    {
        return QString::number(rttSum / targetsAlive, 'f', 2) + " msec";
    }

    return "N/A";
}
