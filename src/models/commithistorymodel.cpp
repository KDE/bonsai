#include "commithistorymodel.h"
#include <QDebug>
#include "libGitWrap/ObjectId.hpp"

CommitHistoryModel::CommitHistoryModel(QObject * parent) :MauiList(parent)
{

}

void CommitHistoryModel::setRepo(Git::Repository &repo)
{
    m_repo = repo;
    Git::Result r;

    auto walker = Git::RevisionWalker::create(r, m_repo);

    if(r)
    {
        walker.setSorting(r, true, true);

        if(r)
        {
            walker.pushHead(r);

            if(r)
            {
                auto ids = walker.all(r);

                if(r)
                {
                    emit this->preListChanged();
                    for(const auto &id : ids)
                    {
                        auto commit = m_repo.lookupCommit(r, id);

                        if(r)
                        {
                            qDebug()  << commit.message() << id.toString();
                            this->m_list << FMH::MODEL {{FMH::MODEL_KEY::ID, id.toString(Git::ObjectId::SHA1_Length)}, {FMH::MODEL_KEY::MESSAGE, commit.message()}, {FMH::MODEL_KEY::AUTHOR, commit.author().name()},  {FMH::MODEL_KEY::EMAIL, commit.author().email()}};

                        }
                    }

                    emit this->postListChanged();
                }
            }
        }
    }
}

void CommitHistoryModel::componentComplete()
{
}

const FMH::MODEL_LIST &CommitHistoryModel::items() const
{
    return m_list;
}
