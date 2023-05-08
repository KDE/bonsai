#pragma once
#include <QObject>

#include <QProcess>
#include <QByteArray>

namespace Git
{
class AbstractCommand;
}


class ActionRunner : public QObject
{
    Q_OBJECT
public:
    explicit ActionRunner(const QString &workingDir, QObject *parent = nullptr);
    ~ActionRunner();

    void run(Git::AbstractCommand *command);
    void run(const QStringList &args);

private Q_SLOTS:
    void readyReadStandardOutput();
    void readyReadStandardError();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *mGitProcess = nullptr;
    Git::AbstractCommand *mCmd{nullptr};

    QByteArray mStandardOutput;
    QByteArray mErrorOutput;

    enum Mode { None, RunByArgs, RunByCommand };
       Mode mMode{None};


Q_SIGNALS:
    void actionFished(bool ok, const QString &message);

};

