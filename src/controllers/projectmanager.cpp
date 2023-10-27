#include "projectmanager.h"
#include <QDebug>
#include <MauiKit3/FileBrowsing/fmstatic.h>

#include <QSettings>

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

QUrl ProjectManager::projectLogo(const QUrl &url)
{
    QUrl res;
    auto logo = url.toString() + "/logo.png";

    if(FMStatic::fileExists(QUrl(logo)))
    {
       res = QUrl(logo);
    }

    return res;
}

QUrl ProjectManager::readmeFile(const QUrl &url)
{
    QUrl res;
    auto readme = url.toString() + "/README.md";

    if(FMStatic::fileExists(QUrl(readme)))
    {
       res = QUrl(readme);
    }

    return res;
}

void ProjectManager::addProject(const QString &url)
{
    const QUrl localUrl = QUrl::fromUserInput(url);
    if(isGitDir(localUrl.toString()))
    {
        qDebug() << "Prtoject exists and it is a directory" << url;

        if(this->saveHistory(localUrl))
        {
            if(m_projectsModel)
                m_projectsModel->insert(repoInfo(localUrl));

        }
    }
}

void ProjectManager::addRemoteProject(const QString &remoteUrl, const QString &localUrl)
{

}

bool ProjectManager::isGitDir(const QString &url)
{
    if(FMH::fileExists(url))
    {
       if(FMH::fileExists(url+"/.git"))
           return true;
    }

    return false;
}

FMH::MODEL ProjectManager::repoInfo(const QUrl &url)
{
    FMH::MODEL res = FMStatic::getFileInfoModel(url);

    res[FMH::MODEL_KEY::ARTWORK] = ProjectManager::projectLogo(url).toString();


    return res;
}

GlobalSettings *ProjectManager::settings()
{
    return GlobalSettings::instance();
}

bool ProjectManager::saveHistory(const QUrl &url)
{
    auto urls = this->loadHistory();
    if (urls.contains(url))
        return false;

    urls << url;
    QSettings settings;
	settings.beginGroup("HISTORY");
settings.setValue("URLS", QUrl::toStringList(urls));
settings.endGroup();	
    return true;
}

QList<QUrl> ProjectManager::loadHistory()
{
 QSettings settings;
	settings.beginGroup("HISTORY");
    auto urls = settings.value("URLS", QStringList()).toStringList();
settings.endGroup();	

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
        if(isGitDir(url.toString()))
        {
            res << repoInfo(url);
        }
    }
    return res;
}

QString GlobalSettings::cloneDir() const
{
    return m_cloneDir;
}

void GlobalSettings::setCloneDir(QString cloneDir)
{
    if (m_cloneDir == cloneDir)
        return;

    m_cloneDir = cloneDir;

    m_settings->beginGroup("General");
    m_settings->setValue("CloneDir", m_cloneDir);
    m_settings->endGroup();
    m_settings->sync();

    emit cloneDirChanged(m_cloneDir);
}

GlobalSettings::GlobalSettings() : QObject(nullptr)
  ,m_settings(new QSettings(this))
  ,m_cloneDir(FMStatic::HomePath)
{
    m_settings->beginGroup("General");
    m_cloneDir = m_settings->value("CloneDir", m_cloneDir).toString();

    m_settings->endGroup();
}
