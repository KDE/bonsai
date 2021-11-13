#include "projectmanager.h"
#include <QDebug>
#include <MauiKit/FileBrowsing/fmstatic.h>


ProjectManager::ProjectManager(QObject *parent) : QObject(parent)
  ,m_projectsModel(nullptr)
{

}

ProjectsModel *ProjectManager::projectsModel()
{
    if(m_projectsModel)
        return m_projectsModel;

    m_projectsModel = new ProjectsModel(this);
    return m_projectsModel;
}

void ProjectManager::openProject(const QString &url)
{
    const QUrl localUrl = QUrl::fromUserInput(url, "/", QUrl::AssumeLocalFile);
    auto repo = gitDir(localUrl);
    if(&repo)
    {
        qDebug() << "Prtoject exists and it is a directory" << repo.name();

        m_projectsModel->insert(repoInfo(localUrl, repo));
    }
}

Git::Repository ProjectManager::gitDir(const QUrl &url)
{
    if(FMStatic::fileExists(url))
    {
        if(FMStatic::isDir(url))
        {
            Git::Result r;
            Git::Repository repo = Git::Repository::open(r, url.toLocalFile() );
            if ( !r )
            {
                qDebug()  << "Unable to open repository at %s." << url << r.errorText();
                return nullptr;
            }

            qDebug() << "repo ok << " << repo.name();
            return repo;

        }
    }

    return nullptr;
}

FMH::MODEL ProjectManager::repoInfo(const QUrl &url, const Git::Repository &repo)
{
    FMH::MODEL res = FMStatic::getFileInfoModel(url);

    res[FMH::MODEL_KEY::TITLE] = repo.name();
    return res;
}

void ProjectManager::saveHistory(const QUrl &url)
{
    Q_UNUSED(url);
}

QList<QUrl> ProjectManager::loadHistory()
{
    QList<QUrl> res;
    return res;
}
