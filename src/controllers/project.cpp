#include "project.h"
#include "projectmanager.h"
#include <QDebug>

Project::Project(QObject *parent) : QObject(parent)
  , m_commitsModel(nullptr)
{
connect(this, &Project::urlChanged, this, &Project::setData);
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

    qDebug() << "references names";

    auto refs = m_repo.allReferenceNames(r);

    if ( !r )
    {
        qDebug()  << "Unable to get repo refs names" << r.errorText();
    }else
    {
        qDebug() << refs;
    }


    auto commit = m_repo.lookupCommit(r, "HEAD");

    if ( !r )
    {
        qDebug()  << "Unable to get repo commit" << r.errorText();
    }else
    {
        auto commits = commit.parentCommits(r);

        if(!r)
        {
            qDebug() << "failed to get commits";
        }else
        {
            qDebug() << "Commits" << commits;
        }
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

QStringList Project::getBranches() const
{
    return m_branches;
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

void Project::setUrl(QUrl url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(m_url);
}

