#ifndef EXTERNS_H
#define EXTERNS_H


#include <QtCore/QString>
#include <QtCore/QMutex>


extern const QString PROJECTNAME;
extern const QString PROGRAMNAME;
extern const QString PROGRAMAUTHOR;
extern const QString PROGRAMVERSION;
extern const QString PROGRAMURL;
extern const QString PROGRAMDISCUSSURL;
extern const QString PROGRAMUPDATECHECKURL;
extern const QString TARGETLISTURL;
extern const quint8 UPDATECHECKTIMEOUT;
extern const quint8 PINGTIMEOUT;
extern const quint8 DOWNLOADTESTSECS;
extern quint8 PINGSPERTARGET;
extern QMutex MUTEX;
extern bool STOPBENCHMARK;


#endif // EXTERNS_H
