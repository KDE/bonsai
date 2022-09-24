#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"

#include "models/projectsmodel.h"

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProjectsModel *projectsModel READ projectsModel FINAL)

public:
    explicit ProjectManager(QObject *parent = nullptr);

    ProjectsModel *projectsModel();

    static QUrl projectLogo(const QUrl &url);
    static QUrl readmeFile(const QUrl &url);

    static Git::Repository gitDir(const QUrl &url);
    static FMH::MODEL repoInfo(const QUrl &url, Git::Repository &repo);

public slots:
    void addProject(const QString &url);

private:
    ProjectsModel * m_projectsModel;

    bool saveHistory(const QUrl &url);
    QList<QUrl> loadHistory();
    FMH::MODEL_LIST reposData(const QList<QUrl> &urls);

signals:

};

#endif // PROJECTMANAGER_H
