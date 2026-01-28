#pragma once

#include "iraydetector_global.h"
#include <Windows.h>
#include <QString>
#include <QDebug>
#include <QQueue>
#include <QSemaphore>
#include <QMutex>
#include <QFile>

class QTimer;

class IRAYDETECTOR_EXPORT QtLogger
{
public:
    static void initialize();
    static void setMessagePattern();
    static void installMessageHandler();
    static QString getLogsDir();
    static bool cleanupLogs();

private:
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    static LONG WINAPI applicationCrashHandler(EXCEPTION_POINTERS* pException);

    // 异步日志写入相关
    static void processLogQueue();
    static bool rotateCurrentLogFile();

    // 私有成员
    static QQueue<QString> logQueue;
    static QTimer* logTimer;
    static QMutex logMutex;
    static QString currentLogPath;
    static QFile* currentLogFile;
    static const qint64 maxLogFileSize;
};
