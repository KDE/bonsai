#ifndef BRANCHESMANAGER_H
#define BRANCHESMANAGER_H

#include <QObject>
#include "libGitWrap/Repository.hpp"

class BranchesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList allBranches READ allBranches CONSTANT)
    Q_PROPERTY(QStringList localBranches READ localBranches CONSTANT)
    Q_PROPERTY(QStringList remoteBranches READ remoteBranches CONSTANT)

public:
    explicit BranchesManager(QObject *parent = nullptr);

    void setRepo(Git::Repository &repo);

    QStringList allBranches();

    QStringList localBranches();

    QStringList remoteBranches();

public slots:
    QString upstreamRemote(const QString &branchName);
    QString upstream(const QString &branchName);

private:
    Git::Repository m_repo;


signals:

};

#endif // BRANCHESMANAGER_H
