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
#include "externs.h"
#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

QSpeedTest::QSpeedTest(int argc, char **argv) : QApplication(argc, argv)
{
#ifndef Q_WS_WIN
    QFont appFont = font();

    appFont.setPointSize(9);
    setFont(appFont);
#endif

    _preferences = new Preferences(QSettings::IniFormat, QSettings::UserScope, PROJECTNAME, PROJECTNAME.toLower() + ".preferences", this);
    _hostlist = new Hostlist(QSettings::IniFormat, QSettings::UserScope, PROJECTNAME, PROJECTNAME.toLower() + ".hostlist", _results, this);
    _hostInfo = new HostInfo(_results, this);    // Starts some partial host info extraction in the background
    _controller = new TestController(_results, this);
    _connectSignalsSlots();
    _w.show();
    _preferences->init();
    _checkForProgramUpdates();

    if(!_hostlist->initOk())
    {
        emit hostlistOk(false);    // Disable Start button (although it is by default enabled, this should not be forgotten when loading hostlist from an external file!
        return;
    }

    emit hostlistOk(true);    // Enable Start button
    _w.slotLog(trUtf8("Ready"));
}

void QSpeedTest::_connectSignalsSlots()
{
    connect(_preferences, SIGNAL(message(QString)), &_w, SLOT(slotLog(QString)));
    connect(_hostlist, SIGNAL(message(QString)), &_w, SLOT(slotLog(QString)));
    connect(this, SIGNAL(hostlistOk(bool)), &_w, SLOT(_slotEnablePushButtonStartStop(bool)));
    connect(&_w, SIGNAL(pushButtonStartClicked()), this, SLOT(_slotStartTests()));
    connect(&_w, SIGNAL(pushButtonStopClicked()), this, SLOT(_slotStopTests()));
    connect(&_w, SIGNAL(copyReport(ReportFormat::Format)), this, SLOT(_slotCopyReport(ReportFormat::Format)));
    connect(&_w, SIGNAL(saveReport(ReportFormat::Format)), this, SLOT(_slotSaveReport(ReportFormat::Format)));
    connect(&_w.d1, SIGNAL(savePreferences()), this, SLOT(_slotSavePreferences()));
    connect(_controller, SIGNAL(progressRangeChanged(int,int)), &_w, SLOT(_slotSetProgressBarRange(int,int)));
    connect(_controller, SIGNAL(progressValueChanged(int)), &_w, SLOT(_slotUpdateProgressBar(int)));
}

void QSpeedTest::_checkForProgramUpdates()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    _w.slotLog(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUTSECS * 1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROJECTVERSIONURL)));    // From here on, download pointer does not require manual deletion!
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        _w.slotLog(trUtf8("Update site unreachable"));
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROJECTVERSION.mid(1).toInt())
    {
        _w.slotLog(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        int reply = QMessageBox::question(&_w, trUtf8("Update available"), trUtf8("An updated version of %1 (r%2) is available online.") + "\n\n" + trUtf8("Would you like to open the download page in your browser?").arg(PROGRAMNAME).arg(remoteVersion), QMessageBox::Yes, QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            _w.slotLog(trUtf8("Opening download page %1").arg(PROJECTDOWNLOADURL));
            QDesktopServices::openUrl(QUrl(PROJECTDOWNLOADURL));
        }
    }
    else {
        _w.slotLog(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
    }
}

void QSpeedTest::_slotCopyReport(ReportFormat::Format format)
{
    QString report;
    QString reportType;

    switch(format)
    {
    case ReportFormat::PlainText:
        report = _results.plainText();
        reportType = trUtf8("Plain text");
        break;

    case ReportFormat::BbCode:
        report = _results.bbCode();
        reportType = trUtf8("bbCode");
        break;

    case ReportFormat::Html:
        report = _results.html();
        reportType = trUtf8("HTML");
        break;
    default:
        break;
    }

    QApplication::clipboard()->setText(report);
    _w.slotLog(trUtf8("A report in %1 format was copied to the clipboard").arg(reportType));
}

void QSpeedTest::_slotSaveReport(ReportFormat::Format format)
{
    QString filename = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QString("/%1_%2%3").arg(_results.programName()).arg(_results.testDate()).arg(_results.testTime());
    QString filter;

    switch(format)
    {
    case ReportFormat::PlainText:
        filename += ".txt";
        filter = trUtf8("Plain text (*.txt)");
        break;
    case ReportFormat::BbCode:
        filename += ".bb.txt";
        filter = "bbCode (*.bb.txt)";
        break;
    case ReportFormat::Html:
        filename += ".html";
        filter += "HTML (*.html)";
        break;
    default:
        break;
    }

    filename = QFileDialog::getSaveFileName(&_w, trUtf8("Select path and filename for the report"), filename, filter);

    if(filename.isNull())
    {
        return;
    }

    if(!_results.saveReport(format, filename))
    {
        _w.slotLog(trUtf8("Error writing report to") + ' ' + QDir::toNativeSeparators(filename));
        _w.slotLog(trUtf8("Check directory access permissions"));
        return;
    }

    _w.slotLog(trUtf8("Report written to") + ' ' + QDir::toNativeSeparators(filename));
    QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(filename)));
}

void QSpeedTest::_slotSavePreferences()
{
    _preferences->setValue("TestMode", TESTMODE);
    _preferences->setValue("PingsPerHost", PINGSPERHOST);
    _preferences->setValue("PingThreads", PINGTHREADS);
    _preferences->setValue("PingTimeoutSecs", PINGTIMEOUTSECS);
    _preferences->setValue("DownloadTestSecs", DOWNLOADTESTSECS);
    _preferences->setValue("HostlistUrl", HOSTLISTURL);
    _preferences->sync();
    _w.slotLog(trUtf8("Preferences saved to") + ' ' + QDir::toNativeSeparators(_preferences->fileName()));
}

void QSpeedTest::_slotStartTests()
{
    _timer.start();
    _results.reset();
     connect(_controller, SIGNAL(finished()), _hostInfo, SLOT(slotStartInfoTest()));
     connect(_controller, SIGNAL(result(QString)), &_w, SLOT(slotResult(QString)));
     connect(_hostInfo, SIGNAL(finished()), this, SLOT(_slotTestsFinished()));
     _hostInfo->startDetection();

    switch(TESTMODE)
    {
    case TestMode::Info:
        _hostInfo->slotStartInfoTest();
        break;

    case TestMode::Ping:

    case TestMode::All:
        _w.showProgressBar();
        _controller->startPingTest();
        break;

    case TestMode::Download:
        _w.showProgressBar();
        _controller->startDownloadTest();

    default:
        break;
    }
}

void QSpeedTest::_slotStopTests()
{
    _w.hideProgressBar();
    disconnect(_controller, SIGNAL(result(QString)), &_w, SLOT(slotResult(QString)));
    disconnect(_controller, SIGNAL(finished()), _hostInfo, SLOT(slotStartInfoTest()));
    disconnect(_hostInfo, SIGNAL(finished()), this, SLOT(_slotTestsFinished()));
    _controller->stopTests();
    _w.slotTestFinished(false);
}

void QSpeedTest::_slotTestsFinished()
{
    _w.hideProgressBar();
    disconnect(_controller, SIGNAL(result(QString)), &_w, SLOT(slotResult(QString)));
    disconnect(_controller, SIGNAL(finished()), _hostInfo, SLOT(slotStartInfoTest()));
    disconnect(_hostInfo, SIGNAL(finished()), this, SLOT(_slotTestsFinished()));
    _results.setTestDuration((_timer.elapsed() * 1.0) / 1000);
    _w.slotResult("\n" + _results.summary());
    _w.slotTestFinished(true);
}
