#ifndef PROJECTSMODEL_H
#define PROJECTSMODEL_H

#include <MauiKit/Core/mauilist.h>

class ProjectsModel : public MauiList
{
    Q_OBJECT
public:
    ProjectsModel(QObject *parent = nullptr);

    void componentComplete() override final;

    const FMH::MODEL_LIST &items() const override final;

    void setList(const FMH::MODEL_LIST &list);

public slots:
    void insert(const FMH::MODEL &item);

private:
    FMH::MODEL_LIST m_list;
};

#endif // PROJECTSMODEL_H
