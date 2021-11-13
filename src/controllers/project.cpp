#include "project.h"

Project::Project(const QUrl &url, const Git::Repository &repo, QObject *parent) : QObject(parent)
 , m_url(url)
  , m_repo(repo)
{

}

QUrl Project::url() const
{
    return m_url;
}

