#include "commithistorymodel.h"
#include <QDebug>
#include "libGitWrap/ObjectId.hpp"
#include "libGitWrap/BranchRef.hpp"

#include "controllers/project.h"

CommitHistoryModel::CommitHistoryModel(Project *parent) : MauiList(parent)
  ,m_project(parent)
{
    connect(m_project, &Project::currentBranchChanged, [this](QString)
    {
        this->setData();
    });

}

void CommitHistoryModel::setRepo(Git::Repository &repo)
{
    m_repo = repo;
    this->setData();
}

const FMH::MODEL_LIST &CommitHistoryModel::items() const
{
    return m_list;
}

void CommitHistoryModel::setData()
{
    if(!m_repo.isValid())
        return;

    Git::Result r;

    auto walker = Git::RevisionWalker::create(r, m_repo);

    if(r)
    {
        walker.setSorting(r, true, true);
        if(r)
        {
            auto ref =  m_repo.branchRef(r, m_project->currentBranch());
            if(!r)
            {
                return;
            }

            walker.pushRef(r, ref.name());

            if(r)
            {
                auto ids = walker.all(r);

                if(r)
                {
                    this->m_list.clear();
                    emit this->preListChanged();
                    for(const auto &id : ids)
                    {
                        auto commit = m_repo.lookupCommit(r, id);

                        if(r)
                        {
                            qDebug()  << commit.message() << id.toString();
                            this->m_list << FMH::MODEL {{FMH::MODEL_KEY::ID, id.toString(Git::ObjectId::SHA1_Length)}, {FMH::MODEL_KEY::MESSAGE, commit.message()}, {FMH::MODEL_KEY::AUTHOR, commit.author().name()}, {FMH::MODEL_KEY::EMAIL, commit.author().email()}, {FMH::MODEL_KEY::DATE, commit.author().when().toString()}};

                        }
                    }

                    emit this->postListChanged();
                }
            }
        }
    }
}
