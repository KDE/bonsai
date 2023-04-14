/*
SPDX-FileCopyrightText: 2021 Hamed Masafi <hamed.masfi@gmail.com>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once
#include "libkommit_export.h"
#include <QList>
#include <QString>

namespace Git
{

class LIBKOMMIT_EXPORT RemoteBranch
{
public:
    bool configuredPull{false};
    bool configuredPush{false};

    QString name;
    QString remotePushBranch;
    QString remotePullBranch;
    enum class Status { Unknown, UpToDate, FastForwardable, LocalOutOfDate };
    Status status{Status::Unknown};

    Q_REQUIRED_RESULT QString statusText() const;
};

class Remote
{
public:
    QString name;
    QString headBranch;
    QString fetchUrl;
    QString pushUrl;
    QList<RemoteBranch> branches;
    //    QStringList
    Remote();
    void parse(const QString &output);
};

} // namespace Git
