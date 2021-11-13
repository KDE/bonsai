#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"

#include "models/projectsmodel.h"
#include "controllers/project.h"

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProjectsModel *projectsModel READ projectsModel FINAL)

public:
    explicit ProjectManager(QObject *parent = nullptr);

    ProjectsModel *projectsModel();

public slots:
    Project* openProject(const QString &url);

private:
    ProjectsModel * m_projectsModel;

    Git::Repository gitDir(const QUrl &url);
    FMH::MODEL repoInfo(const QUrl &url, const Git::Repository &repo);

    bool saveHistory(const QUrl &url);
    QList<QUrl> loadHistory();
    FMH::MODEL_LIST reposData(const QList<QUrl> &urls);

signals:

};

#endif // PROJECTMANAGER_H
