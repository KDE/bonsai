#include "projectmanager.h"
#include <QDebug>
#include <MauiKit/FileBrowsing/fmstatic.h>
#include <MauiKit/Core/utils.h>


ProjectManager::ProjectManager(QObject *parent) : QObject(parent)
  ,m_projectsModel(nullptr)
{

}

ProjectsModel *ProjectManager::projectsModel()
{
    if(m_projectsModel)
        return m_projectsModel;

    m_projectsModel = new ProjectsModel(this);
    m_projectsModel->setList(reposData(this->loadHistory()));
    return m_projectsModel;
}

Project *ProjectManager::openProject(const QString &url)
{
    const QUrl localUrl = QUrl::fromUserInput(url, "/", QUrl::AssumeLocalFile);
    auto repo = gitDir(localUrl);
    if(repo.isValid())
    {
        qDebug() << "Prtoject exists and it is a directory" << repo.name();

        if(this->saveHistory(localUrl))
        {
            m_projectsModel->insert(repoInfo(localUrl, repo));
        }

        return new Project(localUrl, repo, this);
    }

    return nullptr;
}

Git::Repository ProjectManager::gitDir(const QUrl &url)
{
    Git::Repository repo;

    if(FMStatic::fileExists(url))
    {
        if(FMStatic::isDir(url))
        {
            Git::Result r;
            repo = Git::Repository::open(r, url.toLocalFile() );
            if ( !r )
            {
                qDebug()  << "Unable to open repository at %s." << url << r.errorText();
            }

            qDebug() << "repo ok << " << repo.name();
        }
    }

    return repo;
}

FMH::MODEL ProjectManager::repoInfo(const QUrl &url, const Git::Repository &repo)
{
    FMH::MODEL res = FMStatic::getFileInfoModel(url);

    res[FMH::MODEL_KEY::TITLE] = repo.name();

    auto logo = url.toString() + "/logo.png";

    if(FMStatic::fileExists(QUrl(logo)))
    {
       res[FMH::MODEL_KEY::ARTWORK] = logo;
    }

    return res;
}

bool ProjectManager::saveHistory(const QUrl &url)
{
    auto urls = this->loadHistory();
    if (urls.contains(url))
        return false;

    urls << url;

    UTIL::saveSettings("URLS", QUrl::toStringList(urls), "HISTORY");
    return true;
}

QList<QUrl> ProjectManager::loadHistory()
{
    auto urls = UTIL::loadSettings("URLS", "HISTORY", QStringList()).toStringList();
    urls.removeDuplicates();
    auto res = QUrl::fromStringList(urls);
    res.removeAll(QString(""));
    return res;
}

FMH::MODEL_LIST ProjectManager::reposData(const QList<QUrl> &urls)
{
    FMH::MODEL_LIST  res;

    for(const auto &url : urls)
    {
        auto repo = this->gitDir(url);

        if(repo.isValid())
        {
            res << repoInfo(url, repo);
        }
    }
    return res;
}
