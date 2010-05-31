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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDateTime>
#include <QtConcurrentMap>


QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv)
{
    winSystemInfo.setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_WS_WIN
    if(QSysInfo::WindowsVersion == QSysInfo::WV_VISTA || QSysInfo::WindowsVersion == QSysInfo::WV_WINDOWS7)
    {
        winSystemInfo.start("systeminfo", QIODevice::ReadOnly);
    }
#endif

    STOPBENCHMARK = false;
    connect(this, SIGNAL(initOK()), &mainWindow, SLOT(enablePushButtonStart()));
    connect(&mainWindow, SIGNAL(pushButtonStartClicked()), this, SLOT(startBenchmark()));
    connect(&targetList, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
    connect(this, SIGNAL(newVbCode(QString)), &mainWindow, SLOT(updateVbCode(QString)));
    connect(this, SIGNAL(newHtmlCode(QString)), &mainWindow, SLOT(updateHtmlCode(QString)));
    connect(this, SIGNAL(benchmarkFinished(bool)), &mainWindow, SLOT(updateButtons(bool)));
    mainWindow.show();
    checkForProgramUpdates();

    if(targetList.init())
    {
        for(int i = 0; i < targetList.numberOfGroups; i++)
        {
            for(int j = 0; j < targetList.groups.at(i).size; j++)
            {
                connect(&targetList.groups[i].targets[j], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
                connect(&targetList.groups[i].targets[j], SIGNAL(newVbCode(QString)), &mainWindow, SLOT(updateVbCode(QString)));
                connect(&targetList.groups[i].targets[j], SIGNAL(newHtmlCode(QString)), &mainWindow, SLOT(updateHtmlCode(QString)));
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
        int reply = QMessageBox::question(NULL, trUtf8("Update available"), trUtf8("An updated version of %1 is available online.\n\nWould you like to open the download page in your browser?").arg(PROGRAMNAME), QMessageBox::Yes, QMessageBox::No);

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
    QString hostOS;
    QProcess proc;
    QEventLoop loop;

    testDateTime = QDateTime::currentDateTime().toString("dddd dd/MM/yyyy hh:mm:ss");
    emit newTestResult(trUtf8("Report created by: %1 %2\n"
                        "Target list version: v%3\n"
                        "Target list comment: %4\n"
                        "Target list contact URL: %5").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(targetList.version).arg(targetList.comment).arg(targetList.contactURL));
    proc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
#ifdef Q_WS_WIN
    bool x64 = QProcessEnvironment::systemEnvironment().contains("ProgramFiles(x86)");

    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_2000:
            hostOS = "Windows 2000";
            break;

        case QSysInfo::WV_XP:
            hostOS = "Windows XP x86";
            break;

        case QSysInfo::WV_2003:
            proc.start("cmd /c ver", QIODevice::ReadOnly);
            loop.exec();

            if(proc.readAll().contains("2003"))
            {
                hostOS = QString("Windows Server 2003 ") + QString((x64)? "x64" : "x86");
            }
            else
            {
                hostOS = "Windows XP x64";
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
                hostOS = QString("Windows Server 2008 ") + QString((x64)? "x64" : "x86");
            }
            else
            {
                hostOS = QString("Windows Vista ") + QString((x64)? "x64" : "x86");
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
                hostOS = QString("Windows Server 2008 R2 ") + QString((x64)? "x64" : "x86");
            }
            else
            {
                hostOS = QString("Windows 7 ") + QString((x64)? "x64" : "x86");
            }

            break;

        default:
            hostOS = "Windows";
    }
#else
#ifdef Q_WS_MAC
    proc.start("sw_vers -productVersion", QIODevice::ReadOnly);
    loop.exec();
    hostOS = "Mac OS X " + proc.readLine().trimmed();
    proc.start("uname -m", QIODevice::ReadOnly);
#else
#ifdef Q_OS_LINUX
    proc.start("uname -o", QIODevice::ReadOnly);
#else
    proc.start("uname -s", QIODevice::ReadOnly);
#endif
    loop.exec();
    hostOS = proc.readLine().trimmed();
    proc.start("uname -rm", QIODevice::ReadOnly);
    loop.exec();
    hostOS += " " + proc.readLine().trimmed();
#endif
#endif

    emit newTestResult(trUtf8("Host OS: %1\n"
                        "Test date and time: %2").arg(hostOS).arg(testDateTime));
    emit newVbCode(trUtf8("[table=head] | Client info\n"
                          "Report created by | [center]%1 %2 - [url=%3]Download[/url] - [url=%4]Discuss[/url][/center] |\n"
                          "Target list version | [center]%5[/center] |\n"
                          "Target list comment | [center][url=%6]%7[/url][/center] |\n"
                          "Host OS | [center]%8[/center] |\n"
                          "Test date and time | [center]%9[/center] |\n").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(PROGRAMURL).arg(PROGRAMDISCUSSURL).arg(targetList.version).arg(targetList.contactURL).arg(targetList.comment).arg(hostOS).arg(testDateTime));
    emit newHtmlCode(trUtf8("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                            "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                            "    <head>\n"
                            "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
                            "        <title>%1 report</title>\n"
                            "    </head>\n"
                            "    <body>\n"
                            "        <table border=\"1\" cellpadding=\"4\">\n"
                            "            <tr><td>&nbsp;</td><td><div align=\"center\">Client info</div></td></tr>\n"));
    emit newHtmlCode(trUtf8("            <tr><td>Report created by</td><td><div align=\"center\">%1 %2 - <a href=\"%3\" target=\"_blank\">Download</a> - <a href=\"%4\" target=\"_blank\">Discuss</a></div></td></tr>\n").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(PROGRAMURL).arg(PROGRAMDISCUSSURL));
    emit newHtmlCode(trUtf8("            <tr><td>Target list used</td><td><div align=\"center\"><a href=\"%1\">%2</a></div></td></tr>\n").arg(targetList.contactURL).arg(targetList.comment));
    emit newHtmlCode(trUtf8("            <tr><td>Host OS</td><td><div align=\"center\">%1</div></td></tr>\n"
                            "            <tr><td>Test date and time</td><td><div align=\"center\">%2</div></td></tr>\n").arg(hostOS).arg(testDateTime));
}


void QSpeedTest::printLineInfo()
{
    QNetworkAccessManager manager;
    QEventLoop loop1, loop2;
    QNetworkReply *download;
    QByteArray contents;
    bool found = false;
    QStringList list;
    QProcess traceroute;
    QString tracerouteCommand;
    QString bbrasLine;

#ifdef Q_WS_WIN
    tracerouteCommand = "pathping -h 2 -q 1 8.8.8.8";
    bbrasLine = "  2  ";
#else
    tracerouteCommand = "traceroute -m 2 -q 1 8.8.8.8";
    bbrasLine = " 2  ";
#endif

    IP.clear();
    ISP.clear();
    traceroute.setProcessChannelMode(QProcess::MergedChannels);
    connect(&traceroute, SIGNAL(finished(int)), &loop2, SLOT(quit()));
    traceroute.start(tracerouteCommand, QIODevice::ReadOnly);
    download = manager.get(QNetworkRequest(QUrl("http://www.speedtest.net/summary.php")));
    connect(download, SIGNAL(finished()), &loop1, SLOT(quit()));
    loop1.exec();

    while(!(contents = download->readLine()).isEmpty() && !found)
    {
        if(contents.contains("<div class=\"ip\">"))
        {
            contents.chop(7);
            IP = QString(contents.trimmed().mid(16));
            list = IP.split('.');
            IP = list[0] + "." + list[1] + ".xxx.xxx";
            continue;
        }
        else
        {
            if(contents.contains("<div class=\"isp\">"))
            {
                contents.chop(7);
                ISP = QString(contents.trimmed().mid(17));
                found = true;
            }
        }
    }

    if(IP.isEmpty())
    {
        IP = trUtf8("speedtest.net unreachable!");
    }

    if(ISP.isEmpty())
    {
        ISP = trUtf8("speedtest.net unreachable!");
    }

    delete download;

    for(int i = 0; i < targetList.numberOfGroups; i++)
    {
        for(int j = 0; j < targetList.groups.at(i).size; j++)
        {
            targetList.groups[i].targets[j].reset();
        }
    }

    if(traceroute.state() != QProcess::NotRunning)
    {
        loop2.exec();
    }

    for(found = false; !found && !(contents = traceroute.readLine()).isEmpty();)
    {
        if(contents.contains(bbrasLine.toAscii()))
        {
            found = true;

            if(contents.contains(QString("*").toAscii()))
            {
                BBRAS = trUtf8("N/A (non-responsive BBRAS)");
            }
            else
            {
#ifdef Q_WS_WIN
                BBRAS = contents.mid(3).trimmed();
#else
                BBRAS = contents.mid(4, contents.indexOf(')') - 1).trimmed();
#endif
            }
        }
    }

    if(!found)
    {
        BBRAS = trUtf8("N/A");
    }

    emit newTestResult(trUtf8("ISP: %1\nInternet IP: %2\nBBRAS: %3\n").arg(ISP).arg(IP).arg(BBRAS));
    emit newVbCode(trUtf8("ISP | [center]%1[/center] |\n"
                          "Internet IP | [center]%2[/center] |\n"
                          "BBRAS | [center]%3[/center] |\n"
                          "[/table]\n").arg(ISP).arg(IP).arg(BBRAS));
    emit newHtmlCode(trUtf8("            <tr><td>ISP</td><td><div align=\"center\">%1</div></td></tr>\n"
                            "            <tr><td>Internet IP</td><td><div align=\"center\">%2</div></td></tr>\n"
                            "            <tr><td>BBRAS</td><td><div align=\"center\">%3</div></td></tr>\n"
                            "        </table>\n").arg(ISP).arg(IP).arg(BBRAS));
}


void QSpeedTest::startBenchmark()
{
    int parallelThreads = mainWindow.parallelThreads();
    bool pingTestEnabled = mainWindow.pingTestEnabled();
    bool downloadTestEnabled = mainWindow.downloadTestEnabled();
    QString testMode;
    QString name;
    int nameSize;
    QString hr;
    QEventLoop loop;
    QString rttGroupSum;
    double rttGroupSumAsDouble;
    int groupTargetsAlive;
    QString rttGroupAvg;
    QString packetLossGroupAvg;
    QString rankGroupAvg;
    double rttTestSum = 0.0;
    int testTargetsAlive = 0;
    double speedInKbps = 0.0;
    double speedInMBps = 0.0;

    timer.start();
    emit logMessage(trUtf8("Test started"));
    QThreadPool::globalInstance()->setMaxThreadCount(parallelThreads - 1);
    STOPBENCHMARK = false;
    printHostAndProgramInfo();
    printLineInfo();
    emit newVbCode((pingTestEnabled)? ("[spoiler]\n") : NULL);

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabled; i++)
    {
        if(STOPBENCHMARK)
        {
            emit benchmarkFinished(true);
            return;
        }

        name = targetList.groups.at(i).name;
        emit newTestResult(name.leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        nameSize = name.size();
        hr.clear();

        for(int j = 0; j < 79; j++)
        {
            hr += "-";
        }

        emit newTestResult(hr);
        emit newVbCode(trUtf8("[b]%1[/b]\n"
                              "[spoiler]\n"
                              "[table=head][center]Target[/center] | [center]Average ping time[/center] | Packet loss | Jitter | Rank\n").arg(name));
        emit newHtmlCode(trUtf8("        <br/>\n"
                                "        <b>%1</b>\n"
                                "        <br/>\n"
                                "        <table border=\"1\" cellpadding=\"4\">\n").arg(name));
        emit newHtmlCode(trUtf8("            <tr><td>Target</td><td>Average ping time</td><td>Packet loss</td><td><div align=\"center\">Jitter</div></td><td>Rank</td></tr>\n"));

        if(mainWindow.parallelThreads() > 1)
        {
            QtConcurrent::blockingMap(targetList.groups[i].targets, &Target::ping);
        }
        else
        {
            for(int j = 0; j < targetList.groups.at(i).size; j++)
            {
                if(STOPBENCHMARK)
                {
                    emit benchmarkFinished(true);
                    return;
                }

                targetList.groups[i].targets[j].ping();
            }
        }

        rttTestSum += targetList.groups[i].rttGroupSum();
        groupTargetsAlive = targetList.groups[i].countTargetsAlive();
        testTargetsAlive += groupTargetsAlive;

        if(groupTargetsAlive)
        {
            rttGroupSumAsDouble = targetList.groups[i].rttGroupSum();
            rttGroupSum = QString::number(rttGroupSumAsDouble, 'f', 2) + " msec";
            rttGroupAvg = QString::number((rttGroupSumAsDouble / groupTargetsAlive), 'f', 2) + " msec";
            packetLossGroupAvg = QString::number(targetList.groups[i].packetLossGroupAvg(), 'f', 2) + "%";
            rankGroupAvg = targetList.groups[i].rankGroupAvg(rttGroupSumAsDouble / groupTargetsAlive);
        }
        else
        {
            rttGroupSum = "N/A";
            rttGroupAvg = "N/A";
            packetLossGroupAvg = "100%";
            rankGroupAvg = "N/A";
        }

        processEvents();

        if(!STOPBENCHMARK)
        {
            emit newTestResult(trUtf8("Group sum: %1\nGroup average: %2\n").arg(rttGroupSum).arg(rttGroupAvg));
            emit newVbCode(trUtf8("[b]Group sum[/b] | [right][b]%1[/b][/right] |\n"
                                  "[b]Group average[/b] | [right][b]%2[/b][/right] | [right][b]%3[/b][/right] | | [center][b]%4[/b][/center]\n"
                                  "[/table]\n"
                                  "[/spoiler]\n").arg(rttGroupSum).arg(rttGroupAvg).arg(packetLossGroupAvg).arg(rankGroupAvg));
            emit newHtmlCode(trUtf8("            <tr><td><b>Group sum</b></td><td><div align=\"right\"><b>%1</b></div></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"
                                    "            <tr><td><b>Group average</b></td><td><div align=\"right\"><b>%2</b></div></td><td><div align=\"right\"><b>%3</b></div></td><td>&nbsp;</td><td><div align=\"center\"><b>%4</b></div></td></tr>\n"
                                    "        </table>\n"
                                    "        <br/>\n").arg(rttGroupSum).arg(rttGroupAvg).arg(packetLossGroupAvg).arg(rankGroupAvg));
        }
    }

    if(!STOPBENCHMARK)
    {
        if(downloadTestEnabled)
        {
            mainWindow.pushButtonStopEnable(false);
            emit newTestResult(trUtf8("Downloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
            QThreadPool::globalInstance()->setMaxThreadCount(targetList.fileHosts.size());
            BYTESDOWNLOADED = 0;
            QtConcurrent::blockingMap(targetList.fileHosts, &FileHost::downloadTest);
            processEvents();
            speedInKbps = (BYTESDOWNLOADED * 8) / (DOWNLOADTESTSECS * 1024 * 1.0);
            speedInMBps = speedInKbps / (8 * 1024);

            if(pingTestEnabled)
            {
                testMode = trUtf8("Ping and download");
            }
            else
            {
                testMode = trUtf8("Download only");
            }
        }
        else
        {
            testMode = trUtf8("Ping only");
        }

        secondsElapsed = (timer.elapsed() * 1.0) / 1000;
        emit newTestResult(trUtf8("\nTest completed in: %1 sec").arg(secondsElapsed));
        emit newVbCode((pingTestEnabled)? "[/spoiler]\n" : NULL);
        emit newVbCode(trUtf8("[table=head][center]Variable[/center] | Value\n"
                              "[b]Test mode[/b] | [center]%1[/center] |\n"
                              "[b]Test completed in[/b] | [center]%2 sec[/center] |\n").arg(testMode).arg(secondsElapsed));
        emit newHtmlCode(trUtf8("        <table border=\"1\" cellpadding=\"4\">\n"
                                "            <tr><td><div align=\"center\">Variable</div></td><td><div align=\"center\">Value</div></td></tr>\n"
                                "            <tr><td><b>Test mode</b></td><td><div align=\"center\">%1</div></td></tr>\n"
                                "            <tr><td><b>Test completed in</b></td><td><div align=\"center\">%2 sec</div></td></tr>\n").arg(testMode).arg(secondsElapsed));

        if(pingTestEnabled)
        {
            emit newTestResult(trUtf8("Pings/target: %1\nParallel ping threads: %2").arg(PINGSPERTARGET).arg(parallelThreads));
            emit newVbCode(trUtf8("[b]Pings/target[/b] | [center]%1[/center] |\n"
                                  "[b]Parallel ping threads[/b] | [center]%2[/center] |\n").arg(PINGSPERTARGET).arg(parallelThreads));
            emit newHtmlCode(trUtf8("            <tr><td><b>Pings/target</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(PINGSPERTARGET));
            emit newHtmlCode(trUtf8("            <tr><td><b>Parallel ping threads</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(parallelThreads));

            if(testTargetsAlive)
            {
                emit newTestResult(trUtf8("Targets unreachable: %1 / %2\n"
                                    "Test total ping time: %3 msec\n"
                                    "Average ping time/target: %4 msec").arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets).arg(rttTestSum).arg(rttTestSum / testTargetsAlive));
                emit newVbCode(trUtf8("[b]Targets unreachable[/b] | [center]%1 / %2[/center] |\n"
                                      "[b]Test total ping time[/b] | [center]%3 msec[/center] |\n"
                                      "[b]Average ping time/target[/b] | [center]%4 msec[/center] |\n").arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets).arg(rttTestSum).arg(rttTestSum / testTargetsAlive));
                emit newHtmlCode(trUtf8("            <tr><td><b>Targets unreachable</b></td><td><div align=\"center\">%1 / %2</div></td></tr>\n").arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets));
                emit newHtmlCode(trUtf8("            <tr><td><b>Test total ping time</b></td><td><div align=\"center\">%1 msec</div></td></tr>\n").arg(rttTestSum));
                emit newHtmlCode(trUtf8("            <tr><td><b>Average ping time/target</b></td><td><div align=\"center\">%1 msec</div></td></tr>\n").arg(rttTestSum / testTargetsAlive));
            }
            else
            {
                emit newTestResult(trUtf8("Targets unreachable: %1 / %1\n"
                                    "Test total ping time: N/A\n"
                                    "Average ping time/target: N/A").arg(targetList.numberOfTargets));
                emit newVbCode(trUtf8("[b]Targets unreachable[/b] | [center]%1 / %1[/center] |\n"
                                      "[b]Test total ping time[/b] | [center]N/A[/center] |\n"
                                      "[b]Average ping time/target[/b] | [center]N/A[/center] |\n").arg(targetList.numberOfTargets));
                emit newHtmlCode(trUtf8("            <tr><td><b>Targets unreachable</b></td><td><div align=\"center\">%1 / %1</div></td></tr>\n").arg(targetList.numberOfTargets));
                emit newHtmlCode(trUtf8("            <tr><td><b>Test total ping time</b></td><td><div align=\"center\">N/A</div></td></tr>\n"
                                        "            <tr><td><b>Average ping time/target</b></td><td><div align=\"center\">N/A</div></td></tr>\n"));
            }
        }

        if(downloadTestEnabled)
        {
            emit newTestResult(trUtf8("Download speed (Kbps): %1 Kbps\n"
                                "Download speed (MBps): %2 MB/sec").arg(speedInKbps).arg(speedInMBps));
            emit newVbCode(trUtf8("[b]Download speed (Kbps)[/b] | [center]%1 Kbps[/center] |\n"
                                  "[b]Download speed (MBps)[/b] | [center]%2 MB/sec[/center] |\n").arg(speedInKbps).arg(speedInMBps));
            emit newHtmlCode(trUtf8("            <tr><td><b>Download speed (Kbps)</b></td><td><div align=\"center\">%1 Kbps</div></td></tr>\n"
                                    "            <tr><td><b>Download speed (MBps)</b></td><td><div align=\"center\">%2 MB/sec</div></td></tr>\n").arg(speedInKbps).arg(speedInMBps));
        }

        emit newVbCode("[/table]\n\n");
        emit newHtmlCode("        </table>\n"
                         "        <br/>\n"
                         "        <p>\n"
                         "            <a href=\"http://validator.w3.org\">\n"
                         "                <img src=\"http://www.w3.org/Icons/valid-xhtml10\" alt=\"Valid XHTML 1.0 Transitional\" height=\"31\" width=\"88\" />\n"
                         "            </a>\n"
                         "        </p>\n"
                         "    </body>\n"
                         "</html>\n");
        emit logMessage(trUtf8("Test complete"));
    }

    if(pingTestEnabled)
    {
        for(int i = 0; i < targetList.numberOfGroups; i++)
        {
            for(int j = 0; j < targetList.groups.at(i).size; j++)
            {
                targetList.groups[i].targets[j].reset();
            }
        }
    }

    emit benchmarkFinished(STOPBENCHMARK);
}
