#include "project.h"
#include "projectmanager.h"
#include <QDebug>

#include "controllers/branchesmanager.h"


Project::Project(QObject *parent) : QObject(parent)
  , m_commitsModel(nullptr)
  ,m_branchesManager(nullptr)
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

    qDebug() << "status states";

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

void Project::setUrl(QUrl url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(m_url);
}

void Project::setCurrentBranch(QString currentBranch)
{
    if (m_currentBranch == currentBranch)
        return;

    m_currentBranch = currentBranch;
    emit currentBranchChanged(m_currentBranch);
}

