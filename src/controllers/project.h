#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QUrl>
#include "libGitWrap/Repository.hpp"

#include "models/commithistorymodel.h"

class BranchesManager;
class Project;


class Project : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Project)

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QUrl logo READ getLogo NOTIFY logoChanged)
    Q_PROPERTY(QString currentBranch READ currentBranch WRITE setCurrentBranch NOTIFY currentBranchChanged)
    Q_PROPERTY(QVariantMap currentBranchRemote READ currentBranchRemote NOTIFY currentBranchRemoteChanged)
    Q_PROPERTY(QVariantMap headBranch READ getHeadBranch NOTIFY headBranchChanged)

    Q_PROPERTY(CommitHistoryModel *commitsModel READ getCommitsModel CONSTANT FINAL)
    Q_PROPERTY(BranchesManager* branches READ getBranches CONSTANT FINAL)
    Q_PROPERTY(QStringList status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl readmeFile READ readmeFile NOTIFY readmeFileChanged CONSTANT)
    Q_PROPERTY(QVariantList remotesModel READ getRemotesModel NOTIFY remotesModelChanged)

public:
    explicit Project(QObject *parent = nullptr);

    QUrl url() const;
    void setData(const QUrl &url);
    QString getTitle() const;

    QUrl getLogo() const;

    QString currentBranch() const;

    CommitHistoryModel * getCommitsModel();

    BranchesManager* getBranches();

    QStringList status() const;

    QUrl readmeFile() const;

    QVariantList getRemotesModel() const;

    QVariantMap currentBranchRemote() const;

    QVariantMap getHeadBranch() const;

public slots:
    void setUrl(QUrl url);

    void setCurrentBranch(const QString &currentBranch);
    QString fileStatusIcon(const QString &file);

    QVariantMap commitAuthor(const QString &id);
    QVariantMap remoteInfo(const QString &remoteName);

private:
    QUrl m_url;
    Git::Repository m_repo;

    QString m_title;

    QUrl m_logo;

    QString m_currentBranch;

    CommitHistoryModel * m_commitsModel;
    BranchesManager* m_branchesManager;

    QStringList m_status;

    QUrl m_readmeFile;

    QVariantList m_remotesModel;

    QVariantMap m_currentBranchRemote;
    QVariantMap m_headBranch;

    void setCurrentBranchRemote(const QString &currentBranch);
    void setHeadBranch();

signals:
    void titleChanged(QString title);
    void logoChanged(QUrl logo);
    void urlChanged(QUrl url);
    void error(QString message);
    void currentBranchChanged(QString currentBranch);
    void statusChanged(QStringList status);
    void readmeFileChanged(QUrl readmeFile);
    void remotesModelChanged(QVariantList remotesModel);
    void currentBranchRemoteChanged(QVariantMap currentBranchRemote);
    void headBranchChanged(QVariantMap headBranch);
};

#endif // PROJECT_H
