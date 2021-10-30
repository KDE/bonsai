#ifndef PROJECTSMODEL_H
#define PROJECTSMODEL_H

#include <MauiKit/Core/mauilist.h>

class ProjectsModel : public MauiList
{
    Q_OBJECT
public:
    ProjectsModel(QObject *parent = nullptr);
};

#endif // PROJECTSMODEL_H
