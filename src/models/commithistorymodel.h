#pragma once

#include <QObject>
#include <MauiKit3/Core/mauilist.h>
#include "libGitWrap/Repository.hpp"

class Project;
class CommitHistoryModel : public MauiList
{
    Q_OBJECT
public:
    CommitHistoryModel(Project *parent = nullptr);

    // QQmlParserStatus interface
    void setRepo(Git::Repository &repo);

public:
    const FMH::MODEL_LIST &items() const override final;

private:
    FMH::MODEL_LIST m_list;
    Git::Repository m_repo;
    Project *m_project;

    void setData();
};

