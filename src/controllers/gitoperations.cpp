#include "gitoperations.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QMutexLocker>


extern "C" {
#include <git2.h>
}

QMutex GitOperations::gitMutex;

GitOperations::GitOperations(QObject *parent) : QObject(parent)
{

}
