/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

//
// Created by hamed on 25.03.22.
//

#include "remotesmodel.h"
#include "gitmanager.h"
#include "gitremote.h"

namespace Git
{

RemotesModel::RemotesModel(Manager *git, QObject *parent)
    : AbstractGitItemsModel(git, parent)
{
}

int RemotesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mData.count();
}

QVariant RemotesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto remote = mData.at(index.row());

    switch (role) {
    case Name:
        return remote->name;
    case HeadBranch:
        return remote->headBranch;
    case FetchUrl:
        return remote->fetchUrl;
    case PushUrl:
        return remote->pushUrl;
    }
    return {};
}

Remote *RemotesModel::fromIndex(const QModelIndex &index)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mData.size())
        return nullptr;

    return mData.at(index.row());
}

Remote *RemotesModel::findByName(const QString &name)
{
    for (const auto &d : std::as_const(mData))
        if (d->name == name)
            return d;
    return nullptr;
}

void RemotesModel::rename(const QString &oldName, const QString &newName)
{
    mGit->runGit({QStringLiteral("remote"), QStringLiteral("rename"), oldName, newName});
    load();
}

void RemotesModel::setUrl(const QString &remoteName, const QString &newUrl)
{
    mGit->runGit({QStringLiteral("remote"), QStringLiteral("set-url"), remoteName, newUrl});
    load();
}

void RemotesModel::fill()
{
    qDeleteAll(mData.begin(), mData.end());
    mData.clear();

    if (!mGit)
        return;

    const auto remotes = mGit->remotes();
    for (const auto &remote : remotes) {
        auto r = new Remote;
        r->name = remote;
        auto ret = QString(mGit->runGit({QStringLiteral("remote"), QStringLiteral("show"), remote}));
        r->parse(ret);
        mData.append(r);
    }
}

}


QHash<int, QByteArray> Git::RemotesModel::roleNames() const
{
    return {{Name, "name"},
        {HeadBranch, "headBranch"},
        {FetchUrl, "fetchUrl"},
        {PushUrl, "pushUrl"}};
}
