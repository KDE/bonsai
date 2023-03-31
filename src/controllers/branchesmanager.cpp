#include "branchesmanager.h"
#include "libGitWrap/BranchRef.hpp"
#include "libGitWrap/Reference.hpp"

BranchesManager::BranchesManager(QObject *parent) : QObject(parent)
{

}

void BranchesManager::setRepo(Git::Repository &repo)
{
    m_repo = repo;
    Q_EMIT repoChanged();
}

QStringList BranchesManager::allBranches()
{
    QStringList res;
    Git::Result r;

    if(m_repo.isValid())
    {
        res = m_repo.allBranchNames(r);
    }

    return res;
}

QStringList BranchesManager::localBranches()
{
    QStringList res;
    Git::Result r;

    if(m_repo.isValid())
    {
        res = m_repo.branchNames(r, true, false);
    }

    return res;
}

QStringList BranchesManager::remoteBranches()
{
    QStringList res;
    Git::Result r;

    if(m_repo.isValid())
    {
        res = m_repo.branchNames(r, false, true);
    }

    return res;
}

QString BranchesManager::upstreamRemote(const QString &branchName)
{
    QString res;
    Git::Result r;

    if(m_repo.isValid())
    {
        auto branch = m_repo.branchRef(r, branchName);
        if(r)
        {
            res = branch.upstreamRemoteName(r);
        }
    }
    return res;
}

QString BranchesManager::upstream(const QString &branchName)
{
    QString res;
    Git::Result r;

    if(m_repo.isValid())
    {
        auto branch = m_repo.branchRef(r, branchName);
        if(r)
        {
            res = branch.upstreamName(r);
        }
    }
    return res;
}
