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
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QDir>


const QString PROJECTNAME = "QSpeedTest";
const QString PROGRAMNAME = "QSpeedTestCLI";
const QString PROGRAMVERSION = "r38";
const QString PROGRAMAUTHOR = "parsifal";
const QString PROGRAMURL = "https://sourceforge.net/projects/qspeedtest/files/";
const QString PROGRAMDISCUSSURL = "https://sourceforge.net/apps/phpbb/qspeedtest";
const QString PROGRAMUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtestcli.version";
quint8 PARALLELPINGS = 4;


QSpeedTestCli::QSpeedTestCli() {
    results.programName = PROGRAMNAME;
    results.programVersion = PROGRAMVERSION;
    results.programUrl = PROGRAMURL;
    results.programDiscussUrl = PROGRAMDISCUSSURL;
    targetList = NULL;
    hostInfo = NULL;
    testMode = TestMode::All;
    testModeAsString = trUtf8("Ping and speed");
    pingTestEnabled = results.pingTestEnabled = true;
    downloadTestEnabled = results.downloadTestEnabled = true;
    htmlOutputEnabled = true;
    vbOutputEnabled = true;
}


QSpeedTestCli::~QSpeedTestCli() {
    if(targetList) delete targetList;
    if(hostInfo) delete hostInfo;
}


void QSpeedTestCli::start() {
    parseArguments();
    targetList = new TargetList(PROJECTNAME, PROGRAMNAME);
    hostInfo = new HostInfo(&results);
    connect(targetList, SIGNAL(logMessage(QString)), this, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(logMessage(QString)), this, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));
    checkForProgramUpdates();

    if(!targetList->init()) {
        updateLogMessages(trUtf8("Unsuccessful initialization of the target list, %1 will now exit").arg(PROGRAMNAME));
        exit(8);
    }

    for(int i = 0; i < targetList->groups.size(); i++)
        for(int j = 0; j < targetList->groups[i].getSize(); j++)
            connect(&targetList->groups[i].targets[j], SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));

    foreach(QList<DownloadTarget> *targets, QList<QList<DownloadTarget>*>() << &targetList->fileHostsDomestic << &targetList->fileHostsInternational)
        for(int i = 0; i < targets->size(); i++)
            connect(&(targets->operator [](i)), SIGNAL(newTestResult(QString)), this, SLOT(updateTestResults(QString)));

    connect(&results, SIGNAL(message(QString)), this, SLOT(updateTestResults(QString)));
    results.targetListVersion = targetList->getVersion();
    results.targetListComment = targetList->getComment();
    results.targetListContactUrl = targetList->getContactUrl();
    runBenchmark();
    saveReports();
    qApp->processEvents();
    qApp->quit();
}


void QSpeedTestCli::parseArguments() {
    QStringList args(qApp->arguments());
    int count = args.count();
    QString arg;
    QString argNext;
    QString message;
    int number;

    if(count == 1) {
        qDebug() << qPrintable(trUtf8("No parameters specified, running with defaults:\n"
                                      "Mode:             Ping and speed\n"
                                      "Pings per target: %1\n"
                                      "Threads:          %2\n"
                                      "HTML output:      Enabled\n"
                                      "vBulletin output: Enabled\n").arg(PINGSPERTARGET).arg(PARALLELPINGS));
        return;
    }

    for(int i = 1; i < count; i++) {   // no point checking arg[0], it is always the executable's name
        arg = args.at(i).toLower();

        if(i < count - 1)    // if there is a next argument after the current
            argNext = args.at(i + 1);
        else
            argNext.clear();

        if(arg == "--help" || arg == "-h" || arg == "help" || arg == "/?" || arg == "?") {
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
                             "    \"speed\": will only perform a download speed test\n"
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

        if(arg == "--version" || arg == "-v") {
            qDebug() << qPrintable(trUtf8("%1 %2").arg(PROGRAMNAME).arg(PROGRAMVERSION));
            exit(0);
        }

        if(arg == "--mode" || arg == "-m") {
            if(argNext.isEmpty()) {
                qDebug() << qPrintable(trUtf8("Error #1: No value specified after --mode (-m) switch"));
                exit(1);
            }

            if(argNext == "info") {
                testMode = TestMode::Info;
                testModeAsString = trUtf8("Info only");
                pingTestEnabled = results.pingTestEnabled = false;
                downloadTestEnabled = results.downloadTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "ping") {
                testMode = TestMode::Ping;
                testModeAsString = trUtf8("Ping only");
                downloadTestEnabled = results.downloadTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "speed") {
                testMode = TestMode::Download;
                testModeAsString = trUtf8("Speed only");
                pingTestEnabled = results.pingTestEnabled = false;
                i++;
                continue;
            }

            if(argNext == "all") {
                testMode = TestMode::All;
                i++;
                continue;
            }

            qDebug() << qPrintable(trUtf8("Error #2: Value \"%1\" after --mode (-m) switch not a known mode").arg(argNext));
            exit(2);
        }

        if(arg == "--pings" || arg == "-p") {
            if(argNext.isEmpty()) {
                qDebug() << qPrintable(trUtf8("Error #3: No value specified after --pings (-p) switch"));
                exit(3);
            }

            if((!(number = argNext.toInt())) || number < 1 || number > 100) {
                qDebug() << qPrintable(trUtf8("Error #4: Value \"%1\" after --pings (-p) switch not an integer between 1 and 100").arg(argNext));
                exit(4);
            }

            PINGSPERTARGET = number;
            i++;
            continue;
        }

        if(arg == "--threads" || arg == "-t") {
            if(argNext.isEmpty()) {
                qDebug() << qPrintable(trUtf8("Error #5: No value specified after --threads (-t) switch"));
                exit(5);
            }

            if((!(number = argNext.toInt())) || number < 1 || number > 8) {
                qDebug() << qPrintable(trUtf8("Error #6: Value \"%1\" after --threads (-t) switch not an integer between 1 and 8").arg(argNext));
                exit(6);
            }

            PARALLELPINGS = number;
            i++;
            continue;
        }

        if(arg == "--nohtml" || arg == "-nh") {
            htmlOutputEnabled = false;
            continue;
        }

        if(arg == "--novb" || arg == "-nv") {
            vbOutputEnabled = false;
            continue;
        }

        qDebug() << qPrintable(trUtf8("Error #7: Unknown switch or parameter \"%1\"").arg(arg));
        exit(7);
    }

    // if execution flow reaches this point, all arguments were valid
    qDebug() << qPrintable(trUtf8("Custom parameters specified:\n"
                                  "Mode:             %1\n"
                                  "Pings per target: %2\n"
                                  "Threads:          %3\n"
                                  "HTML output:      %4\n"
                                  "vBulletin output: %5\n").arg(testModeAsString).arg(PINGSPERTARGET).arg(PARALLELPINGS).arg((htmlOutputEnabled)? trUtf8("Enabled") : trUtf8("Disabled")).arg((vbOutputEnabled)? trUtf8("Enabled") : trUtf8("Disabled")));
}


void QSpeedTestCli::checkForProgramUpdates() {
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    updateLogMessages(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROGRAMUPDATECHECKURL)));    // according to Qt documentation, manager will be set as the parent of download, so manual deletion of download should not be necessary
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error()) {
        download->abort();
        download->deleteLater();
        updateLogMessages(trUtf8("Update site unreachable"));
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROGRAMVERSION.mid(1).toInt()) {
        updateLogMessages(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        updateLogMessages(trUtf8("You can find the new version at %1").arg(PROGRAMURL));
    }
    else
        updateLogMessages(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
}


void QSpeedTestCli::updateLogMessages(QString value) {
    qDebug() << qPrintable((QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + value));
}


void QSpeedTestCli::updateTestResults(QString value) {
    qDebug() << qPrintable(value);
}


void QSpeedTestCli::runBenchmark() {
    QTime timer;

    timer.start();
    updateLogMessages(trUtf8("Test started\n"));
    results.testMode = testModeAsString;
    results.targetsTotal = targetList->getNumberOfTargets();
    results.pingsPerTarget = PINGSPERTARGET;
    results.parallelPingThreads = PARALLELPINGS;
    vbCode.clear();
    htmlCode.clear();
    results.reset();
    QThreadPool::globalInstance()->setMaxThreadCount(PARALLELPINGS - 1);
    hostInfo->init();

    for(int i = 0; i < targetList->groups.size() && pingTestEnabled; i++) {
        targetList->groups[i].reset();
        updateTestResults(targetList->groups[i].getName().leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        updateTestResults("-------------------------------------------------------------------------------");

        if(PARALLELPINGS > 1)    // multithreaded pinging
            QtConcurrent::blockingMap(targetList->groups[i].targets, &PingTarget::ping);
        else    // single-threaded pinging
            for(int j = 0; j < targetList->groups[i].getSize(); j++)
                targetList->groups[i].targets[j].ping();

        qApp->processEvents();
        targetList->groups[i].sort();
        results.rttSum += targetList->groups[i].getRttSum();
        results.targetsAlive += targetList->groups[i].getTargetsAlive();
        updateTestResults(trUtf8("Group sum:     %1\n"
                                 "Group average: %2\n").arg(targetList->groups[i].getRttSumAsString()).arg(targetList->groups[i].getRttAvgAsString()));
    }

    if(downloadTestEnabled) {
        foreach(QList<DownloadTarget> *targets, QList<QList<DownloadTarget>*>() << &targetList->fileHostsDomestic << &targetList->fileHostsInternational) {
            updateTestResults(trUtf8("\nDownloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
            QThreadPool::globalInstance()->setMaxThreadCount(targets->size());
            QtConcurrent::blockingMap(*targets, &DownloadTarget::downloadTest);
            qApp->processEvents();
        }

        results.speedInKbpsDomestic = targetList->getBytesDownloaded(targetList->fileHostsDomestic) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
        results.speedInMBpsDomestic = results.speedInKbpsDomestic / 8192;    // (results.speedInKbpsDomestic / 1024) / 8
        results.speedInKbpsInternational = targetList->getBytesDownloaded(targetList->fileHostsInternational) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
        results.speedInMBpsInternational = results.speedInKbpsInternational / 8192;    // (results.speedInKbpsInernational / 1024) / 8
    }

    hostInfo->retrieve();
    results.testDuration = (timer.elapsed() * 1.0) / 1000;
    results.print();
    qDebug() << "\n\n";
    updateLogMessages(trUtf8("Test complete"));
}


void QSpeedTestCli::saveReports() {
    QFile file;
    QString fileName;
    QTextStream outStream;

    if(htmlOutputEnabled) {
        generateHtmlCode();
        fileName = QDir::currentPath() + QString("/%1_%2%3_%4.html").arg(results.programName).arg(results.testDate).arg(results.testTime).arg(results.ip);
        fileName = QDir::toNativeSeparators(fileName);
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            updateLogMessages(trUtf8("Error writing HTML report %1").arg(fileName));
            return;
        }

        outStream << htmlCode;
        file.close();
        updateLogMessages(trUtf8("HTML report saved to %1").arg(fileName));
    }

    if(vbOutputEnabled) {
        generateVbCode();
        fileName = QDir::currentPath() + QString("/%1_%2%3_%4.vb.txt").arg(results.programName).arg(results.testDate).arg(results.testTime).arg(results.ip);
        fileName = QDir::toNativeSeparators(fileName);
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            updateLogMessages(trUtf8("Error writing vBulletin report %1").arg(fileName));
            return;
        }

        outStream << vbCode;
        file.close();
        updateLogMessages(trUtf8("vBulletin report saved to %1").arg(fileName));
    }
}


void QSpeedTestCli::generateHtmlCode() {
    if(!htmlCode.isEmpty())
        return;

    htmlCode = results.getHtmlCode(targetList);
}


void QSpeedTestCli::generateVbCode() {
    if(!vbCode.isEmpty())
        return;

    vbCode = results.getVbCode(targetList);
}
