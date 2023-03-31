#include "project.h"
#include "projectmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include <QtConcurrent>
#include <QFuture>

#include <KI18n/KLocalizedString>

#include "controllers/branchesmanager.h"

#include "libGitWrap/Signature.hpp"
#include "libGitWrap/Commit.hpp"
#include "libGitWrap/ObjectId.hpp"
#include "libGitWrap/Remote.hpp"
#include "libGitWrap/BranchRef.hpp"

#include "libGitWrap/Operations/CheckoutOperation.hpp"
#include "libGitWrap/Operations/RemoteOperations.hpp"

#include <qgit2.h>
#include <qgit2/qgitglobal.h>
#include <qgit2/qgitcommit.h>
#include <qgit2/qgitrepository.h>
#include <qgit2/qgitcredentials.h>

#include <qgit2/qgitexception.h>


Project::Project(QObject *parent) : QObject(parent)
  ,m_commitsModel(nullptr)
  ,m_branchesManager(nullptr)
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
}

QString Project::url() const
{
    return m_url;
}

void Project::setData(const QString &url)
{    
    const auto mUrl = QUrl::fromUserInput(url);

    if(!QFileInfo(mUrl.toLocalFile()).exists())
    {
        setStatus(StatusMessage::Error, i18n("Directory does not exists."));
        return;
    }

    m_repo = ProjectManager::gitDir(mUrl);

    if(!m_repo.isValid())
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

    m_title = m_repo.name();
    Q_EMIT this->titleChanged(m_title);

    Git::Result r;
    m_currentBranch = m_repo.currentBranch(r);

    if ( !r )
    {
        qDebug()  << "Unable to get repo current branch" << r.errorText();

    }else
    {
        Q_EMIT this->currentBranchChanged(m_currentBranch);
    }

    this->setHeadBranch();

    qDebug() << "status states";

    auto remotes = m_repo.allRemotes(r);
    if(r)
    {
        for(const auto &remote : remotes)
        {
            m_remotesModel.append(QVariantMap{{"name", remote.name()}, {"url", remote.url()}, {"isValid", remote.isValid()}});
        }
        Q_EMIT remotesModelChanged(m_remotesModel);
    }

    auto status = m_repo.status(r);

    if ( !r )
    {
        qDebug()  << "Unable to get repo status" << r.errorText();
    }else
    {
        qDebug() << "PROJECT STATUS" << status.keys();
        qDebug() << status.keys();
        m_repoStatus = status.keys();
    }


    //    auto head = m_repo.HEAD(r);
    //    if(r)
    //    {
    //        head.
    //    }
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

CommitHistoryModel *Project::getCommitsModel()
{
    if(!m_commitsModel)
    {
        m_commitsModel = new CommitHistoryModel(this);
        m_commitsModel->setRepo(this->m_repo);
    }

    return m_commitsModel;
}

BranchesManager *Project::getBranches()
{
    if(!m_branchesManager)
    {
        m_branchesManager = new BranchesManager(this);
        m_branchesManager->setRepo(this->m_repo);
    }
    return m_branchesManager;
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
    if(!m_repo.isValid())
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

    Git::Result r;

    Git::StatusFlags status = m_repo.status(r, relativeUrl);

    if(status.testFlag(Git::Status::FileInvalidStatus))
    {
        return "vcs-conflicting";
    }


    if(status.testFlag(Git::Status::FileUnchanged))
    {
        return "vcs-normal";
    }

    if(status.testFlag(Git::Status::FileIgnored))
    {
        return "object-hidden";
    }

    if(status.testFlag(Git::Status::FileWorkingTreeNew) || status.testFlag(Git::Status::FileIndexNew))
    {
        return "vcs-added";
    }

    if(status.testFlag(Git::Status::FileWorkingTreeModified) ||status.testFlag(Git::Status::FileIndexModified) || status.testFlag(Git::Status::FileWorkingTreeTypeChange) || status.testFlag(Git::Status::FileIndexTypeChange) || status.testFlag(Git::Status::FileIndexRenamed))
    {
        return "vcs-locally-modified";
    }

    if(status.testFlag(Git::Status::FileWorkingTreeDeleted) || status.testFlag(Git::Status::FileIndexDeleted))
    {
        return "vcs-removed";
    }

    return "question";
}

QVariantMap Project::commitAuthor(const QString &id)
{
    QVariantMap res;
    if(!m_repo.isValid())
        return res;

    Git::Result r;

    auto commit = m_repo.lookupCommit(r, Git::ObjectId::fromString(id));
    if(r)
    {
        res.insert("message", commit.message());
        res.insert("parentCommits", commit.numParentCommits());
        res.insert("shortMessage", commit.shortMessage());

        auto author = commit.author(r);
        if(r)
        {
            res.insert("name", author.name());
            res.insert("fullName", author.fullName());
            res.insert("email", author.email());
            res.insert("date", author.when());
        }
    }
    return res;
}

QVariantMap Project::remoteInfo(const QString &remoteName)
{
    QVariantMap res;
    if(!m_repo.isValid())
        return res;

    Git::Result r;

    auto remote = m_repo.remote(r, remoteName);
    if(r)
    {
        res.insert("name", remote.name());
        res.insert("url", remote.url());
    }
    return res;
}

void Project::pull()
{
    if(!m_repo.isValid())
    {
        return;
    }

    qDebug() << "PULLIN OP";

    auto op = new Git::FetchOperation(m_repo);
    //    op->setRepository(m_repo);
    //    op->setMode(Git::CheckoutSafe);
    //    op->setBackgroundMode(true);
    op->execute();
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
        }
    }else
    {
        m_repo = ProjectManager::gitDir(mUrl);
        if(m_repo.isValid())
        {
            return;
        }
    }

    auto op = [remoteUrl = url, where = mUrl]()
    {
        LibQGit2::initLibQGit2();
    LibQGit2::Repository op;
    op.setRemoteCredentials("origin",  LibQGit2::Credentials());

    try {
        op.clone(remoteUrl, where.toLocalFile());
    }
    catch (const LibQGit2::Exception& ex) {
        qDebug() << ex.what() << ex.category();
    }
    };

    m_cloneWatcher = new QFutureWatcher<void>;

    connect(m_cloneWatcher, &QFutureWatcher<void>::finished, [this]()
    {
       this->setData(m_url);
    });

    auto future = QtConcurrent::run(op);
    m_cloneWatcher->setFuture(future);

    setStatus(StatusMessage::Loading, i18n("Start cloning new repo."));

}


StatusMessage* Project::status() const
{
    return m_status;
}

void Project::setCurrentBranchRemote(const QString &currentBranch)
{
    if(!m_repo.isValid())
        return;

    Git::Result r;
    m_currentBranchRemote.clear();
    auto branchRef = m_repo.branchRef(r, m_currentBranch);
    if(r)
    {
        auto remoteName = branchRef.upstreamRemoteName(r);

        if(r)
        {
            m_currentBranchRemote = remoteInfo(remoteName);

        }

        if(!r)
        {
            qDebug() << "Could nto find current branch remote name";
        }
    }

    Q_EMIT this->currentBranchRemoteChanged(m_currentBranchRemote);

}

void Project::setHeadBranch()
{
    if(!m_repo.isValid())
        return;

    Git::Result r;
    m_headBranch.clear();
    auto headRef = m_repo.headBranch(r);
    if(r)
    {
        m_headBranch.insert("name", headRef.name());
        m_headBranch.insert("isLocal", headRef.isLocal());
        m_headBranch.insert("isRemote", headRef.isRemote());
        m_headBranch.insert("prefix", headRef.prefix());
        m_headBranch.insert("upstreamRemoteName", headRef.upstreamRemoteName(r));
        m_headBranch.insert("upstreamName", headRef.upstreamName(r));
        m_headBranch.insert("isCurrentBranch", headRef.isCurrentBranch());

    }

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
