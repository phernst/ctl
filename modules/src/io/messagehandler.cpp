#include "messagehandler.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>

namespace CTL {

MessageHandler::MessageHandler() : QObject(nullptr)
{
}

bool MessageHandler::isBlacklistedClassOrFct(const QMessageLogContext &context) const
{
    if(!context.function)
        return false;

    QString functionName(context.function);
    for(const auto& cl : _blacklistClassFct)
        if(functionName.contains(cl))
            return true;

    return false;
}

bool MessageHandler::isBlacklistedFile(const QMessageLogContext& context) const
{
    if(!context.file)
        return false;

    const QString fileName(context.file);
    for(const auto& cl : _blacklistFiles)
        if(fileName.contains(cl))
            return true;

    return false;
}

bool MessageHandler::isBlacklistedMessageType(QtMsgType type) const
{
    return _blacklistMsgType[type];
}

MessageHandler& MessageHandler::instance()
{
    static MessageHandler theInstance;

    return theInstance;
}

void MessageHandler::processMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    bool isBlacklisted = isBlacklistedMessageType(type) || isBlacklistedClassOrFct(context) || isBlacklistedFile(context);

    if(isBlacklisted && !_logBlacklistedMsg)
        return;

    constexpr int dateTagSize{12}, timeTagSize{15}, typeTagSize{10};

    QString logString;
    logString.reserve(msg.size() + _showDateTag * dateTagSize
                                 + _showTimeTag * timeTagSize
                                 + _showTypeTag * typeTagSize
                                 + _showMsgOrig * 256);

    if(_showTimeTag) logString += dateTimeTag();
    if(_showTypeTag) logString += typeTag(type);

    // append actual message
    logString += msg;

    if(_showMsgOrig) logString += messageOriginString(context);

    // append the current message to the full log
    _theLog.append(logString);
    emit newLogEntry();

    // print message if appropriate
    if(!isBlacklisted && !_squelched)
        printMessage(logString, type);

    // terminate for fatal messages
    if(type == QtFatalMsg) abort();
}

const QStringList& MessageHandler::log() const { return _theLog; }

QString MessageHandler::lastMessage() const
{
    return _theLog.isEmpty() ? QStringLiteral("") : _theLog.last();
}

void MessageHandler::blacklistClassOrFunction(const QString& classOrFunction, bool blacklist)
{
    if(blacklist)
        _blacklistClassFct.append(classOrFunction);
    else
        _blacklistClassFct.removeAll(classOrFunction);
}

void MessageHandler::blacklistFile(const QString& fileName, bool blacklist)
{
    if(blacklist)
        _blacklistFiles.append(fileName);
    else
        _blacklistFiles.removeAll(fileName);
}

void MessageHandler::blacklistMessageType(QtMsgType type, bool blacklist)
{
    if(type < 5)
        _blacklistMsgType[type] = blacklist;
}

void MessageHandler::clearAllBlacklists()
{
    _blacklistClassFct.clear();
    _blacklistFiles.clear();
    std::fill_n(_blacklistMsgType, 5, false);
}

void MessageHandler::enforceLoggingOfBlacklistMsg(bool enabled)
{
    _logBlacklistedMsg = enabled;
}

void MessageHandler::setLogFileName(const QString& fileName)
{
    _logfileName = fileName;
}

void MessageHandler::setQuiet(bool enabled) { _squelched = enabled; }

void MessageHandler::squelch(bool enabled) { _squelched = enabled; }

void MessageHandler::toggleDateTag(bool show) { _showDateTag = show; }

void MessageHandler::toggleTimeTag(bool show) { _showTimeTag = show; }

void MessageHandler::toggleTypeTag(bool show) { _showTypeTag = show; }

void MessageHandler::toggleMessageOriginTag(bool show)
{
    _showMsgOrig = show;
}


void MessageHandler::toggleAllTags(bool show)
{
    toggleDateTag(show);
    toggleMessageOriginTag(show);
    toggleTimeTag(show);
    toggleTypeTag(show);
}

bool MessageHandler::writeLogFile() const
{
    QFile file(_logfileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning("Could not write log file. Failed to open file: %s", _logfileName.toLocal8Bit().constData());
        return false;
    }

    QTextStream out(&file);
    out << _theLog.join("\n");

    return out.status() == QTextStream::Ok;
}

void MessageHandler::messageFromSignal(QString msg)
{
    processMessage(QtInfoMsg, QMessageLogContext(), msg);
}


void MessageHandler::qInstaller(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    instance().processMessage(type, context, msg);
}

QString MessageHandler::typeTag(QtMsgType type) const
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("Debug: ");
    case QtInfoMsg:
        return QStringLiteral("Info: ");
    case QtWarningMsg:
        return QStringLiteral("Warning: ");
    case QtCriticalMsg:
        return QStringLiteral("Critical: ");
    case QtFatalMsg:
        return QStringLiteral("Fatal: ");
    }

    return QStringLiteral("Unknown: ");
}

QString MessageHandler::dateTimeTag() const
{
    QString format = _showDateTag ? QStringLiteral("MM-dd-yyyy ") : QStringLiteral("");
    if(_showTimeTag) format += QStringLiteral("hh:mm:ss.zzz");

    return QStringLiteral("[") + QDateTime::currentDateTime().toString(format) + QStringLiteral("] ");
}

QString MessageHandler::messageOriginString(const QMessageLogContext& context) const
{
    QString ret;
    ret.reserve(256);
    if(context.function) ret += QStringLiteral(" | ") + QString(context.function);
    if(context.file)     ret += QStringLiteral("; ") + QString(context.file) + QStringLiteral(":") + QString::number(context.line);

    return ret;
}

void MessageHandler::printMessage(const QString &finalMsg, QtMsgType type) const
{
    FILE* stream = stderr;
    if(type == QtDebugMsg || type == QtInfoMsg)
        stream = stdout;

    fprintf(stream, "%s\n", finalMsg.toLocal8Bit().constData());
    fflush(stream);

    emit messagePrinted();
}

} // namespace CTL
