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

#ifndef LIBSPEEDTEST_GLOBAL_H
#define LIBSPEEDTEST_GLOBAL_H
#include <QtCore/qglobal.h>
#if defined(LIBSPEEDTEST_LIBRARY)
#  define LIBSPEEDTEST_EXPORT Q_DECL_EXPORT
#else
#  define LIBSPEEDTEST_EXPORT Q_DECL_IMPORT
#endif
#endif // LIBSPEEDTEST_GLOBAL_H
