#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"

class Project : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Project)

    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)

public:
    explicit Project(const QUrl &url, const Git::Repository &repo, QObject *parent = nullptr);

    QUrl url() const;

private:
    QUrl m_url;
    Git::Repository m_repo;

signals:
};

#endif // PROJECT_H
