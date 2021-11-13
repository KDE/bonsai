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

void ProjectsModel::insert(const FMH::MODEL &item)
{
    qDebug() << "inserting item << " << item;
    preItemAppended();
    this->m_list << item;
    postItemAppended();
}
