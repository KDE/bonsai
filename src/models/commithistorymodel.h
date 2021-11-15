#ifndef COMMITHISTORYMODEL_H
#define COMMITHISTORYMODEL_H

#include <QObject>
#include <MauiKit/Core/mauilist.h>
#include "libGitWrap/Repository.hpp"

class CommitHistoryModel : public MauiList
{
    Q_OBJECT
public:
    CommitHistoryModel(QObject *parent = nullptr);

    // QQmlParserStatus interface
    void setRepo(Git::Repository &repo);
public:
    void componentComplete() override final;
    const FMH::MODEL_LIST &items() const override final;

private:
    FMH::MODEL_LIST m_list;
    Git::Repository m_repo;
};

#endif // COMMITHISTORYMODEL_H
