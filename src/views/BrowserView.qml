import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtGraphicalEffects 1.0
import org.kde.kirigami 2.14 as Kirigami

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB


Maui.Page
{
    id: control
    headBar.forceCenterMiddleContent: false

    headBar.middleContent: Maui.TextField
    {
        Layout.fillWidth: true
        Layout.maximumWidth: 500
        placeholderText: i18np("Filter", "Filter %1 repos", _listView.model.list.count);
        onAccepted: _listView.model.filter = text
        onCleared:  _listView.model.filter = text
    }

    headBar.rightContent:[
        Maui.ToolButtonMenu
        {
            id: _viewMenu
            icon.name: "view-sort"

            Action
            {
                text: i18n("Date")
                checked: _projectsModel.sort === "lastread"
                checkable: true
                onTriggered:
                {
                    _projectsModel.sort = "lastread"
                    _projectsModel.sortOrder = Qt.DescendingOrder
                }
            }

            Action
            {
                text: i18n("Modified")
                checked: _projectsModel.sort === "modified"
                checkable: true
                onTriggered:
                {
                    _projectsModel.sort = "modified"
                    _projectsModel.sortOrder = Qt.DescendingOrder
                }
            }

            Action
            {
                text: i18n("Name")
                checked: _projectsModel.sort === "title"
                checkable: true

                onTriggered:
                {
                    _projectsModel.sort = "title"
                    _projectsModel.sortOrder =  Qt.AscendingOrder
                }
            }
        }
    ]

    Maui.ListBrowser
    {
        id: _listView
        anchors.fill: parent
        holder.visible: count === 0
        holder.title : i18n("Nothing here!")
        holder.body: i18n("Previously opened repos will be listed here.")
        holder.emoji: "qrc:/assets/assets/folder-add.svg"

        model: Maui.BaseModel
        {
            id: _projectsModel
            list: _projectManager.projectsModel

            sort: "title"
            sortOrder: Qt.AscendingOrder
            recursiveFilteringEnabled: true
            sortCaseSensitivity: Qt.CaseInsensitive
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate:  Maui.ListBrowserDelegate
        {
            width: ListView.view.width
//            height: implicitHeight + Maui.Style.space.medium
            iconSource: model.icon
            imageSource: model.artwork
            template.headerSizeHint: 32
            iconSizeHint: 22
            template.imageSizeHint: iconSizeHint

            label1.text: model.title
            label2.text: model.branch
            tooltipText: model.url

//            label3.text : i18np("%1 item", "%1 items", model.count)
//            label4.text: Maui.Handy.formatDate(model.lastread, "MM/dd/yyyy")

            onClicked:
            {
                _listView.currentIndex = index
                if(Maui.Handy.singleClick || Kirigami.Settings.isMobile)
                {
                    openProject(model.url)
                }
            }

            onDoubleClicked:
            {
                _listView.currentIndex = index
                if(!Maui.Handy.singleClick)
                {
                    openProject(model.url)
                }
            }

        }
    }


}
