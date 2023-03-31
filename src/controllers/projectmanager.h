#pragma once

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"

#include "models/projectsmodel.h"

class QSettings;
class GlobalSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cloneDir READ cloneDir WRITE setCloneDir NOTIFY cloneDirChanged)

public:
    static GlobalSettings *instance()
    {
        if(!m_instance)
        {
            m_instance = new GlobalSettings();
        }

        return m_instance;
    }

    QString cloneDir() const;

public Q_SLOTS:
    void setCloneDir(QString cloneDir);

Q_SIGNALS:
    void cloneDirChanged(QString cloneDir);

private:
    GlobalSettings();
    inline static GlobalSettings *m_instance = nullptr;
    QSettings *m_settings;
    QString m_cloneDir;
};

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProjectsModel *projectsModel READ projectsModel FINAL)
//    Q_PROPERTY(GlobalSettings* settings READ settings CONSTANT FINAL)

public:
    explicit ProjectManager(QObject *parent = nullptr);

    ProjectsModel *projectsModel();

    static QUrl projectLogo(const QUrl &url);
    static QUrl readmeFile(const QUrl &url);

    static Git::Repository gitDir(const QUrl &url);
    static FMH::MODEL repoInfo(const QUrl &url, Git::Repository &repo);

    static GlobalSettings* settings();

public Q_SLOTS:
    /**
     * @brief addProject
     * Adds an existing local project/repo
     * @param url
     * Directory path of the repository
     */
    void addProject(const QString &url);

    /**
     * @brief addRemoteProject
     * Adds a project/repo that needs to be cloned.
     * @param remoteUrl
     * Remote url of the reposotory to clone
     * @param localUrl
     * Where to clone the repository
     */
    void addRemoteProject(const QString &remoteUrl, const QString &localUrl);

private:
    ProjectsModel * m_projectsModel;

    bool saveHistory(const QUrl &url);
    QList<QUrl> loadHistory();
    FMH::MODEL_LIST reposData(const QList<QUrl> &urls);

    GlobalSettings* m_settings;

Q_SIGNALS:

};

