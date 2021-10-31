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
    void clone(const QUrl &url, const QUrl &);
signals:

};

#endif // GITOPERATIONS_H
