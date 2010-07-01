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


#include "qspeedtest.h"
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrl>
#include <QtGui/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDateTime>
#include <QtCore/QtConcurrentMap>
#include <QtGui/QClipboard>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QDir>


QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv)
{
    QEventLoop loop;
    QProcess winSystemInfo, proc;

    download = NULL;
    results.cpuCores = QThread::idealThreadCount();
    winSystemInfo.setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_WS_WIN
    if(QSysInfo::WindowsVersion == QSysInfo::WV_VISTA || QSysInfo::WindowsVersion == QSysInfo::WV_WINDOWS7)
    {
        winSystemInfo.start("systeminfo", QIODevice::ReadOnly);
    }
#endif // Q_WS_WIN
    STOPBENCHMARK = false;
    connect(this, SIGNAL(initOK()), &mainWindow, SLOT(enablePushButtonStartStop()));
    connect(&mainWindow, SIGNAL(pushButtonStartClicked()), this, SLOT(startBenchmark()));
    connect(&targetList, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
    connect(this, SIGNAL(benchmarkFinished(bool)), &mainWindow, SLOT(benchmarkFinished(bool)));
    connect(&mainWindow, SIGNAL(pushButtonHtmlCodeClicked(bool)), this, SLOT(showReport(bool)));
    connect(&mainWindow, SIGNAL(pushButtonVbCodeClicked(bool)), this, SLOT(showReport(bool)));
    mainWindow.show();
    checkForProgramUpdates();

    if(targetList.init())
    {
        emit logMessage(trUtf8("Gathering system info, please wait..."));
        results.targetListVersion = targetList.version;
        results.targetListComment = targetList.comment;
        results.targetListContactUrl = targetList.contactUrl;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        connect(&proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
#ifdef Q_WS_WIN
        bbrasLine = "  2  ";
        tracerouteCommand = "pathping -h 2 -q 1 8.8.8.8";
        QString winArch = (QProcessEnvironment::systemEnvironment().contains("ProgramFiles(x86)"))? "x64" : "x86";

        switch(QSysInfo::WindowsVersion)
        {
            case QSysInfo::WV_2000:
                results.hostOS = "Windows 2000";
                break;

            case QSysInfo::WV_XP:
                results.hostOS = "Windows XP x86";
                break;

            case QSysInfo::WV_2003:
                proc.start("cmd /c ver", QIODevice::ReadOnly);
                loop.exec();

                if(proc.readAll().contains("2003"))
                {
                    results.hostOS = "Windows Server 2003 " + winArch;
                }
                else
                {
                    results.hostOS = "Windows XP x64";
                }

                break;

            case QSysInfo::WV_VISTA:
                if(winSystemInfo.state() != QProcess::NotRunning)
                {
                    connect(&winSystemInfo, SIGNAL(finished(int)), &loop, SLOT(quit()));
                    loop.exec();
                }

                if(winSystemInfo.readAll().contains("Windows Server 2008"))
                {
                    results.hostOS = "Windows Server 2008 " + winArch;
                }
                else
                {
                    results.hostOS = "Windows Vista " + winArch;
                }

                break;

            case QSysInfo::WV_WINDOWS7:
                if(winSystemInfo.state() != QProcess::NotRunning)
                {
                    connect(&winSystemInfo, SIGNAL(finished(int)), &loop, SLOT(quit()));
                    loop.exec();
                }

                if(winSystemInfo.readAll().contains("Windows Server 2008"))
                {
                    results.hostOS = "Windows Server 2008 R2 " + winArch;
                }
                else
                {
                   results.hostOS = "Windows 7 " + winArch;
                }

                break;

            default:
                results.hostOS = "Windows (unknown version)";
        }
#else
        bbrasLine = " 2  ";
        tracerouteCommand = "traceroute -m 2 -q 1 8.8.8.8";
#ifdef Q_WS_MAC
        proc.start("sw_vers -productVersion", QIODevice::ReadOnly);
        loop.exec();
        results.hostOS = "Mac OS X " + proc.readLine().trimmed();
        proc.start("uname -m", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
        proc.start("uname -o", QIODevice::ReadOnly);
#else
        proc.start("uname -s", QIODevice::ReadOnly);
#endif // Q_OS_LINUX
        loop.exec();
        results.hostOS = proc.readLine().trimmed();
        proc.start("uname -rm", QIODevice::ReadOnly);
        loop.exec();
        results.hostOS += " " + proc.readLine().trimmed();
#endif // Q_WS_MAC
#endif // Q_WS_WIN
        for(int i = 0; i < targetList.numberOfGroups; i++)
        {
            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                connect(&targetList.groups[i].targets[j], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
            }
        }

        for(int i = 0; i < targetList.fileHostsDomestic.size(); i++)
        {
            connect(&targetList.fileHostsDomestic[i], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
        }

        for(int i = 0; i < targetList.fileHostsInternational.size(); i++)
        {
            connect(&targetList.fileHostsInternational[i], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
        }

        emit logMessage(trUtf8("Ready"));
        emit initOK();
    }
}


void QSpeedTest::checkForProgramUpdates()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    emit logMessage(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROGRAMUPDATECHECKURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        emit logMessage(trUtf8("Update site unreachable"));
        delete download;
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROGRAMVERSION.mid(1).toInt())
    {
        emit logMessage(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        int reply = QMessageBox::question(NULL, trUtf8("Update available"), trUtf8("An updated version of %1 (%2) is available online.\n\nWould you like to open the download page in your browser?").arg(PROGRAMNAME).arg(remoteVersion), QMessageBox::Yes, QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            emit logMessage(trUtf8("Opening download page %1").arg(PROGRAMURL));
            QDesktopServices::openUrl(QUrl(PROGRAMURL));
        }
    }
    else
    {
        emit logMessage(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
    }

    delete download;
}


void QSpeedTest::printHostAndProgramInfo()
{
    results.testDate = QDate::currentDate().toString("yyyyMMdd");
    results.testTime = QTime::currentTime().toString("hhmmss");
    results.testDateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    emit newTestResult(trUtf8("\n\n"
                              "Report created by:        %1 %2\n"
                              "Target list used:         %3 %4\n"
                              "Target list contact URL:  %5").arg(results.programName).arg(results.programVersion).arg(results.targetListVersion).arg(results.targetListComment).arg(results.targetListContactUrl));
    emit newTestResult(trUtf8("Test date and time:       %1\n"
                              "Host OS & no. of CPUs:    %2 - %3 CPU cores").arg(results.testDateTime).arg(results.hostOS).arg(results.cpuCores));
}


void QSpeedTest::getLineInfo()
{
    if(download)
    {
        download->abort();
        delete download;
        download = NULL;
    }

    traceroute.setProcessChannelMode(QProcess::MergedChannels);
    traceroute.start(tracerouteCommand, QIODevice::ReadOnly);
    delete download;
    download = manager.get(QNetworkRequest(QUrl("http://www.speedtest.net/summary.php")));
}


void QSpeedTest::printLineInfo()
{
    QEventLoop loop1, loop2;
    QByteArray contents;
    bool foundFlag = false;
    QStringList list;

    results.ip.clear();
    results.isp.clear();

    if(download->isRunning())
    {
        connect(download, SIGNAL(finished()), &loop1, SLOT(quit()));
        loop1.exec();
    }

    while(!(contents = download->readLine()).isEmpty() && !foundFlag)
    {
        if(contents.contains("<div class=\"ip\">"))
        {
            contents.chop(7);
            results.ip = QString(contents.trimmed().mid(16));
            list = results.ip.split('.');
            results.ip = list[0] + "." + list[1] + ".xxx.xxx";
            continue;
        }
        else
        {
            if(contents.contains("<div class=\"isp\">"))
            {
                contents.chop(7);
                results.isp = QString(contents.trimmed().mid(17));
                foundFlag = true;
            }
        }
    }

    if(results.ip.isEmpty())
    {
        results.ip = trUtf8("speedtest.net unreachable!");
    }

    if(results.isp.isEmpty())
    {
        results.isp = trUtf8("speedtest.net unreachable!");
    }

    if(traceroute.state() != QProcess::NotRunning)
    {
        connect(&traceroute, SIGNAL(finished(int)), &loop2, SLOT(quit()));
        loop2.exec();
    }

    for(foundFlag = false; !foundFlag && !(contents = traceroute.readLine()).isEmpty();)
    {
        if(contents.contains(bbrasLine.toAscii()))
        {
            foundFlag = true;

            if(contents.contains(QString("*").toAscii()))
            {
                results.bbras = trUtf8("N/A (non-responsive BBRAS)");
            }
            else
            {
#ifdef Q_WS_WIN
                results.bbras = contents.mid(3).trimmed();
#else
                results.bbras = contents.mid(4, contents.indexOf(')') - 1).trimmed();
#endif // Q_WS_WIN
            }
        }
    }

    if(!foundFlag)
    {
        results.bbras = trUtf8("N/A");
    }

    delete download;
    download = NULL;
    emit newTestResult(trUtf8("ISP & WAN IP:             %1 - %2\n"
                              "BBRAS:                    %3").arg(results.isp).arg(results.ip).arg(results.bbras));
}


void QSpeedTest::startBenchmark()
{
    QTime timer;

    timer.start();
    emit logMessage(trUtf8("Test started"));
    results.targetsTotal = targetList.numberOfTargets;
    results.pingsPerTarget = PINGSPERTARGET;
    results.parallelPingThreads = mainWindow.parallelThreads();
    pingTestEnabled = mainWindow.pingTestEnabled();
    downloadTestEnabled = mainWindow.downloadTestEnabled();
    vbCode.clear();
    htmlCode.clear();
    results.reset();
    QThreadPool::globalInstance()->setMaxThreadCount(results.parallelPingThreads - 1);
    getLineInfo();

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabled; i++)
    {
        targetList.groups[i].reset();
        emit newTestResult(targetList.groups[i].getName().leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        emit newTestResult("-------------------------------------------------------------------------------");

        if(results.parallelPingThreads > 1)    // multithreaded pinging
        {
            QtConcurrent::blockingMap(targetList.groups[i].targets, &Target::ping);
        }
        else    // single-threaded pinging
        {
            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                if(STOPBENCHMARK)
                {
                    emit benchmarkFinished(true);
                    return;
                }

                targetList.groups[i].targets[j].ping();
            }
        }

        for(int j = 0; j < targetList.groups[i].getSize(); j++)
        {
            targetList.groups[i].plainTargets.append(PlainTarget(targetList.groups[i].targets[j]));
        }

        qSort(targetList.groups[i].plainTargets);

        if(STOPBENCHMARK)
        {
            emit benchmarkFinished(STOPBENCHMARK);
            return;
        }

        processEvents();
        results.rttSum += targetList.groups[i].getRttSum();
        results.targetsAlive += targetList.groups[i].getTargetsAlive();
        emit newTestResult(trUtf8("Group sum:     %1\n"
                                  "Group average: %2\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()));
    }

    if(STOPBENCHMARK)
    {
        emit benchmarkFinished(STOPBENCHMARK);
        return;
    }

    if(downloadTestEnabled)
    {
        foreach(QList<FileHost>* fileHosts, QList<QList<FileHost>*>() << &targetList.fileHostsDomestic << &targetList.fileHostsInternational)
        {
            for(int i = 0; i < fileHosts->size(); i++)
            {
                connect(&mainWindow, SIGNAL(pushButtonStopClicked()), &(fileHosts->operator [](i)), SLOT(abortDownload()));
            }

            BYTESDOWNLOADED = 0;
            emit newTestResult(trUtf8("\nDownloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
            QThreadPool::globalInstance()->setMaxThreadCount(fileHosts->size());
            QtConcurrent::blockingMap(*fileHosts, &FileHost::downloadTest);
            processEvents();

            if(fileHosts == &targetList.fileHostsDomestic)
            {
                MUTEX.lock();
                results.speedInKbpsDomestic = (BYTESDOWNLOADED) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
                MUTEX.unlock();
                results.speedInMBpsDomestic = results.speedInKbpsDomestic / 8192;    // (results.speedInKbpsDomestic / 1024) / 8
            }
            else
            {
                MUTEX.lock();
                results.speedInKbpsInternational = (BYTESDOWNLOADED) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
                MUTEX.unlock();
                results.speedInMBpsInternational = results.speedInKbpsInternational / 8192;    // (results.speedInKbpsInernational / 1024) / 8
            }

            for(int i = 0; i < fileHosts->size(); i++)
            {
                disconnect(&mainWindow, SIGNAL(pushButtonStopClicked()), &(fileHosts->operator [](i)), SLOT(abortDownload()));
            }

            if(STOPBENCHMARK)
            {
                emit benchmarkFinished(STOPBENCHMARK);
                return;
            }
        }

        if(pingTestEnabled)
        {
            results.testMode = trUtf8("Ping and speed");
        }
        else
        {
            results.testMode = trUtf8("Speed only");
        }
    }
    else
    {
        results.testMode = trUtf8("Ping only");
    }

    printHostAndProgramInfo();
    printLineInfo();
    results.testDuration = (timer.elapsed() * 1.0) / 1000;
    emit newTestResult(trUtf8(""
                                  "Test completed in:        %1 sec").arg(results.testDuration));

    if(pingTestEnabled)
    {
        emit newTestResult(trUtf8("Pings/target:             %1\n"
                                  "Parallel ping threads:    %2").arg(results.pingsPerTarget).arg(results.parallelPingThreads));
        emit newTestResult(trUtf8("Targets alive:            %1 / %2\n"
                                  "Test total ping time:     %3\n"
                                  "Average ping/target:      %4").arg(results.targetsAlive).arg(results.targetsTotal).arg(results.getRttSumAsString()).arg(results.getRttAvgAsString()));
    }

    if(downloadTestEnabled)
    {
        emit newTestResult(trUtf8("Speed test domestic:      %1 Kbps\n"
                                  "                          %2 MB/sec").arg(results.speedInKbpsDomestic, 0, 'f', 0).arg(results.speedInMBpsDomestic, 0, 'f', 3));
        emit newTestResult(trUtf8("Speed test international: %1 Kbps\n"
                                  "                          %2 MB/sec").arg(results.speedInKbpsInternational, 0, 'f', 0).arg(results.speedInMBpsInternational, 0, 'f', 3));
    }

    emit logMessage(trUtf8("Test complete"));
    emit benchmarkFinished(STOPBENCHMARK);
}


void QSpeedTest::showReport(bool showHtml)
{
    QFile file;
    QString fileName;
    QTextStream outStream;

    if(showHtml)
    {
        generateHtmlCode();
        fileName = QDir::tempPath() + QString("/%1_%2%3.html").arg(results.programName).arg(results.testDate).arg(results.testTime);
    }
    else
    {
        generateVbCode();
        fileName = QDir::tempPath() + QString("/%1_%2%3.vb.txt").arg(results.programName).arg(results.testDate).arg(results.testTime);
    }

    fileName = QDir::toNativeSeparators(fileName);
    file.setFileName(fileName);
    outStream.setDevice(&file);
    outStream.setCodec("UTF-8");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(NULL, trUtf8("Error"), trUtf8("Error writing temporary file %1").arg(fileName));
        return;
    }

    outStream << ((showHtml)? htmlCode : vbCode);
    file.close();
    QApplication::clipboard()->setText((showHtml)? htmlCode : vbCode);
    mainWindow.showStatusBarMessage(trUtf8("Code copied to clipboard"));
    QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(fileName)));
}


void QSpeedTest::generateHtmlCode()
{
    if(!htmlCode.isEmpty())
    {
        return;
    }

    htmlCode  = trUtf8("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                       "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                       "    <head>\n"
                       "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
                       "        <title>%1 %2 report - %3</title>\n").arg(results.programName).arg(results.programVersion).arg(results.testDateTime);
    htmlCode += trUtf8("    </head>\n"
                       "    <body>\n"
                       "        <table border=\"1\" cellpadding=\"4\">\n"
                       "            <tr><td>Report created by</td><td align=\"center\">%1 %2 - <a href=\"%3\">Download</a> - <a href=\"%4\">Discuss</a></td></tr>\n").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl);
    htmlCode += trUtf8("            <tr><td>Target list used</td><td align=\"center\">%1 <a href=\"%2\">%3</a></td></tr>\n").arg(results.targetListVersion).arg(results.targetListContactUrl).arg(results.targetListComment);
    htmlCode += trUtf8("            <tr><td>Test date and time</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Host OS & no. of CPUs</td><td align=\"center\">%2 - %3 CPU cores</td></tr>\n").arg(results.testDateTime).arg(results.hostOS).arg(results.cpuCores);
    htmlCode += trUtf8("            <tr><td>ISP & WAN IP</td><td align=\"center\">%1 - %2</td></tr>\n"
                       "            <tr><td>BBRAS</td><td align=\"center\">%3</td></tr>\n").arg(results.isp).arg(results.ip).arg(results.bbras);
    htmlCode += trUtf8("            <tr><td>Test mode</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Test completed in</td><td align=\"center\">%2 sec</td></tr>\n").arg(results.testMode).arg(results.testDuration);

    if(pingTestEnabled)
    {
        htmlCode += trUtf8(""
                       "            <tr><td>Pings/target</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Parallel ping threads</td><td align=\"center\">%2</td></tr>\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
        htmlCode += trUtf8(""
                       "            <tr><td>Targets alive</td><td align=\"center\">%1 / %2</td></tr>\n").arg(results.targetsAlive).arg(results.targetsTotal);
        htmlCode += trUtf8(""
                       "            <tr><td><b>Average ping/target</b></td><td align=\"center\"><b>%1</b></td></tr>\n").arg(results.getRttAvgAsString());
    }

    if(downloadTestEnabled)
    {
        htmlCode += trUtf8(""
                       "            <tr><td><b>Speed test domestic</b></td><td align=\"center\"><b>%1 Kbps or %2 MB/sec</b></td></tr>\n"
                       "").arg(results.speedInKbpsDomestic, 0, 'f', 0).arg(results.speedInMBpsDomestic, 0, 'f', 3);
        htmlCode += trUtf8(""
                       "            <tr><td><b>Speed test international</b></td><td align=\"center\"><b>%1 Kbps or %2 MB/sec</b></td></tr>\n"
                       "").arg(results.speedInKbpsInternational, 0, 'f', 0).arg(results.speedInMBpsInternational, 0, 'f', 3);
    }

    htmlCode +=        "        </table>\n";
    htmlCode += (pingTestEnabled)? (""
                       "        <p style=\"margin-bottom: 2px;\"><b>Detailed results</b></p>\n"
                       "        <div>\n"
                       "            <input type=\"button\" id=\"GroupsSpoilerButton\" value=\"Show\" style=\"width: 80px; height: 25px; font-size: 9pt;\" onclick=\""
                       "                if(document.getElementById('GroupsDiv').style.display != '')\n"
                       "                {\n"
                       "                    document.getElementById('GroupsDiv').style.display = '';\n"
                       "                    this.innerText = '';\n"
                       "                    this.value = 'Hide';\n"
                       "                }\n"
                       "                else\n"
                       "                {\n"
                       "                    document.getElementById('GroupsDiv').style.display = 'none';\n"
                       "                    this.innerText = '';\n"
                       "                    this.value = 'Show';\n"
                       "                }\"/>\n"
                       "        </div>\n"
                       "        <div id=\"GroupsDiv\" style=\"display: none; border: 1px solid black; padding: 5px; margin: 2px;\">\n") : NULL;

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabled; i++)
    {
        htmlCode += trUtf8(""
                       "            <p style=\"margin-top: 2px; margin-bottom: 2px;\"><b>%1</b></p>\n"
                       "            <div>\n"
                       "                <input type=\"button\" id=\"Group%2SpoilerButton\" value=\"Show\" style=\"width: 80px; height: 25px; font-size: 9pt;\" onclick=\""
                       "                    if(document.getElementById('Group%2Div').style.display != '')\n"
                       "                    {\n"
                       "                        document.getElementById('Group%2Div').style.display = '';\n"
                       "                        this.innerText = '';\n"
                       "                        this.value = 'Hide';\n"
                       "                    }\n"
                       "                    else\n"
                       "                    {\n"
                       "                        document.getElementById('Group%2Div').style.display = 'none';\n"
                       "                        this.innerText = '';\n"
                       "                        this.value = 'Show';\n"
                       "                    }\"/>\n"
                       "            </div>\n"
                       "            <div id=\"Group%2Div\" style=\"display: none; border: 1px solid black; padding: 5px; margin: 2px;\">\n"
                       "                <table border=\"1\" cellpadding=\"4\">\n"
                       "                    <tr><td>Target</td><td>Average ping time</td><td>Packet loss</td><td align=\"center\">Jitter</td><td>Rank</td></tr>\n").arg(targetList.groups[i].getName()).arg(i);

        for(int j = 0; j < targetList.groups[i].getSize(); j++)
        {
            htmlCode += QString(""
                       "                    <tr><td><a href=\"http://%1\">%2</a></td><td align=\"right\">%3</td><td align=\"right\">%4</td><td align=\"right\">%5</td><td align=\"center\">%6</td></tr>\n").arg(targetList.groups[i].plainTargets[j].getAddress()).arg(targetList.groups[i].plainTargets[j].getName()).arg(targetList.groups[i].plainTargets[j].getRttAsString()).arg(targetList.groups[i].plainTargets[j].getPacketLossAsString()).arg(targetList.groups[i].plainTargets[j].getJitterAsString()).arg(targetList.groups[i].plainTargets[j].getRank());
        }

        htmlCode += trUtf8(""
                       "                    <tr><td><b>Group sum</b></td><td align=\"right\"><b>%1</b></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"
                       "                    <tr><td><b>Group average</b></td><td align=\"right\"><b>%2</b></td><td align=\"right\"><b>%3</b></td><td>&nbsp;</td><td align=\"center\"><b>%4</b></td></tr>\n"
                       "                </table>\n"
                       "            </div>\n"
                       "            <br/>\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()).arg(targetList.groups[i].getPacketLossAvgAsString()).arg(targetList.groups[i].getRank());
    }

    htmlCode += (pingTestEnabled)? (""
                       "        </div>\n") : NULL;
    htmlCode +=        "        <p>\n"
                       "            <a href=\"http://validator.w3.org\">\n"
                       "                <img src=\"http://www.w3.org/Icons/valid-xhtml11\" alt=\"Valid XHTML 1.1\" height=\"31\" width=\"88\" />\n"
                       "            </a>\n"
                       "        </p>\n"
                       "    </body>\n"
                       "</html>\n";
    htmlCode.replace('&', "&amp;");
    htmlCode.replace("&amp;nbsp", "&nbsp");
}


void QSpeedTest::generateVbCode()
{
    if(!vbCode.isEmpty())
    {
        return;
    }

    vbCode  = trUtf8("[table=head] | \n"
                     "Report created by | [center]%1 %2 - [url=%3]Download[/url] - [url=%4]Discuss[/url][/center] |\n"
                     "Target list used | [center]%5 [url=%6]%7[/url][/center] |\n"
                     "Test date and time | [center]%8[/center] |\n"
                     "Host OS & no. of CPUs | [center]%9 - %10 CPU cores[/center] |\n"
                     "").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl).arg(results.targetListVersion).arg(results.targetListContactUrl).arg(results.targetListComment).arg(results.testDateTime).arg(results.hostOS).arg(results.cpuCores);
    vbCode += trUtf8("ISP & WAN IP | [center]%1 - %2[/center] |\n"
                     "BBRAS | [center]%3[/center] |\n"
                     "Test mode | [center]%4[/center] |\n"
                     "Test completed in | [center]%5 sec[/center] |\n").arg(results.isp).arg(results.ip).arg(results.bbras).arg(results.testMode).arg(results.testDuration);

    if(pingTestEnabled)
    {
        vbCode += trUtf8(""
                     "Pings/target | [center]%1[/center] |\n"
                     "Parallel ping threads | [center]%2[/center] |\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
        vbCode += trUtf8(""
                     "Targets alive | [center]%1 / %2[/center] |\n"
                     "[b]Average ping/target[/b] | [center][b]%3[/b][/center] |\n").arg(results.targetsAlive).arg(results.targetsTotal).arg(results.getRttAvgAsString());
    }

    if(downloadTestEnabled)
    {
        vbCode += trUtf8(""
                     "[b]Speed test domestic[/b] | [center][b]%1 Kbps or %2 MB/sec[/b][/center] |\n"
                     "").arg(results.speedInKbpsDomestic, 0, 'f', 0).arg(results.speedInMBpsDomestic, 0, 'f', 3);
        vbCode += trUtf8(""
                     "[b]Speed test international[/b] | [center][b]%1 Kbps or %2 MB/sec[/b][/center] |\n"
                     "").arg(results.speedInKbpsInternational, 0, 'f', 0).arg(results.speedInMBpsInternational, 0, 'f', 3);
    }

    vbCode +=        "[/table]\n\n";
    vbCode += (pingTestEnabled)? (""
                     "\n[b]Detailed results[/b]\n"
                     "[spoiler]\n") : NULL;

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabled; i++)
    {
        vbCode += trUtf8(""
                     "[b]%1[/b]\n"
                     "[spoiler]\n"
                     "[table=head][center]Target[/center] | [center]Average ping time[/center] | Packet loss | Jitter | Rank\n").arg(targetList.groups[i].getName());

        for(int j = 0; j < targetList.groups[i].getSize(); j++)
        {
            vbCode += QString(""
                     "%1 | [right]%2[/right] | [right]%3[/right] | [right]%4[/right] | [center]%5[/center] |\n").arg(targetList.groups[i].plainTargets[j].getName()).arg(targetList.groups[i].plainTargets[j].getRttAsString()).arg(targetList.groups[i].plainTargets[j].getPacketLossAsString()).arg(targetList.groups[i].plainTargets[j].getJitterAsString()).arg(targetList.groups[i].plainTargets[j].getRank());
        }

        vbCode += trUtf8(""
                     "[b]Group sum[/b] | [right][b]%1[/b][/right] |\n"
                     "[b]Group average[/b] | [right][b]%2[/b][/right] | [right][b]%3[/b][/right] | | [center][b]%4[/b][/center]\n"
                     "[/table]\n"
                     "[/spoiler]\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()).arg(targetList.groups[i].getPacketLossAvgAsString()).arg(targetList.groups[i].getRank());
    }

    vbCode += ((pingTestEnabled)? ""
                     "[/spoiler]\n" : NULL);
}
