#pragma once
#include <QObject>
#include <QUrl>

#include <QFutureWatcher>
#include <libkommit/filestatus.h>

class QFileSystemWatcher;
class QTimer;

namespace Git
{
class Manager;
class LogsModel;
class RemotesModel;
}

class StatusMessage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(StatusCode code MEMBER code CONSTANT FINAL)
    Q_PROPERTY(QString message MEMBER message CONSTANT FINAL)
public:
    explicit StatusMessage(QObject *parent = nullptr);

    enum StatusCode
    {
        Loading,
        Ready,
        Error
    };  Q_ENUM(StatusCode)

    StatusCode code = StatusCode::Ready;
    QString message;
};


class Project : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Project)

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

    Q_PROPERTY(StatusMessage* status READ status NOTIFY statusChanged FINAL)

    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QUrl logo READ getLogo NOTIFY logoChanged)
    Q_PROPERTY(QString currentBranch READ currentBranch WRITE setCurrentBranch NOTIFY currentBranchChanged)
    Q_PROPERTY(QVariantMap headBranch READ getHeadBranch NOTIFY headBranchChanged)

    Q_PROPERTY(Git::LogsModel *commitsModel READ getCommitsModel CONSTANT FINAL)
    Q_PROPERTY(QStringList repoStatus READ repoStatus NOTIFY repoStatusChanged)
    Q_PROPERTY(QUrl readmeFile READ readmeFile NOTIFY readmeFileChanged CONSTANT)
    Q_PROPERTY(Git::RemotesModel *remotesModel READ remotesModel CONSTANT FINAL)

    Q_PROPERTY(QStringList allBranches READ allBranches NOTIFY repoChanged)
    Q_PROPERTY(QStringList remoteBranches READ remoteBranches NOTIFY repoChanged)

public:    

    explicit Project(QObject *parent = nullptr);
    ~Project();

    QString url() const;

    /**
     * @brief setUrl
     * @param url
     */
    void setUrl(const QString &url);

    [[nodiscard]] QString getTitle() const;

    QUrl getLogo() const;

    QString currentBranch() const;
    void setCurrentBranch(const QString &currentBranch);

    Git::LogsModel * getCommitsModel();

    QStringList repoStatus() const;
    QUrl readmeFile() const;

    Git::RemotesModel *remotesModel() const;


    QVariantMap getHeadBranch() const;

    StatusMessage* status() const;

    QStringList allBranches() const;

    QStringList remoteBranches() const;

public Q_SLOTS:
    QString fileStatusIcon(const QString &file);

    QVariantMap commitAuthor(const QString &id);
    QVariantMap remoteInfo(const QString &remoteName);

    void pull();
    void clone(const QString &url);

private:
    Git::Manager *m_manager;
    QFutureWatcher<void> *m_cloneWatcher;
    QList<Git::FileStatus> m_filesStatus;
    QFileSystemWatcher *m_gitDirWacther;
    QTimer *m_watcherTimer;

    QString m_url;
    QString m_remoteUrl;

    QString m_title;

    QUrl m_logo;

    QString m_currentBranch;

    QStringList m_repoStatus;

    QUrl m_readmeFile;

    QVariantMap m_headBranch;
    StatusMessage *m_status;

    void setData(const QString &url);
    bool loadData();

    void setCurrentBranchRemote(const QString &currentBranch);
    void setHeadBranch();

    void setStatus(StatusMessage::StatusCode code, const QString &message);
    QString createHashLink(const QString &hash) const;

Q_SIGNALS:
    void titleChanged(QString title);
    void logoChanged(QUrl logo);
    void urlChanged(QString url);
    void error(QString message);
    void currentBranchChanged(QString currentBranch);
    void repoStatusChanged(QStringList status);
    void readmeFileChanged(QUrl readmeFile);
    void currentBranchRemoteChanged(QVariantMap currentBranchRemote);
    void headBranchChanged(QVariantMap headBranch);
    void remoteUrlChanged(QString remoteUrl);
    void statusChanged();
    void repoChanged();
    void modelsChanged();
};



