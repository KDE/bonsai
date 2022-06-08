#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"
#include "models/commithistorymodel.h"
class Project : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Project)

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QUrl logo READ getLogo NOTIFY logoChanged)
    Q_PROPERTY(QStringList branches READ getBranches NOTIFY branchesChanged)
    Q_PROPERTY(QString currentBranch READ currentBranch NOTIFY currentBranchChanged)
    Q_PROPERTY(CommitHistoryModel *commitsModel READ getCommitsModel CONSTANT FINAL)

public:
    explicit Project(QObject *parent = nullptr);

    QUrl url() const;
    void setData(const QUrl &url);
    QString getTitle() const;

    QUrl getLogo() const;

    QStringList getBranches() const;

    QString currentBranch() const;

    CommitHistoryModel * getCommitsModel();

public slots:
    void setUrl(QUrl url);

private:
    QUrl m_url;
    Git::Repository m_repo;

    QString m_title;

    QUrl m_logo;

    QStringList m_branches;

    QString m_currentBranch;

    CommitHistoryModel * m_commitsModel;

signals:
    void titleChanged(QString title);
    void logoChanged(QUrl logo);
    void urlChanged(QUrl url);
    void error(QString message);
    void branchesChanged(QStringList branches);
    void currentBranchChanged(QString currentBranch);
};

#endif // PROJECT_H
