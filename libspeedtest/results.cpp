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

#include "results.h"
#include <QtCore/QObject>
#include <QFile>
#include <QtCore/QTextStream>

void Results::reset()
{
    quint8 size = _pingGroups.size();

    for(int i = 0; i < size; i++) _pingGroups[i].reset();

    size = _downloadGroups.size();

    for(int i = 0; i < size; i++) _downloadGroups[i].reset();
}

QString Results::plainText() const
{
    QString plainText;

    plainText += QObject::trUtf8(    "Report created by:      %1 %2\n").arg(_programName).arg(_projectVersion);
    plainText += QObject::trUtf8(    "Test date and time:     %1\n").arg(_testDateTime);
    plainText += QObject::trUtf8(    "Hostlist used:          %1 %2\n").arg(_hostlistVersion).arg(_hostlistComment);
    plainText += QObject::trUtf8(    "Hostlist contact URL:   %1\n").arg(_hostlistContactUrl);
    plainText += QObject::trUtf8(    "Host OS & no. of CPUs:  %1 / %2\n").arg(_hostOS).arg(_cpuCores);
    plainText += QObject::trUtf8(    "BBRAS:                  %1\n").arg(_bbras);
    plainText += QObject::trUtf8(    "WAN IP:                 %1\n").arg(_ip);
    plainText += QObject::trUtf8(    "ISP Name & Network:     %1 - %2\n").arg(_isp).arg("[coming soon!]");//.arg(_ispNetwork);
    plainText += QObject::trUtf8(    "Network advertised via: %1\n").arg("[coming soon!]");//.arg(_ispNetworkAdvertisers.join(", "));
    plainText += QObject::trUtf8(    "Test mode:              %1\n").arg(_testModeString());
    plainText += QObject::trUtf8(    "Total test duration:    %1 sec\n").arg(_testDuration);

    if(_testMode & TestMode::Ping)
    {
        plainText += QObject::trUtf8("Pings per host:         %1\n").arg(_pingsPerHost);
        plainText += QObject::trUtf8("Ping threads:           %1\n").arg(_pingThreads);
        plainText += QObject::trUtf8("Ping hosts alive:       %1 / %2\n").arg(_pingHostsAlive()).arg(_pingHostsTotal());
        plainText += QObject::trUtf8("Avg. latency:           %1\n").arg(_pingAverageString());
    }

    if(_testMode & TestMode::Download)
    {
        plainText += QObject::trUtf8("Max. bandwidth:         %1 or %2\n").arg(_maxBandwidth(SpeedUnit::Mbps)).arg(_maxBandwidth(SpeedUnit::MBps));
        plainText += QObject::trUtf8("Each download ran for:  %1 sec\n").arg(_downloadTestSecs);
    }

    return plainText;
}

QString Results::bbCode() const
{
    QString bbCode;
    quint8 pingGroupsSize = _pingGroups.size();
    quint8 downloadGroupsSize = _downloadGroups.size();

    bbCode += QObject::trUtf8(        "[table=head] | [center][b]%1 %2 Report - [url=%3]Homepage[/url][/b][/center] | [right]Test timestamp[/right] | [center][b]%4[/b][/center] |\n").arg(_programName).arg(_projectVersion).arg(_projectUrl).arg(_testDateTime);
    bbCode += QObject::trUtf8(        "[right]Hostlist used[/right] | [center]%1 [url=%2]%3[/url][/center] | ").arg(_hostlistVersion).arg(_hostlistContactUrl).arg(_hostlistComment);
    bbCode += QObject::trUtf8(        "[right]Host OS & no. of CPUs[/right] | [center]%1 - %2 CPU cores[/center] |\n").arg(_hostOS).arg(_cpuCores);
    bbCode += QObject::trUtf8(        "[right]BBRAS[/right] | [center]%1[/center] | ").arg(_bbras);
    bbCode += QObject::trUtf8(        "[right]WAN IP[/right] | [center]%1[/center] |\n").arg(_ipCensored);
    bbCode += QObject::trUtf8(        "[right]ISP name & network[/right] | [center]%1 - %2[/center] | ").arg(_isp).arg("[coming soon!]");//.arg(_ispNetwork);
    bbCode += QObject::trUtf8(        "[right]Network advertised via[/right] | [center]%1[/center] |\n").arg("[coming soon!]");//.arg(_ispNetworkAdvertisers.join(", "));
    bbCode += QObject::trUtf8(        "[right]Test mode[/right] | [center]%1[/center] | ").arg(_testModeString());
    bbCode += QObject::trUtf8(        "[right]Total test duration[/right] | [center]%1 sec[/center] |\n").arg(_testDuration);

    if(_testMode & TestMode::Ping)
    {
        bbCode += QObject::trUtf8(    "[right]Pings per host[/right] | [center]%1[/center] | ").arg(_pingsPerHost);
        bbCode += QObject::trUtf8(    "[right]Ping threads[/right] | [center]%1[/center] |\n").arg(_pingThreads);
        bbCode += QObject::trUtf8(    "[right]Hosts alive[/right] | [center]%1 / %2[/center] | ").arg(_pingHostsAlive()).arg(_pingHostsTotal());
        bbCode += QObject::trUtf8(    "[right][b][size=3]Avg. latency[/size][/b][/right] | [center][b][size=3]%1[/size][/b][/center] |\n").arg(_pingAverageString());
    }

    if(_testMode & TestMode::Download)
    {
        bbCode += QObject::trUtf8(    "[right]Downloads ran for[/right] | [center]%1 sec each[/center] | ").arg(_downloadTestSecs);
        bbCode += QObject::trUtf8(    "[right][b][size=3]Max. bandwidth[/size][/b][/right] | [center][b][size=3]%1 or %2[/size][/b][/center] |\n").arg(_maxBandwidth(SpeedUnit::Mbps)).arg(_maxBandwidth(SpeedUnit::MBps));
    }

    bbCode +=                         "[/table]\n\n";

    if(_testMode & TestMode::Ping)
    {
        bbCode += QString(            "[img]%1[/img] ").arg(_googleChartPingGroupsUrl());
    }

    if(_testMode & TestMode::Download)
    {
        for(int i = 0; i < downloadGroupsSize; i++)
        {
            bbCode += QString(        "[img]%1[/img] ").arg(_googleChartDownloadGroupUrl(i));
        }
    }

    if((_testMode & TestMode::Ping) || (_testMode & TestMode::Download))
    {
        bbCode +=                     "\n";
    }

    if(_testMode & TestMode::Ping)
    {
        bbCode +=                     "\n";
        bbCode += QObject::trUtf8(    "[b]Detailed ping results[/b]\n");
        bbCode +=                     "[spoiler]\n";

        for(int i = 0; i < pingGroupsSize; i++)
        {
            bbCode += QString(        "[b]%1[/b]\n").arg(_pingGroups[i].name());
            bbCode +=                 "[spoiler]\n";
            bbCode += QObject::trUtf8("[table=head][center]Host[/center] | [center]Ping average[/center] | Packet loss | Jitter | Rank\n");

            quint16 size = _pingGroups[i]._hosts.size();

            for(int j = 0; j < size; j++)
            {
                bbCode += QString(    "%1 | [right]%2[/right] | [right]%3[/right] | [right]%4[/right] | [center]%5[/center] |\n").arg(_pingGroups[i]._hosts[j].name()).arg(_pingGroups[i]._hosts[j].rttString()).arg(_pingGroups[i]._hosts[j].packetLossString()).arg(_pingGroups[i]._hosts[j].jitterString()).arg(_pingGroups[i]._hosts[j].rank());
            }

            bbCode += QObject::trUtf8("[b]Group sum[/b] | [right][b]%1[/b][/right] |\n").arg(_pingGroups[i].rttSumString());
            bbCode += QObject::trUtf8("[b]Group average[/b] | [right][b]%1[/b][/right] | [right][b]%2[/b][/right] | | [center][b]%3[/b][/center]\n").arg(_pingGroups[i].rttAverageString()).arg(_pingGroups[i].packetLossAverageString()).arg(_pingGroups[i].rank());
            bbCode +=                 "[/table]\n"
                                      "[/spoiler]\n";
        }

        bbCode +=                     "[/spoiler]\n";
    }

    return bbCode;
}

QString Results::html() const
{
    QString html;
    quint8 pingGroupsSize = _pingGroups.size();
    quint8 downloadGroupsSize = _downloadGroups.size();

    html +=                         "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                                    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                                    "    <head>\n"
                                    "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
    html += QObject::trUtf8(        "        <title>%1 %2 report - %3</title>\n").arg(_programName).arg(_projectVersion).arg(_testDateTime);
    html +=                         "    </head>\n"
                                    "    <body>\n"
                                    "        <table border=\"1\" cellpadding=\"4\">\n";
    html += QObject::trUtf8(        "            <tr><td>&nbsp;</td><td align=\"center\" style=\"font-size:130%;\"><b>%1 %2 report - <a href=\"%3\">Homepage</a></b></td><td align=\"right\" style=\"font-size:130%;\"><b>Test timestamp</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%4</b></td></tr>\n").arg(_programName).arg(_projectVersion).arg(_projectUrl).arg(_testDateTime);
    html += QObject::trUtf8(        "            <tr><td align=\"right\">Hostlist used</td><td align=\"center\">%1 <a href=\"%2\">%3</a></td><td align=\"right\">Host OS & no. of CPUs</td><td align=\"center\">%4 - %5 CPU cores</td></tr>\n").arg(_hostlistVersion).arg(_hostlistContactUrl).arg(_hostlistComment).arg(_hostOS).arg(_cpuCores);
    html += QObject::trUtf8(        "            <tr><td align=\"right\">BBRAS</td><td align=\"center\">%1</td><td align=\"right\">WAN IP</td><td align=\"center\">%2</td></tr>\n").arg(_bbras).arg(_ipCensored);
    html += QObject::trUtf8(        "            <tr><td align=\"right\">ISP name & network</td><td align=\"center\">%1 - %2</td><td align=\"right\">Network advertised via</td><td align=\"center\">%3</td></tr>\n").arg(_isp).arg("[coming soon!]").arg("[coming soon!]");
    html += QObject::trUtf8(        "            <tr><td align=\"right\">Test mode</td><td align=\"center\">%1</td><td align=\"right\">Total test duration</td><td align=\"center\">%2 sec</td></tr>\n").arg(_testModeString()).arg(_testDuration);

    if(_testMode & TestMode::Ping)
    {
        html += QObject::trUtf8(    "            <tr><td align=\"right\">Pings per host</td><td align=\"center\">%1</td><td align=\"right\">Ping threads</td><td align=\"center\">%2</td></tr>\n").arg(_pingsPerHost).arg(_pingThreads);
        html += QObject::trUtf8(    "            <tr><td align=\"right\">Hosts alive</td><td align=\"center\">%1 / %2</td><td align=\"right\" style=\"font-size:130%;\"><b>Avg. latency</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%3</b></td></tr>\n").arg(_pingHostsAlive()).arg(_pingHostsTotal()).arg(_pingAverageString());
    }

    if(_testMode & TestMode::Download)
    {
        html += QObject::trUtf8(    "            <tr><td align=\"right\">Downloads ran for</td><td align=\"center\">%1 sec each</td><td align=\"right\" style=\"font-size:130%;\"><b>Max. bandwidth</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%2 or %3</b></td></tr>\n").arg(_downloadTestSecs).arg(_maxBandwidth(SpeedUnit::Mbps)).arg(_maxBandwidth(SpeedUnit::MBps));
    }

    html +=                         "        </table>\n";

    if((_testMode & TestMode::Ping) || (_testMode & TestMode::Download))
    {
        html +=                     "        <p>\n";

        if(_testMode & TestMode::Ping)
        {
            html += QString(        "            <img src=\"%1\" alt=\"Google Chart - Pings\" />\n").arg(_googleChartPingGroupsUrl());
        }

        if(_testMode & TestMode::Download)
        {
            for(int i = 0; i < downloadGroupsSize; i++)
            {
                html += QString(    "<img src=\"%1\" alt=\"Google Chart - Download speeds - Group %2\" />\n").arg(_googleChartDownloadGroupUrl(i)).arg(_downloadGroups[i].name());
            }
        }

        html +=                     "        </p>\n";
    }

    if(_testMode & TestMode::Ping)
    {
        html +=                     "        <p style=\"margin-bottom: 2px;\"><b>Detailed ping results</b></p>\n"
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
                                    "        <div id=\"GroupsDiv\" style=\"display: none; border: 1px solid black; padding: 5px; margin: 2px;\">\n";

        for(int i = 0; i < pingGroupsSize; i++)
        {
            html += QString(        "            <p style=\"margin-top: 2px; margin-bottom: 2px;\"><b>%1</b></p>\n").arg(_pingGroups[i].name());
            html +=                 "            <div>\n";
            html += QString(        "                <input type=\"button\" id=\"Group%2SpoilerButton\" value=\"Show\" style=\"width: 80px; height: 25px; font-size: 9pt;\" onclick=\"\n").arg(i);
            html += QString(        "                    if(document.getElementById('Group%2Div').style.display != '')\n").arg(i);
            html +=                 "                    {\n";
            html += QString(        "                        document.getElementById('Group%2Div').style.display = '';\n").arg(i);
            html +=                 "                        this.innerText = '';\n"
                                    "                        this.value = 'Hide';\n"
                                    "                    }\n"
                                    "                    else\n"
                                    "                    {\n";
            html += QString(        "                        document.getElementById('Group%2Div').style.display = 'none';\n").arg(i);
            html +=                 "                        this.innerText = '';\n"
                                    "                        this.value = 'Show';\n"
                                    "                    }\"/>\n"
                                    "            </div>\n";
            html += QString(        "            <div id=\"Group%2Div\" style=\"display: none; border: 1px solid black; padding: 5px; margin: 2px;\">\n").arg(i);
            html +=                 "                <table border=\"1\" cellpadding=\"4\">\n";
            html += QObject::trUtf8("                    <tr><td>Host</td><td>Average ping time</td><td>Packet loss</td><td align=\"center\">Jitter</td><td>Rank</td></tr>\n");

            quint16 size = _pingGroups[i]._hosts.size();

            for(int j = 0; j < size; j++)
            {
                html += QString(    "                    <tr><td><a href=\"http://%1\">%2</a></td><td align=\"right\">%3</td><td align=\"right\">%4</td><td align=\"right\">%5</td><td align=\"center\">%6</td></tr>\n").arg(_pingGroups[i]._hosts[j].url()).arg(_pingGroups[i]._hosts[j].name()).arg(_pingGroups[i]._hosts[j].rttString()).arg(_pingGroups[i]._hosts[j].packetLossString()).arg(_pingGroups[i]._hosts[j].jitterString()).arg(_pingGroups[i]._hosts[j].rank());
            }

            html += QObject::trUtf8("                    <tr><td><b>Group sum</b></td><td align=\"right\"><b>%1</b></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n").arg(_pingGroups[i].rttSumString());
            html += QObject::trUtf8("                    <tr><td><b>Group average</b></td><td align=\"right\"><b>%1</b></td><td align=\"right\"><b>%2</b></td><td>&nbsp;</td><td align=\"center\"><b>%3</b></td></tr>\n").arg(_pingGroups[i].rttAverageString()).arg(_pingGroups[i].packetLossAverageString()).arg(_pingGroups[i].rank());
            html +=                 "                </table>\n"
                                    "            </div>\n"
                                    "            <br/>\n";
        }

        html +=                     "        </div>\n";
    }

    html +=                         "        <p>\n"
                                    "            <a href=\"http://validator.w3.org\">\n"
                                    "                <img src=\"http://www.w3.org/Icons/valid-xhtml11\" alt=\"Valid XHTML 1.1\" height=\"31\" width=\"88\" />\n"
                                    "            </a>\n"
                                    "        </p>\n"
                                    "    </body>\n"
                                    "</html>\n";
    html.replace('&', "&amp;");
    html.replace("&amp;nbsp", "&nbsp");

    return html;
}

bool Results::saveReport(ReportFormat::Format format, const QString &filename) const
{
    QFile file(filename);
    QTextStream outStream(&file);
    QString content;

    outStream.setCodec("UTF-8");

    switch(format)
    {
    case ReportFormat::PlainText:
        content = plainText();
        break;
    case ReportFormat::BbCode:
        content = bbCode();
        break;
    case ReportFormat::Html:
        content = html();
        break;
    default:
        break;
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    outStream << content;
    file.close();

    return true;
}

QString Results::_testModeString() const
{
    switch(_testMode)
    {
        case TestMode::Info:
            return QObject::trUtf8("Info");

        case TestMode::Ping:
            return QObject::trUtf8("Ping");

        case TestMode::Download:
            return QObject::trUtf8("Download");

        case TestMode::All:
            return QObject::trUtf8("All tests");

        default:
            return QObject::trUtf8("Unknown mode");
    }
}

quint16 Results::_pingHostsAlive() const
{
    quint16 size = _pingGroups.size();
    quint16 sum = 0;

    for(int i = 0; i < size; i++)
    {
        sum += _pingGroups[i].hostsAlive();
    }

    return sum;
}

quint16 Results::_pingHostsTotal() const
{
    quint8 size = _pingGroups.size();
    quint16 sum = 0;

    for(int i = 0; i < size; i++)
    {
        sum += _pingGroups[i]._hosts.size();
    }

    return sum;
}

QString Results::_pingAverageString() const
{
    double sum = 0.0;
    double average = 0.0;
    quint8 groups = _pingGroups.size();
    quint16 alive = _pingHostsAlive();

    if(alive)
    {
        for(int i = 0; i < groups; i++)
        {
            quint16 groupSize = _pingGroups[i]._hosts.size();

            for(int j = 0; j < groupSize; j++)
            {
                if(_pingGroups[i]._hosts[j].isAlive())
                {
                    sum += _pingGroups[i]._hosts[j].rtt();
                }
            }
        }

        average = sum /alive;
    }

    return QString::number(average, 'f', 2) + " msec";
}

quint16 Results::_downloadHostsTotal() const
{
    quint8 size = _downloadGroups.size();
    quint16 sum = 0;

    for(int i = 0; i < size; i++)
    {
        sum += _downloadGroups[i]._hosts.size();
    }

    return sum;
}

QString Results::_maxBandwidth(SpeedUnit::Unit unit) const
{
    double max = 0.0, speed = 0.0;
    QString string = "0.00";
    quint8 size = _downloadGroups.size();

    if(size)
    {
        for(int i = 0; i < size; i++)
        {
            speed = _downloadGroups[i]._speedBpsParallel;

            if(speed > max)
            {
                max = speed;
            }
        }
    }

    switch(unit)
    {
    case SpeedUnit::bps:
        max *= 8.0;
        string = QString::number(max, 'f', 2) + " bps";
        break;
    case SpeedUnit::Bps:
        string = QString::number(max, 'f', 2) + " B/sec";
        break;
    case SpeedUnit::Kbps:
        max /= 128.0;    // Multiply by 8 to get bps, then divide by 1024 to get Kbps
        string = QString::number(max, 'f', 2) + " Kbps";
        break;
    case SpeedUnit::KBps:
        max /= 1024.0;
        string = QString::number(max, 'f', 2) + " KB/sec";
        break;
    case SpeedUnit::Mbps:
        max /= 131072.0;
        string = QString::number(max, 'f', 2) + " Mbps";
        break;
    case SpeedUnit::MBps:
        max /= 1048576.0;
        string = QString::number(max, 'f', 2) + " MB/sec";
        break;
    default:
        break;
    }

    return string;
}

QString Results::_googleChartPingGroupsUrl() const
{
    QString chartTitle = QObject::trUtf8("Average+ping|(msec)");
    QString url = QString("http://chart.apis.google.com/chart?chof=gif&chtt=%1&cht=bhs&chxt=x,y&chm=N++,000000,0,,11&chco=FF0000|CD8500|808080|00FF00|0000FF&chxl=1:").arg(chartTitle);
    quint16 chartWidth = 300;
    quint16 chartHeight = 60;
    quint8 size = _pingGroups.size();
    double max = 0.0;
    double ping;
    QString scalingPart;

    for(int i = size - 1; i >= 0; i--)    // When building a horizontal bars Google Charts URL, if you want the label names with a top-to-bottom order, you must insert them in the reverse order of their respective data!
    {
        url += QString("|%1").arg(_pingGroups[i].name());
        chartHeight += 30;
    }

    url += QString("&chs=%1x%2&chd=t:").arg(chartWidth).arg(chartHeight);

    if(size)
    {
        max = _pingGroups[0].rttAverage();
    }

    for(int i = 0; i < size; i++)
    {
        ping = _pingGroups[i].rttAverage();
        url += QString::number(ping, 'f', 2) + ',';

        if(ping > max)
        {
            max = ping;
        }
    }

    if(size)
    {
        url.chop(1);
    }

    if(max < 200)
    {
        scalingPart = "&chxr=0,0,280&chds=0,280";
    }
    else
    {
        scalingPart = QString("&chxr=0,0,%1&chds=0,%1").arg(QString::number(max + 80, 'f', 0));
    }

    url += scalingPart;

    return url.replace(' ', '+');
}

QString Results::_googleChartDownloadGroupUrl(quint8 index) const
{
    QString chartTitle = QObject::trUtf8("%1|(Mbps)").arg(_downloadGroups[index].name());
    QString url = QString("http://chart.apis.google.com/chart?chof=gif&chtt=%1&cht=bhs&chxt=x,y&chm=N++,000000,0,,11&chco=FF0000|CD8500|808080|00FF00|0000FF&chxl=1:").arg(chartTitle);
    quint16 chartWidth = 280;
    quint16 chartHeight = 50;
    quint8 size = _downloadGroups[index]._hosts.size();
    double max = 0.0;
    double speed;
    QString scalingPart;

    for(int i = size - 1; i >= 0; i--)    // When building a horizontal bars Google Charts URL, if you want the label names with a top-to-bottom order, you must insert them in the reverse order of their respective data!
    {
        url += QString("|%1").arg(_downloadGroups[index]._hosts[i].name());
        chartHeight += 30;
    }

    url += QObject::trUtf8("|Avg. serial");
    chartHeight += 30;
    url += QObject::trUtf8("|Parallel");
    chartHeight += 30;

    url += QString("&chs=%1x%2&chd=t:").arg(chartWidth).arg(chartHeight);

    if(size)
    {
        max = _downloadGroups[0].speedParallel(SpeedUnit::Mbps);
    }

    url += QString::number(_downloadGroups[index].speedParallel(SpeedUnit::Mbps), 'f', 2) + ',';
    url += QString::number(_downloadGroups[index].speedSerial(SpeedUnit::Mbps), 'f', 2) + ',';

    for(int i = 0; i < size; i++)
    {
        speed = _downloadGroups[index]._hosts[i].speed(SpeedUnit::Mbps);
        url += QString::number(speed, 'f', 2) + ',';

        if(speed > max)
        {
            max = speed;
        }
    }

    url.chop(1);

    if(max < 21)
    {
        scalingPart = "&chxr=0,0,25,2&chds=0,25";
    }
    else
    {
        scalingPart = QString("&chxr=0,0,%1,2&chds=0,%1").arg(QString::number(max + 4, 'f', 0));
    }

    url += scalingPart;

    return url.replace(' ', '+');
}
