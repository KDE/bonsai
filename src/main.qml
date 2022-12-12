import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.maui.bonsai 1.0 as Bonsai

import "views"

Maui.ApplicationWindow
{
    id: root

    property alias gitOperations : _gitOperations

    Bonsai.GitOperations
    {
        id: _gitOperations

        onRepoCloned: openProject(url)
    }

    Bonsai.ProjectManager
    {
        id: _projectManager
    }

    Component
    {
        id: _cloneDialogComponent

        CloneDialog
        {

        }
    }

    property alias dialog : _dialogLoader.item

    Loader
    {
        id: _dialogLoader
    }

    Component
    {
        id: _openFileDialogComponent

        FB.FileDialog
        {

        }
    }

    Component
    {
        id: _browserViewComponent
        BrowserView
        {
            id: _browserView
            headBar.leftContent: ToolButton
            {
                icon.name: "go-previous"
                onClicked: _mainStackView.pop()
            }
        }
    }

    Component
    {
        id: _projectPageComponent

        ProjectView
        {

        }
    }

    Action
    {
        id: _recentAction
        icon.name: "shallow-history"
        text: i18n("Recent")

        onTriggered:
        {
            _mainStackView.push(_browserViewComponent)
        }
    }

    Action
    {
        id: _cloneAction
        icon.name: "vcs-merge"
        text: i18n("Clone")
        onTriggered:
        {
            _dialogLoader.sourceComponent = _cloneDialogComponent
            dialog.open()
        }
    }

    Action
    {

        id: _newAction
        icon.name: "folder-new"
        text: i18n("Create")

    }

    Action
    {

        id: _openAction
        icon.name: "document-open"
        text: i18n("Open")

        onTriggered: openLocalRepo()
    }

    StackView
    {
        id: _mainStackView
        anchors.fill: parent

        initialItem: Maui.TabView
        {
            id: _tabView
            //        mobile: true
            anchors.fill: parent
            holder.title : i18n("Let's Start")
            holder.body: i18n("Open or clone an existing repository, or create a new one.")
            holder.emoji: "qrc:/assets/assets/folder-add.svg"

            holder.actions:[  _recentAction, _cloneAction, _openAction, _newAction    ]

            tabBar.visible: true
            tabBar.showNewTabButton: false

            tabBar.rightContent: [
                Maui.ToolButtonMenu
                {
                    icon.name: "list-add"


                    MenuItem
                    {
                        action: _recentAction
                    }

                    MenuItem
                    {
                        action: _cloneAction

                    }

                    MenuItem
                    {
                        action: _newAction
                    }

                    MenuItem
                    {
                        action: _openAction
                    }

                    MenuSeparator {}

                    MenuItem
                    {
                        text: i18n("Settings")
                        icon.name: "settings-configure"
                        onTriggered: openConfigDialog()
                    }

                    MenuItem
                    {
                        text: i18n("About")
                        icon.name: "documentinfo"
                        onTriggered: root.about()
                    }
                },

                Maui.WindowControls {}
            ]


        }
    }


    function openLocalRepo()
    {
        _dialogLoader.sourceComponent = _openFileDialogComponent
        dialog.singleSelection = true
        dialog.callback = function(paths)
        {

            for(var path of paths)
            openProject(path)
        }

        dialog.open()
    }

    function openProject(url)
    {
        _projectManager.addProject(url)

        const index = tabIndex(url)
        if(index > -1)
        {
            _tabView.setCurrentIndex(index)
            return
        }

        _tabView.addTab(_projectPageComponent, {'url' : url})
        if(_mainStackView.depth === 2)
        {
            _mainStackView.pop()
        }
    }

    function tabIndex(path) //find the tab index for a path
    {
        if(path.length === 0)
        {
            return -1
        }

        for(var i = 0; i < _tabView.count; i++)
        {
            const tab =  _tabView.contentModel.get(i)
            console.log("FIN TAB INDEX", i, tab.url, path)
            if(tab.url.toString() === path)
            {
                return i
            }
        }
        return -1
    }
}
