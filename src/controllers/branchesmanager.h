#pragma once
#include <QObject>

class BranchesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList allBranches READ allBranches NOTIFY repoChanged)
    Q_PROPERTY(QStringList localBranches READ localBranches NOTIFY repoChanged)
    Q_PROPERTY(QStringList remoteBranches READ remoteBranches NOTIFY repoChanged)

public:
    explicit BranchesManager(QObject *parent = nullptr);

    QStringList allBranches();

    QStringList localBranches();

    QStringList remoteBranches();

public Q_SLOTS:
    QString upstreamRemote(const QString &branchName);
    QString upstream(const QString &branchName);

private:
//    Git::Repository m_repo;

Q_SIGNALS:
    void repoChanged();

};

