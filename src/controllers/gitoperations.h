#ifndef GITOPERATIONS_H
#define GITOPERATIONS_H
#include <QObject>
#include <QUrl>


class GitOperations : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit GitOperations(QObject *parent = nullptr);

    QString url() const;
    void setUrl(const QString &newUrl);

public slots:
    void clone(const QString &url, const QString &, const QString &name, bool bare = false, bool recursive = false);
    void pull(const QString &password);

signals:

    void repoCloned(QString url);

    void urlChanged();
private:
    QString m_url;
};

#endif // GITOPERATIONS_H
