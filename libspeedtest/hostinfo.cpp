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

HostInfo::HostInfo(Results &results, QObject *parent) : QObject(parent), _results(results)
{
    connect(&_osDetectProc, SIGNAL(finished(int)), this, SLOT(_slotOsDetectProcFinished()));
    _osDetectProc.setProcessChannelMode(QProcess::MergedChannels);
    _results._hostOS.clear();
    _bbrasDetectProc.setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_WS_WIN
    _winArchitecture = (QProcessEnvironment::systemEnvironment().contains("ProgramFiles(x86)"))? "x64" : "x86";
    _bbrasDetectProcCmd = "pathping -h 2 -q 1 8.8.8.8";
    _bbrasLine = "2  ";

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
    _bbrasDetectProcCmd = "traceroute -m 2 -q 1 8.8.8.8";
    _bbrasLine = "2  ";
#ifdef Q_WS_MAC
    _osDetectProc.start("sw_vers -productVersion", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
    _osDetectProc.start("uname -o", QIODevice::ReadOnly);
#else
#ifdef Q_OS_OS2
    _results._hostOS = "IBM OS/2";
#else
    _osDetectProc.start("uname -s", QIODevice::ReadOnly);
#endif // W_OS_OS2
#endif // Q_OS_LINUX
#endif // Q_WS_MAC
#endif // Q_WS_WIN
}

void HostInfo::startDetection()
{
    _ipDetectDownload = _manager.get(QNetworkRequest(QUrl(IPDETECTIONSERVICEURL)));
    connect(_ipDetectDownload, SIGNAL(finished()), this, SLOT(_slotIpDetectDownloadFinished()));
    _bbrasDetectProc.start(_bbrasDetectProcCmd, QIODevice::ReadOnly);
    connect(&_bbrasDetectProc, SIGNAL(finished(int)), this, SLOT(_slotBbrasDetectProcFinished()));
    _results._testDate = QDate::currentDate().toString("yyyyMMdd");
    _results._testTime = QTime::currentTime().toString("hhmmss");
    _results._testDateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    _results._testMode = TESTMODE;
    _results._pingsPerHost = PINGSPERHOST;
    _results._pingThreads = PINGTHREADS;
    _results._downloadTestSecs = DOWNLOADTESTSECS;
    _results._ip.clear();
    _results._ipCensored.clear();
    _results._bbras.clear();
    _results._isp.clear();
    _results._ispAsn.clear();
    _results._ispNetwork.clear();
    _results._ispNetworkAdvertisers.clear();
}

void HostInfo::_slotOsDetectProcFinished()
{
    disconnect(&_osDetectProc, SIGNAL(finished(int)), this, SLOT(_slotOsDetectProcFinished()));

#ifdef Q_WS_WIN
    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_2003:
            _results._hostOS = (_osDetectProc.readAll().contains("2003"))? ("Windows Server 2003" + _winArchitecture) : "Windows XP x64";
            break;

        case QSysInfo::WV_VISTA:
            _results._hostOS = (_osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 " + _winArchitecture) : ("Windows Vista " + _winArchitecture);
            break;

        case QSysInfo::WV_WINDOWS7:
            _results._hostOS = (_osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 R2 " + _winArchitecture) : ("Windows 7 " + _winArchitecture);
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
    _osDetectProc.waitForFinished();
    _results._hostOS += " " + _osDetectProc.readLine().trimmed();
#endif // Q_WS_MAC
#endif // Q_WS_WIN

    _osDetectProc.close();
    emit osDetectionFinished();
}

void HostInfo::_slotIpDetectDownloadFinished()
{
    QString line;
    QStringList list;
    QString queryUrl = "http://lab.db.ripe.net/whois/search.xml?flags=Kl&type-filter=route&source=ripe&query-string=";

    disconnect(_ipDetectDownload, SIGNAL(finished()), this, SLOT(_slotIpDetectDownloadFinished()));

    if((line = _ipDetectDownload->readLine().trimmed()).isEmpty() || !QUrl(line).isValid())
    {
        _results._ip = trUtf8("whatismyip.com unavailable");
        _results._ipCensored = trUtf8("whatismyip.com unavailable");
    }
    else
    {
        _results._ip = line;
        list = line.split('.');
        _results._ipCensored = list[0] + "." + list[1] + ".xxx.xxx";
        queryUrl += _results._ip;
        _ispNetworkDetectDownload = _manager.get(QNetworkRequest(QUrl(queryUrl)));
        connect(_ispNetworkDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNetworkDetectDownloadFinished()));
    }

    emit ipDetectionFinished();
}

void HostInfo::_slotBbrasDetectProcFinished()
{
    QString line;
    bool bbrasLineFound = false;

    disconnect(&_bbrasDetectProc, SIGNAL(finished(int)), this, SLOT(_slotBbrasDetectProcFinished()));

    while(!bbrasLineFound && _bbrasDetectProc.canReadLine())
    {
        line = _bbrasDetectProc.readLine().trimmed();

        if(line.contains(_bbrasLine))
        {
            bbrasLineFound = true;

            if(line.contains('*'))
            {
                _results._bbras = trUtf8("N/A (non-responsive BBRAS)");
            }
#ifdef Q_WS_WIN
            else
            {
                _results._bbras = line.mid(3);
            }
#else
            else
            {
                _results._bbras = line.mid(4, line.indexOf(')') - 1);
            }
#endif // Q_WS_WIN
        }
    }

    if(!bbrasLineFound)
    {
        _results._bbras = trUtf8("N/A");
    }

    _bbrasDetectProc.close();
    emit bbrasDetectionFinished();
}

void HostInfo::_slotIspNetworkDetectDownloadFinished()
{
    QString line;
    QStringList list;
    bool ispNetworkFound = false;
    QString queryUrl = "http://lab.db.ripe.net/whois/search.xml?flags=rx&type-filter=aut-num&source=ripe&query-string=AS";

    disconnect(_ispNetworkDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNetworkDetectDownloadFinished()));

    while(!ispNetworkFound && !(line = _ispNetworkDetectDownload->readLine().trimmed()).isEmpty())
    {
        if(line.contains(QString("attribute name=\"route\"")))
        {
            list = line.split(QString("value=\""));
            line = list[1];
            line.chop(3);
            _results._ispNetwork = line;
            continue;
        }

        if(line.contains(QString("attribute name=\"origin\"")))
        {
            ispNetworkFound = true;
            list = line.split(QString("value=\""));
            line = list[1];
            line.chop(3);
            _results._ispAsn = line.mid(2);
        }
    }

    if(!ispNetworkFound)
    {
        _results._ispNetwork = trUtf8("RIPE DB unavailable");
        _results._ispAsn = trUtf8("RIPE DB unavailable");
    }
    else
    {
        queryUrl += _results._ispAsn;
        _ispNameDetectDownload = _manager.get(QNetworkRequest(QUrl(queryUrl)));
        connect(_ispNameDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNameDetectDownloadFinished()));

    }

    emit ispNetworkDetectionFinished();
}

void HostInfo::_slotIspNameDetectDownloadFinished()
{
    QString line;
    QStringList list;
    bool ispNameFound = false;

    disconnect(_ispNameDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNameDetectDownloadFinished()));

    while(!ispNameFound && !(line = _ispNameDetectDownload->readLine().trimmed()).isEmpty())
    {
        if(line.contains(QString("attribute name=\"descr\"")))
        {
            ispNameFound = true;
            list = line.split(QString("value=\""));
            line = list[1];
            line.chop(3);
            _results._isp = line;
        }
    }

    if(!ispNameFound)
    {
        _results._isp = trUtf8("RIPE DB unavailable");
    }

    emit ispNameDetectionFinished();
}

void HostInfo::_retrieve()
{
    // Make sure all pending operations have finished
    if(_results._hostOS.isEmpty())
    {
        connect(this, SIGNAL(osDetectionFinished()), &_loop, SLOT(quit()));
        _loop.exec();
        disconnect(this, SIGNAL(osDetectionFinished()), &_loop, SLOT(quit()));
    }

    if(_results._ip.isEmpty())
    {
        connect(this, SIGNAL(ipDetectionFinished()), &_loop, SLOT(quit()));
        _loop.exec();
        disconnect(this, SIGNAL(ipDetectionFinished()), &_loop, SLOT(quit()));
    }

    _ipDetectDownload->abort();
    delete _ipDetectDownload;
    _ipDetectDownload = NULL;

    if(_results._ispAsn.isEmpty())
    {
        connect(this, SIGNAL(ispNetworkDetectionFinished()), &_loop, SLOT(quit()));
        _loop.exec();
        disconnect(this, SIGNAL(ispNetworkDetectionFinished()), &_loop, SLOT(quit()));
    }

    _ispNetworkDetectDownload->abort();
    delete _ispNetworkDetectDownload;
    _ispNetworkDetectDownload = NULL;

    if(_results._isp.isEmpty())
    {
        connect(this, SIGNAL(ispNameDetectionFinished()), &_loop, SLOT(quit()));
        _loop.exec();
        disconnect(this, SIGNAL(ispNameDetectionFinished()), &_loop, SLOT(quit()));
    }

    _ispNameDetectDownload->abort();
    delete _ispNameDetectDownload;
    _ispNameDetectDownload = NULL;

    if(_results._bbras.isEmpty())
    {
        connect(this, SIGNAL(bbrasDetectionFinished()), &_loop, SLOT(quit()));
        _loop.exec();
        disconnect(this, SIGNAL(bbrasDetectionFinished()), &_loop, SLOT(quit()));
    }
    // At this poing, we are sure that all pending operations have finished
}

void HostInfo::slotStartInfoTest()
{
    _retrieve();
    emit finished();
}

void HostInfo::stopInfoTest()
{
    if(_ipDetectDownload)
    {
        if(_ipDetectDownload->isRunning())
        {
            disconnect(_ipDetectDownload, SIGNAL(finished()), this, SLOT(_slotIpDetectDownloadFinished()));
            _ipDetectDownload->abort();
        }

        delete _ipDetectDownload;
        _ipDetectDownload = NULL;
    }

    if(_ispNetworkDetectDownload)
    {
        if(_ispNetworkDetectDownload->isRunning())
        {
            disconnect(_ispNetworkDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNetworkDetectDownloadFinished()));
            _ispNetworkDetectDownload->abort();
        }

        delete _ispNetworkDetectDownload;
        _ispNetworkDetectDownload = NULL;
    }

    if(_ispNameDetectDownload)
    {
        if(_ispNameDetectDownload->isRunning())
        {
            disconnect(_ispNameDetectDownload, SIGNAL(finished()), this, SLOT(_slotIspNameDetectDownloadFinished()));
            _ispNameDetectDownload->abort();
        }

        delete _ispNameDetectDownload;
        _ispNameDetectDownload = NULL;
    }

    if(_bbrasDetectProc.state() != QProcess::NotRunning)
    {
        disconnect(&_bbrasDetectProc, SIGNAL(finished(int)), this, SLOT(_slotBbrasDetectProcFinished()));
        _bbrasDetectProc.close();
    }
}
