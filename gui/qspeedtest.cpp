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
#include "pingtarget.h"
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


QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv) {
    results.programName = PROGRAMNAME;
    results.programVersion = PROGRAMVERSION;
    results.programUrl = PROGRAMURL;
    results.programDiscussUrl = PROGRAMDISCUSSURL;
    connect(this, SIGNAL(initOK()), &mainWindow, SLOT(enablePushButtonStartStop()));
    connect(&mainWindow, SIGNAL(pushButtonStartClicked()), this, SLOT(runBenchmark()));
    connect(this, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    connect(this, SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
    connect(this, SIGNAL(benchmarkFinished(bool)), &mainWindow, SLOT(benchmarkFinished(bool)));
    connect(&mainWindow, SIGNAL(pushButtonHtmlCodeClicked(bool)), this, SLOT(showReport(bool)));
    connect(&mainWindow, SIGNAL(pushButtonVbCodeClicked(bool)), this, SLOT(showReport(bool)));
    connect(&results, SIGNAL(message(QString)), &mainWindow, SLOT(updateTestResults(QString)));
    mainWindow.show();
    targetList = new TargetList(PROJECTNAME, PROGRAMNAME);
    connect(targetList, SIGNAL(logMessage(QString)), &mainWindow, SLOT(updateLogMessages(QString)));
    hostInfo = new HostInfo(&results);
    checkForProgramUpdates();

    if(!targetList->init())
        return;

    results.targetListVersion = targetList->getVersion();
    results.targetListComment = targetList->getComment();
    results.targetListContactUrl = targetList->getContactUrl();

    for(int i = 0; i < targetList->groups.size(); i++)
        connectPingGroup(targetList->groups[i]);

    foreach(QList<DownloadTarget> *targets, QList<QList<DownloadTarget>*>() << &targetList->fileHostsDomestic << &targetList->fileHostsInternational) {
        connect(&(targets->operator [](0)), SIGNAL(sigProgress(quint8)), this, SLOT(slotUpdateProgress(quint8)));

        for(int i = 0; i < targets->size(); i++)
            connect(&(targets->operator [](i)), SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
    }

    emit logMessage(trUtf8("Ready"));
    emit initOK();
}


QSpeedTest::~QSpeedTest() {
    delete targetList;
    delete hostInfo;
}


void QSpeedTest::checkForProgramUpdates() {
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    emit logMessage(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUT *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROGRAMUPDATECHECKURL)));
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error()) {
        download->abort();
        emit logMessage(trUtf8("Update site unreachable"));
        delete download;
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROGRAMVERSION.mid(1).toInt()) {
        emit logMessage(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        int reply = QMessageBox::question(NULL, trUtf8("Update available"), trUtf8("An updated version of %1 (%2) is available online.\n\nWould you like to open the download page in your browser?").arg(PROGRAMNAME).arg(remoteVersion), QMessageBox::Yes, QMessageBox::No);

        if(reply == QMessageBox::Yes) {
            emit logMessage(trUtf8("Opening download page %1").arg(PROGRAMURL));
            QDesktopServices::openUrl(QUrl(PROGRAMURL));
        }
    }
    else {
        emit logMessage(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
    }

    delete download;
}


void QSpeedTest::connectPingGroup(PingGroup &group) {
    for(int i = 0; i < group.targets.size(); i++)
        connect(&group.targets[i], SIGNAL(newTestResult(QString)), &mainWindow, SLOT(updateTestResults(QString)));
}


void QSpeedTest::runBenchmark() {
    QTime timer;

    timer.start();
    emit logMessage(trUtf8("Test started"));
    results.targetsTotal = targetList->getNumberOfTargets();
    results.pingsPerTarget = PINGSPERTARGET;
    results.parallelPingThreads = mainWindow.parallelThreads();
    pingTestEnabled = results.pingTestEnabled = mainWindow.pingTestEnabled();
    downloadTestEnabled = results.downloadTestEnabled = mainWindow.downloadTestEnabled();
    vbCode.clear();
    htmlCode.clear();
    results.reset();
    QThreadPool::globalInstance()->setMaxThreadCount(results.parallelPingThreads - 1);
    hostInfo->init();

    for(int i = 0; i < targetList->groups.size() && pingTestEnabled; i++) {
        targetList->groups[i].reset();
        emit newTestResult(targetList->groups[i].getName().leftJustified(27, ' ', true) + "    " + trUtf8("Avg ping").rightJustified(11, ' ', true) + "    " + trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + trUtf8("Rank").rightJustified(4, ' ', true));
        emit newTestResult("-------------------------------------------------------------------------------");

        if(results.parallelPingThreads > 1)    // multithreaded pinging
            QtConcurrent::blockingMap(targetList->groups[i].targets, &PingTarget::ping);
        else    // single-threaded pinging
            for(int j = 0; j < targetList->groups[i].getSize(); j++) {
                MUTEX.lock();
                if(STOPBENCHMARK) {
                    MUTEX.unlock();
                    emit benchmarkFinished(false);
                    return;
                }
                MUTEX.unlock();

                targetList->groups[i].targets[j].ping();
            }


        MUTEX.lock();
        if(STOPBENCHMARK) {
            MUTEX.unlock();
            emit benchmarkFinished(false);
            return;
        }
        MUTEX.unlock();

        processEvents();
        targetList->groups[i].sort();
        connectPingGroup(targetList->groups[i]);
        results.rttSum += targetList->groups[i].getRttSum();
        results.targetsAlive += targetList->groups[i].getTargetsAlive();
        emit newTestResult(trUtf8("Group sum:     %1\n"
                                  "Group average: %2\n").arg(targetList->groups[i].getRttSumAsString()).arg(targetList->groups[i].getRttAvgAsString()));
        processEvents();
    }

    if(downloadTestEnabled) {
        foreach(QList<DownloadTarget> *targets, QList<QList<DownloadTarget>*>() << &targetList->fileHostsDomestic << &targetList->fileHostsInternational) {
            for(int i = 0; i < targets->size(); i++)
                connect(&mainWindow, SIGNAL(pushButtonStopClicked()), &(targets->operator [](i)), SLOT(abortDownload()));

            emit newTestResult(trUtf8("\nDownloading the following files, please wait approx. %1 seconds:").arg(DOWNLOADTESTSECS));
            QThreadPool::globalInstance()->setMaxThreadCount(targets->size());
            mainWindow.showProgressBar();
            QtConcurrent::blockingMap(*targets, &DownloadTarget::downloadTest);
            mainWindow.hideProgressBar();
            processEvents();

            for(int i = 0; i < targets->size(); i++)
                disconnect(&mainWindow, SIGNAL(pushButtonStopClicked()), &(targets->operator [](i)), SLOT(abortDownload()));

            MUTEX.lock();
            if(STOPBENCHMARK) {
                MUTEX.unlock();
                emit benchmarkFinished(false);
                return;
            }
            MUTEX.unlock();
        }

        results.speedInKbpsDomestic = targetList->getBytesDownloaded(targetList->fileHostsDomestic) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
        results.speedInMBpsDomestic = results.speedInKbpsDomestic / 8192;    // (results.speedInKbpsDomestic / 1024) / 8
        results.speedInKbpsInternational = targetList->getBytesDownloaded(targetList->fileHostsInternational) / (DOWNLOADTESTSECS * 128.0);    // ((BYTESDOWNLOADED * 8) / 1024) / (DOWNLOADTESTSECS * 1.0)
        results.speedInMBpsInternational = results.speedInKbpsInternational / 8192;    // (results.speedInKbpsInernational / 1024) / 8
        results.testMode = (pingTestEnabled)? trUtf8("Ping and speed") : trUtf8("Speed only");
    }
    else
        results.testMode = trUtf8("Ping only");

    hostInfo->retrieve();
    results.testDuration = (timer.elapsed() * 1.0) / 1000;
    results.print();
    emit logMessage(trUtf8("Test complete"));
    emit benchmarkFinished(true);
}


void QSpeedTest::slotUpdateProgress(quint8 value) {
    mainWindow.updateProgressBar(value);
}


void QSpeedTest::showReport(bool showHtml) {
    QFile file;
    QString fileName;
    QTextStream outStream;

    if(showHtml) {
        generateHtmlCode();
        fileName = QDir::tempPath() + QString("/%1_%2%3.html").arg(results.programName).arg(results.testDate).arg(results.testTime);
    }
    else {
        generateVbCode();
        fileName = QDir::tempPath() + QString("/%1_%2%3.vb.txt").arg(results.programName).arg(results.testDate).arg(results.testTime);
    }

    fileName = QDir::toNativeSeparators(fileName);
    file.setFileName(fileName);
    outStream.setDevice(&file);
    outStream.setCodec("UTF-8");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(NULL, trUtf8("Error"), trUtf8("Error writing temporary file %1").arg(fileName));
        return;
    }

    outStream << ((showHtml)? htmlCode : vbCode);
    file.close();
    QApplication::clipboard()->setText((showHtml)? htmlCode : vbCode);
    mainWindow.showStatusBarMessage(trUtf8("Code copied to clipboard"));
    QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(fileName)));
}


void QSpeedTest::generateHtmlCode() {
    if(!htmlCode.isEmpty())
        return;

    htmlCode = results.getHtmlCode(targetList);
}


void QSpeedTest::generateVbCode()
{
    if(!vbCode.isEmpty())
        return;

    vbCode = results.getVbCode(targetList);
}
