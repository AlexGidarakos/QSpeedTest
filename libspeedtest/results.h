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

#ifndef RESULTS_H
#define RESULTS_H
#include "libspeedtest_global.h"
#include "pinggroup.h"
#include "downloadgroup.h"
#include "externs.h"
#include <QtCore/QStringList>

class LIBSPEEDTEST_EXPORT Results
{
    friend class Hostlist;
    friend class HostInfo;
    friend class TestController;

public:
    Results() : _programName(PROGRAMNAME), _projectVersion(PROJECTVERSION), _projectUrl(PROJECTURL), _projectDiscussUrl(PROJECTDISCUSSURL) {}
    inline void clear() { _pingGroups.clear(); _downloadGroups.clear(); }
    inline QString programName() { return _programName; }
    inline QString testDate() { return _testDate; }
    inline QString testTime() { return _testTime; }
    inline QString ipCensored() { return _ipCensored; }
    inline void setTestMode(TestMode::Mode mode) { _testMode = mode; }
    inline void setTestDuration(double duration) { _testDuration = duration; }
    void reset();
    QString testModeString() const;
    QString summary() const;
    QString plainText() const;
    QString bbCode() const;
    QString html() const;
    bool saveReport(ReportFormat::Format, const QString &) const;

private:
    quint16 _pingHostsAlive() const;
    quint16 _pingHostsTotal() const;
    double _pingAverage() const;
    quint16 _downloadHostsTotal() const;
    QString _maxBandwidth(SpeedUnit::Unit) const;
    QString _googleChartPingGroupsUrl() const;
    QString _googleChartDownloadGroupUrl(quint8) const;

    QString _programName;
    QString _projectVersion;
    QString _projectUrl;
    QString _projectDiscussUrl;
    QString _hostlistVersion;
    QString _hostlistComment;
    QString _hostlistContactUrl;
    QString _testDate;
    QString _testTime;
    QString _testDateTime;
    QString _hostOS;
    quint8 _cpuCores;
    QString _isp;
    QString _ip;
    QString _ipCensored;
    QString _ispNetwork;
    QString _ispAsn;
    QStringList _ispNetworkAdvertisers;
    QString _bbras;
    TestMode::Mode _testMode;
    double _testDuration;
    quint8 _pingsPerHost;
    quint8 _pingThreads;
    quint8 _downloadTestSecs;

    QList<PingGroup> _pingGroups;
    QList<DownloadGroup> _downloadGroups;
};
#endif // RESULTS_H
