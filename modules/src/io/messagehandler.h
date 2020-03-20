#ifndef CTL_MESSAGEHANDLER_H
#define CTL_MESSAGEHANDLER_H

#include <QMessageLogContext>
#include <QObject>
#include <QStringList>

namespace CTL {

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    // non-copyable
    MessageHandler(const MessageHandler&) = delete;
    MessageHandler(MessageHandler&&) = delete;
    MessageHandler& operator=(const MessageHandler&) = delete;
    MessageHandler& operator=(MessageHandler&&) = delete;
    ~MessageHandler() = default;

    // get the instance of the message handler
    static MessageHandler& instance();
    // installer method to be used to set in qInstallMessageHandler()
    static void qInstaller(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    // central logging function, processes log message received from Qt streams
    void processMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    // getter for full log
    const QStringList& log() const;

    // getter for last message (convenience)
    QString lastMessage() const;

    // add a class to the log blacklist
    void blacklistClassOrFunction(const QString& classOrFunction, bool blacklist = true);
    // add a file to the log blacklist
    void blacklistFile(const QString& fileName, bool blacklist = true);
    // add a messsage type to the blacklist
    void blacklistMessageType(QtMsgType type, bool blacklist = true);
    // clears all blacklists (every message will be processed)
    void clearAllBlacklists();

    // enforce that messages appear in log even when blacklisted (no stream output)
    void enforceLoggingOfBlacklistMsg(bool enabled = true);

    // sets the file name for log file to 'fileName'
    void setLogFileName(const QString& fileName);

    // suppress output to stream (still log in history)
    void squelch(bool enabled = true);
    void setQuiet(bool enabled = true); // same as squelch(bool)

    // change what information is included in message string
    void toggleDateTag(bool show);
    void toggleMessageOriginTag(bool show);
    void toggleTimeTag(bool show);
    void toggleTypeTag(bool show);
    void toggleAllTags(bool show);

    // writes the log to the file set via setLogFileName. returns true on success
    bool writeLogFile() const;

signals:
    void newLogEntry() const;
    void messagePrinted() const;

public slots:
    void messageFromSignal(QString msg);

private:
    MessageHandler();

    QStringList _theLog;
    QString _logfileName = QStringLiteral("ctllog.txt");

    bool _blacklistMsgType[5] = {};
    bool _logBlacklistedMsg = false;
    bool _showDateTag = false;
    bool _showMsgOrig = false;
    bool _showTimeTag = true;
    bool _showTypeTag = true;
    bool _squelched = false;

    QStringList _blacklistClassFct;
    QStringList _blacklistFiles;

    bool isBlacklistedClassOrFct(const QMessageLogContext& context) const;
    bool isBlacklistedFile(const QMessageLogContext& context) const;
    bool isBlacklistedMessageType(QtMsgType type) const;

    QString dateTimeTag() const;
    QString messageOriginString(const QMessageLogContext& context) const;
    QString typeTag(QtMsgType type) const;

    void printMessage(const QString& finalMsg, QtMsgType type) const;
};

} // namespace CTL

#endif // CTL_MESSAGEHANDLER_H
