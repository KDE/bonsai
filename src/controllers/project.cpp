#include "project.h"
#include "projectmanager.h"
#include <QDebug>
#include <QFileInfo>

#include "controllers/branchesmanager.h"

#include "libGitWrap/Signature.hpp"
#include "libGitWrap/Commit.hpp"
#include "libGitWrap/ObjectId.hpp"
#include "libGitWrap/Remote.hpp"
#include "libGitWrap/BranchRef.hpp"

Project::Project(QObject *parent) : QObject(parent)
  , m_commitsModel(nullptr)
  ,m_branchesManager(nullptr)
{
    connect(this, &Project::urlChanged, this, &Project::setData);
    connect(this, &Project::currentBranchChanged, this, &Project::setCurrentBranchRemote);
}

QUrl Project::url() const
{
    return m_url;
}

void Project::setData(const QUrl &url)
{
    m_repo = ProjectManager::gitDir(url);

    m_logo = ProjectManager::projectLogo(url);
    emit this->logoChanged(m_logo);

    m_readmeFile = ProjectManager::readmeFile(url);
    emit this->readmeFileChanged(m_readmeFile);

    if(!m_repo.isValid())
    {
        emit error("URL is nto a valid repo");
        return;
    }

    m_title = m_repo.name();
    emit this->titleChanged(m_title);

    Git::Result r;
    m_currentBranch = m_repo.currentBranch(r);

    if ( !r )
    {
        qDebug()  << "Unable to get repo current branch" << r.errorText();
    }else
    {
        emit this->currentBranchChanged(m_currentBranch);
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
        emit remotesModelChanged(m_remotesModel);
    }

    auto status = m_repo.status(r);

    if ( !r )
    {
        qDebug()  << "Unable to get repo status" << r.errorText();
    }else
    {
        qDebug() << "PROJECT STATUS" << status.keys();
        qDebug() << status.keys();
        m_status = status.keys();
    }


    //    auto head = m_repo.HEAD(r);
    //    if(r)
    //    {
    //        head.
    //    }

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

QStringList Project::status() const
{
    return m_status;
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

void Project::setUrl(QUrl url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(m_url);
}

void Project::setCurrentBranch(const QString &currentBranch)
{
    if (m_currentBranch == currentBranch)
        return;

    m_currentBranch = currentBranch;
    emit currentBranchChanged(m_currentBranch);
}

QString Project::fileStatusIcon(const QString &file)
{
    if(!m_repo.isValid())
    {
        return "error";
    }

    if(!m_url.isParentOf(QUrl::fromUserInput(file)))
    {
        return "love";
    }

    if(QFileInfo(QUrl::fromUserInput(file).toLocalFile()).isDir())
    {
        return "folder";
    }

    auto relativeUrl = QString(file).replace(m_url.toString()+"/", "");
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

    emit this->currentBranchRemoteChanged(m_currentBranchRemote);

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

    emit headBranchChanged(m_headBranch);

}

