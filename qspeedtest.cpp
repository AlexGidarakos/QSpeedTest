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
#include <QDateTime>
#include <QClipboard>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QProcess>
#include <QtConcurrentMap>


QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv)
{
    fileHosts.append(FileHost("NTUA FTP", QUrl("ftp://ftp.ntua.gr/pub/linux/ubuntu-releases/10.04/ubuntu-10.04-desktop-i386.iso")));
    connect(&fileHosts[0], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    fileHosts.append(FileHost("FORTHNET FTP", QUrl("ftp://ftp.forthnet.gr/pub/SPEEDTEST/CentOS-5.4-i386-LiveCD.iso")));
    connect(&fileHosts[1], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    fileHosts.append(FileHost("NVIDIA Germany", QUrl("http://de.download.nvidia.com/Windows/197.45/197.45_desktop_win7_winvista_64bit_international_whql.exe")));
    connect(&fileHosts[2], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    fileHosts.append(FileHost("NVIDIA USA", QUrl("http://us.download.nvidia.com/Windows/197.45/197.45_desktop_win7_winvista_32bit_english_whql.exe")));
    connect(&fileHosts[3], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    fileHosts.append(FileHost("Microsoft", QUrl("http://download.microsoft.com/download/E/E/1/EE17FF74-6C45-4575-9CF4-7FC2597ACD18/directx_feb2010_redist.exe")));
    connect(&fileHosts[4], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    fileHosts.append(FileHost("Apple", QUrl("http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iTunes9/061-8203.20100427.1J2kd/iTunesSetup.exe")));
    connect(&fileHosts[5], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));

    STOPBENCHMARK = false;
    connect(this, SIGNAL(initOK()), &mainWindow, SLOT(enablePushButtonStart()));
    connect(&targetList, SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    connect(&mainWindow, SIGNAL(pushButtonStartClicked()), this, SLOT(startBenchmark()));
    connect(this, SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
    connect(this, SIGNAL(benchmarkFinished(bool)), &mainWindow, SLOT(updateButtons(bool)));
    connect(&mainWindow, SIGNAL(pushButtonCopyvBulletinCodeClicked()), this, SLOT(copyvBulletinCode()));
    connect(&mainWindow, SIGNAL(pushButtonCopyHTMLClicked()), this, SLOT(copyHTML()));
    mainWindow.show();

    if(targetList.init())
    {
        for(int i = 0; i < targetList.numberOfGroups; i++)
        {
            for(int j = 0; j < targetList.groups.at(i).size; j++)
            {
                connect(&targetList.groups[i].targets[j], SIGNAL(message(QString)), &mainWindow, SLOT(updateConsole(QString)));
                connect(&targetList.groups[i].targets[j], SIGNAL(changevBulletinCode(QString)), this, SLOT(updatevBulletinCode(QString)));
                connect(&targetList.groups[i].targets[j], SIGNAL(changeHTML(QString)), this, SLOT(updateHTML(QString)));
            }
        }

        emit initOK();
    }
}


void QSpeedTest::printHostAndProgramInfo()
{
    QString hostOS;
#ifdef Q_WS_WIN
    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_2000:
            hostOS = "Windows 2000";
            break;

        case QSysInfo::WV_XP:
            hostOS = "Windows XP 32bit";
            break;

        case QSysInfo::WV_2003:
            hostOS = "Windows XP 64bit or Server 2003";
            break;

        case QSysInfo::WV_VISTA:
            hostOS = "Windows Vista";
            break;

        case QSysInfo::WV_WINDOWS7:
            hostOS = "Windows 7";
            break;

        default:
            hostOS = "Windows";
    }
#else
    hostOS = "Linux";
#endif
    testDateTime = QDateTime::currentDateTime().toString("dddd dd/MM/yyyy hh:mm:ss");
    emit message(trUtf8("Report created by: %1 %2\n"
                        "Target list version: v%3\n"
                        "Target list comment: %4\n"
                        "Target list contact URL: %5\n"
                        "Host OS: %6\n"
                        "Test date and time: %7").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(targetList.version).arg(targetList.comment).arg(targetList.contactURL).arg(hostOS).arg(testDateTime));
    vBulletinCode = trUtf8( "[table=head] | Client info\n"
                            "Report created by | [center]%1 %2 - [url=%3]Homepage[/url] - [url=%4]Discuss[/url][/center] |\n"
                            "Target list version | [center]%5[/center] |\n"
                            "Target list comment | [center][url=%6]%7[/url][/center] |\n"
                            "Host OS | [center]%8[/center] |\n"
                            "Test date and time | [center]%9[/center] |\n").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(PROGRAMURL).arg(PROGRAMDISCUSSURL).arg(targetList.version).arg(targetList.contactURL).arg(targetList.comment).arg(hostOS).arg(testDateTime);
    HTML  = trUtf8("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
    HTML += trUtf8("<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n");
    HTML += trUtf8("    <head>\n");
    HTML += trUtf8("        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n");
    HTML += trUtf8("        <title>%1 report</title>\n").arg(PROGRAMNAME);
    HTML += trUtf8("    </head>\n");
    HTML += trUtf8("    <body>\n");
    HTML += trUtf8("        <table border=\"1\" cellpadding=\"4\">\n");
    HTML += trUtf8("            <tr><td>&nbsp;</td><td><div align=\"center\">Client info</div></td></tr>\n");
    HTML += trUtf8("            <tr><td>Report created by</td><td><div align=\"center\">%1 %2 - <a href=\"%3\" target=\"_blank\">Download</a> - <a href=\"%4\" target=\"_blank\">Discuss</a></div></td></tr>\n").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(PROGRAMURL).arg(PROGRAMDISCUSSURL);
    HTML += trUtf8("            <tr><td>Target list used</td><td><div align=\"center\"><a href=\"%1\">%2</a></div></td></tr>\n").arg(targetList.contactURL).arg(targetList.comment);
    HTML += trUtf8("            <tr><td>Host OS</td><td><div align=\"center\">%1</div></td></tr>\n").arg(hostOS);
    HTML += trUtf8("            <tr><td>Test date and time</td><td><div align=\"center\">%1</div></td></tr>\n").arg(testDateTime);
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
#ifdef Q_WS_WIN
    QString tracerouteCommand = "pathping -h 2 -q 1 8.8.8.8";
    QString bbrasLine = "  2  ";
#else
    QString tracerouteCommand = "traceroute -m 2 -q 1 8.8.8.8";
    QString bbrasLine = " 2  ";
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
                BBRAS = trUtf8("N/A (no reply to traceroute)");
            }
            else
            {
#ifdef Q_WS_WIN
                BBRAS = contents.mid(3);
#else
                BBRAS = contents.mid(4, contents.indexOf(')') - 1);
#endif
            }
        }
    }

    if(!found)
    {
        BBRAS = trUtf8("N/A");
    }

    emit message(trUtf8("ISP: %1\n"
                        "Internet IP: %2\n"
                        "BBRAS: %3\n"
                        "\n").arg(ISP).arg(IP).arg(BBRAS));
    vBulletinCode += trUtf8("ISP | [center]%1[/center] |\n"
                            "Internet IP | [center]%2[/center] |\n"
                            "BBRAS | [center]%3[/center] |\n"
                            "[/table]\n"
                            "[spoiler]\n").arg(ISP).arg(IP).arg(BBRAS);
    HTML += trUtf8("            <tr><td>ISP</td><td><div align=\"center\">%1</div></td></tr>\n").arg(ISP);
    HTML += trUtf8("            <tr><td>Internet IP</td><td><div align=\"center\">%1</div></td></tr>\n").arg(IP);
    HTML += trUtf8("            <tr><td>BBRAS</td><td><div align=\"center\">%1</div></td></tr>\n").arg(BBRAS);
    HTML += trUtf8("        </table>\n");
}


void QSpeedTest::startBenchmark()
{
    int parallelThreads = mainWindow.parallelThreads();
    QString name;
    int nameSize;
    QString hr;
    QEventLoop loop;
    QString rttGroupSum;
    double rttGroupSumAsDouble;
    int groupTargetsAlive;
    QString rttGroupAvg;
    double rttTestSum = 0.0;
    int testTargetsAlive = 0;
    double speedInKbps = 0.0;

    for(int i = 0; i < targetList.numberOfGroups; i++)
    {
        for(int j = 0; j < targetList.groups.at(i).size; j++)
        {
            targetList.groups[i].targets[j].reset();
        }
    }

    QThreadPool::globalInstance()->setMaxThreadCount(parallelThreads - 1);
    STOPBENCHMARK = false;
    printHostAndProgramInfo();
    printLineInfo();
    time.start();

    for(int i = 0; i < targetList.numberOfGroups; i++)
    {
        name = targetList.groups.at(i).name;
        emit message(name.leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        nameSize = name.size();
        hr.clear();

        for(int j = 0; j < 79; j++)
        {
            hr += "-";
        }

        emit message(hr);
        vBulletinCode += trUtf8("[b]%1[/b]\n"
                                "[spoiler]\n"
                                "[table=head][center]Target[/center] | [center]Average ping time[/center] | Packet loss | Jitter | Rank\n").arg(name);
        HTML += trUtf8("        <br/>\n");
        HTML += trUtf8("        <b>%1</b>\n").arg(name);
        HTML += trUtf8("        <br/>\n");
        HTML += trUtf8("        <table border=\"1\" cellpadding=\"4\">\n");
        HTML += trUtf8("            <tr><td>Target</td><td>Average ping time</td><td>Packet loss</td><td><div align=\"center\">Jitter</div></td><td>Rank</td></tr>\n");

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
                    emit message("\n\nBenchmark stopped before completion.");
                    emit benchmarkFinished(false);
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
        }
        else
        {
            rttGroupSum = "N/A";
            rttGroupAvg = "N/A";
        }

        processEvents();

        if(!STOPBENCHMARK)
        {
            emit message(trUtf8("\nGroup total ping time: %1\n"
                                "Group average ping time: %2\n"
                                "\n").arg(rttGroupSum).arg(rttGroupAvg));
            vBulletinCode += trUtf8("[b]Group total ping time[/b] | [right][b]%1[/b][/right] |\n"
                                    "[b]Group average ping time[/b] | [right][b]%2[/b][/right] |\n"
                                    "[/table]\n"
                                    "[/spoiler]\n").arg(rttGroupSum).arg(rttGroupAvg);
            HTML += trUtf8("            <tr><td><b>Group total ping time</b></td><td><div align=\"right\"><b>%1 msec</b></div></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n").arg(rttGroupSum);
            HTML += trUtf8("            <tr><td><b>Group average ping time</b></td><td><div align=\"right\"><b>%1 msec</b></div></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n").arg(rttGroupAvg);
            HTML += trUtf8("        </table>\n");
            HTML += trUtf8("        <br/>\n");
        }
    }

    emit message(trUtf8("Please wait approx. %1 seconds for the download speed test to complete...").arg(DOWNLOADTESTSECS));
    QThreadPool::globalInstance()->setMaxThreadCount(fileHosts.size());
    BYTESDOWNLOADED = 0;
    QtConcurrent::blockingMap(fileHosts, &FileHost::downloadTest);
    processEvents();
    speedInKbps = (BYTESDOWNLOADED * 8) / (DOWNLOADTESTSECS * 1024 * 1.0);
    emit message(trUtf8("Total data downloaded in %1 secs: %2 bytes\nAverage speed: %3 Kbps\n").arg(DOWNLOADTESTSECS).arg(BYTESDOWNLOADED).arg(speedInKbps));
    secondsElapsed = (time.elapsed() * 1.0) / 1000;

    if(!STOPBENCHMARK)
    {
        if(testTargetsAlive)
        {
            emit message(trUtf8("Pings per target: %1\n"
                                "Threads used: %2\n"
                                "Test completed in: %3 sec\n"
                                "Targets unreachable: %4 / %5\n"
                                "Test total ping time: %6 msec\n"
                                "Average ping time per target: %7 msec\n"
                                "Speed test result: %8 Kbps").arg(PINGSPERTARGET).arg(parallelThreads).arg(secondsElapsed).arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets).arg(rttTestSum).arg(rttTestSum / testTargetsAlive).arg(speedInKbps));
            vBulletinCode += trUtf8("[/spoiler]\n"
                                    "[table=head][center]Test metric[/center] | Value\n"
                                    "[b]Pings per target[/b] | [center]%1[/center] |\n"
                                    "[b]Threads used[/b] | [center]%2[/center] |\n"
                                    "[b]Test completed in[/b] | [center]%3 sec[/center] |\n"
                                    "[b]Targets unreachable[/b] | [center]%4 / %5[/center] |\n"
                                    "[b]Test total ping time[/b] | [center]%6 msec[/center] |\n"
                                    "[b]Average ping time per target[/b] | [center]%7 msec[/center] |\n"
                                    "[b]Speed test result[/b] | [center]%8 Kbps[/center] | [/table]\n"
                                    "\n").arg(PINGSPERTARGET).arg(parallelThreads).arg(secondsElapsed).arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets).arg(rttTestSum).arg(rttTestSum / testTargetsAlive).arg(speedInKbps);
            HTML += trUtf8("        <table border=\"1\" cellpadding=\"4\">\n");
            HTML += trUtf8("            <tr><td><div align=\"center\">Test metric</div></td><td><div align=\"center\">Value</div></td></tr>\n");
            HTML += trUtf8("            <tr><td><b>Pings per target</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(PINGSPERTARGET);
            HTML += trUtf8("            <tr><td><b>Threads used</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(parallelThreads);
            HTML += trUtf8("            <tr><td><b>Test completed in</b></td><td><div align=\"center\">%1 sec</div></td></tr>\n").arg(secondsElapsed);
            HTML += trUtf8("            <tr><td><b>Targets unreachable</b></td><td><div align=\"center\">%1 / %2</div></td></tr>\n").arg(targetList.numberOfTargets - testTargetsAlive).arg(targetList.numberOfTargets);
            HTML += trUtf8("            <tr><td><b>Test total ping time</b></td><td><div align=\"center\">%1 msec</div></td></tr>\n").arg(rttTestSum);
            HTML += trUtf8("            <tr><td><b>Average ping time per target</b></td><td><div align=\"center\">%1 msec</div></td></tr>\n").arg(rttTestSum / testTargetsAlive);
            HTML += trUtf8("            <tr><td><b>Speed test result</b></td><td><div align=\"center\">%1 Kbps</div></td></tr>\n").arg(speedInKbps);
        }
        else
        {
            emit message(trUtf8("Pings per target: %1\n"
                                "Threads used: %2\n"
                                "Test completed in: %3 sec\n"
                                "Targets unreachable: %4 / %4\n"
                                "Test total ping time: N/A\n"
                                "Average ping time per target: N/A").arg(PINGSPERTARGET).arg(parallelThreads).arg(secondsElapsed).arg(targetList.numberOfTargets));
            vBulletinCode += trUtf8("[/spoiler]\n"
                                    "[table=head][center]Test metric[/center] | Value\n"
                                    "[b]Pings per target[/b] | [center]%1[/center] |\n"
                                    "[b]Threads used[/b] | [center]%2[/center] |\n"
                                    "[b]Test completed in[/b] | [center]%3 sec[/center] |\n"
                                    "[b]Targets unreachable[/b] | [center]%4 / %4[/center] |\n"
                                    "[b]Test total ping time[/b] | [center]N/A[/center] |\n"
                                    "[b]Average ping time per target[/b] | [center]N/A[/center] |\n"
                                    "[b]Speed test result[/b] | [center]%5 Kbps[/center] | [/table]\n"
                                    "\n").arg(PINGSPERTARGET).arg(parallelThreads).arg(secondsElapsed).arg(targetList.numberOfTargets).arg(speedInKbps);
            HTML += trUtf8("        <table border=\"1\" cellpadding=\"4\">\n");
            HTML += trUtf8("            <tr><td><div align=\"center\">Test metric</div></td><td><div align=\"center\">Value</div></td></tr>\n");
            HTML += trUtf8("            <tr><td><b>Pings per target</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(PINGSPERTARGET);
            HTML += trUtf8("            <tr><td><b>Threads used</b></td><td><div align=\"center\">%1</div></td></tr>\n").arg(parallelThreads);
            HTML += trUtf8("            <tr><td><b>Test completed in</b></td><td><div align=\"center\">%1 sec</div></td></tr>\n").arg(secondsElapsed);
            HTML += trUtf8("            <tr><td><b>Targets unreachable</b></td><td><div align=\"center\">%1 / %1</div></td></tr>\n").arg(targetList.numberOfTargets);
            HTML += trUtf8("            <tr><td><b>Test total ping time</b></td><td><div align=\"center\">N/A</div></td></tr>\n");
            HTML += trUtf8("            <tr><td><b>Average ping time per target</b></td><td><div align=\"center\">N/A</div></td></tr>\n");
            HTML += trUtf8("            <tr><td><b>Speed test result</b></td><td><div align=\"center\">%1 Kbps</div></td></tr>\n").arg(speedInKbps);
        }

            HTML += trUtf8("        </table>\n");
            HTML += trUtf8("        <br/>\n");
            HTML += trUtf8("        <p>\n");
            HTML += trUtf8("            <a href=\"http://validator.w3.org/check?uri=referer\">\n");
            HTML += trUtf8("                <img src=\"http://www.w3.org/Icons/valid-xhtml10\" alt=\"Valid XHTML 1.0 Transitional\" height=\"31\" width=\"88\" />\n");
            HTML += trUtf8("            </a>\n");
            HTML += trUtf8("        </p>\n");
            HTML += trUtf8("    </body>\n");
            HTML += trUtf8("</html>\n");
        HTML.replace('&', "&amp;");
        HTML.replace("&amp;nbsp", "&nbsp");
        emit message(trUtf8("Test completed."));
    }

    emit benchmarkFinished(STOPBENCHMARK);
}


void QSpeedTest::updatevBulletinCode(QString code)
{
    vBulletinCode += code;
}


void QSpeedTest::updateHTML(QString code)
{
    HTML += code;
}


void QSpeedTest::copyvBulletinCode()
{
    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setText(vBulletinCode);
}


void QSpeedTest::copyHTML()
{
    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setText(HTML);
}
