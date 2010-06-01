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


#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include <QString>


class TestResults
{
    public:
        TestResults();
        void reset();
        QString getRttSumAsString();
        QString getRttAvgAsString();
        QString programName;
        QString programVersion;
        QString programUrl;
        QString programDiscussUrl;
        QString targetListVersion;
        QString targetListComment;
        QString targetListContactUrl;
        QString testDate;
        QString testTime;
        QString hostOS;
        QString isp;
        QString ip;
        QString bbras;
        double rttSum;
        int targetsTotal;
        int targetsAlive;
        double speedInKbps;
        double speedInMBps;
        QString testMode;
        double testDuration;
        int pingsPerTarget;
        int parallelPingThreads;
};

#endif // TESTRESULTS_H
