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
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDateTime>
#include <QtConcurrentMap>
#include <QClipboard>


QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv)
{
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
    connect(this, SIGNAL(benchmarkFinished(bool)), &mainWindow, SLOT(updateButtons(bool)));
    connect(&mainWindow, SIGNAL(pushButtonCopyVbCodeClicked()), this, SLOT(copyVbCode()));
    connect(&mainWindow, SIGNAL(pushButtonCopyHtmlCodeClicked()), this, SLOT(copyHtmlCode()));
    mainWindow.show();
    checkForProgramUpdates();

    if(targetList.init())
    {
        for(int i = 0; i < targetList.numberOfGroups; i++)
        {
            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                connect(&targetList.groups[i].targets[j], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
            }
        }

        for(int i = 0; i < targetList.fileHosts.size(); i++)
        {
            connect(&targetList.fileHosts[i], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
        }

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
    download = manager.get(QNetworkRequest(QUrl(PROGRAMUPDATECHECKURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->error())
    {
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
    results.targetListVersion = targetList.version;
    results.targetListComment = targetList.comment;
    results.targetListContactUrl = targetList.contactUrl;
    results.testDate = QDate::currentDate().toString("yyyyMMdd");
    results.testTime = QTime::currentTime().toString("hhmmss");
    testDateTime = QDateTime::currentDateTime().toString("dddd dd/MM/yyyy hh:mm:ss");
    emit newTestResult(trUtf8("Report created by: %1 %2\n"
                              "Target list version: %3\n"
                              "Target list comment: %4\n"
                              "Target list contact URL: %5").arg(results.programName).arg(results.programVersion).arg(results.targetListVersion).arg(results.targetListComment).arg(results.targetListContactUrl));
    proc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&proc, SIGNAL(finished(int)), &loop1, SLOT(quit()));
#ifdef Q_WS_WIN
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
            loop1.exec();

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
                connect(&winSystemInfo, SIGNAL(finished(int)), &loop1, SLOT(quit()));
                loop1.exec();
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
                connect(&winSystemInfo, SIGNAL(finished(int)), &loop1, SLOT(quit()));
                loop1.exec();
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
#ifdef Q_WS_MAC
    proc.start("sw_vers -productVersion", QIODevice::ReadOnly);
    loop1.exec();
    results.hostOS = "Mac OS X " + proc.readLine().trimmed();
    proc.start("uname -m", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
    proc.start("uname -o", QIODevice::ReadOnly);
#else
    proc.start("uname -s", QIODevice::ReadOnly);
#endif // Q_OS_LINUX
    loop1.exec();
    results.hostOS = proc.readLine().trimmed();
    proc.start("uname -rm", QIODevice::ReadOnly);
    loop1.exec();
    results.hostOS += " " + proc.readLine().trimmed();
#endif // Q_WS_MAC
#endif // Q_WS_WIN

    emit newTestResult(trUtf8("Host OS: %1\n"
                        "Test date and time: %2").arg(results.hostOS).arg(testDateTime));
}


void QSpeedTest::printLineInfo()
{
    foundFlag = false;
#ifdef Q_WS_WIN
    tracerouteCommand = "pathping -h 2 -q 1 8.8.8.8";
    bbrasLine = "  2  ";
#else
    tracerouteCommand = "traceroute -m 2 -q 1 8.8.8.8";
    bbrasLine = " 2  ";
#endif // Q_WS_WIN

    results.ip.clear();
    results.isp.clear();
    traceroute.setProcessChannelMode(QProcess::MergedChannels);
    connect(&traceroute, SIGNAL(finished(int)), &loop2, SLOT(quit()));
    traceroute.start(tracerouteCommand, QIODevice::ReadOnly);
    download = manager.get(QNetworkRequest(QUrl("http://www.speedtest.net/summary.php")));
    connect(download, SIGNAL(finished()), &loop1, SLOT(quit()));
    loop1.exec();

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
    emit newTestResult(trUtf8("ISP: %1\nInternet IP: %2\nBBRAS: %3\n").arg(results.isp).arg(results.ip).arg(results.bbras));
}


void QSpeedTest::startBenchmark()
{
    timer.start();
    emit logMessage(trUtf8("Test started"));
    results.targetsTotal = targetList.numberOfTargets;
    results.parallelPingThreads = mainWindow.parallelThreads();
    pingTestEnabledFlag = mainWindow.pingTestEnabled();
    downloadTestEnabledFlag = mainWindow.downloadTestEnabled();
    vbCode.clear();
    htmlCode.clear();
    results.reset();
    QThreadPool::globalInstance()->setMaxThreadCount(results.parallelPingThreads - 1);
    printHostAndProgramInfo();
    printLineInfo();

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabledFlag; i++)
    {
        targetList.groups[i].reset();
        emit newTestResult(targetList.groups[i].getName().leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        emit newTestResult("-------------------------------------------------------------------------------");

        if(results.parallelPingThreads > 1)
        {
            QtConcurrent::blockingMap(targetList.groups[i].targets, &Target::ping);
        }
        else
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

        if(STOPBENCHMARK)
        {
            emit benchmarkFinished(STOPBENCHMARK);
            return;
        }

        processEvents();
        results.rttSum += targetList.groups[i].getRttSum();
        results.targetsAlive += targetList.groups[i].getTargetsAlive();
        emit newTestResult(trUtf8("Group sum: %1\nGroup average: %2\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()));
    }

    if(STOPBENCHMARK)
    {
        emit benchmarkFinished(STOPBENCHMARK);
        return;
    }

    if(downloadTestEnabledFlag)
    {
        emit newTestResult(trUtf8("Downloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
        QThreadPool::globalInstance()->setMaxThreadCount(targetList.fileHosts.size());

        for(int i = 0; i < targetList.fileHosts.size(); i++)
        {
            connect(&mainWindow, SIGNAL(pushButtonStopClicked()), &targetList.fileHosts[i], SLOT(abortDownload()));
        }

        BYTESDOWNLOADED = 0;
        QtConcurrent::blockingMap(targetList.fileHosts, &FileHost::downloadTest);
        processEvents();
        results.speedInKbps = (BYTESDOWNLOADED * 8) / (DOWNLOADTESTSECS * 1024 * 1.0);
        results.speedInMBps = results.speedInKbps / (8 * 1024);

        for(int i = 0; i < targetList.fileHosts.size(); i++)
        {
            disconnect(&mainWindow, SIGNAL(pushButtonStopClicked()), &targetList.fileHosts[i], SLOT(abortDownload()));
        }

        if(STOPBENCHMARK)
        {
            emit benchmarkFinished(STOPBENCHMARK);
            return;
        }

        if(pingTestEnabledFlag)
        {
            results.testMode = trUtf8("Ping and download");
        }
        else
        {
            results.testMode = trUtf8("Download only");
        }
    }
    else
    {
        results.testMode = trUtf8("Ping only");
    }

    results.testDuration = (timer.elapsed() * 1.0) / 1000;
    emit newTestResult(trUtf8("\nTest completed in: %1 sec").arg(results.testDuration));

    if(pingTestEnabledFlag)
    {
        emit newTestResult(trUtf8("Pings/target: %1\nParallel ping threads: %2").arg(results.pingsPerTarget).arg(results.parallelPingThreads));
        emit newTestResult(trUtf8("Targets unreachable: %1 / %2\n"
                                  "Test total ping time: %3\n"
                                  "Average ping time/target: %4").arg(results.targetsTotal - results.targetsAlive).arg(results.targetsTotal).arg(results.getRttSumAsString()).arg(results.getRttAvgAsString()));
    }

    if(downloadTestEnabledFlag)
    {
        emit newTestResult(trUtf8("Download speed (Kbps): %1 Kbps\n"
                                  "Download speed (MBps): %2 MB/sec").arg(results.speedInKbps).arg(results.speedInMBps));
    }

    emit logMessage(trUtf8("Test complete"));
    emit benchmarkFinished(STOPBENCHMARK);
}


void QSpeedTest::copyVbCode()
{
    if(vbCode.isEmpty())
    {
        vbCode  = trUtf8("[table=head] | Client info\n"
                         "Report created by | [center]%1 %2 - [url=%3]Download[/url] - [url=%4]Discuss[/url][/center] |\n"
                         "Target list version | [center]%5[/center] |\n"
                         "Target list comment | [center][url=%6]%7[/url][/center] |\n"
                         "Host OS | [center]%8[/center] |\n"
                         "Test date and time | [center]%9[/center] |\n").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl).arg(results.targetListVersion).arg(results.targetListContactUrl).arg(results.targetListComment).arg(results.hostOS).arg(testDateTime);
        vbCode += trUtf8("ISP | [center]%1[/center] |\n"
                         "Internet IP | [center]%2[/center] |\n"
                         "BBRAS | [center]%3[/center] |\n"
                         "[/table]\n").arg(results.isp).arg(results.ip).arg(results.bbras);
        vbCode += (pingTestEnabledFlag)? (""
                         "[spoiler]\n") : NULL;

        for(int i = 0; i < targetList.numberOfGroups && pingTestEnabledFlag; i++)
        {
            vbCode += trUtf8(""
                         "[b]%1[/b]\n"
                         "[spoiler]\n"
                         "[table=head][center]Target[/center] | [center]Average ping time[/center] | Packet loss | Jitter | Rank\n").arg(targetList.groups[i].getName());

            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                vbCode += QString(""
                         "%1 | [right]%2[/right] | [right]%3[/right] | [right]%4[/right] | [center]%5[/center] |\n").arg(targetList.groups[i].targets[j].getName()).arg(targetList.groups[i].targets[j].getRttAvgAsString()).arg(targetList.groups[i].targets[j].getPacketLossAsString()).arg(targetList.groups[i].targets[j].getJitterAsString()).arg(targetList.groups[i].targets[j].getRank());
            }

            vbCode += trUtf8(""
                         "[b]Group sum[/b] | [right][b]%1[/b][/right] |\n"
                         "[b]Group average[/b] | [right][b]%2[/b][/right] | [right][b]%3[/b][/right] | | [center][b]%4[/b][/center]\n"
                         "[/table]\n"
                         "[/spoiler]\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()).arg(targetList.groups[i].getPacketLossAvgAsString()).arg(targetList.groups[i].getRank());
        }

        vbCode += ((pingTestEnabledFlag)? ""
                         "[/spoiler]\n" : NULL);
        vbCode += trUtf8("[table=head][center]Variable[/center] | Value\n"
                         "[b]Test mode[/b] | [center]%1[/center] |\n"
                         "[b]Test completed in[/b] | [center]%2 sec[/center] |\n").arg(results.testMode).arg(results.testDuration);

        if(pingTestEnabledFlag)
        {
            vbCode += trUtf8(""
                         "[b]Pings/target[/b] | [center]%1[/center] |\n"
                         "[b]Parallel ping threads[/b] | [center]%2[/center] |\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
            vbCode += trUtf8(""
                         "[b]Targets unreachable[/b] | [center]%1 / %2[/center] |\n"
                         "[b]Test total ping time[/b] | [center]%3[/center] |\n"
                         "[b]Average ping time/target[/b] | [center]%4[/center] |\n").arg(results.targetsTotal - results.targetsAlive).arg(results.targetsTotal).arg(results.getRttSumAsString()).arg(results.getRttAvgAsString());
        }

        if(downloadTestEnabledFlag)
        {
            vbCode += trUtf8(""
                         "[b]Download speed (Kbps)[/b] | [center]%1 Kbps[/center] |\n"
                         "[b]Download speed (MBps)[/b] | [center]%2 MB/sec[/center] |\n").arg(results.speedInKbps).arg(results.speedInMBps);
        }

        vbCode +=        "[/table]\n\n";
    }

    QApplication::clipboard()->setText(vbCode);
    mainWindow.showClipboardConfirmation(trUtf8("<p style=\"align: center; color: green;\">vBulletin code copied to clipboard</p>"));
    logMessage(trUtf8("vBulletin code copied to clipboard"));
}


void QSpeedTest::copyHtmlCode()
{
    if(htmlCode.isEmpty())
    {
        htmlCode  = trUtf8("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                           "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                           "    <head>\n"
                           "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
                           "        <title>%1 report</title>\n").arg(results.programName);
        htmlCode += trUtf8("    </head>\n"
                           "    <body>\n"
                           "        <table border=\"1\" cellpadding=\"4\">\n"
                           "            <tr><td>&nbsp;</td><td><div align=\"center\">Client info</div></td></tr>\n"
                           "            <tr><td>Report created by</td><td><div align=\"center\">%1 %2 - <a href=\"%3\" target=\"_blank\">Download</a> - <a href=\"%4\" target=\"_blank\">Discuss</a></div></td></tr>\n").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl);
        htmlCode += trUtf8("            <tr><td>Target list used</td><td><div align=\"center\"><a href=\"%1\">%2</a></div></td></tr>\n").arg(results.targetListContactUrl).arg(results.targetListComment);
        htmlCode += trUtf8("            <tr><td>Host OS</td><td><div align=\"center\">%1</div></td></tr>\n"
                           "            <tr><td>Test date and time</td><td><div align=\"center\">%2</div></td></tr>\n").arg(results.hostOS).arg(testDateTime);
        htmlCode += trUtf8("            <tr><td>ISP</td><td><div align=\"center\">%1</div></td></tr>\n"
                           "            <tr><td>Internet IP</td><td><div align=\"center\">%2</div></td></tr>\n"
                           "            <tr><td>BBRAS</td><td><div align=\"center\">%3</div></td></tr>\n"
                           "        </table>\n").arg(results.isp).arg(results.ip).arg(results.bbras);

        for(int i = 0; i < targetList.numberOfGroups && pingTestEnabledFlag; i++)
        {
            htmlCode += trUtf8(""
                           "<br/>\n"
                           "        <b>%1</b>\n"
                           "        <br/>\n"
                           "        <table border=\"1\" cellpadding=\"4\">\n"
                           "            <tr><td>Target</td><td>Average ping time</td><td>Packet loss</td><td><div align=\"center\">Jitter</div></td><td>Rank</td></tr>\n").arg(targetList.groups[i].getName());

            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                htmlCode += QString("            <tr><td><a href=\"%1\">%2</a></td><td><div align=\"right\">%3</div></td><td><div align=\"right\">%4</div></td><td><div align=\"right\">%5</div></td><td><div align=\"center\">%6</div></td></tr>\n").arg(targetList.groups[i].targets[j].getAddress()).arg(targetList.groups[i].targets[j].getName()).arg(targetList.groups[i].targets[j].getRttAvgAsString()).arg(targetList.groups[i].targets[j].getPacketLossAsString()).arg(targetList.groups[i].targets[j].getJitterAsString()).arg(targetList.groups[i].targets[j].getRank());
            }

            htmlCode += trUtf8(""
                           "            <tr><td><b>Group sum</b></td><td><div align=\"right\"><b>%1</b></div></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"
                           "            <tr><td><b>Group average</b></td><td><div align=\"right\"><b>%2</b></div></td><td><div align=\"right\"><b>%3</b></div></td><td>&nbsp;</td><td><div align=\"center\"><b>%4</b></div></td></tr>\n"
                           "        </table>\n"
                           "        <br/>\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()).arg(targetList.groups[i].getPacketLossAvgAsString()).arg(targetList.groups[i].getRank());
        }

        htmlCode += trUtf8("        <table border=\"1\" cellpadding=\"4\">\n"
                           "            <tr><td><div align=\"center\">Variable</div></td><td><div align=\"center\">Value</div></td></tr>\n"
                           "            <tr><td><b>Test mode</b></td><td><div align=\"center\">%1</div></td></tr>\n"
                           "            <tr><td><b>Test completed in</b></td><td><div align=\"center\">%2 sec</div></td></tr>\n").arg(results.testMode).arg(results.testDuration);

        if(pingTestEnabledFlag)
        {
            htmlCode += trUtf8(""
                           "            <tr><td><b>Pings/target</b></td><td><div align=\"center\">%1</div></td></tr>\n"
                           "            <tr><td><b>Parallel ping threads</b></td><td><div align=\"center\">%2</div></td></tr>\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
            htmlCode += trUtf8(""
                           "            <tr><td><b>Targets unreachable</b></td><td><div align=\"center\">%1 / %2</div></td></tr>\n").arg(results.targetsTotal - results.targetsAlive).arg(results.targetsTotal);
            htmlCode += trUtf8(""
                           "            <tr><td><b>Test total ping time</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(results.getRttSumAsString());
            htmlCode += trUtf8(""
                           "            <tr><td><b>Average ping time/target</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(results.getRttAvgAsString());
        }

        if(downloadTestEnabledFlag)
        {
            htmlCode += trUtf8(""
                           "            <tr><td><b>Download speed (Kbps)</b></td><td><div align=\"center\">%1 Kbps</div></td></tr>\n"
                           "            <tr><td><b>Download speed (MBps)</b></td><td><div align=\"center\">%2 MB/sec</div></td></tr>\n").arg(results.speedInKbps).arg(results.speedInMBps);
        }

        htmlCode +=        "        </table>\n"
                           "        <br/>\n"
                           "        <p>\n"
                           "            <a href=\"http://validator.w3.org\">\n"
                           "                <img src=\"http://www.w3.org/Icons/valid-xhtml10\" alt=\"Valid XHTML 1.0 Transitional\" height=\"31\" width=\"88\" />\n"
                           "            </a>\n"
                           "        </p>\n"
                           "    </body>\n"
                           "</html>\n";
        htmlCode.replace('&', "&amp;");
        htmlCode.replace("&amp;nbsp", "&nbsp");
    }

    QApplication::clipboard()->setText(htmlCode);
    mainWindow.showClipboardConfirmation(trUtf8("<p style=\"align: center; color: green;\">HTML code copied to clipboard</p>"));
    logMessage(trUtf8("HTML code copied to clipboard"));
}
