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


#include "testresults.h"


TestResults::TestResults() {
    hostOS.clear();
}


void TestResults::reset() {
    rttSum = 0.0;
    targetsAlive = 0;
    speedInKbpsDomestic = 0.0;
    speedInMBpsDomestic = 0.0;
    speedInKbpsInternational = 0.0;
    speedInMBpsInternational = 0.0;
    testDuration = 0.0;
}


void TestResults::print() {
    emit message(trUtf8("\n\n\n" "Report created by:        %1 %2\n"
                                 "Target list used:         %3 %4\n"
                                 "Target list contact URL:  %5").arg(programName).arg(programVersion).arg(targetListVersion).arg(targetListComment).arg(targetListContactUrl));
    emit message(trUtf8(         "Test date and time:       %1\n"
                                 "Host OS & no. of CPUs:    %2 / %3").arg(testDateTime).arg(hostOS).arg(cpuCores));
    emit message(trUtf8(         "ISP & WAN IP:             %1 - %2\n"
                                 "BBRAS:                    %3").arg(isp).arg(ip).arg(bbras));

    emit message(trUtf8(         "Test mode:                %1\n"
                                 "Test completed in:        %2 sec").arg(testMode).arg(testDuration));

    if(pingTestEnabled) {
        emit message(trUtf8(     "Pings/target:             %1\n"
                                 "Parallel ping threads:    %2").arg(pingsPerTarget).arg(parallelPingThreads));
        emit message(trUtf8(     "Targets alive:            %1 / %2\n"
                                 "Average ping/target:      %3").arg(targetsAlive).arg(targetsTotal).arg(getRttAvgAsString()));
    }

    if(downloadTestEnabled) {
        emit message(trUtf8(     "Speed test domestic:      %1 Kbps\n"
                                 "                          %2 MB/sec").arg(speedInKbpsDomestic, 0, 'f', 0).arg(speedInMBpsDomestic, 0, 'f', 3));
        emit message(trUtf8(     "Speed test international: %1 Kbps\n"
                                 "                          %2 MB/sec").arg(speedInKbpsInternational, 0, 'f', 0).arg(speedInMBpsInternational, 0, 'f', 3));
    }
}


QString TestResults::getRttAvgAsString() {
    if(targetsAlive)
        return QString::number(rttSum / targetsAlive, 'f', 2) + " msec";

    return "N/A";
}


QString TestResults::getHtmlCode(TargetList *targetList) {
    QString htmlCode;

    htmlCode  = QObject::trUtf8(    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                                    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"el\">\n"
                                    "    <head>\n"
                                    "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
                                    "        <title>%1 %2 report - %3</title>\n").arg(programName).arg(programVersion).arg(testDateTime);
    htmlCode += QObject::trUtf8(    "    </head>\n"
                                    "    <body>\n"
                                    "        <table border=\"1\" cellpadding=\"4\">\n"
                                    "            <tr><td>Report created by</td><td align=\"center\">%1 %2 - <a href=\"%3\">Download</a> - <a href=\"%4\">Discuss</a></td></tr>\n").arg(programName).arg(programVersion).arg(programUrl).arg(programDiscussUrl);
    htmlCode += QObject::trUtf8(    "            <tr><td>Target list used</td><td align=\"center\">%1 <a href=\"%2\">%3</a></td></tr>\n").arg(targetListVersion).arg(targetListContactUrl).arg(targetListComment);
    htmlCode += QObject::trUtf8(    "            <tr><td>Test date and time</td><td align=\"center\">%1</td></tr>\n"
                                    "            <tr><td>Host OS & no. of CPUs</td><td align=\"center\">%2 - %3 CPU cores</td></tr>\n").arg(testDateTime).arg(hostOS).arg(cpuCores);
    htmlCode += QObject::trUtf8(    "            <tr><td>ISP & WAN IP</td><td align=\"center\">%1 - %2</td></tr>\n"
                                    "            <tr><td>BBRAS</td><td align=\"center\">%3</td></tr>\n").arg(isp).arg(ip).arg(bbras);
    htmlCode += QObject::trUtf8(    "            <tr><td>Test mode</td><td align=\"center\">%1</td></tr>\n"
                                    "            <tr><td>Test completed in</td><td align=\"center\">%2 sec</td></tr>\n").arg(testMode).arg(testDuration);

    if(pingTestEnabled) {
        htmlCode += QObject::trUtf8("            <tr><td>Pings/target</td><td align=\"center\">%1</td></tr>\n"
                                    "            <tr><td>Parallel ping threads</td><td align=\"center\">%2</td></tr>\n").arg(pingsPerTarget).arg(parallelPingThreads);
        htmlCode += QObject::trUtf8("            <tr><td>Targets alive</td><td align=\"center\">%1 / %2</td></tr>\n").arg(targetsAlive).arg(targetsTotal);
        htmlCode += QObject::trUtf8("            <tr><td><b>Average ping/target</b></td><td align=\"center\"><b>%1</b></td></tr>\n").arg(getRttAvgAsString());
    }

    if(downloadTestEnabled) {
        htmlCode += QObject::trUtf8("            <tr><td><b>Speed test domestic</b></td><td align=\"center\"><b>%1 Kbps or %2 MB/sec</b></td></tr>\n").arg(speedInKbpsDomestic, 0, 'f', 0).arg(speedInMBpsDomestic, 0, 'f', 3);
        htmlCode += QObject::trUtf8("            <tr><td><b>Speed test international</b></td><td align=\"center\"><b>%1 Kbps or %2 MB/sec</b></td></tr>\n").arg(speedInKbpsInternational, 0, 'f', 0).arg(speedInMBpsInternational, 0, 'f', 3);
    }

    htmlCode +=                     "        </table>\n";

    if(pingTestEnabled)
        htmlCode += QString(        "        <p>"
                                    "            <img src=\"%1\" alt=\"Google Chart\" />"
                                    "        </p>").arg(getGoogleChartUrl(targetList));

    htmlCode += (pingTestEnabled)? ("        <p style=\"margin-bottom: 2px;\"><b>Detailed results</b></p>\n"
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

    for(int i = 0; i < targetList->groups.size() && pingTestEnabled; i++) {
        htmlCode += QObject::trUtf8("            <p style=\"margin-top: 2px; margin-bottom: 2px;\"><b>%1</b></p>\n"
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
                                    "                    <tr><td>Target</td><td>Average ping time</td><td>Packet loss</td><td align=\"center\">Jitter</td><td>Rank</td></tr>\n").arg(targetList->groups[i].getName()).arg(i);

        for(int j = 0; j < targetList->groups[i].getSize(); j++)
            htmlCode += QString(    "                    <tr><td><a href=\"http://%1\">%2</a></td><td align=\"right\">%3</td><td align=\"right\">%4</td><td align=\"right\">%5</td><td align=\"center\">%6</td></tr>\n").arg(targetList->groups[i].targets[j].getAddress()).arg(targetList->groups[i].targets[j].getName()).arg(targetList->groups[i].targets[j].getRttAvgAsString()).arg(targetList->groups[i].targets[j].getPacketLossAsString()).arg(targetList->groups[i].targets[j].getJitterAsString()).arg(targetList->groups[i].targets[j].getRank());

        htmlCode += QObject::trUtf8("                    <tr><td><b>Group sum</b></td><td align=\"right\"><b>%1</b></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"
                                    "                    <tr><td><b>Group average</b></td><td align=\"right\"><b>%2</b></td><td align=\"right\"><b>%3</b></td><td>&nbsp;</td><td align=\"center\"><b>%4</b></td></tr>\n"
                                    "                </table>\n"
                                    "            </div>\n"
                                    "            <br/>\n").arg(targetList->groups[i].getRttSumAsString()).arg(targetList->groups[i].getRttAvgAsString()).arg(targetList->groups[i].getPacketLossAvgAsString()).arg(targetList->groups[i].getRank());
    }

    htmlCode += (pingTestEnabled)? ("        </div>\n") : NULL;
    htmlCode +=                     "        <p>\n"
                                    "            <a href=\"http://validator.w3.org\">\n"
                                    "                <img src=\"http://www.w3.org/Icons/valid-xhtml11\" alt=\"Valid XHTML 1.1\" height=\"31\" width=\"88\" />\n"
                                    "            </a>\n"
                                    "        </p>\n"
                                    "    </body>\n"
                                    "</html>\n";
    htmlCode.replace('&', "&amp;");
    htmlCode.replace("&amp;nbsp", "&nbsp");

    return htmlCode;
}


QString TestResults::getVbCode(TargetList *targetList) {
    QString vbCode;

    vbCode  = QObject::trUtf8(    "[table=head] | \n"
                                  "Report created by | [center]%1 %2 - [url=%3]Download[/url] - [url=%4]Discuss[/url][/center] |\n"
                                  "Target list used | [center]%5 [url=%6]%7[/url][/center] |\n"
                                  "Test date and time | [center]%8[/center] |\n"
                                  "Host OS & no. of CPUs | [center]%9 - %10 CPU cores[/center] |\n").arg(programName).arg(programVersion).arg(programUrl).arg(programDiscussUrl).arg(targetListVersion).arg(targetListContactUrl).arg(targetListComment).arg(testDateTime).arg(hostOS).arg(cpuCores);
    vbCode += QObject::trUtf8(    "ISP & WAN IP | [center]%1 - %2[/center] |\n"
                                  "BBRAS | [center]%3[/center] |\n"
                                  "Test mode | [center]%4[/center] |\n"
                                  "Test completed in | [center]%5 sec[/center] |\n").arg(isp).arg(ip).arg(bbras).arg(testMode).arg(testDuration);

    if(pingTestEnabled) {
        vbCode += QObject::trUtf8("Pings/target | [center]%1[/center] |\n"
                                  "Parallel ping threads | [center]%2[/center] |\n").arg(pingsPerTarget).arg(parallelPingThreads);
        vbCode += QObject::trUtf8("Targets alive | [center]%1 / %2[/center] |\n"
                                  "[b]Average ping/target[/b] | [center][b]%3[/b][/center] |\n").arg(targetsAlive).arg(targetsTotal).arg(getRttAvgAsString());
    }

    if(downloadTestEnabled) {
        vbCode += QObject::trUtf8("[b]Speed test domestic[/b] | [center][b]%1 Kbps or %2 MB/sec[/b][/center] |\n").arg(speedInKbpsDomestic, 0, 'f', 0).arg(speedInMBpsDomestic, 0, 'f', 3);
        vbCode += QObject::trUtf8("[b]Speed test international[/b] | [center][b]%1 Kbps or %2 MB/sec[/b][/center] |\n").arg(speedInKbpsInternational, 0, 'f', 0).arg(speedInMBpsInternational, 0, 'f', 3);
    }

    vbCode +=                     "[/table]\n\n";

    if(pingTestEnabled)
        vbCode += QString(        "[img]%1[/img]\n").arg(getGoogleChartUrl(targetList));

    vbCode += (pingTestEnabled)? ("\n"
                                  "[b]Detailed results[/b]\n"
                                  "[spoiler]\n") : NULL;

    for(int i = 0; i < targetList->groups.size() && pingTestEnabled; i++) {
        vbCode += QObject::trUtf8("[b]%1[/b]\n"
                                  "[spoiler]\n"
                                  "[table=head][center]Target[/center] | [center]Average ping time[/center] | Packet loss | Jitter | Rank\n").arg(targetList->groups[i].getName());

        for(int j = 0; j < targetList->groups[i].getSize(); j++)
            vbCode += QString(    "%1 | [right]%2[/right] | [right]%3[/right] | [right]%4[/right] | [center]%5[/center] |\n").arg(targetList->groups[i].targets[j].getName()).arg(targetList->groups[i].targets[j].getRttAvgAsString()).arg(targetList->groups[i].targets[j].getPacketLossAsString()).arg(targetList->groups[i].targets[j].getJitterAsString()).arg(targetList->groups[i].targets[j].getRank());

        vbCode += QObject::trUtf8("[b]Group sum[/b] | [right][b]%1[/b][/right] |\n"
                                  "[b]Group average[/b] | [right][b]%2[/b][/right] | [right][b]%3[/b][/right] | | [center][b]%4[/b][/center]\n"
                                  "[/table]\n"
                                  "[/spoiler]\n").arg(targetList->groups[i].getRttSumAsString()).arg(targetList->groups[i].getRttAvgAsString()).arg(targetList->groups[i].getPacketLossAvgAsString()).arg(targetList->groups[i].getRank());
    }

    vbCode += ((pingTestEnabled)? "[/spoiler]\n" : NULL);

    return vbCode;
}


QString TestResults::getGoogleChartUrl(TargetList *targetList) {
    QString googleChartUrl = trUtf8("http://chart.apis.google.com/chart?chtt=Average+latency+per+group|(msec)&chs=400x160&cht=bhs&chxt=x,y&chxr=0,0,275&chds=0,275&chm=N++,000000,0,,12&chco=FF0000|00FF00|0000FF&chxl=1:");

    for(int i = targetList->groups.size() - 1; i >= 0; i--)
        googleChartUrl += QString("|%1").arg(targetList->groups[i].getName().replace(' ', '+'));

    googleChartUrl += "&chd=t:";

    for(int i = 0; i < targetList->groups.size(); i++)
        googleChartUrl += QString::number(targetList->groups[i].getRttAvg(), 'f', 2) + ",";

    googleChartUrl.chop(1);

    return googleChartUrl;
}
