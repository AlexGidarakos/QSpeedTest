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

QString Results::testModeString() const
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

QString Results::summary() const
{
    QString summary;

    summary += QString("%1 %2 %3\n").arg(QObject::trUtf8("Report created by:").leftJustified(25, ' ', true)).arg(_programName).arg(_projectVersion);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("Test date and time:").leftJustified(25, ' ', true)).arg(_testDateTime);
    summary += QString("%1 %2 %3\n").arg(QObject::trUtf8("Hostlist version:").leftJustified(25, ' ', true)).arg(_hostlistVersion).arg(_hostlistComment);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("Hostlist contact URL:").leftJustified(25, ' ', true)).arg(_hostlistContactUrl);
    summary += QString("%1 %2 / %3 %4\n").arg(QObject::trUtf8("Host OS & no. of CPUs:").leftJustified(25, ' ', true)).arg(_hostOS).arg(_cpuCores).arg(QObject::trUtf8("CPU cores"));
    summary += QString("%1 %2\n").arg(QObject::trUtf8("BBRAS:").leftJustified(25, ' ', true)).arg(_bbras);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("WAN IP:").leftJustified(25, ' ', true)).arg(_ip);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("ISP name:").leftJustified(25, ' ', true)).arg(_isp);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("ISP network:").leftJustified(25, ' ', true)).arg(_ispNetwork);
    summary += QString("%1 %2\n").arg(QObject::trUtf8("Advertised via:").leftJustified(25, ' ', true)).arg("[coming soon!]");//.arg(_ispNetworkAdvertisers.join(", "));
    summary += QString("%1 %2\n").arg(QObject::trUtf8("Test mode:").leftJustified(25, ' ', true)).arg(testModeString());
    summary += QString("%1 %2 %3\n").arg(QObject::trUtf8("Total test duration:").leftJustified(25, ' ', true)).arg(_testDuration).arg(QObject::trUtf8("sec"));

    if(_testMode & TestMode::Ping)
    {
        summary += QString("%1 %2\n").arg(QObject::trUtf8("Pings per host:").leftJustified(25, ' ', true)).arg(_pingsPerHost);
        summary += QString("%1 %2\n").arg(QObject::trUtf8("Ping threads:").leftJustified(25, ' ', true)).arg(_pingThreads);
        summary += QString("%1 %2 / %3\n").arg(QObject::trUtf8("Hosts alive:").leftJustified(25, ' ', true)).arg(_pingHostsAlive()).arg(_pingHostsTotal());
        summary += QString("%1 %2 %3\n").arg(QObject::trUtf8("Avg. latency:").leftJustified(25, ' ', true)).arg(_pingAverage()).arg(QObject::trUtf8("msec"));
    }

    if(_testMode & TestMode::Download)
    {
        summary += QString("%1 %2 %3 %4\n").arg(QObject::trUtf8("Max. speed:").leftJustified(25, ' ', true)).arg(_maxBandwidth(SpeedUnit::Mbps)).arg(QObject::trUtf8("or")).arg(_maxBandwidth(SpeedUnit::MBps));
        summary += QString("%1 %2 %3\n").arg(QObject::trUtf8("Downloads ran for:").leftJustified(25, ' ', true)).arg(_downloadTestSecs).arg(QObject::trUtf8("sec each"));
    }

    return summary;
}

QString Results::plainText() const
{
    quint8 pingGroupsSize = _pingGroups.size();
    QString plainText;

    plainText += QString("%1 %2 %3\n").arg(QObject::trUtf8("Report created by:").leftJustified(25, ' ', true)).arg(_programName).arg(_projectVersion);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("Test date and time:").leftJustified(25, ' ', true)).arg(_testDateTime);
    plainText += QString("%1 %2 %3\n").arg(QObject::trUtf8("Hostlist version:").leftJustified(25, ' ', true)).arg(_hostlistVersion).arg(_hostlistComment);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("Hostlist contact URL:").leftJustified(25, ' ', true)).arg(_hostlistContactUrl);
    plainText += QString("%1 %2 / %3 %4\n").arg(QObject::trUtf8("Host OS & no. of CPUs:").leftJustified(25, ' ', true)).arg(_hostOS).arg(_cpuCores).arg(QObject::trUtf8("CPU cores"));
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("BBRAS:").leftJustified(25, ' ', true)).arg(_bbras);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("WAN IP:").leftJustified(25, ' ', true)).arg(_ipCensored);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("ISP name:").leftJustified(25, ' ', true)).arg(_isp);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("ISP network:").leftJustified(25, ' ', true)).arg(_ispNetwork);
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("Advertised via:").leftJustified(25, ' ', true)).arg("[coming soon!]");//.arg(_ispNetworkAdvertisers.join(", "));
    plainText += QString("%1 %2\n").arg(QObject::trUtf8("Test mode:").leftJustified(25, ' ', true)).arg(testModeString());
    plainText += QString("%1 %2 %3\n").arg(QObject::trUtf8("Total test duration:").leftJustified(25, ' ', true)).arg(_testDuration).arg(QObject::trUtf8("sec"));

    if(_testMode & TestMode::Ping)
    {
        plainText += QString("%1 %2\n").arg(QObject::trUtf8("Pings per host:").leftJustified(25, ' ', true)).arg(_pingsPerHost);
        plainText += QString("%1 %2\n").arg(QObject::trUtf8("Ping threads:").leftJustified(25, ' ', true)).arg(_pingThreads);
        plainText += QString("%1 %2 / %3\n").arg(QObject::trUtf8("Hosts alive:").leftJustified(25, ' ', true)).arg(_pingHostsAlive()).arg(_pingHostsTotal());
        plainText += QString("%1 %2 %3\n").arg(QObject::trUtf8("Avg. latency:").leftJustified(25, ' ', true)).arg(_pingAverage()).arg(QObject::trUtf8("msec"));
    }

    if(_testMode & TestMode::Download)
    {
        plainText += QString("%1 %2 %3 %4\n").arg(QObject::trUtf8("Max. speed:").leftJustified(25, ' ', true)).arg(_maxBandwidth(SpeedUnit::Mbps)).arg(QObject::trUtf8("or")).arg(_maxBandwidth(SpeedUnit::MBps));
        plainText += QString("%1 %2 %3\n").arg(QObject::trUtf8("Downloads ran for:").leftJustified(25, ' ', true)).arg(_downloadTestSecs).arg(QObject::trUtf8("sec each"));
    }

    if(_testMode & TestMode::Ping)
    {
        plainText += "\n\n";
        plainText += QObject::trUtf8("Detailed ping results") + '\n';

        for(int i = 0; i < pingGroupsSize; i++)
        {
            plainText += _pingGroups[i].name().leftJustified(27, ' ', true) + "    " + QObject::trUtf8("Avg. ping").rightJustified(11, ' ', true) + "    " + QObject::trUtf8("Pckt loss").rightJustified(9, ' ', true) + "    " + QString("Jitter").rightJustified(12, ' ', true) + "    " + QObject::trUtf8("Rank").rightJustified(4, ' ', true) + '\n';
            plainText += "-------------------------------------------------------------------------------\n";
            quint16 size = _pingGroups[i]._hosts.size();

            for(int j = 0; j < size; j++)
            {
                plainText +=  _pingGroups[i]._hosts[j].name().leftJustified(27, ' ', true) + "    " + _pingGroups[i]._hosts[j].rttString().rightJustified(11, ' ', true) + "    " + _pingGroups[i]._hosts[j].packetLossString().rightJustified(9, ' ', true) + "    " + _pingGroups[i]._hosts[j].jitterString().rightJustified(12, ' ', true) + "    " + _pingGroups[i]._hosts[j].rank().rightJustified(3, ' ', true) + '\n';
            }

            plainText += QObject::trUtf8("Group sum:").leftJustified(14, ' ', true) + ' ' + _pingGroups[i].rttSumString() + '\n';
            plainText += QObject::trUtf8("Group average:").leftJustified(14, ' ', true) + ' ' + _pingGroups[i].rttAverageString() + '\n';
            plainText += '\n';
        }
    }

    return plainText;
}

QString Results::bbCode() const
{
    QString bbCode;
    quint8 pingGroupsSize = _pingGroups.size();
    quint8 downloadGroupsSize = _downloadGroups.size();

    bbCode += QString(            "[table=head] | [center][b]%1 %2 %3 - [url=%4]%5[/url][/b][/center] | [right]%6[/right] | [b]%7[/b]\n").arg(QObject::trUtf8("Report by")).arg(_programName).arg(_projectVersion).arg(_projectUrl).arg(QObject::trUtf8("Homepage")).arg(QObject::trUtf8("Test timestamp")).arg(_testDateTime);
    bbCode += QString(            "[right]%1[/right] | [center]%2 [url=%3]%4[/url][/center] | ").arg(QObject::trUtf8("Hostlist version")).arg(_hostlistVersion).arg(_hostlistContactUrl).arg(_hostlistComment);
    bbCode += QString(            "[right]%1[/right] | [center]%2 - %3 %4[/center] |\n").arg(QObject::trUtf8("Host OS & no. of CPUs")).arg(_hostOS).arg(_cpuCores).arg(QObject::trUtf8("CPU cores"));
    bbCode += QString(            "[right]%1[/right] | [center]%2[/center] | ").arg(QObject::trUtf8("BBRAS")).arg(_bbras);
    bbCode += QString(            "[right]%1[/right] | [center]%2[/center] |\n").arg(QObject::trUtf8("WAN IP")).arg(_ipCensored);
    bbCode += QString(            "[right]%1[/right] | [center]%2 - %3[/center] | ").arg(QObject::trUtf8("ISP & network")).arg(_isp).arg(_ispNetwork);
    bbCode += QString(            "[right]%1[/right] | [center]%2[/center] |\n").arg(QObject::trUtf8("Advertised via")).arg("[coming soon!]");//.arg(_ispNetworkAdvertisers.join(", "));
    bbCode += QString(            "[right]%1[/right] | [center]%2[/center] | ").arg(QObject::trUtf8("Test mode")).arg(testModeString());
    bbCode += QString(            "[right]%1[/right] | [center]%2 %3[/center] |\n").arg(QObject::trUtf8("Total test duration")).arg(_testDuration).arg(QObject::trUtf8("sec"));

    if(_testMode & TestMode::Ping)
    {
        bbCode += QString(        "[right]%1[/right] | [center]%2[/center] | ").arg(QObject::trUtf8("Pings per host")).arg(_pingsPerHost);
        bbCode += QString(        "[right]%1[/right] | [center]%2[/center] |\n").arg(QObject::trUtf8("Ping threads")).arg(_pingThreads);
        bbCode += QString(        "[right]%1[/right] | [center]%2 / %3[/center] | ").arg(QObject::trUtf8("Hosts alive")).arg(_pingHostsAlive()).arg(_pingHostsTotal());
        bbCode += QString(        "[right][b][size=3]%1[/size][/b][/right] | [center][b][size=3]%2 %3[/size][/b][/center] |\n").arg(QObject::trUtf8("Avg. latency")).arg(_pingAverage()).arg(QObject::trUtf8("msec"));
    }

    if(_testMode & TestMode::Download)
    {
        bbCode += QString(        "[right]%1[/right] | [center]%2 %3[/center] | ").arg(QObject::trUtf8("Downloads ran for")).arg(_downloadTestSecs).arg(QObject::trUtf8("sec each"));
        bbCode += QString(        "[right][b][size=3]%1[/size][/b][/right] | [center][b][size=3]%2 %3 %4[/size][/b][/center] |\n").arg(QObject::trUtf8("Max. speed")).arg(_maxBandwidth(SpeedUnit::Mbps)).arg(QObject::trUtf8("or")).arg(_maxBandwidth(SpeedUnit::MBps));
    }

    bbCode +=                     "[/table]\n\n";

    if(_testMode & TestMode::Ping)
    {
        bbCode += QString(        "[img]%1[/img] ").arg(_googleChartPingGroupsUrl());
    }

    if(_testMode & TestMode::Download)
    {
        for(int i = 0; i < downloadGroupsSize; i++)
        {
            bbCode += QString(    "[img]%1[/img] ").arg(_googleChartDownloadGroupUrl(i));
        }
    }

    if((_testMode & TestMode::Ping) || (_testMode & TestMode::Download))
    {
        bbCode +=                 '\n';
    }

    if(_testMode & TestMode::Ping)
    {
        bbCode +=                 '\n';
        bbCode += QString(        "[b]%1[/b]\n").arg(QObject::trUtf8("Detailed ping results"));
        bbCode +=                 "[spoiler]\n";

        for(int i = 0; i < pingGroupsSize; i++)
        {
            bbCode += QString(    "[b]%1[/b]\n").arg(_pingGroups[i].name());
            bbCode +=             "[spoiler]\n";
            bbCode += QString(    "[table=head][center]%1[/center] | [center]%2[/center] | [center]%3[/center] | [center]%4[/center] | %5\n").arg(QObject::trUtf8("Host")).arg(QObject::trUtf8("Ping average")).arg(QObject::trUtf8("Packet loss")).arg(QObject::trUtf8("Jitter")).arg(QObject::trUtf8("Rank"));

            quint16 size = _pingGroups[i]._hosts.size();

            for(int j = 0; j < size; j++)
            {
                bbCode += QString("%1 | [right]%2[/right] | [right]%3[/right] | [right]%4[/right] | [center]%5[/center] |\n").arg(_pingGroups[i]._hosts[j].name()).arg(_pingGroups[i]._hosts[j].rttString()).arg(_pingGroups[i]._hosts[j].packetLossString()).arg(_pingGroups[i]._hosts[j].jitterString()).arg(_pingGroups[i]._hosts[j].rank());
            }

            bbCode += QString(    "[b]%1[/b] | [right][b]%2[/b][/right] |\n").arg(QObject::trUtf8("Group sum")).arg(_pingGroups[i].rttSumString());
            bbCode += QString(    "[b]%1[/b] | [right][b]%2[/b][/right] | [right][b]%3[/b][/right] | | [center][b]%4[/b][/center]\n").arg(QObject::trUtf8("Group average")).arg(_pingGroups[i].rttAverageString()).arg(_pingGroups[i].packetLossAverageString()).arg(_pingGroups[i].rank());
            bbCode +=             "[/table]\n"
                                  "[/spoiler]\n";
        }

        bbCode +=                 "[/spoiler]\n";
    }

    return bbCode;
}

QString Results::html() const
{
    QString html;
    quint8 pingGroupsSize = _pingGroups.size();
    quint8 downloadGroupsSize = _downloadGroups.size();

    html +=                     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                                "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                                "    <head>\n"
                                "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
    html += QString(            "        <title>%1 %2 %3 - %4 - %5</title>\n").arg(QObject::trUtf8("Report by")).arg(_programName).arg(_projectVersion).arg(_testDateTime).arg(_ipCensored);
    html +=                     "    </head>\n"
                                "    <body>\n"
                                "        <table border=\"1\" cellpadding=\"4\">\n";
    html += QString(            "            <tr><td>&nbsp;</td><td align=\"center\" style=\"font-size:130%;\"><b>%1 %2 %3 - <a href=\"%4\">%5</a></b></td><td align=\"right\" style=\"font-size:130%;\"><b>%6</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%7</b></td></tr>\n").arg(QObject::trUtf8("Report by")).arg(_programName).arg(_projectVersion).arg(_projectUrl).arg(QObject::trUtf8("Homepage")).arg(QObject::trUtf8("Test timestamp")).arg(_testDateTime);
    html += QString(            "            <tr><td align=\"right\">%1</td><td align=\"center\">%2 <a href=\"%3\">%4</a></td><td align=\"right\">%5</td><td align=\"center\">%6 - %7 %8</td></tr>\n").arg(QObject::trUtf8("Hostlist version")).arg(_hostlistVersion).arg(_hostlistContactUrl).arg(_hostlistComment).arg(QObject::trUtf8("Host OS & no. of CPUs")).arg(_hostOS).arg(_cpuCores).arg(QObject::trUtf8("CPU cores"));
    html += QString(            "            <tr><td align=\"right\">%1</td><td align=\"center\">%2</td><td align=\"right\">%3</td><td align=\"center\">%4</td></tr>\n").arg(QObject::trUtf8("BBRAS")).arg(_bbras).arg(QObject::trUtf8("WAN IP")).arg(_ipCensored);
    html += QString(            "            <tr><td align=\"right\">%1</td><td align=\"center\">%2 - %3</td><td align=\"right\">%4</td><td align=\"center\">%5</td></tr>\n").arg(QObject::trUtf8("ISP & network")).arg(_isp).arg(_ispNetwork).arg(QObject::trUtf8("Advertised via")).arg("[coming soon]");
    html += QString(            "            <tr><td align=\"right\">%1</td><td align=\"center\">%2</td><td align=\"right\">%3</td><td align=\"center\">%4 %5</td></tr>\n").arg(QObject::trUtf8("Test mode")).arg(testModeString()).arg(QObject::trUtf8("Total test duration")).arg(_testDuration).arg(QObject::trUtf8("sec"));

    if(_testMode & TestMode::Ping)
    {
        html += QString(        "            <tr><td align=\"right\">%1</td><td align=\"center\">%2</td><td align=\"right\">%3</td><td align=\"center\">%4</td></tr>\n").arg(QObject::trUtf8("Pings per host")).arg(_pingsPerHost).arg(QObject::trUtf8("Ping threads")).arg(_pingThreads);
        html += QString(        "            <tr><td align=\"right\">%1</td><td align=\"center\">%2 / %3</td><td align=\"right\" style=\"font-size:130%;\"><b>%4</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%5 %6</b></td></tr>\n").arg(QObject::trUtf8("Hosts alive")).arg(_pingHostsAlive()).arg(_pingHostsTotal()).arg(QObject::trUtf8("Avg. latency")).arg(_pingAverage()).arg(QObject::trUtf8("msec"));
    }

    if(_testMode & TestMode::Download)
    {
        html += QString(        "            <tr><td align=\"right\">%1</td><td align=\"center\">%2 %3</td><td align=\"right\" style=\"font-size:130%;\"><b>%4</b></td><td align=\"center\" style=\"font-size:130%;\"><b>%5 %6 %7</b></td></tr>\n").arg(QObject::trUtf8("Downloads ran for")).arg(_downloadTestSecs).arg(QObject::trUtf8("sec each")).arg(QObject::trUtf8("Max. speed")).arg(_maxBandwidth(SpeedUnit::Mbps)).arg(QObject::trUtf8("or")).arg(_maxBandwidth(SpeedUnit::MBps));
    }

    html +=                     "        </table>\n";

    if((_testMode & TestMode::Ping) || (_testMode & TestMode::Download))
    {
        html +=                 "        <p>\n";

        if(_testMode & TestMode::Ping)
        {
            html += QString(    "            <img src=\"%1\" alt=\"Google Chart - Pings\" />\n").arg(_googleChartPingGroupsUrl());
        }

        if(_testMode & TestMode::Download)
        {
            for(int i = 0; i < downloadGroupsSize; i++)
            {
                html += QString("<img src=\"%1\" alt=\"Google Chart - Download speeds - Group %2\" />\n").arg(_googleChartDownloadGroupUrl(i)).arg(_downloadGroups[i].name());
            }
        }

        html +=                 "        </p>\n";
    }

    if(_testMode & TestMode::Ping)
    {
        html +=                 "        <p style=\"margin-bottom: 2px;\"><b>Detailed ping results</b></p>\n"
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
            html += QString(    "            <p style=\"margin-top: 2px; margin-bottom: 2px;\"><b>%1</b></p>\n").arg(_pingGroups[i].name());
            html +=             "            <div>\n";
            html += QString(    "                <input type=\"button\" id=\"Group%2SpoilerButton\" value=\"Show\" style=\"width: 80px; height: 25px; font-size: 9pt;\" onclick=\"\n").arg(i);
            html += QString(    "                    if(document.getElementById('Group%2Div').style.display != '')\n").arg(i);
            html +=             "                    {\n";
            html += QString(    "                        document.getElementById('Group%2Div').style.display = '';\n").arg(i);
            html +=             "                        this.innerText = '';\n"
                                "                        this.value = 'Hide';\n"
                                "                    }\n"
                                "                    else\n"
                                "                    {\n";
            html += QString(    "                        document.getElementById('Group%2Div').style.display = 'none';\n").arg(i);
            html +=             "                        this.innerText = '';\n"
                                "                        this.value = 'Show';\n"
                                "                    }\"/>\n"
                                "            </div>\n";
            html += QString(    "            <div id=\"Group%2Div\" style=\"display: none; border: 1px solid black; padding: 5px; margin: 2px;\">\n").arg(i);
            html +=             "                <table border=\"1\" cellpadding=\"4\">\n";
            html += QString(    "                    <tr><td>%1</td><td>%2</td><td>%3</td><td align=\"center\">%4</td><td>%5</td></tr>\n").arg(QObject::trUtf8("Host")).arg(QObject::trUtf8("Avg. ping time")).arg(QObject::trUtf8("Packet loss")).arg(QObject::trUtf8("Jitter")).arg(QObject::trUtf8("Rank"));

            quint16 size = _pingGroups[i]._hosts.size();

            for(int j = 0; j < size; j++)
            {
                html += QString("                    <tr><td><a href=\"http://%1\">%2</a></td><td align=\"right\">%3</td><td align=\"right\">%4</td><td align=\"right\">%5</td><td align=\"center\">%6</td></tr>\n").arg(_pingGroups[i]._hosts[j].url()).arg(_pingGroups[i]._hosts[j].name()).arg(_pingGroups[i]._hosts[j].rttString()).arg(_pingGroups[i]._hosts[j].packetLossString()).arg(_pingGroups[i]._hosts[j].jitterString()).arg(_pingGroups[i]._hosts[j].rank());
            }

            html += QString(    "                    <tr><td><b>%1</b></td><td align=\"right\"><b>%2</b></td></tr>\n").arg(QObject::trUtf8("Group sum")).arg(_pingGroups[i].rttSumString());
            html += QString(    "                    <tr><td><b>%1</b></td><td align=\"right\"><b>%2</b></td><td align=\"right\"><b>%3</b></td><td>&nbsp;</td><td align=\"center\"><b>%4</b></td></tr>\n").arg(QObject::trUtf8("Group avg.")).arg(_pingGroups[i].rttAverageString()).arg(_pingGroups[i].packetLossAverageString()).arg(_pingGroups[i].rank());
            html +=             "                </table>\n"
                                "            </div>\n"
                                "            <br/>\n";
        }

        html +=                 "        </div>\n";
    }

    html +=                     "        <p>\n"
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

double Results::_pingAverage() const
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

    return average;
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
    quint16 size2 = 0;

    if(size)
    {
        for(int i = 0; i < size; i++)
        {
            speed = _downloadGroups[i]._speedBpsParallel;

            if(speed > max)
            {
                max = speed;
            }

            size2 = _downloadGroups[i]._hosts.size();

            for(int j = 0; j < size2; j++)    // There is a slight possibility that one of the single downloads gave a bigger speed result than the parallel download result. The inner loop deals with this possibility
            {
                speed = _downloadGroups[i]._hosts[j].speed(SpeedUnit::Bps);

                if(speed > max)
                {
                    max = speed;
                }
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
    double max = 0.0, speed = 0.0;
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
    speed = _downloadGroups[index].speedParallel(SpeedUnit::Mbps);
    url += QString::number(speed, 'f', 2) + ',';
    max = speed;
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
        scalingPart = QString("&chxr=0,0,%1&chds=0,%1").arg(QString::number(max * 1.18, 'f', 0));    // The 1.18 factor seems to give enough space available for the arithmetic values right next to the bars, so they do not overlap with the bar itself
    }

    url += scalingPart;

    return url.replace(' ', '+');
}
