#include "actionrunner.h"
#include <libkommit/commands/abstractcommand.h>

#include <KI18n/KLocalizedString>

#include <QDebug>

ActionRunner::ActionRunner(const QString &workingDir, QObject *parent) : QObject(parent)
  , mGitProcess(new QProcess{this})
{
    mGitProcess->setProgram(QStringLiteral("git"));
    mGitProcess->setWorkingDirectory(workingDir);

    connect(mGitProcess, &QProcess::readyReadStandardOutput, this, &ActionRunner::readyReadStandardOutput);
    connect(mGitProcess, &QProcess::readyReadStandardError, this, &ActionRunner::readyReadStandardError);

    connect(mGitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ActionRunner::finished);

}

ActionRunner::~ActionRunner()
{
    if(mCmd)
        mCmd->deleteLater();
}

void ActionRunner::run(Git::AbstractCommand *command)
{
    if(!command)
        return;

    mMode = RunByCommand;

    mCmd = command;
    const auto args = command->generateArgs();
    mGitProcess->setArguments(args);

    qDebug() << "Running action as << " << mGitProcess->program() << mGitProcess->arguments();
    mGitProcess->start();
}

void ActionRunner::run(const QStringList &args)
{
    mMode = RunByArgs;
    mGitProcess->setArguments(args);
    mGitProcess->start();
}

void ActionRunner::readyReadStandardOutput()
{
    const auto buffer = mGitProcess->readAllStandardOutput();
    mErrorOutput.append(buffer);

    if (mCmd && mCmd->supportProgress())
        mCmd->parseOutputSection(buffer, QByteArray());

}

void ActionRunner::readyReadStandardError()
{
    const auto buffer = mGitProcess->readAllStandardError();
    mStandardOutput.append(buffer);

    if (mCmd && mCmd->supportProgress())
        mCmd->parseOutputSection(QByteArray(), buffer);

}

void ActionRunner::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    if (mCmd)
        mCmd->parseOutputSection(mStandardOutput, mErrorOutput);

    bool isSuccessful = false;

    if (mCmd)
        isSuccessful = mCmd->status() != Git::AbstractCommand::Status::Error;
    else
        isSuccessful = exitStatus == QProcess::NormalExit;

    QString exitMessage;

    if (isSuccessful)
    {
        //        exitMessage = i18n("Process finished");
        exitMessage = mStandardOutput;
    } else
    {
        if (mCmd)
            exitMessage = mCmd->errorMessage();
        else
//            exitMessage = i18n("The git process crashed");
        exitMessage = mStandardOutput;
    }

    Q_EMIT actionFished(isSuccessful, exitMessage);

}
