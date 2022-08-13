#include "gitoperations.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QString>
#include "libGitWrap/Operations/CloneOperation.hpp"
#include "libGitWrap/Repository.hpp"
#include "libGitWrap/DiffList.hpp"
#include "libGitWrap/Events/Private/GitEventCallbacks.hpp"

GitOperations::GitOperations(QObject *parent) : QObject(parent)
{
    //    Git::GitWrap git;
    //    git.init();
}

void GitOperations::clone(const QUrl &url, const QUrl &path)
{
    bool bare = false;
    auto repo = new Git::CloneOperation();

    qDebug() << "WORKIG WITH" << url << path.toLocalFile();
    repo->setUrl(url.toString());
    repo->setPath(path.toLocalFile());
    repo->setBare(bare);
    repo->setBackgroundMode(true);
repo->
    connect(repo, &Git::CloneOperation::doneCheckout,[]()
    {
        qDebug() << "DOne checkout" ;
    });

    connect(repo, &Git::CloneOperation::checkoutProgress,[](const QString& path, quint64 total, quint64 completed)
    {
        qDebug() << "Progress checkout" << path << total << completed;
    });

    connect(repo, &Git::CloneOperation::transportProgress,[](quint64 totalObj, quint64 indexedObj, quint64 recivedObj , quint64 totalBytes)
    {
        qDebug() << "Progress transport" << totalObj << indexedObj<< recivedObj << totalBytes;
    });

//    connect(repo, &Git::CloneOperation::checkoutNotify,[repo](const Git::CheckoutNotify& why,
//            const QString& path,
//            const Git::DiffFile& baseline,
//            const Git::DiffFile& target,
//            const Git::DiffFile& workdir)
//    {
//        qDebug() << "Progress checkout"  << path<< baseline.isValid() << target.isValid() << workdir.isValid();
//    });

    connect(repo, &Git::CloneOperation::doneDownloading,[]()
    {
        qDebug() << "DOne downloading" ;
    });

    connect(repo, &Git::CloneOperation::finished,[repo]()
    {
        qDebug() << "FInished" << repo->repository().gitPath();
    }); //if the backgorun mode is set to a thread

    Git::GitWrap git;
    git.init();
    repo->execute();
    qDebug() << repo->isRunning() << repo->reference() << repo->backgroundMode() << repo->path() << repo->result().errorText() << repo->refSpecs() << repo->remoteAlias() << repo->refLogMessage() << repo->repository().name() << repo->repository().gitPath() << repo->repository().basePath();
}
