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
along with QSpeedTest. If not, see <http://www.gnu.org/licenses/>.
*/

#include "downloadhost.h"

double DownloadHost::speed(SpeedUnit::Unit unit) const
{
    switch(unit)
    {
    case SpeedUnit::bps:
        return (_speedBps * 8.0);
    case SpeedUnit::Bps:
        return _speedBps;
    case SpeedUnit::Kbps:
        return (_speedBps / 128.0);
    case SpeedUnit::KBps:
        return (_speedBps / 1024.0);
    case SpeedUnit::Mbps:
        return (_speedBps / 131072.0);
    case SpeedUnit::MBps:
        return (_speedBps / 1048576.0);
    default:
        return 0.0;
    }
}
