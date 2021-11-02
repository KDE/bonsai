#include "project.h"

Project::Project(QObject *parent) : QObject(parent)
{

}

QString Project::url() const
{
    return m_url;
}

void Project::setUrl(QString url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(m_url);
}
