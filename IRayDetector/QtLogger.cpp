#include "QtLogger.h"

#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QMutex>
#include <QApplication>
#include <QQueue>
#include <QTimer>
#include <QThread>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

// 初始化静态成员
QQueue<QString> QtLogger::logQueue;
QTimer* QtLogger::logTimer = nullptr;
QMutex QtLogger::logMutex;
QString QtLogger::currentLogPath;
QFile* QtLogger::currentLogFile = nullptr;
const qint64 QtLogger::maxLogFileSize = 20LL * 1024 * 1024;  // 20 MB

void QtLogger::initialize()
{
    // 创建日志文件对象
    if (!currentLogFile)
    {
        currentLogFile = new QFile();
    }

    // 创建定时器用于处理日志队列
    if (!logTimer)
    {
        logTimer = new QTimer();
        // 使用 lambda 连接 timeout 信号到处理函数
        QObject::connect(logTimer, &QTimer::timeout, []() {
            QtLogger::processLogQueue();
        });
        logTimer->start(100);  // 每100ms处理一次队列
    }

    // 设置 Qt 日志格式并安装消息处理器
    // setMessagePattern();
    installMessageHandler();

    // 注册 Windows 未处理异常回调，生成 dump
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)applicationCrashHandler);

    qDebug() << "日志模块已初始化";
    qInfo() << "日志模块已初始化";
    qWarning() << "日志模块已初始化";
    qCritical() << "日志模块已初始化";
}

LONG WINAPI QtLogger::applicationCrashHandler(EXCEPTION_POINTERS* pException)
{
    // 创建 Dump 文件
    HANDLE hDumpFile =
        CreateFile(L"XRayMEMORY.DMP", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
        CloseHandle(hDumpFile);
    }

    // 弹窗提示并退出
    EXCEPTION_RECORD* record = pException->ExceptionRecord;
    QString errCode(QString::number(record->ExceptionCode, 16));
    QString errAdr(QString::number((uint)record->ExceptionAddress, 16));
    QMessageBox::critical(NULL, "程序崩溃",
                          QString("<div><b>对于发生的错误，表示诚挚的歉意</b><br/>错误代码：%1<br/>错误地址：%2</div>")
                              .arg(errCode)
                              .arg(errAdr),
                          QMessageBox::Ok);

    return EXCEPTION_EXECUTE_HANDLER;
}

void QtLogger::setMessagePattern()
{
    qSetMessagePattern(
        "[%{time yyyy-MM-dd hh:mm:ss.zzz}] "
        "[%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] "
        "[%{threadid}] "
        "[%{file}:%{line} - %{function}] "
        "%{message}");
}

void QtLogger::installMessageHandler()
{
    qInstallMessageHandler(customMessageHandler);
}

void QtLogger::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static QMutex formatMutex;
    QMutexLocker locker(&formatMutex);

    // 级别映射
    QString level = "UNKNOWN";
    switch (type)
    {
        case QtDebugMsg:
            level = "DEBUG";
            break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QtInfoMsg:
            level = "INFO";
            break;
#endif
        case QtWarningMsg:
            level = "WARN";
            break;
        case QtCriticalMsg:
            level = "ERROR";
            break;
        case QtFatalMsg:
            level = "FATAL";
            break;
        default:
            break;
    }

    // 保护 context 字段可能为 nullptr 的情况
    const char* cfile = context.file ? context.file : "";
    const char* cfunc = context.function ? context.function : "";
    QString fileName = QFileInfo(QString::fromUtf8(cfile)).fileName();
    QString funcName = QString::fromUtf8(cfunc);

    QString threadInfo = QString("Thread[0x%1]").arg((quintptr)QThread::currentThreadId(), 0, 16);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[%1] [%2] [%3] [%4:%5] [%6] %7\n")
                             .arg(timestamp)
                             .arg(threadInfo)
                             .arg(level)
                             .arg(fileName.isEmpty() ? QString("-") : fileName)
                             .arg(context.line)
                             .arg(funcName.isEmpty() ? QString("-") : funcName)
                             .arg(msg);

    // 输出到 Visual Studio 调试窗口（Windows），否则回退到 stderr
#ifdef Q_OS_WIN
    std::wstring wmsg = logMessage.toStdWString();
    OutputDebugStringW(wmsg.c_str());
#else
    QByteArray out = logMessage.toLocal8Bit();
    fprintf(stderr, "%s", out.constData());
    fflush(stderr);
#endif

    // 将日志消息加入队列（快速路径）
    {
        QMutexLocker queueLock(&logMutex);
        logQueue.enqueue(logMessage);
    }

    // 致命错误立即终止进程
    if (type == QtFatalMsg)
    {
        abort();
    }
}

QString QtLogger::getLogsDir()
{
    return qApp->applicationDirPath() + "/logs";
}

void QtLogger::processLogQueue()
{
    QMutexLocker queueLock(&logMutex);

    // 检查日期变化和打开日志文件
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString logsDir = getLogsDir();
    QString desiredLogPath = logsDir + "/app-" + today + ".log";

    if (currentLogPath != desiredLogPath)
    {
        // 日期变化，关闭当前文件并打开新文件
        if (currentLogFile && currentLogFile->isOpen())
        {
            currentLogFile->close();
        }
        currentLogPath = desiredLogPath;
        QDir().mkpath(logsDir);
        currentLogFile->setFileName(desiredLogPath);
        currentLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

    // 确保文件已打开
    if (currentLogFile && !currentLogFile->isOpen())
    {
        QDir().mkpath(logsDir);
        currentLogFile->setFileName(currentLogPath);
        currentLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

    // 处理队列中的所有日志
    while (!logQueue.isEmpty())
    {
        QString logMessage = logQueue.dequeue();

        if (currentLogFile && currentLogFile->isOpen())
        {
            QTextStream stream(currentLogFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            stream.setCodec("UTF-8");
#else
            stream.setEncoding(QStringConverter::Utf8);
#endif
            stream << logMessage;
            stream.flush();

            // 检查文件大小并进行轮转
            QFileInfo fi(currentLogFile->fileName());
            if (fi.exists() && fi.size() >= maxLogFileSize)
            {
                rotateCurrentLogFile();
            }
        }
    }
}

bool QtLogger::rotateCurrentLogFile()
{
    if (!currentLogFile)
        return false;

    QString filePath = currentLogFile->fileName();
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
        return false;

    // 关闭当前文件
    if (currentLogFile->isOpen())
    {
        currentLogFile->close();
    }

    // 生成备份文件名: app-2024-01-15.log -> app-2024-01-15_HHmmss.log
    QString baseName = fileInfo.completeBaseName();  // "app-2024-01-15"
    QString suffix = fileInfo.suffix();              // "log"
    QString backupName = QString("%1%2.%3")
                             .arg(baseName)
                             .arg(QDateTime::currentDateTime().toString("_HHmmss"))
                             .arg(suffix);

    QString backupPath = fileInfo.absolutePath() + "/" + backupName;

    // 重命名为备份文件
    bool success = QFile::rename(filePath, backupPath);

    // 重新打开新的日志文件
    currentLogFile->setFileName(filePath);
    currentLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    currentLogPath = filePath;

    return success;
}

bool QtLogger::cleanupLogs()
{
    // 备份当前的日志文件
    QMutexLocker queueLock(&logMutex);

    if (currentLogFile && currentLogFile->isOpen())
    {
        return rotateCurrentLogFile();
    }

    return false;
}
