#include "project.h"
#include "projectmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QColor>

#include <QtConcurrent>
#include <QFuture>

#include <KI18n/KLocalizedString>


#include <libkommit/commands/commandclone.h>
#include <libkommit/models/logsmodel.h>
#include <libkommit/gitglobal.h>
#include <libkommit/gitlog.h>

#include <libkommit/gitmanager.h>

Project::Project(QObject *parent) : QObject(parent)
  ,m_manager(new Git::Manager())
  ,m_cloneWatcher(nullptr)
  ,m_status(new StatusMessage(this))
{
    //    qRegisterMetaType<Status>("Status"); // this is needed for QML to know of WindowDecorations

    connect(this, &Project::urlChanged, this, &Project::setData);
    connect(this, &Project::currentBranchChanged, this, &Project::setCurrentBranchRemote);
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
        setStatus(StatusMessage::Error, url);
        return;
    }

    setStatus(StatusMessage::Loading, i18n("Loading local repository."));

    m_logo = ProjectManager::projectLogo(mUrl);
    Q_EMIT this->logoChanged(m_logo);

    m_readmeFile = ProjectManager::readmeFile(mUrl);
    Q_EMIT this->readmeFileChanged(m_readmeFile);

    m_title = fileInfo.fileName();
    Q_EMIT this->titleChanged(m_title);

    m_currentBranch = m_manager->currentBranch();
    Q_EMIT this->currentBranchChanged(m_currentBranch);

    this->setHeadBranch();

    qDebug() << "status states";

    //    auto remotes = m_manager->remotes();

    //        for(const auto &remote : remotes)
    //        {
    //            auto info = m_manager->remoteDetails(remote);
    //            m_remotesModel.append(QVariantMap{{"name", info.name}, {"url", info.fetchUrl}});
    //        }
    //        Q_EMIT remotesModelChanged(m_remotesModel);



    setStatus(StatusMessage::Ready, i18n("Ready."));

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


QVariantList Project::getRemotesModel() const
{
    return m_remotesModel;
}

QVariantMap Project::currentBranchRemote() const
{
    return m_currentBranchRemote;
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

    if(QFileInfo(QUrl::fromUserInput(file).toLocalFile()).isDir())
    {
        return "folder";
    }

    auto relativeUrl = QString(file).replace(url.toString()+"/", "");
    qDebug() << "GET STATUS FROM RELATIVE URL" <<  relativeUrl << m_url << file;

    //    auto files = m_manager->repoFilesStatus();



    //    return Git::statusIcon()
    return "question";
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

void Project::pull()
{

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

    return m_manager->remoteBranches();
}

void Project::setCurrentBranchRemote(const QString &currentBranch)
{
    if(!m_manager->isValid())
        return;


    //    m_currentBranchRemote.clear();
    //    auto branchRef = m_repo.branchRef(r, m_currentBranch);
    //    if(r)
    //    {
    //        auto remoteName = branchRef.upstreamRemoteName(r);

    //        if(r)
    //        {
    //            m_currentBranchRemote = remoteInfo(remoteName);

    //        }

    //        if(!r)
    //        {
    //            qDebug() << "Could nto find current branch remote name";
    //        }
    //    }

    //    Q_EMIT this->currentBranchRemoteChanged(m_currentBranchRemote);

}

void Project::setHeadBranch()
{
    if(!m_manager->isValid())
        return;


    m_headBranch.clear();
    auto headRef = m_manager->currentBranch();

    m_headBranch.insert("name", headRef);
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
