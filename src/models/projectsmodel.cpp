#include "projectsmodel.h"
#include <QDebug>

ProjectsModel::ProjectsModel(QObject *parent) : MauiList(parent)
{

}


void ProjectsModel::componentComplete()
{
}

const FMH::MODEL_LIST &ProjectsModel::items() const
{
    return m_list;
}

void ProjectsModel::setList(const FMH::MODEL_LIST &list)
{
    this->preListChanged();
    this->m_list << list;
    this->postListChanged();
}

void ProjectsModel::insert(const FMH::MODEL &item)
{
    qDebug() << "inserting item << " << item;
    emit this->preItemAppended();
    this->m_list << item;
    emit this->postItemAppended();
    emit this->countChanged();
}
