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
#include "externs.h"
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrl>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QFile>
#include <QtCore/QDir>

QSpeedTestCli::QSpeedTestCli()
{
    PROGRAMNAME += "CLI";
    UPDATECHECKTIMEOUTSECS = UPDATECHECKTIMEOUTSECSDEFAULT;
    PINGSPERHOST = PINGSPERHOSTDEFAULT;
    PINGTHREADS = PINGTHREADSDEFAULT;
    PINGTIMEOUTSECS = PINGTIMEOUTSECSDEFAULT;
    DOWNLOADTESTSECS = DOWNLOADTESTSECSDEFAULT;
    HOSTLISTURL = HOSTLISTURLDEFAULT;
    _hostlist = NULL;
    _hostInfo = NULL;
    TESTMODE = TestMode::All;
    _results.setTestMode(TESTMODE);
    _htmlEnabled = true;
    _bbCodeEnabled = true;
}

QSpeedTestCli::~QSpeedTestCli()
{
    if(_hostlist)
    {
        delete _hostlist;
    }

    if(_hostInfo)
    {
        delete _hostInfo;
    }
}

void QSpeedTestCli::_slotStart()
{
    _parseArguments();
    _hostlist = new Hostlist(QSettings::IniFormat, QSettings::UserScope, PROJECTNAME, PROJECTNAME.toLower() + ".hostlist", _results, this);
    _hostInfo = new HostInfo(_results, this);    // Starts some partial host info extraction in the background
    _controller = new TestController(_results, this);
    connect(_hostlist, SIGNAL(message(QString)), this, SLOT(_slotLog(QString)));
    connect(_controller, SIGNAL(result(QString)), this, SLOT(_slotResult(QString)));
    _checkForProgramUpdates();

    if(!_hostlist->initOk())
    {
        _slotLog(trUtf8("Unsuccessful initialization of the target list, %1 will now exit").arg(PROGRAMNAME));

        exit(10);
    }

    _slotResult("\n\n");
    _startTests();
}

void QSpeedTestCli::_parseArguments()
{
    QStringList args(qApp->arguments());
    int count = args.count();
    QString arg;
    QString argNext;
    QString message;
    int number;

    if(count == 1)
    {
        _slotLog(trUtf8("No parameters specified, running with defaults:"));
        _slotLog(trUtf8("Test mode:        All tests"));
        _slotLog(trUtf8("Pings per host:   %1").arg(PINGSPERHOST));
        _slotLog(trUtf8("Ping threads:     %1").arg(PINGTHREADS));
        _slotLog(trUtf8("Each download:    %1 %2").arg(DOWNLOADTESTSECS).arg(trUtf8("sec")));
        _slotLog(trUtf8("HTML output:      Enabled"));
        _slotLog(trUtf8("bbCode output:    Enabled"));

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
            message =  QString("%1 %2 %3 %4 - %5\n\n").arg(PROGRAMNAME).arg(PROJECTVERSION).arg(trUtf8("by")).arg(PROJECTAUTHOR).arg(PROJECTURL);
            message += trUtf8("    Usage: %1 [PARAMETERS]").arg(args[0]) + "\n\n";
            message += trUtf8("    [PARAMETERS] are optional and can be any from the following") + "\n\n";
            message +=        "--help, -h\n";
            message += trUtf8("    Prints program usage and exits") + "\n\n";
            message +=        "--version, -V\n";
            message += trUtf8("    Prints version info and exits") + "\n\n";
            message +=        "--mode MODE, -m MODE\n";
            message += trUtf8("    where MODE can be any one from (without the double quotes)") + '\n';
            message += trUtf8("    \"info\": will only print some host system and ISP related information") + '\n';
            message += trUtf8("    \"ping\": will only perform a ping test") + '\n';
            message += trUtf8("    \"speed\": will only perform a download speed test") + '\n';
            message += trUtf8("    \"all\": will perform all tests in the above order") + "\n\n";
            message +=        "--pings NUMBER, -p NUMBER\n";
            message += trUtf8("    where NUMBER can be any integer between 1 and 100, default is 4") + '\n';
            message += trUtf8("    During a ping test, each target will be pinged this many times") + "\n\n";
            message +=        "--threads NUMBER, -t NUMBER\n";
            message += trUtf8("    where NUMBER can be any integer between 1 and 8, default is 4") + '\n';
            message += trUtf8("    During a ping test, simultaneous pinging of this many targets") + "\n\n";
            message +=        "--downloadtime NUMBER, -d NUMBER\n";
            message += trUtf8("    where NUMBER can be any integer between 5 and 30, default is 10") + '\n';
            message += trUtf8("    During download test, each download will run for this many seconds") + "\n\n";
            message +=        "--nohtml, -nh\n";
            message += trUtf8("    Disables HTML file output in the current directory") + "\n\n";
            message +=        "--novb, -nv\n";
            message += trUtf8("    Disables vBulletin code file output in the current directory");
            qDebug() << qPrintable(message);

            exit(0);
        }

        if(arg == "--version" || arg == "-v")
        {
            qDebug() << qPrintable(PROGRAMNAME + ' ' + PROJECTVERSION);

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
                TESTMODE = TestMode::Info;
                _results.setTestMode(TESTMODE);
                i++;
                continue;
            }

            if(argNext == "ping")
            {
                TESTMODE = TestMode::Ping;
                _results.setTestMode(TESTMODE);
                i++;
                continue;
            }

            if(argNext == "speed")
            {
                TESTMODE = TestMode::Download;
                _results.setTestMode(TESTMODE);
                i++;
                continue;
            }

            if(argNext == "all")
            {
                TESTMODE = TestMode::All;
                _results.setTestMode(TESTMODE);
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

            PINGSPERHOST = number;
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

            PINGTHREADS = number;
            i++;
            continue;
        }

        if(arg == "--downloadtime" || arg == "-d")
        {
            if(argNext.isEmpty())
            {
                qDebug() << qPrintable(trUtf8("Error #7: No value specified after --downloadtime (-d) switch"));

                exit(7);
            }

            if((!(number = argNext.toInt())) || number < 5 || number > 30)
            {
                qDebug() << qPrintable(trUtf8("Error #8: Value \"%1\" after --downloadtime (-d) switch not an integer between 5 and 30").arg(argNext));

                exit(8);
            }

            DOWNLOADTESTSECS = number;
            i++;
            continue;
        }

        if(arg == "--nohtml" || arg == "-nh")
        {
            _htmlEnabled = false;
            continue;
        }

        if(arg == "--novb" || arg == "-nv")
        {
            _bbCodeEnabled = false;
            continue;
        }

        qDebug() << qPrintable(trUtf8("Error #9: Unknown switch or parameter \"%1\"").arg(arg));

        exit(9);
    }

    // if execution flow reaches this point, all arguments were valid
    _slotLog(trUtf8("Custom parameters specified:"));
    _slotLog(trUtf8("Test mode:        %1").arg(_results.testModeString()));
    _slotLog(trUtf8("Pings per host:   %1").arg(PINGSPERHOST));
    _slotLog(trUtf8("Ping threads:     %1").arg(PINGTHREADS));
    _slotLog(trUtf8("Each download:    %1 %2").arg(DOWNLOADTESTSECS).arg(trUtf8("sec")));
    _slotLog(trUtf8("HTML output:      %1").arg(_htmlEnabled? trUtf8("Yes") : trUtf8("No")));
    _slotLog(trUtf8("bbCode output     %1").arg(_bbCodeEnabled? trUtf8("Yes") : trUtf8("No")));
}

void QSpeedTestCli::_checkForProgramUpdates()
{
    QNetworkAccessManager manager;
    QNetworkReply *download;
    QEventLoop loop;
    int remoteVersion;

    _slotLog(trUtf8("Checking online for an updated version of %1").arg(PROGRAMNAME));
    QTimer::singleShot(UPDATECHECKTIMEOUTSECS *1000, &loop, SLOT(quit()));
    download = manager.get(QNetworkRequest(QUrl(PROJECTVERSIONURL)));    // according to Qt documentation, manager will be set as the parent of download, so manual deletion of download should not be necessary
    connect(download, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(download->isRunning() || download->error())
    {
        download->abort();
        download->deleteLater();
        _slotLog(trUtf8("Update site unreachable"));
        return;
    }

    if((remoteVersion = download->readLine().mid(1).toInt()) > PROJECTVERSION.mid(1).toInt())
    {
        _slotLog(trUtf8("%1 update available, remote version: r%2").arg(PROGRAMNAME).arg(remoteVersion));
        _slotLog(trUtf8("You can find the new version at %1").arg(PROJECTURL));
    }
    else
    {
        _slotLog(trUtf8("You are using the latest version of %1").arg(PROGRAMNAME));
    }
}

void QSpeedTestCli::_startTests()
{
    _timer.start();
    _results.reset();
     connect(_controller, SIGNAL(finished()), _hostInfo, SLOT(slotStartInfoTest()));
     connect(_hostInfo, SIGNAL(finished()), this, SLOT(_slotTestsFinished()));
     _hostInfo->startDetection();

    switch(TESTMODE)
    {
    case TestMode::Info:
        _hostInfo->slotStartInfoTest();
        break;

    case TestMode::Ping:

    case TestMode::All:
        _controller->startPingTest();
        break;

    case TestMode::Download:
        _controller->startDownloadTest();

    default:
        break;
    }
}

void QSpeedTestCli::_slotTestsFinished()
{
    disconnect(_controller, SIGNAL(finished()), _hostInfo, SLOT(slotStartInfoTest()));
    disconnect(_hostInfo, SIGNAL(finished()), this, SLOT(_slotTestsFinished()));
    _results.setTestDuration((_timer.elapsed() * 1.0) / 1000);
    _slotResult("\n" + _results.summary());
    _saveReports();
    qApp->quit();
}

void QSpeedTestCli::_saveReports()
{
    QFile file;
    QString fileName;
    QTextStream outStream;

    if(_htmlEnabled)
    {
        fileName = QDir::currentPath() + QString("/%1_%2%3_%4.html").arg(_results.programName()).arg(_results.testDate()).arg(_results.testTime()).arg(_results.ipCensored());
        fileName = QDir::toNativeSeparators(fileName);

        if(!_results.saveReport(ReportFormat::Html, fileName))
        {
            _slotLog(trUtf8("Error writing HTML report %1").arg(fileName));

            return;
        }

        _slotLog(trUtf8("HTML report saved to %1").arg(fileName));

/*
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            _slotLog(trUtf8("Error writing HTML report %1").arg(fileName));

            return;
        }

        outStream << _results.html();
        file.close();
        _slotLog(trUtf8("HTML report saved to %1").arg(fileName));
*/
    }

    if(_bbCodeEnabled)
    {
        fileName = QDir::currentPath() + QString("/%1_%2%3_%4.bb.txt").arg(_results.programName()).arg(_results.testDate()).arg(_results.testTime()).arg(_results.ipCensored());
        fileName = QDir::toNativeSeparators(fileName);

        if(!_results.saveReport(ReportFormat::BbCode, fileName))
        {
            _slotLog(trUtf8("Error writing bbCode report %1").arg(fileName));

            return;
        }

        _slotLog(trUtf8("bbCode report saved to %1").arg(fileName));



/*
        file.setFileName(fileName);
        outStream.setDevice(&file);
        outStream.setCodec("UTF-8");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            _slotLog(trUtf8("Error writing bbCode report %1").arg(fileName));

            return;
        }

        outStream << _results.bbCode();
        file.close();
        _slotLog(trUtf8("bbCode report saved to %1").arg(fileName));
*/
    }
}
