#include "branchesmanager.h"

BranchesManager::BranchesManager(QObject *parent) : QObject(parent)
{

}

void BranchesManager::setRepo(Git::Repository &repo)
{
    m_repo = repo;
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
