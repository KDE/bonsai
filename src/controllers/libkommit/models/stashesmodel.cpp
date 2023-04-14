/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "stashesmodel.h"

#include "gitmanager.h"
#include <KLocalizedString>

namespace Git
{

StashesModel::StashesModel(Manager *git, QObject *parent)
    : AbstractGitItemsModel(git, parent)
{
}

int StashesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mData.size();
}

int StashesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(StashesModel::LastColumn) + 1;
}

QVariant StashesModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid() || index.row() < 0 || index.row() >= mData.size())
        return {};

    auto remote = mData.at(index.row());

    switch (index.column()) {
    case Subject:
        return remote->subject();
    case AuthorName:
        return remote->authorName();
    case AuthorEmail:
        return remote->authorEmail();
    case Time:
        return remote->pushTime();
    }
    return {};
}

QVariant StashesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        switch (section) {
        case Subject:
            return i18n("Subject");
        case AuthorName:
            return i18n("Author name");
        case AuthorEmail:
            return i18n("Author email");
        case Time:
            return i18n("Time");
        }

    return {};
}

Stash *StashesModel::fromIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mData.size())
        return nullptr;

    return mData.at(index.row());
}

void StashesModel::fill()
{
    qDeleteAll(mData);
    mData.clear();

    const auto list = mGit->readAllNonEmptyOutput({QStringLiteral("stash"), QStringLiteral("list"), QStringLiteral("--format=format:%s%m%an%m%ae%m%aD")});
    int id{0};
    for (const auto &item : std::as_const(list)) {
        const auto parts = item.split(QLatin1Char('>'));
        if (parts.size() != 4)
            continue;

        const auto subject = parts.first();
        auto stash = new Stash(mGit, QStringLiteral("stash@{%1}").arg(id));

        stash->mSubject = subject;
        stash->mAuthorName = parts.at(1);
        stash->mAuthorEmail = parts.at(2);
        stash->mPushTime = QDateTime::fromString(parts.at(3), Qt::RFC2822Date);

        mData.append(stash);
        id++;
    }
}

}
