#pragma once

#include <MauiKit3/Core/mauilist.h>

class ProjectsModel : public MauiList
{
    Q_OBJECT
public:
    ProjectsModel(QObject *parent = nullptr);

    void componentComplete() override final;

    const FMH::MODEL_LIST &items() const override final;

    void setList(const FMH::MODEL_LIST &list);

public Q_SLOTS:
    void insert(const FMH::MODEL &item);

private:
    FMH::MODEL_LIST m_list;
};

