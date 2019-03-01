﻿#include <QCoreApplication>
#include <QApplication>
#include "tcpserver.h"
#include "SCADARunTime.h"
#include "public.h"
#include "TimerTask.h"
#include "httpserver.h"
#include "VersionInfo.h"
#include "Log.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <iostream>


#ifndef Q_OS_WIN
#include "eventdispatcher_libev/eventdispatcher_libev.h"
#endif

void customMessageHandler(QtMsgType type, const QMessageLogContext &context,const QString & msg)
{
        QString txt;
        switch (type) {
        //调试信息提示
        case QtDebugMsg:
//            txt = QString("%1: Debug: at:%2,%3 on %4; %5").arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
//                    .arg(context.file).arg(context.line).arg(context.function).arg(msg);
            txt = msg;
                break;

        //一般的warning提示
        case QtWarningMsg:
                txt = QString("%1:Warning: at:%2,%3 on %4; %5").arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(context.file).arg(context.line).arg(context.function).arg(msg);
        break;
        //严重错误提示
        case QtCriticalMsg:
                txt = QString("%1:Critical: at:%2,%3 on %4; %5").arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(context.file).arg(context.line).arg(context.function).arg(msg);
        break;
        //致命错误提示
        case QtFatalMsg:
                txt = QString("%1:Fatal: at:%2,%3 on %4; %5").arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(context.file).arg(context.line).arg(context.function).arg(msg);
                abort();
        }
        QFile outFile(QString("%1/%2.txt").arg(QDir::currentPath()).arg(QDate::currentDate().toString("yyyy-MM-dd")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
}

QString getSystemInfo() {

    QString s = "\n";
    s.append(QString("Build:%1 Version:%2\n").arg(GIT_HASH).arg(VER_FILE));
    s.append("System Info: \n");

    QTextStream out(&s);

    QSysInfo systemInfo;
#ifdef Q_OS_WIN
    out << " Windows Version:          " << systemInfo.windowsVersion()         << endl;
#endif
    out << " Build Cpu Architecture:   " << systemInfo.buildCpuArchitecture()   << endl;
    out << " Current Cpu Architecture: " << systemInfo.currentCpuArchitecture() << endl;
    out << " Kernel Type:              " << systemInfo.kernelType()             << endl;
    out << " Kernel Version:           " << systemInfo.kernelVersion()          << endl;
    out << " Machine Host Name:        " << systemInfo.machineHostName()        << endl;
    out << " Product Type:             " << systemInfo.productType()            << endl;
    out << " Product Version:          " << systemInfo.productVersion()         << endl;
    out << " Byte Order:               " << systemInfo.buildAbi()               << endl;
    out << " Pretty ProductName:       " << systemInfo.prettyProductName()      << endl;

    // root
    QStorageInfo storage = QStorageInfo::root();

    out << storage.rootPath()                                             << endl;
    out << "isReadOnly:" << storage.isReadOnly()                          << endl;
    out << "name:" << storage.name()                                      << endl;
    out << "fileSystemType:" << storage.fileSystemType()                  << endl;
    out << "size:" << storage.bytesTotal()/1000/1000 << "MB"              << endl;
    out << "availableSize:" << storage.bytesAvailable()/1000/1000 << "MB" << endl;

    // current volumn
    QStorageInfo storageCurrent(qApp->applicationDirPath());

    out << qApp->applicationDirPath()                                            << endl;
    out << storageCurrent.rootPath()                                             << endl;
    out << "isReadOnly:" << storageCurrent.isReadOnly()                          << endl;
    out << "name:" << storageCurrent.name()                                      << endl;
    out << "fileSystemType:" << storageCurrent.fileSystemType()                  << endl;
    out << "size:" << storageCurrent.bytesTotal()/1000/1000 << "MB"              << endl;
    out << "availableSize:" << storageCurrent.bytesAvailable()/1000/1000 << "MB" << endl;

    return s;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    //    qInstallMessageHandler(customMessageHandler);
#ifndef Q_OS_WIN
    //QCoreApplication::setEventDispatcher(new EventDispatcherLibEv());
#endif
    //QCoreApplication a(argc, argv);
    QApplication  a(argc, argv);
    //std::cout << "???" << std::endl;

    Log4Info(getSystemInfo());
    Log4Info("start SCADARunTime!");

    HttpServer server;
    server.init(60000);

    QString projPath = QCoreApplication::applicationDirPath() + "/RunProject";
    QDir dir(projPath);
    if (!dir.exists())
    {
        dir.mkpath(projPath);
    }

    SCADARunTime runTime(projPath);
    runTime.Load(DATA_SAVE_FORMAT);
    runTime.Start();
    g_SCADARunTimePtr = &runTime;

    QString projSavePath = QCoreApplication::applicationDirPath() + "/Project";
    QDir projSaveDir(projSavePath);
    if(!projSaveDir.exists())
    {
        projSaveDir.mkpath(projSavePath);
    }
    TcpServer ser(&runTime);
    ser.listen(QHostAddress::Any, 6000);

    TimerTask *pTimerTask = new TimerTask();

    ret = a.exec();

    delete pTimerTask;

    return ret;
}
