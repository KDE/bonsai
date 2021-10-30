#ifndef GITOPERATIONS_H
#define GITOPERATIONS_H
#include <QtCore/QMutex>
#include <QObject>

class GitOperations : public QObject
{
    Q_OBJECT
public:
    explicit GitOperations(QObject *parent = nullptr);
    static QMutex gitMutex;
signals:

};

#endif // GITOPERATIONS_H
