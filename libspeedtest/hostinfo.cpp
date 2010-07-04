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
#include <QtCore/QThread>
#include <QtCore/QDateTime>


HostInfo::HostInfo(TestResults *results) {
    this->results = results;
    download = NULL;
    results->cpuCores = QThread::idealThreadCount();
    osDetectProc.setProcessChannelMode(QProcess::MergedChannels);
    traceroute.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&osDetectProc, SIGNAL(finished(int)), &loop, SLOT(quit()));
    QObject::connect(&traceroute, SIGNAL(finished(int)), &loop, SLOT(quit()));

#ifdef Q_WS_WIN
    winArch = (QProcessEnvironment::systemEnvironment().contains("ProgramFiles(x86)"))? "x64" : "x86";
    tracerouteCmd = "pathping -h 2 -q 1 8.8.8.8";
    bbrasLine = "  2  ";

    if(QSysInfo::WindowsVersion == QSysInfo::WV_VISTA || QSysInfo::WindowsVersion == QSysInfo::WV_WINDOWS7)
        osDetectProc.start("systeminfo", QIODevice::ReadOnly);
    else
        osDetectProc.start("cmd /c ver", QIODevice::ReadOnly);
#else
    tracerouteCmd = "traceroute -m 2 -q 1 8.8.8.8";
    bbrasLine = " 2  ";
#ifdef Q_WS_MAC
    osDetectProc.start("sw_vers -productVersion", QIODevice::ReadOnly);
#endif // Q_WS_MAC
#endif // Q_WS_WIN
}


void HostInfo::init() {
    if(download) {
        download->abort();
        delete download;
        download = NULL;
    }

    traceroute.start(tracerouteCmd, QIODevice::ReadOnly);
    download = manager.get(QNetworkRequest(QUrl("http://www.speedtest.net/summary.php")));
    QObject::connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    results->testDate = QDate::currentDate().toString("yyyyMMdd");
    results->testTime = QTime::currentTime().toString("hhmmss");
    results->testDateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
}


void HostInfo::retrieve() {
    QByteArray contents;
    bool foundFlag = false;
    QStringList list;

    results->ip.clear();
    results->isp.clear();
    if(osDetectProc.state() != QProcess::NotRunning) loop.exec();    // Make sure all pending processes have finished
    if(traceroute.state() != QProcess::NotRunning) loop.exec();
    if(download->isRunning()) loop.exec();

    if(results->hostOS.isEmpty()) {
#ifdef Q_WS_WIN
        switch(QSysInfo::WindowsVersion) {
            case QSysInfo::WV_2000:
                results->hostOS = "Windows 2000";
                break;

            case QSysInfo::WV_XP:
                results->hostOS = "Windows XP x86";
                break;

            case QSysInfo::WV_2003:
                results->hostOS = (osDetectProc.readAll().contains("2003"))? ("Windows Server 2003" + winArch) : "Windows XP x64";
                break;

            case QSysInfo::WV_VISTA:
                results->hostOS = (osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 " + winArch) : ("Windows Vista " + winArch);
                break;

            case QSysInfo::WV_WINDOWS7:
                results->hostOS = (osDetectProc.readAll().contains("Windows Server 2008"))? ("Windows Server 2008 R2 " + winArch) : ("Windows 7 " + winArch);
                break;

            default:
                results->hostOS = "Windows (unknown version)";
        }
#else
#ifdef Q_WS_MAC
        results->hostOS = "Mac OS X " + osDetectProc.readLine().trimmed();
        osDetectProc.start("uname -m", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
        osDetectProc.start("uname -o", QIODevice::ReadOnly);
#else
        osDetectProc.start("uname -s", QIODevice::ReadOnly);
#endif // Q_OS_LINUX
        loop.exec();
        results->hostOS = osDetectProc.readLine().trimmed();
        osDetectProc.start("uname -rm", QIODevice::ReadOnly);
        loop.exec();
        results->hostOS += " " + osDetectProc.readLine().trimmed();
#endif // Q_WS_MAC
#endif // Q_WS_WIN
    }

    while(!(contents = download->readLine()).isEmpty() && !foundFlag)
        if(contents.contains("<div class=\"ip\">")) {
            contents.chop(7);
            results->ip = QString(contents.trimmed().mid(16));
            list = results->ip.split('.');
            results->ip = list[0] + "." + list[1] + ".xxx.xxx";
            continue;
        }
        else {
            if(contents.contains("<div class=\"isp\">")) {
                contents.chop(7);
                results->isp = QString(contents.trimmed().mid(17));
                foundFlag = true;
            }
        }

    if(results->ip.isEmpty()) results->ip = QObject::trUtf8("speedtest.net unreachable");
    if(results->isp.isEmpty()) results->isp = QObject::trUtf8("speedtest.net unreachable");

    for(foundFlag = false; !foundFlag && !(contents = traceroute.readLine()).isEmpty();)
        if(contents.contains(bbrasLine.toAscii())) {
            foundFlag = true;
            if(contents.contains(QString("*").toAscii())) results->bbras = QObject::trUtf8("N/A (non-responsive BBRAS)");
#ifdef Q_WS_WIN
            else results->bbras = contents.mid(3).trimmed();
#else
            else results->bbras = contents.mid(4, contents.indexOf(')') - 1).trimmed();
#endif // Q_WS_WIN
        }

    if(!foundFlag) results->bbras = QObject::trUtf8("N/A");
    osDetectProc.close();
    traceroute.close();
    delete download;
    download = NULL;
}
