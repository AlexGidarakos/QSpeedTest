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

#include "hostinfo.h"
#include "externs.h"
#include <QtCore/QDateTime>

HostInfo::HostInfo(Results &results, QObject *parent) : QObject(parent), _results(results), _download(NULL)
{
    _osDetectProc.setProcessChannelMode(QProcess::MergedChannels);
    _tracerouteProc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&_osDetectProc, SIGNAL(finished(int)), this, SLOT(_slotOsDetectProcFinished()));
    connect(&_osDetectProc, SIGNAL(finished(int)), &_loop, SLOT(quit()));
    connect(&_tracerouteProc, SIGNAL(finished(int)), &_loop, SLOT(quit()));

#ifdef Q_WS_WIN
    _winArch = (QProcessEnvironment::systemEnvironment().contains("ProgramFiles(x86)"))? "x64" : "x86";
    _tracerouteCmd = "pathping -h 2 -q 1 8.8.8.8";
    _bbrasLine = "  2  ";

    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_2000:
            _results._hostOS = "Windows 2000";
            break;

        case QSysInfo::WV_XP:
            _results._hostOS = "Windows XP x86";
            break;

        case QSysInfo::WV_2003:
            _osDetectProc.start("cmd /c ver", QIODevice::ReadOnly);
            break;

        case QSysInfo::WV_VISTA:
            _osDetectProc.start("systeminfo", QIODevice::ReadOnly);
            break;

        case QSysInfo::WV_WINDOWS7:
            _osDetectProc.start("systeminfo", QIODevice::ReadOnly);
            break;

        default:
            _results._hostOS = "Windows (unknown version)";
    }
#else
    _tracerouteCmd = "traceroute -m 2 -q 1 8.8.8.8";
    _bbrasLine = " 2  ";
#ifdef Q_WS_MAC
    _macVersionDetectProc.start("sw_vers -productVersion", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
    _osDetectProc.start("uname -o", QIODevice::ReadOnly);
#else
    _osDetectProc.start("uname -s", QIODevice::ReadOnly);
#endif // Q_OS_LINUX
#endif // Q_WS_MAC
#endif // Q_WS_WIN
}

HostInfo::~HostInfo()
{
    if(_osDetectProc.state() != QProcess::NotRunning) _osDetectProc.close();

    stopInfoTest();
}

void HostInfo::_slotOsDetectProcFinished()
{
    disconnect(&_osDetectProc, SIGNAL(finished(int)), this, SLOT(_slotOsDetectProcFinished()));    // To avoid recursion!

#ifdef Q_WS_WIN
    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_2003:
            _results._hostOS = (_osDetectProc.readAll().contains("2003"))? ("Windows Server 2003" + _winArch) : "Windows XP x64";
            break;

        case QSysInfo::WV_VISTA:
            _results._hostOS = (_osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 " + _winArch) : ("Windows Vista " + _winArch);
            break;

        case QSysInfo::WV_WINDOWS7:
            _results._hostOS = (_osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 R2 " + _winArch) : ("Windows 7 " + _winArch);
            break;

        default:
            _results._hostOS = "Windows (unknown version)";
    }
#else
#ifdef Q_WS_MAC
    _results._hostOS = "Mac OS X " + _osDetectProc.readLine().trimmed();
#else
    _results._hostOS = _osDetectProc.readLine().trimmed();
    _osDetectProc.start("uname -rm", QIODevice::ReadOnly);
    _loop.exec();
    _results._hostOS += " " + _osDetectProc.readLine().trimmed();
#endif // Q_WS_MAC
#endif // Q_WS_WIN
    emit osDetectionFinished();
}

void HostInfo::startDetection()
{
    _tracerouteProc.start(_tracerouteCmd, QIODevice::ReadOnly);
    _download = _manager.get(QNetworkRequest(QUrl("http://www.speedtest.net/summary.php")));
    connect(_download, SIGNAL(finished()), &_loop, SLOT(quit()));
    _results._testDate = QDate::currentDate().toString("yyyyMMdd");
    _results._testTime = QTime::currentTime().toString("hhmmss");
    _results._testDateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    _results._testMode = TESTMODE;
    _results._pingsPerHost = PINGSPERHOST;
    _results._pingThreads = PINGTHREADS;
    _results._downloadTestSecs = DOWNLOADTESTSECS;
}

void HostInfo::_retrieve()
{
    QByteArray contents;
    bool foundFlag = false;
    QStringList list;

    if(_osDetectProc.state() != QProcess::NotRunning) _loop.exec();    // Make sure all pending processes have finished

    if(_tracerouteProc.state() != QProcess::NotRunning) _loop.exec();

    if(_download->isRunning()) _loop.exec();

    while(!(contents = _download->readLine()).isEmpty() && !foundFlag)
    {
        if(contents.contains("<div class=\"ip\">"))
        {
            contents.chop(7);
            _results._ip = QString(contents.trimmed().mid(16));
            list = _results._ip.split('.');
            _results._ip = list[0] + "." + list[1] + ".xxx.xxx";
            continue;
        }
        else
        {
            if(contents.contains("<div class=\"isp\">"))
            {
                contents.chop(7);
                _results._isp = QString(contents.trimmed().mid(17));
                foundFlag = true;
            }
        }
    }

    if(_results._isp.isEmpty())
    {
        _results._isp = trUtf8("speedtest.net unreachable");
        _results._ip.clear();
    }

    for(foundFlag = false; !foundFlag && !(contents = _tracerouteProc.readLine()).isEmpty();)
    {
        if(contents.contains(_bbrasLine.toAscii()))
        {
            foundFlag = true;

            if(contents.contains(QString("*").toAscii())) _results._bbras = trUtf8("N/A (non-responsive BBRAS)");

#ifdef Q_WS_WIN
            else _results._bbras = contents.mid(3).trimmed();
#else
            else _results._bbras = contents.mid(4, contents.indexOf(')') - 1).trimmed();
#endif // Q_WS_WIN

        }
    }

    if(!foundFlag) _results._bbras = trUtf8("N/A");

    _tracerouteProc.close();

    if(_download) delete _download;

    _download = NULL;
}

void HostInfo::startInfoTest()
{
    QEventLoop loop;

    _retrieve();
    connect(this, SIGNAL(osDetectionFinished()), &loop, SLOT(quit()));

    if(_results._hostOS.isEmpty()) loop.exec();

    disconnect(this, SIGNAL(osDetectionFinished()), &loop, SLOT(quit()));
    emit finished();
}

void HostInfo::stopInfoTest()
{
    if(_tracerouteProc.state() != QProcess::NotRunning) _tracerouteProc.close();

    if(_download)
    {
        if(_download->isRunning())
        {
            _download->abort();
            _download->close();
        }

        delete _download;
        _download = NULL;
    }
}
