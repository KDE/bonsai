/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "abstractgititemsmodel.h"
#include "gitmanager.h"
#include <QDebug>

namespace Git
{

AbstractGitItemsModel::AbstractGitItemsModel(Manager *git, QObject *parent)
    : QAbstractListModel(parent)
    , mGit(git)
{
    //    connect(git, &Manager::pathChanged, this, &Cache::load);
}

bool AbstractGitItemsModel::isLoaded() const
{
    return m_status == Loaded;
}

AbstractGitItemsModel::Status AbstractGitItemsModel::status() const
{
    return m_status;
}

void AbstractGitItemsModel::load()
{
    if (!mGit->isValid())
        return;

    setStatus(Loading);
    fill();
}

void AbstractGitItemsModel::reset()
{
    qDebug() << "Reset model now that it is ready?";
    beginResetModel();
    setStatus(Loaded);
    endResetModel();
}

void AbstractGitItemsModel::setStatus(Status newStatus)
{
    if (m_status == newStatus)
        return;
    m_status = newStatus;

    if (m_status == Loaded)
        Q_EMIT loaded();

    Q_EMIT statusChanged();
}

} // namespace Git
