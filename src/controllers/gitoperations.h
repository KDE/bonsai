#ifndef GITOPERATIONS_H
#define GITOPERATIONS_H
#include <QObject>
#include <QUrl>


class GitOperations : public QObject
{
    Q_OBJECT
public:
    explicit GitOperations(QObject *parent = nullptr);

public slots:
    void clone(const QString &url, const QString &, const QString &name, bool bare = false, bool recursive = false);

signals:

    void repoCloned(QString url);

};

#endif // GITOPERATIONS_H
