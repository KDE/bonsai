#include "project.h"
#include "projectmanager.h"

Project::Project(QObject *parent) : QObject(parent)
{
connect(this, &Project::urlChanged, this, &Project::setData);
}

QUrl Project::url() const
{
    return m_url;
}

void Project::setData(const QUrl &url)
{
    m_repo = ProjectManager::gitDir(url);

    m_logo = ProjectManager::projectLogo(url);
    emit this->logoChanged(m_logo);

    if(!m_repo.isValid())
    {
        emit error("URL is nto a valid repo");
        return;
    }
    m_title = m_repo.name();
    emit this->titleChanged(m_title);
}

QString Project::getTitle() const
{
    return m_title;
}

QUrl Project::getLogo() const
{
    return m_logo;
}

QStringList Project::getBranches() const
{
    return m_branches;
}

void Project::setUrl(QUrl url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(m_url);
}

