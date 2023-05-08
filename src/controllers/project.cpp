#include "project.h"
#include "projectmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QColor>
#include <QFileSystemWatcher>
#include <QTimer>

#include <QtConcurrent>
#include <QFuture>

#include <KI18n/KLocalizedString>

#include <libkommit/commands/commandclone.h>
#include <libkommit/models/logsmodel.h>
#include <libkommit/gitglobal.h>
#include <libkommit/gitlog.h>
#include <libkommit/models/remotesmodel.h>

#include <libkommit/gitmanager.h>

#include "actionrunner.h"

Project::Project(QObject *parent) : QObject(parent)
  ,m_manager(new Git::Manager())
  ,m_cloneWatcher(nullptr)
  ,m_status(new StatusMessage(this))
  ,m_gitDirWacther(new QFileSystemWatcher(this))
  ,m_watcherTimer(new QTimer(this))
{
    //    qRegisterMetaType<Status>("Status"); // this is needed for QML to know of WindowDecorations

    qRegisterMetaType<const Git::RemotesModel*>("const Git::RemotesModel *");
    connect(this, &Project::urlChanged, this, &Project::setData);
//    connect(this, &Project::currentBranchChanged, this, &Project::setCurrentBranchRemote);


    //Watch the git directory in case somehting happens, if so then refresh the needed parts
    connect(m_gitDirWacther, &QFileSystemWatcher::directoryChanged, [this](const QString &dir)
    {
        qDebug() << "GIT DIR CHANGED REACT TO IT ???????????????????" << dir;

        m_watcherTimer->start();
    });

    connect(m_gitDirWacther, &QFileSystemWatcher::fileChanged, [this](const QString &dir)
    {
        qDebug() << "GIT FILE CHANGED REACT TO IT ???????????????????" << dir;

        m_watcherTimer->start();
    });

    m_watcherTimer->setSingleShot(true);
    m_watcherTimer->setInterval(1000);

    connect(m_watcherTimer, &QTimer::timeout, [this]()
    {
        qDebug() << "GIT DIR CHANGED REACT TO IT";

        if(!m_manager->isValid())
            return;

        m_manager->logsModel()->load();
        m_manager->logsModel()->reset();
        loadData();

        Q_EMIT repoChanged();
    });
}

Project::~Project()
{
    if(m_cloneWatcher)
    {
        m_cloneWatcher->cancel();
        m_cloneWatcher->deleteLater();
    }

    m_manager->deleteLater();
}

QString Project::url() const
{
    return m_url;
}

void Project::setData(const QString &url)
{    
    const auto mUrl = QUrl::fromUserInput(url);

    QFileInfo fileInfo(mUrl.toLocalFile());
    if(!fileInfo.exists())
    {
        setStatus(StatusMessage::Error, i18n("Directory does not exists."));
        return;
    }

    m_manager->setPath(mUrl.toLocalFile());
    if(!m_manager->isValid())
    {
        Q_EMIT error(i18n("URL is not a valid repo."));
        setStatus(StatusMessage::Error, mUrl.toString());
        return;
    }

    m_gitDirWacther->addPath(mUrl.toLocalFile());
    m_gitDirWacther->addPath(mUrl.toLocalFile()+"/.git/objects");
    m_gitDirWacther->addPath(mUrl.toLocalFile()+"/.git/refs/heads");
    m_gitDirWacther->addPath(mUrl.toLocalFile()+"/.git/index");

    setStatus(StatusMessage::Loading, i18n("Loading local repository."));

    if(loadData())
        setStatus(StatusMessage::Ready, i18n("Ready."));

}

bool Project::loadData()
{
    const auto mUrl = QUrl::fromUserInput(m_url);
    QFileInfo fileInfo(mUrl.toLocalFile());

    m_logo = ProjectManager::projectLogo(mUrl);
    Q_EMIT this->logoChanged(m_logo);

    m_readmeFile = ProjectManager::readmeFile(mUrl);
    Q_EMIT this->readmeFileChanged(m_readmeFile);

    m_title = fileInfo.fileName();
    Q_EMIT this->titleChanged(m_title);

    m_currentBranch = m_manager->currentBranch();
    Q_EMIT this->currentBranchChanged(m_currentBranch);

    this->setHeadBranch();

    m_filesStatus = m_manager->repoFilesStatus();

    return true;
}

QString Project::getTitle() const
{
    return m_title;
}

QUrl Project::getLogo() const
{
    return m_logo;
}

QString Project::currentBranch() const
{
    return m_currentBranch;
}

Git::LogsModel *Project::getCommitsModel()
{
    if(!m_manager->isValid())
        return nullptr;

    return m_manager->logsModel();
}

QStringList Project::repoStatus() const
{
    return m_repoStatus;
}

QUrl Project::readmeFile() const
{
    return m_readmeFile;
}

Git::RemotesModel *Project::remotesModel() const
{
    if(!m_manager->isValid())
        return nullptr;

    qDebug() << "ASKIGN FOR THE RMEOTES MODEL" << m_manager->remotesModel()->rowCount(QModelIndex());
    return m_manager->remotesModel();
}


QVariantMap Project::getHeadBranch() const
{
    return m_headBranch;
}

void Project::setUrl(const QString &url)
{
    qDebug() << "SET URL " << url;
    if (m_url == url)
        return;

    m_url = url;
    Q_EMIT urlChanged(m_url);
}

void Project::setCurrentBranch(const QString &currentBranch)
{
    if (m_currentBranch == currentBranch)
        return;

    m_currentBranch = currentBranch;
    Q_EMIT currentBranchChanged(m_currentBranch);
}

static QString statusIcon(Git::FileStatus::Status status)
{
    switch (status) {
    case Git::FileStatus::Status::Added:
        return QStringLiteral("git-status-added");
    case Git::FileStatus::Status::Ignored:
        return QStringLiteral("git-status-ignored");
    case Git::FileStatus::Status::Modified:
        return QStringLiteral("git-status-modified");
    case Git::FileStatus::Status::Removed:
        return QStringLiteral("git-status-removed");
    case Git::FileStatus::Status::Renamed:
        return QStringLiteral("git-status-renamed");
    case Git::FileStatus::Status::Unknown:
    case Git::FileStatus::Status::Untracked:
        return QStringLiteral("git-status-unknown");
    case Git::FileStatus::Status::Copied:
    case Git::FileStatus::Status::UpdatedButInmerged:
    case Git::FileStatus::Status::Unmodified:
        return QStringLiteral("git-status-update");
    default:
        qWarning() << "Unknown icon" ;
    }
    return QStringLiteral("git-status-update");
}


QString Project::fileStatusIcon(const QString &file)
{
    if(!m_manager->isValid())
    {
        return "error";
    }

    auto url = QUrl::fromUserInput(m_url);

    if(!url.isParentOf(QUrl::fromUserInput(file)))
    {
        return "love";
    }

    QFileInfo info(QUrl::fromUserInput(file).toLocalFile());
    if(info.isDir())
    {
        Git::FileStatus::Status status = Git::FileStatus::Unmodified;
        for (const auto &s : std::as_const(m_filesStatus))
        {
            const auto filePath = m_manager->path() + QLatin1Char('/') + s.name();

            if (!filePath.startsWith(info.absoluteFilePath()))
            {
                continue;
            }

            if (status == Git::FileStatus::Unmodified)
            {
                status = s.status();
            } else if (status != s.status()) {
                return statusIcon(Git::FileStatus::Modified);
            }
        }

        return statusIcon(status);
    }

    auto relativeUrl = QString(file).replace(url.toString()+"/", "");

    auto result = std::find_if(m_filesStatus.constBegin(), m_filesStatus.constEnd(), [relativeUrl](const Git::FileStatus &file)
    {
        return relativeUrl == file.name();
    });

    if(result != m_filesStatus.constEnd())
    {
        return statusIcon((*result).status());
    }
    //    return Git::statusIcon()
    return statusIcon(Git::FileStatus::Unmodified);

}

QString Project::createHashLink(const QString &hash) const
{
    // TODO: remove also this one
    auto log = m_manager->logsModel()->findLogByHash(hash);
    if (!log)
        return {};

    return QStringLiteral(R"(<a href="hash:%1">%2</a> )").arg(log->commitHash(), log->subject());
}


QVariantMap Project::commitAuthor(const QString &id)
{
    QVariantMap res;
    if(!m_manager->isValid())
        return res;

    auto commit = m_manager->logsModel()->findLogByHash(id, Git::LogsModel::LogMatchType::BeginMatch);

    if(!commit)
        return res;

    res.insert("message", commit->body());
    res.insert("shortMessage", commit->subject());

    res.insert("name", commit->authorName());
    res.insert("fullName", commit->committerName());
    res.insert("email", commit->authorEmail());
    res.insert("date", commit->commitDate());
    res.insert("branch", commit->branch());

    QStringList parentHashHtml;
    for (const auto &parent : commit->parents())
        parentHashHtml.append(createHashLink(parent));

    QStringList childsHashHtml;
    for (const auto &child : commit->childs())
        childsHashHtml.append(createHashLink(child));

    res.insert("parentCommits", parentHashHtml);
    res.insert("childCommits", childsHashHtml);

    auto files = m_manager->changedFiles(id);

    QVariantList filesHtml;

    for (auto i = files.constBegin(); i != files.constEnd(); ++i)
    {
        QString color;
        switch (i.value()) {
        case Git::ChangeStatus::Modified:
            color = "orange";
            break;
        case Git::ChangeStatus::Added:
            color = "green";
            break;
        case Git::ChangeStatus::Removed:
            color = "red";
            break;

        case Git::ChangeStatus::Unknown:
        case Git::ChangeStatus::Unmodified:
        case Git::ChangeStatus::Renamed:
        case Git::ChangeStatus::Copied:
        case Git::ChangeStatus::UpdatedButInmerged:
        case Git::ChangeStatus::Ignored:
        case Git::ChangeStatus::Untracked:
            break;
        }

        filesHtml << QVariantMap {{"color", color}, {"url",i.key()}};
    }


    res.insert("changedFiles", filesHtml);



    qDebug() << "CHANGED FILES" << childsHashHtml;
    return res;
}

QVariantMap Project::remoteInfo(const QString &remoteName)
{
    QVariantMap res;
    if(!m_manager->isValid())
        return res;


    auto remote = m_manager->remoteDetails(remoteName);

    res.insert("name", remote.name);
    res.insert("url", remote.pushUrl);

    return res;
}

void Project::clone(const QString &url)
{
    const auto mUrl = QUrl::fromUserInput(m_url);
    QDir dir (mUrl.toLocalFile());
    if(!dir.exists())
    {
        if(!dir.mkpath("."))
        {
            setStatus(StatusMessage::Error, i18n("Failed to create directory for repository."));
            return;
        }else
        {
            m_title = dir.dirName();
            Q_EMIT titleChanged(m_title);
        }
    }else
    {
        QDir gitDir (mUrl.toLocalFile()+"/.dir");
        if(gitDir.exists())
        {
            return;
        }
    }

    auto op = [remoteUrl = url, where = mUrl]()
    {

        Git::CloneCommand cmd;
        cmd.setRepoUrl(remoteUrl);
        cmd.setLocalPath(where.toLocalFile());
        Git::run(where.toLocalFile(), cmd);
    };

    m_cloneWatcher = new QFutureWatcher<void>;

    connect(m_cloneWatcher, &QFutureWatcher<void>::finished, [this]()
    {
        qDebug() << "Setting data" << m_url;
        this->setData(m_url);
    });

    auto future = QtConcurrent::run(op);
    m_cloneWatcher->setFuture(future);

    setStatus(StatusMessage::Loading, i18n("Start cloning %1 repository.", m_title));
}

void Project::checkout(const QString &target, const QString &remote, bool force, Git::CommandSwitchBranch::Mode mode)
{
    auto cmd = new Git::CommandSwitchBranch(m_manager);

    cmd->setTarget(target);
    cmd->setMode(mode);
    cmd->setForce(force);
    cmd->setRemoteBranch(remote);

    auto runner = new ActionRunner(m_manager->path());
    connect(runner, &ActionRunner::actionFished, [this, runner](bool ok, const QString &message)
    {
        Q_EMIT actionFinished(ok, message);
        runner->deleteLater();
    });
    runner->run(cmd);
}

void Project::stash()
{
    auto runner = new ActionRunner(m_manager->path());
    connect(runner, &ActionRunner::actionFished, [this, runner](bool ok, const QString &message)
    {
        Q_EMIT actionFinished(ok, message);
        runner->deleteLater();
    });
    runner->run({"stash"});
}

void Project::stashPop()
{
    auto runner = new ActionRunner(m_manager->path());
    connect(runner, &ActionRunner::actionFished, [this, runner](bool ok, const QString &message)
    {
        Q_EMIT actionFinished(ok, message);
        runner->deleteLater();
    });
    runner->run({"stash pop"});
}

void Project::pull(const QString &remote, const QString &branch, Git::CommandPull::Rebase rebase, Git::CommandPull::FastForward fastforward, bool squash, bool noCommit, bool prune, bool tags)
{
    auto cmd = new Git::CommandPull();

    cmd->setRemote(remote);
    cmd->setBranch(branch);
    cmd->setRebase(rebase);
    cmd->setFastForward(fastforward);
    cmd->setSquash(squash);
    cmd->setNoCommit(noCommit);
    cmd->setPrune(prune);
    cmd->setTags(tags);

    auto runner = new ActionRunner(m_manager->path());
    connect(runner, &ActionRunner::actionFished, [this, runner](bool ok, const QString &message)
            {
                Q_EMIT actionFinished(ok, message);
                runner->deleteLater();
            });
    runner->run(cmd);
}

StatusMessage* Project::status() const
{
    return m_status;
}

QStringList Project::allBranches() const
{
    if(!m_manager->isValid())
        return {};

    return m_manager->branches();
}

QStringList Project::remoteBranches() const
{
    if(!m_manager->isValid())
        return {};

    return m_manager->remotes();
}


void Project::setHeadBranch()
{
    if(!m_manager->isValid())
        return;


    m_headBranch.clear();
    auto headRef = m_manager->currentBranch();

    m_headBranch.insert("name", headRef);
    m_headBranch.insert("remote", m_manager->currentRemote());
    //        m_headBranch.insert("isLocal", headRef.isLocal());
    //        m_headBranch.insert("isRemote", headRef.isRemote());
    //        m_headBranch.insert("prefix", headRef.prefix());
    //        m_headBranch.insert("upstreamRemoteName", headRef.upstreamRemoteName(r));
    //        m_headBranch.insert("upstreamName", headRef.upstreamName(r));
    //        m_headBranch.insert("isCurrentBranch", headRef.isCurrentBranch());



    Q_EMIT headBranchChanged(m_headBranch);
}

void Project::setStatus(StatusMessage::StatusCode code, const QString &message)
{
    m_status->code = code;
    m_status->message = message;
    Q_EMIT statusChanged();
}


StatusMessage::StatusMessage(QObject *parent) : QObject(parent)
  ,code(StatusCode::Ready)
  ,message(i18n("Nothing to see."))
{

}
