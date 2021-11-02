#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>

class Project : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Project)

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit Project(QObject *parent = nullptr);

    QString url() const;

private:
    QString m_url;

public slots:
    void setUrl(QString url);

signals:
    void urlChanged(QString url);
};

#endif // PROJECT_H
