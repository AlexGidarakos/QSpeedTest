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


#include "qspeedtestcli.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QDir>


QSpeedTestCli::QSpeedTestCli()
{
    testMode = TestMode::All;
    testModeAsString = trUtf8("Ping and download test");
    pingTestEnabled = true;
    downloadTestEnabled = true;
    htmlOutputEnabled = true;
    vbOutputEnabled = true;
    connect(&targetList, SIGNAL(logMessage(QString)), this, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(logMessage(QString)), this, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));
}


void QSpeedTestCli::start()
{
    parseArguments();
    winSystemInfo.setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_WS_WIN
    if(QSysInfo::WindowsVersion == QSysInfo::WV_VISTA || QSysInfo::WindowsVersion == QSysInfo::WV_WINDOWS7)
    {
        winSystemInfo.start("systeminfo", QIODevice::ReadOnly);
    }
#endif // Q_WS_WIN

    checkForProgramUpdates();

    if(!targetList.init())
    {
        updateLogMessages(trUtf8("Unsuccessful initialization of the target list, %1 will now exit").arg(PROGRAMNAME));
        exit(8);
    }

    for(int i = 0; i < targetList.numberOfGroups; i++)
    {
        for(int j = 0; j < targetList.groups[i].getSize(); j++)
        {
            connect(&targetList.groups[i].targets[j], SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));
        }
    }

    for(int i = 0; i < targetList.fileHosts.size(); i++)
    {
        connect(&targetList.fileHosts[i], SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));
    }

    results.targetListVersion = targetList.version;
    results.targetListComment = targetList.comment;
    results.targetListContactUrl = targetList.contactUrl;
    startBenchmark();
    saveReports();
    qApp->processEvents();
    qApp->quit();
}


void QSpeedTestCli::parseArguments()
{
    QStringList args(qApp->arguments());
    int count = args.count();
    QString arg;
    QString argNext;
    QString message;
    int number;

    if(count == 1)
    {
        qDebug() << qPrintable(trUtf8("No parameters specified, running with defaults:\n"
                                      "Mode:             Ping and download test\n"
                                      "Pings per target: %1\n"
                                      "Threads:          %2\n"
                                      "HTML output:      Enabled\n"
                                      "vBulletin output: Enabled\n").arg(PINGSPERTARGET).arg(PARALLELPINGS));
        return;
    }

    for(int i = 1; i < count; i++)    // no point checking arg[0], it is always the executable's name
    {
        arg = args.at(i).toLower();

        if(i < count - 1)    // if there is a next argument after the current
        {
            argNext = args.at(i + 1);
        }
        else
        {
            argNext.clear();
        }

        if(arg == "--help" || arg == "-h" || arg == "help" || arg == "/?" || arg == "?")
        {
            message = trUtf8("%1 %2 by %3 - %4").arg(PROGRAMNAME).arg(PROGRAMVERSION).arg(PROGRAMAUTHOR).arg(PROGRAMURL);
            message = trUtf8("\n    Usage: %1 [PARAMETERS]\n\n"
                             "    [PARAMETERS] are optional and can be any from the following\n\n"
                             "--help, -h\n"
                             "    Prints program usage and exits\n\n"
                             "--version, -V\n"
                             "    Prints version info and exits\n\n"
                             "--mode MODE, -m MODE\n"
                             "    where MODE can be any one from (without the double quotes)\n"
                             "    \"info\": will only print some host system and ISP related information\n"
                             "    \"ping\": will only perform a ping test\n"
                             "    \"download\": will only perform a download speed test\n"
                             "    \"all\": will perform all tests in the above order\n\n"
                             "--pings NUMBER, -p NUMBER\n"
                             "    where NUMBER can be any integer between 1 and 100, default is 4\n"
                             "    During a ping test, each target will be pinged this many times\n\n"
                             "--threads NUMBER, -t NUMBER\n"
                             "    where NUMBER can be any integer between 1 and 8, default is 4\n"
                             "    During a ping test, simultaneous pinging of this many targets\n\n"
                             "--nohtml, -nh\n"
                             "    Disables HTML file output in the current directory\n\n"
                             "--novb, -nv\n"
                             "    Disables vBulletin code file output in the current directory").arg(args[0]);
            qDebug() << qPrintable(message);

            exit(0);
        }

        if(arg == "--version" || arg == "-v")
        {
            qDebug() << qPrintable(trUtf8("%1 %2").arg(PROGRAMNAME).arg(PROGRAMVERSION));

            exit(0);
        }

        if(arg == "--mode" || arg == "-m")
        {
            if(argNext.isEmpty())
            {
                qDebug() << qPrintable(trUtf8("Error #1: No value specified after --mode (-m) switch"));

                exit(1);
            }

            if(argNext == "info")
            {
                testMode = TestMode::Info;
                testModeAsString = trUtf8("Info only");
                pingTestEnabled = false;
                downloadTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "ping")
            {
                testMode = TestMode::Ping;
                testModeAsString = trUtf8("Ping test only");
                downloadTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "download")
            {
                testMode = TestMode::Download;
                testModeAsString = trUtf8("Download test only");
                pingTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "all")
            {
                testMode = TestMode::All;
                i++;
                continue;
            }

            qDebug() << qPrintable(trUtf8("Error #2: Value \"%1\" after --mode (-m) switch not a known mode").arg(argNext));

            exit(2);
        }

        if(arg == "--pings" || arg == "-p")
        {
            if(argNext.isEmpty())
            {
                qDebug() << qPrintable(trUtf8("Error #3: No value specified after --pings (-p) switch"));

                exit(3);
            }

            if((!(number = argNext.toInt())) || number < 1 || number > 100)
            {
                qDebug() << qPrintable(trUtf8("Error #4: Value \"%1\" after --pings (-p) switch not an integer between 1 and 100").arg(argNext));

                exit(4);
            }

            PINGSPERTARGET = number;
            i++;
            continue;
        }

        if(arg == "--threads" || arg == "-t")
        {
            if(argNext.isEmpty())
            {
                qDebug() << qPrintable(trUtf8("Error #5: No value specified after --threads (-t) switch"));

                exit(5);
            }

            if((!(number = argNext.toInt())) || number < 1 || number > 8)
            {
                qDebug() << qPrintable(trUtf8("Error #6: Value \"%1\" after --threads (-t) switch not an integer between 1 and 8").arg(argNext));

                exit(6);
            }

            PARALLELPINGS = number;
            i++;
            continue;
        }

        if(arg == "--nohtml" || arg == "-nh")
        {
            htmlOutputEnabled = false;
            continue;
        }

        if(arg == "--novb" || arg == "-nv")
        {
            vbOutputEnabled = false;
            continue;
        }

        qDebug() << qPrintable(trUtf8("Error #7: Unknown switch or parameter \"%1\"").arg(arg));

        exit(7);
    }

    // if execution flow reaches this point, all arguments were valid
    qDebug() << qPrintable(trUtf8("Custom parameters specified::\n"
                                  "Mode:             %1\n"
                                  "Pings per target: %2\n"
                                  "Threads:          %3\n"
                                  "HTML output:      %4\n"
                                  "vBulletin output: %5\n").arg(testModeAsString).arg(PINGSPERTARGET).arg(PARALLELPINGS).arg((htmlOutputEnabled)? trUtf8("Enabled") : trUtf8("Disabled")).arg((vbOutputEnabled)? trUtf8("Enabled") : trUtf8("Disabled")));
}


void QSpeedTestCli::checkForProgramUpdates()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    updateLogMessages(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROGRAMUPDATECHECKURL)));    // according to Qt documentation, manager will be set as the parent of download, so manual deletion of download should not be necessary
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        download->deleteLater();
        updateLogMessages(trUtf8("Update site unreachable"));
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROGRAMVERSION.mid(1).toInt())
    {
        updateLogMessages(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        updateLogMessages(trUtf8("You can find the new version at %1").arg(PROGRAMURL));
    }
    else
    {
        updateLogMessages(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
    }
}


void QSpeedTestCli::updateLogMessages(QString value)
{
    qDebug() << qPrintable((QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + value));
}


void QSpeedTestCli::updateTestResults(QString value)
{
    qDebug() << qPrintable(value);
}


void QSpeedTestCli::printHostAndProgramInfo()
{
    QEventLoop loop;
    QProcess proc;

    results.testDate = QDate::currentDate().toString("yyyyMMdd");
    results.testTime = QTime::currentTime().toString("hhmmss");
    results.testDateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    updateTestResults(trUtf8("Report created by:       %1 %2\n"
                             "Target list version:     %3\n"
                             "Target list comment:     %4\n"
                             "Target list contact URL: %5").arg(results.programName).arg(results.programVersion).arg(results.targetListVersion).arg(results.targetListComment).arg(results.targetListContactUrl));
    proc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
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

    updateTestResults(trUtf8("Host OS:                 %1\n"
                             "Test date and time:      %2").arg(results.hostOS).arg(results.testDateTime));
}


void QSpeedTestCli::printLineInfo()
{
    QEventLoop loop1, loop2;
    QProcess traceroute;
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QByteArray contents;
    bool foundFlag = false;
    QStringList list;
#ifdef Q_WS_WIN
    QString tracerouteCommand = "pathping -h 2 -q 1 8.8.8.8";
    QString bbrasLine = "  2  ";
#else
    QString tracerouteCommand = "traceroute -m 2 -q 1 8.8.8.8";
    QString bbrasLine = " 2  ";
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
    updateTestResults(trUtf8("ISP:                     %1\n"
                             "Internet IP:             %2\n"
                             "BBRAS:                   %3\n").arg(results.isp).arg(results.ip).arg(results.bbras));
}


void QSpeedTestCli::startBenchmark()
{
    QTime timer;

    timer.start();
    updateLogMessages(trUtf8("Test started\n"));
    results.testMode = testModeAsString;
    results.targetsTotal = targetList.numberOfTargets;
    results.pingsPerTarget = PINGSPERTARGET;
    results.parallelPingThreads = PARALLELPINGS;
    vbCode.clear();
    htmlCode.clear();
    results.reset();
    QThreadPool::globalInstance()->setMaxThreadCount(results.parallelPingThreads - 1);
    printHostAndProgramInfo();
    printLineInfo();

    for(int i = 0; i < targetList.numberOfGroups && pingTestEnabled; i++)
    {
        targetList.groups[i].reset();
        updateTestResults(targetList.groups[i].getName().leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        updateTestResults("-------------------------------------------------------------------------------");

        if(PARALLELPINGS > 1)    // multithreaded pinging
        {
            QtConcurrent::blockingMap(targetList.groups[i].targets, &Target::ping);
        }
        else    // single-threaded pinging
        {
            for(int j = 0; j < targetList.groups[i].getSize(); j++)
            {
                targetList.groups[i].targets[j].ping();
            }
        }

        for(int j = 0; j < targetList.groups[i].getSize(); j++)
        {
            targetList.groups[i].plainTargets.append(PlainTarget(targetList.groups[i].targets[j]));
        }

        qApp->processEvents();
        qSort(targetList.groups[i].plainTargets);
        results.rttSum += targetList.groups[i].getRttSum();
        results.targetsAlive += targetList.groups[i].getTargetsAlive();
        updateTestResults(trUtf8("Group sum: %1\nGroup average: %2\n").arg(targetList.groups[i].getRttSumAsString()).arg(targetList.groups[i].getRttAvgAsString()));
    }

    if(downloadTestEnabled)
    {
        updateTestResults(trUtf8("Downloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
        QThreadPool::globalInstance()->setMaxThreadCount(targetList.fileHosts.size());

        BYTESDOWNLOADED = 0;
        QtConcurrent::blockingMap(targetList.fileHosts, &FileHost::downloadTest);
        qApp->processEvents();
        results.speedInKbps = (BYTESDOWNLOADED) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
        results.speedInMBps = results.speedInKbps / 8192;                        // (results.speedInKbps / 1024) / 8

    }

    results.testDuration = (timer.elapsed() * 1.0) / 1000;
    updateTestResults(trUtf8("\n"
                             "Test mode:             %1\n"
                             "Test completed in:     %2 sec").arg(testModeAsString).arg(results.testDuration));

    if(pingTestEnabled)
    {
        updateTestResults(trUtf8(""
                             "Pings/target:          %1\n"
                             "Parallel ping threads: %2").arg(results.pingsPerTarget).arg(results.parallelPingThreads));
        updateTestResults(trUtf8(""
                             "Targets alive:         %1 / %2\n"
                             "Test total ping time:  %3\n"
                             "Average ping/target:   %4").arg(results.targetsAlive).arg(results.targetsTotal).arg(results.getRttSumAsString()).arg(results.getRttAvgAsString()));
    }

    if(downloadTestEnabled)
    {
        updateTestResults(trUtf8(""
                             "Download speed:        %1 Kbps\n"
                             "                       %2 MB/sec").arg(results.speedInKbps, 0, 'f', 0).arg(results.speedInMBps, 0, 'f', 3));
    }

    updateLogMessages(trUtf8("Test complete"));
}


void QSpeedTestCli::saveReports()
{
    QFile file;
    QString fileName;
    QTextStream outStream;

    if(htmlOutputEnabled)
    {
        generateHtmlCode();
        fileName = QDir::currentPath() + QString("/%1-%2%3-%4.html").arg(results.programName).arg(results.testDate).arg(results.testTime).arg(results.ip);
        fileName = QDir::toNativeSeparators(fileName);
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            updateLogMessages(trUtf8("Error writing HTML report %1").arg(fileName));
            return;
        }

        outStream << htmlCode;
        file.close();
        updateLogMessages(trUtf8("HTML report saved to %1").arg(fileName));
    }

    if(vbOutputEnabled)
    {
        generateVbCode();
        fileName = QDir::currentPath() + QString("/%1-%2%3-%4.vb.txt").arg(results.programName).arg(results.testDate).arg(results.testTime).arg(results.ip);
        fileName = QDir::toNativeSeparators(fileName);
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            updateLogMessages(trUtf8("Error writing vBulletin report %1").arg(fileName));
            return;
        }

        outStream << vbCode;
        file.close();
        updateLogMessages(trUtf8("vBulletin report saved to %1").arg(fileName));
    }
}


void QSpeedTestCli::generateHtmlCode()
{
    if(!htmlCode.isEmpty())
    {
        return;
    }

    htmlCode  = trUtf8("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                       "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                       "    <head>\n"
                       "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
                       "        <title>%1 report - %2</title>\n").arg(results.programName).arg(results.testDateTime);
    htmlCode += trUtf8("    </head>\n"
                       "    <body>\n"
                       "        <table border=\"1\" cellpadding=\"4\">\n"
                       "            <tr><td>&nbsp;</td><td align=\"center\">Client info</td></tr>\n"
                       "            <tr><td>Report created by</td><td align=\"center\">%1 %2 - <a href=\"%3\">Download</a> - <a href=\"%4\">Discuss</a></td></tr>\n").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl);
    htmlCode += trUtf8("            <tr><td>Target list used</td><td align=\"center\"><a href=\"%1\">%2</a></td></tr>\n").arg(results.targetListContactUrl).arg(results.targetListComment);
    htmlCode += trUtf8("            <tr><td>Host OS</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Test date and time</td><td align=\"center\">%2</td></tr>\n").arg(results.hostOS).arg(results.testDateTime);
    htmlCode += trUtf8("            <tr><td>ISP</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Internet IP</td><td align=\"center\">%2</td></tr>\n"
                       "            <tr><td>BBRAS</td><td align=\"center\">%3</td></tr>\n"
                       "        </table>\n").arg(results.isp).arg(results.ip).arg(results.bbras);

    htmlCode += (pingTestEnabled)? (""
                       "        <p style=\"margin-bottom: 2px;\"><b>Results per group</b></p>\n"
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

    htmlCode += trUtf8("        <table border=\"1\" cellpadding=\"4\" style=\"margin-top: 10px;\">\n"
                       "            <tr><td align=\"center\">Variable</td><td align=\"center\">Value</td></tr>\n"
                       "            <tr><td>Test mode</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Test completed in</td><td align=\"center\">%2 sec</td></tr>\n").arg(results.testMode).arg(results.testDuration);

    if(pingTestEnabled)
    {
        htmlCode += trUtf8(""
                       "            <tr><td>Pings/target</td><td align=\"center\">%1</td></tr>\n"
                       "            <tr><td>Parallel ping threads</td><td align=\"center\">%2</td></tr>\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
        htmlCode += trUtf8(""
                       "            <tr><td>Targets alive</td><td align=\"center\">%1 / %2</td></tr>\n").arg(results.targetsAlive).arg(results.targetsTotal);
        htmlCode += trUtf8(""
                       "            <tr><td>Test total ping time</td><td align=\"center\">%1</td></tr>\n").arg(results.getRttSumAsString());
        htmlCode += trUtf8(""
                       "            <tr><td><b>Average ping/target</b></td><td align=\"center\"><b>%1</b></td></tr>\n").arg(results.getRttAvgAsString());
    }

    if(downloadTestEnabled)
    {
        htmlCode += trUtf8(""
                       "            <tr><td><b>Download speed</b></td><td align=\"center\"><b>%1 Kbps</b></td></tr>\n"
                       "            <tr><td>&nbsp;</td><td align=\"center\"><b>%2 MB/sec</b></td></tr>\n").arg(results.speedInKbps, 0, 'f', 0).arg(results.speedInMBps, 0, 'f', 3);
    }

    htmlCode +=        "        </table>\n"
                       "        <p>\n"
                       "            <a href=\"http://validator.w3.org\">\n"
                       "                <img src=\"http://www.w3.org/Icons/valid-xhtml11\" alt=\"Valid XHTML 1.1\" height=\"31\" width=\"88\" />\n"
                       "            </a>\n"
                       "        </p>\n"
                       "    </body>\n"
                       "</html>\n";
    htmlCode.replace('&', "&amp;");
    htmlCode.replace("&amp;nbsp", "&nbsp");
}


void QSpeedTestCli::generateVbCode()
{
    if(!vbCode.isEmpty())
    {
        return;
    }

    vbCode  = trUtf8("[table=head] | Client info\n"
                     "Report created by | [center]%1 %2 - [url=%3]Download[/url] - [url=%4]Discuss[/url][/center] |\n"
                     "Target list version | [center]%5[/center] |\n"
                     "Target list comment | [center][url=%6]%7[/url][/center] |\n"
                     "Host OS | [center]%8[/center] |\n"
                     "Test date and time | [center]%9[/center] |\n").arg(results.programName).arg(results.programVersion).arg(results.programUrl).arg(results.programDiscussUrl).arg(results.targetListVersion).arg(results.targetListContactUrl).arg(results.targetListComment).arg(results.hostOS).arg(results.testDateTime);
    vbCode += trUtf8("ISP | [center]%1[/center] |\n"
                     "Internet IP | [center]%2[/center] |\n"
                     "BBRAS | [center]%3[/center] |\n"
                     "[/table]\n").arg(results.isp).arg(results.ip).arg(results.bbras);
    vbCode += (pingTestEnabled)? (""
                     "\n[b]Results per group[/b]\n"
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
    vbCode += trUtf8("[table=head][center]Variable[/center] | Value\n"
                     "Test mode | [center]%1[/center] |\n"
                     "Test completed in | [center]%2 sec[/center] |\n").arg(results.testMode).arg(results.testDuration);

    if(pingTestEnabled)
    {
        vbCode += trUtf8(""
                     "Pings/target | [center]%1[/center] |\n"
                     "Parallel ping threads | [center]%2[/center] |\n").arg(results.pingsPerTarget).arg(results.parallelPingThreads);
        vbCode += trUtf8(""
                     "Targets alive | [center]%1 / %2[/center] |\n"
                     "Test total ping time | [center]%3[/center] |\n"
                     "[b]Average ping/target[/b] | [center][b]%4[/b][/center] |\n").arg(results.targetsAlive).arg(results.targetsTotal).arg(results.getRttSumAsString()).arg(results.getRttAvgAsString());
    }

    if(downloadTestEnabled)
    {
        vbCode += trUtf8(""
                     "[b]Download speed[/b] | [center][b]%1 Kbps[/b][/center] |\n"
                     " | [center][b]%2 MB/sec[/b][/center] |\n").arg(results.speedInKbps, 0, 'f', 0).arg(results.speedInMBps, 0, 'f', 3);
    }

    vbCode +=        "[/table]\n\n";
}
