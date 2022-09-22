import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

import org.maui.bonsai 1.0 as Bonsai

Maui.Page
{
    id: control

    Maui.TabViewInfo.tabTitle: title
    Maui.TabViewInfo.tabToolTipText: url

    height: ListView.view.height
    width:  ListView.view.width

    property alias project: _project
    property alias url : _project.url

     Bonsai.Project
     {
         id: _project
     }

    title: project.title
    showTitle: false

    headBar.middleContent: Maui.ToolActions
    {
        id: _actionViews
        expanded: true
        autoExclusive: true
        currentIndex: _swipeView.currentIndex

        Action
        {
            text: i18n("Commits")
        }

        Action
        {
            text: i18n("Browser")
        }
    }

    headBar.rightContent: Maui.ComboBox
    {
        model: _project.branches.allBranches
        displayText: _project.currentBranch
        onActivated: _project.currentBranch = currentText
    }

    SwipeView
    {
        id: _swipeView
        currentIndex: _actionViews.currentIndex
        anchors.fill: parent
        clip: false

        Maui.ListBrowser
        {
            model: Maui.BaseModel
            {
                list: project.commitsModel
            }

            delegate: Maui.ListBrowserDelegate
            {
                width: ListView.view.width
                label1.text: model.message
                label3.text: model.id
                label2.text: model.author
                label4.text: Qt.formatDateTime(model.date, "dd MM yyyy")
                rightLabels.visible: true
            }
        }

        FB.FileBrowser
        {
            currentPath: control.project.url
            onItemClicked: openItem(index)
//            gridItemSize: 100
        }
    }

}
